
#define LOCALE_( name) GW_##name

#include "MS_util.h"
#include "minefield.h"
#include "userinterface.h"

#include <epoxy/gl.h>
#include <epoxy/egl.h>
#include <wayland-egl.h>

typedef struct{
  struct wl_egl_window *window;
}Context;

void *
GW_Init( MS_root *root){
  assert( root != NULL);
  return MS_CreateEmpty( char);
}


int
event_dispatch( void *root_void){
  (void)root_void;
  return 0;
}


int
draw( void *context_void, MS_field field){
  (void)context_void;
  (void)field;
  return 0;
}


void
GW_Free( void *gw){
  free( gw);
}
