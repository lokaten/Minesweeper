

#ifdef MS_CS_H__
#else
#define MS_CS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#include "MS_util.h"

typedef struct{
  const size_t blk_size;
  const size_t size;
  char *blk_fetch;
  char *blk_push;
  char *blk_releas;
  char *blk_finish;
  char *fetch;
  char *push;
  char *releas;
  char *finish;
}ComandStream;


static inline ComandStream *CS_CreateStreamFromSize( FreeNode *, const size_t);
static inline void *CS_Fetch( ComandStream *);
static inline void CS_Push( ComandStream *, const void *);
static inline void *CS_Releas( ComandStream *);
static inline void CS_Finish( ComandStream *, const void *);
static inline void CS_Free( FreeNode *, ComandStream *);

_Pragma("GCC diagnostic ignored \"-Wpointer-arith\"")
_Pragma("GCC diagnostic ignored \"-Wcast-align\"")

static inline ComandStream *
CS_CreateStreamFromSize( FreeNode *freenode, const size_t size){
  ComandStream *Stream;
  size_t blk_size;
  char *ptr;
  
  assert( size);

  blk_size = SLAB_SIZE - sizeof( char *);
  
  while( blk_size % size || blk_size % sizeof( char *)){
    blk_size -= blk_size % size;
    blk_size -= blk_size % sizeof( char *);
  }
  
  
  Stream = MS_Create( freenode, ComandStream,
		      .blk_size = blk_size,
		      .size = size);
  
  ptr = ( char *)MS_CreateSlab();
  
  *( char **)( ptr + Stream -> blk_size) = ptr;
  
  Stream -> blk_fetch  = ptr;
  Stream -> blk_push   = ptr;
  Stream -> blk_releas = ptr;
  Stream -> blk_finish = ptr;
  
  Stream -> fetch  = ptr;
  Stream -> push   = ptr;
  Stream -> releas = ptr;
  Stream -> finish = ptr;
  
  return Stream;
}
#define CS_CreateStream( freenode, type) CS_CreateStreamFromSize( freenode, sizeof( MS_pos))


static inline void *
CS_Fetch( ComandStream *Stream){
  void *ret = NULL;
  assert( Stream != NULL);
  
  if unlikely( Stream -> fetch == Stream -> blk_fetch + Stream -> blk_size){
    if unlikely( *( char **)( Stream -> blk_fetch + Stream -> blk_size) == Stream -> blk_finish){
      char *ptr = ( char *)MS_CreateSlab();
      // lock
      *( char **)( ptr + Stream -> blk_size) = *( char **)( Stream -> blk_fetch + Stream -> blk_size);
      *( char **)( Stream -> blk_fetch + Stream -> blk_size) = ptr;
      // unlock
    }
    Stream -> blk_fetch = *( char **)( Stream -> blk_fetch + Stream -> blk_size);
    Stream -> fetch  = Stream -> blk_fetch;
  }
  
  ret = Stream -> fetch;
  Stream -> fetch = Stream -> fetch + Stream -> size;
  
  return ret;
}


static inline void
CS_Push( ComandStream *Stream, const void *ptr){
  assert( Stream != NULL);
  
  if unlikely( Stream -> push == Stream -> blk_push + Stream -> blk_size){
    Stream -> blk_push = *( char **)( Stream -> blk_push + Stream -> blk_size);
    Stream -> push = Stream -> blk_push;
  }
  
  assert( ptr == Stream -> push);
  
  Stream -> push = Stream -> push + Stream -> size;
}


//
// return a pointer to the next element in the stream
//
static inline void *
CS_Releas( ComandStream *Stream){
  void *ret = NULL;
  assert( Stream != NULL);
  
  if unlikely( Stream -> push == Stream -> releas) goto end;
  
  if unlikely( Stream -> releas == Stream -> blk_releas + Stream -> blk_size){
    Stream -> blk_releas = *( char **)( Stream -> blk_releas + Stream -> blk_size);
    Stream -> releas = Stream -> blk_releas;
  }
  
  ret = Stream -> releas;
  Stream -> releas = Stream -> releas + Stream -> size;
  
 end:
  return ret;
}


static inline void
CS_Finish( ComandStream *Stream, const void *ptr){
  assert( Stream != NULL);
  
  if unlikely( Stream -> finish == Stream -> blk_finish + Stream -> blk_size){
    if unlikely( *( char **)( Stream -> blk_fetch + Stream -> blk_size) != Stream -> blk_finish){
      // lock
      char *lptr =  *( char **)( Stream -> blk_fetch + Stream -> blk_size);
      *( char **)( Stream -> blk_fetch + ( Stream -> blk_size)) = *( char **)( lptr + Stream -> blk_size);
      // unlock
      MS_FreeSlab( lptr);
    }
    Stream -> blk_finish = *( char **)( Stream -> blk_finish + Stream -> blk_size);
    Stream -> finish = Stream -> blk_finish;
  }
  
  dassert( ptr == Stream -> finish);
  
  Stream -> finish = Stream -> finish + Stream -> size;
}


//
// free all block...
//
static inline void
CS_Free( FreeNode *freenode, ComandStream *Stream){
  if likely( Stream != NULL){
    char *ptr = Stream -> blk_fetch;
    Stream -> blk_fetch = *( char **)( Stream -> blk_fetch + Stream -> blk_size);
    *( char **)( ptr + Stream -> blk_size) = NULL;
    
    while( Stream -> blk_fetch != NULL){
      ptr = Stream -> blk_fetch;
      Stream -> blk_fetch = *( char **)( Stream -> blk_fetch + Stream -> blk_size);
      MS_FreeSlab( ptr);
    }
    
    MS_Free( freenode, Stream, ComandStream);
  }
}


#ifdef __cplusplus
}
#endif
#endif
