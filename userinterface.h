


#ifdef MS_GW_H__
#else
#define MS_GW_H__
#ifdef __cplusplus
extern "C" {
#endif
  
#include "MS_util.h"
#include "minefield.h"
  
  typedef struct{
    s32 x;
    s32 y;
  }MS_diff;
  
  typedef struct{
    int ( *func)( void *);
    void *data;
  }action;
  
  typedef struct{
    void *GW;
    MS_field *minefield;
    MS_stream *mss;
    ComandStream *actionque;
    u64 tutime;
    u64 nextframe;
    u64 gamestart;
    u64 nexttu;
    _Bool gameover;
    _Bool no_resize;
    MS_diff *diff;
    u32 seed;
    int( *quit)( void *);
  }MS_root;
  
  
  static inline int
  LOCALE_( take_action)( ComandStream *actionque, int ( *func)( void *), void *data){
    int ret = 0;
    action *pact;
    pact = ( action *)CS_Fetch( actionque);
    pact -> func = func;
    pact -> data = data;
    CS_Push( actionque, pact);
    return ret;
  }
#define take_action LOCALE_( take_action)
  
  int event_dispatch( void *);
  void *GW_Init( MS_root *);
  void GW_Free( void *);
  int draw( void *, MS_field);
  
#ifdef __cplusplus
}
#endif
#endif
