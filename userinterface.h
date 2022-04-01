


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
  struct timespec tv_lastpresent;
  u32 lives;
}MS_root;

void *GW_Init( FreeNode *);
void GW_Free( FreeNode *, void *);
void event_dispatch( void);
void draw( void);
void *draw_workthread( void *);

extern MS_root *root;

static inline void
drawelement( ComandStream *drawque, const MS_element *element, s16 w, s16 h){
  DrawComand *dc = ( DrawComand *)CS_Fetch( drawque);
  
  dc -> pos.x = w;
  dc -> pos.y = h;
  dc -> element = *element;
  
  CS_Push( drawque, dc);
}


/**/
FUNC_DEC( void, FUNC_quit, const MS_root *root;) __attribute__((noreturn));
#define quit( ...) FUNC_CALL( FUNC_quit, __VA_ARGS__)
#ifdef __cplusplus
}
#endif
#endif
