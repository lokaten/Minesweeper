


#ifdef MS_GW_H__
#else
#define MS_GW_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "MS_util.h"

#include "minefield.h"

typedef struct{
  MS_pos pos;
  MS_element element;
}DrawComand;

typedef struct{
  FreeNode *freenode;
  void *GW;
  const MS_video real;
  MS_field *minefield;
  const MS_stream *mss;
  const bool no_resize;
  ComandStream *drawque;
  _Bool idle;
}MS_root;

void *GW_Init( FreeNode *, MS_root *);
void GW_Free( FreeNode *, void *);
void event_dispatch( MS_root *);
void draw( MS_root *);
void drawelement( ComandStream *, const MS_element *, s16, s16);


/**/
FUNC_DEC( void, FUNC_quit, const MS_root *root;) __attribute__((noreturn));
#define quit( ...) FUNC_CALL( FUNC_quit, __VA_ARGS__)
#ifdef __cplusplus
}
#endif
#endif
