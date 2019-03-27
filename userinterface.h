


#ifdef MS_GW_H__
#else
#define MS_GW_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "MS_util.h"
#include "minefield.h"

typedef struct{
  FreeNode *freenode;
  void *GW;
  const MS_video real;
  const MS_field *minefield;
  const MS_stream *mss;
  const bool no_resize;
}MS_root;
  
void *GW_Init( FreeNode *, MS_root *);
void GW_Free( void *);
void event_dispatch( const MS_root *);
void draw( void *, MS_field);

/**/
void quit( const MS_root *) __attribute__((noreturn));
#ifdef __cplusplus
}
#endif
#endif
