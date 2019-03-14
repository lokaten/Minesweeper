


#ifdef MS_GW_H__
#else
#define MS_GW_H__
#ifdef __cplusplus
extern "C" {
#endif
  
#include "MS_util.h"
#include "minefield.h"
  
  typedef struct{
    void *GW;
    const MS_video real;
    const MS_field *minefield;
    const MS_stream *mss;
    bool gameover;
    const bool no_resize;
  }MS_root;
  
  void event_dispatch( MS_root *);
  void *GW_Init( MS_root *);
  void GW_Free( void *);
  void draw( void *, MS_field);

  /**/
  void quit( MS_root *) __attribute__((noreturn));
#ifdef __cplusplus
}
#endif
#endif
