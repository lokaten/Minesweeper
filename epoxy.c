
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


void
event_dispatch( const MS_root *root){
  assert( root != NULL);
}


void
draw( void *context_void, MS_field field){
  assert( context_void != NULL);
  (void)field;
}


void
GW_Free( void *gw){
  free( gw);
}
