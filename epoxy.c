

#include "MS_util.h"
#include "minefield.h"
#include "userinterface.h"

#include <string.h>

#include <wayland-egl.h>
#include <epoxy/gl.h>
#include <epoxy/egl.h>

#include "debug.h"

typedef struct{
  EGLContext              egl_context;
  struct wl_surface       *surface;
  struct wl_shell_surface *shell_surface;
  struct wl_egl_window    *egl_window;
  EGLSurface              egl_surface;
}Window;


typedef struct{
  struct wl_display       *display;
  struct wl_seat          *seat;
  EGLDisplay              egl_display;
  struct wl_registry      *registry;
  struct wl_compositor    *compositor;
  struct wl_egl_window    *egl_window;
  struct wl_region        *region;
  struct wl_shell         *shell;
  Window                  *window;
}Context;

static void shell_surface_ping( void *, struct wl_shell_surface *, uint32_t);
static void shell_surface_configure( void *, struct wl_shell_surface *, uint32_t, int32_t, int32_t);
static void shell_surface_popup_done( void *, struct wl_shell_surface *);

static void registry_add_object( void *, struct wl_registry *, uint32_t, const char *, uint32_t);
static void registry_remove_object( void *, struct wl_registry *, uint32_t);

static void seat_handle_capabilities( void *, struct wl_seat *, enum wl_seat_capability);
static void seat_listener_name( void *, struct wl_seat *, const char *);



// global variable do not acses
static Context *global_ctx;

void *
GW_Init( FreeNode *freenode, MS_root *root){
  Context *ctx;
  assert( root != NULL);
  
  ctx = MS_CreateEmpty( freenode, Context);
  assert( ctx != NULL);
  ctx -> window = MS_CreateEmpty( freenode, Window);
  assert( ctx -> window != NULL);
  
  global_ctx = ctx;
  
  {
    ctx -> display = wl_display_connect( NULL);
    assert( ctx -> display);
    
    ctx -> registry = wl_display_get_registry( ctx -> display);
    assert( ctx -> registry);
    
    wl_registry_add_listener( ctx -> registry, &( struct wl_registry_listener){ &registry_add_object, &registry_remove_object}, NULL);
    DEBUG_PRINT( stdout, "\r yey!  \n");
    wl_display_roundtrip( ctx -> display);
    
    ctx -> egl_display = eglGetDisplay( ctx -> display);
    assert( ctx -> egl_display);
    
    eglInitialize( ctx -> egl_display, NULL, NULL);
    
    {
      EGLint attributes[] = { EGL_RED_SIZE, 8,
			      EGL_GREEN_SIZE, 8,
			      EGL_BLUE_SIZE, 8,
			      EGL_NONE};
      EGLConfig config;
      EGLint num_config;
      
      eglBindAPI( EGL_OPENGL_API);
      
      eglChooseConfig( ctx -> egl_display, attributes, &config, 1, &num_config);
      ctx -> window -> egl_context = eglCreateContext( ctx -> egl_display, config, EGL_NO_CONTEXT, NULL);
      assert( ctx -> window -> egl_context);
      
      ctx -> window -> surface = wl_compositor_create_surface( ctx -> compositor);
      assert( ctx -> window -> surface);
      
      
      
      ctx -> egl_window = wl_egl_window_create( ctx -> window -> surface, ( s32)root -> real.realwidth, ( s32)root -> real.realheight);
      assert( ctx -> egl_window);
      
      ctx -> window -> egl_surface = eglCreateWindowSurface( ctx -> egl_display, config, ctx -> egl_window, NULL);
      assert( ctx -> window -> egl_surface);
      
      eglMakeCurrent( ctx -> egl_display, ctx -> window -> egl_surface, ctx -> window -> egl_surface, ctx -> window -> egl_context);
    }
  }
  
  
  
  return global_ctx;
}


void
event_dispatch( const MS_root *root){
  assert( root != NULL);
  quit( root);
}


void
draw( void *pctx, MS_field field){
  Context *ctx = pctx;
  assert( ctx != NULL);
  ( void)field;
  wl_display_dispatch_pending( ctx -> display);
  glClearColor( 0.0, 1.0, 0.0, 1.0);
  glClear( GL_COLOR_BUFFER_BIT);
  eglSwapBuffers( ctx -> egl_display, ctx -> window -> egl_surface);
}

void
drawelement( void *gw, const MS_element *element, s16 w, s16 h){
  assert( gw != NULL);
  assert( element != NULL);
  (void)w;
  (void)h;
}

void
GW_Free( FreeNode *freenode, void *pctx){
  Context *ctx = pctx;
  assert( ctx != NULL);
  
  eglDestroySurface( ctx -> egl_display, ctx -> window -> egl_surface);
  
  // wl_egl_window_destroy( ctx -> window -> egl_window);
  
  wl_shell_surface_destroy( ctx -> window -> shell_surface);
  wl_surface_destroy( ctx -> window -> surface);
  eglDestroyContext( ctx -> egl_display, ctx -> window -> egl_context);
  eglTerminate( ctx -> egl_display);
  wl_display_disconnect( ctx -> display);
  MS_Free( freenode, ctx -> window);
  MS_Free( freenode, ctx);
}


static void
registry_add_object( void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version){
  Context *ctx = global_ctx;
  ( void)data;
  ( void)version;
  if( !strcmp( interface,"wl_compositor")){
    global_ctx -> compositor = wl_registry_bind( registry, id, &wl_compositor_interface, 1);
    
  }else if( !strcmp( interface,"wl_shell")){
    ctx -> shell = wl_registry_bind( ctx -> registry, id, &wl_shell_interface, 1);
    ctx -> window -> shell_surface = wl_shell_get_shell_surface( ctx -> shell, ctx -> window -> surface);
    assert( ctx -> window -> shell_surface);
    
    wl_shell_surface_add_listener( ctx -> window -> shell_surface, &( struct wl_shell_surface_listener){ &shell_surface_ping, &shell_surface_configure, &shell_surface_popup_done}, ctx -> window);
    wl_shell_surface_set_toplevel( ctx -> window -> shell_surface);
    
  }else if( !strcmp(interface, "wl_seat")){
    global_ctx -> seat = wl_registry_bind( registry, id, &wl_seat_interface, 1);
    wl_seat_add_listener( global_ctx -> seat, &( struct wl_seat_listener){ .capabilities = &seat_handle_capabilities, .name = &seat_listener_name}, NULL);
  }else{
    // do nothing
  }
}


static void
registry_remove_object( void *data, struct wl_registry *registry, uint32_t name){
  ( void)data;
  ( void)registry;
  ( void)name;
}


static void
shell_surface_ping( void *data, struct wl_shell_surface *shell_surface, uint32_t serial){
  ( void)data;
  ( void)shell_surface;
  ( void)serial;
  wl_shell_surface_pong( global_ctx -> window -> shell_surface, serial);
}

static void
shell_surface_configure( void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height){
  struct window *window = data;
  ( void)window;
  ( void)shell_surface;
  ( void)edges;
  ( void)width;
  ( void)height;
  wl_egl_window_resize( global_ctx -> egl_window, width, height, 0, 0);
}

static void
shell_surface_popup_done( void *data, struct wl_shell_surface *shell_surface) {
  ( void)data;
  ( void)shell_surface;
}

static void
seat_handle_capabilities( void *data, struct wl_seat *seat, enum wl_seat_capability caps){
  ( void)data;
  ( void)seat;
  if( caps & WL_SEAT_CAPABILITY_POINTER){
    printf("Display has a pointer\n");
  } 
  
  if( caps & WL_SEAT_CAPABILITY_KEYBOARD){
    printf("Display has a keyboard\n");
  }
  
  if( caps & WL_SEAT_CAPABILITY_TOUCH){
    printf("Display has a touch screen\n");
  }
}


static void
seat_listener_name( void *data, struct wl_seat *wl_seat, const char *name){
  ( void)data;
  ( void)wl_seat;
  ( void)name;
}
