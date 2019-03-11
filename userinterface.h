


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
    MS_video real;
    MS_field *minefield;
    MS_stream *mss;
    ComandStream *actionque;
    u64 tutime;
    u64 nextframe;
    u64 gamestart;
    bool gameover;
    bool no_resize;
    u32 seed;
    void( *quit)( void *);
  }MS_root;
  
  void event_dispatch( void *);
  void *GW_Init( MS_root *);
  void GW_Free( void *);
  void draw( void *, MS_field);
  
#ifdef __cplusplus
}
#endif
#endif
