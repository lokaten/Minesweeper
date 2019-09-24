

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
  FreeNode *freenode;
  address blk_fetch;
  address blk_push;
  address blk_releas;
  address blk_finish;
  address fetch;
  address push;
  address releas;
  address finish;
}ComandStream;


static inline ComandStream *CS_CreateStreamFromSize( FreeNode *, const size_t);
static inline void *CS_Fetch( ComandStream *);
static inline void CS_Push( ComandStream *, const void *);
static inline void *CS_Releas( ComandStream *);
static inline void CS_Finish( ComandStream *, const void *);
static inline void CS_Free( FreeNode *, ComandStream *);

static inline ComandStream *
CS_CreateStreamFromSize( FreeNode *freenode, const size_t size){
  ComandStream *Stream;
  size_t blk_size;
  address addr;
  assert( size);
  
  blk_size = 512 - sizeof( address);
  
  while( blk_size % size || blk_size % sizeof( address)){
    blk_size -= blk_size % size;
    blk_size -= blk_size % sizeof( address);
  }

  assert( blk_size >= size);
  
  Stream = MS_Create( freenode, ComandStream,
		      .blk_size = blk_size,
		      .size = size);
  
  addr = MS_CreateFromSize( freenode, blk_size + sizeof( address));
  
  Stream -> freenode = freenode;
  
  *( address *)( addr + Stream -> blk_size) = addr;
  
  Stream -> blk_fetch  = addr;
  Stream -> blk_push   = addr;
  Stream -> blk_releas = addr;
  Stream -> blk_finish = addr;
  
  Stream -> fetch  = addr;
  Stream -> push   = addr;
  Stream -> releas = addr;
  Stream -> finish = addr;
  
  return Stream;
}
#define CS_CreateStream( freenode, type) CS_CreateStreamFromSize( freenode, sizeof( MS_pos))


static inline void *
CS_Fetch( ComandStream *Stream){
  address ret;
  assert( Stream != NULL);
  
  if unlikely( Stream -> fetch == Stream -> blk_fetch + Stream -> blk_size){
    if unlikely( *( address *)( Stream -> blk_fetch + Stream -> blk_size) == Stream -> blk_finish){
      address addr = MS_CreateFromSize( Stream -> freenode, Stream -> blk_size + sizeof( address));
      // lock
      *( address *)( addr + Stream -> blk_size) = *( address *)( Stream -> blk_fetch + Stream -> blk_size);
      *( address *)( Stream -> blk_fetch + Stream -> blk_size) = addr;
      // unlock
    }
    Stream -> blk_fetch = *( address *)( Stream -> blk_fetch + Stream -> blk_size);
    Stream -> fetch  = Stream -> blk_fetch;
  }
  
  ret = Stream -> fetch;
  Stream -> fetch = Stream -> fetch + Stream -> size;
  
  return ( void *)ret;
}


static inline void
CS_Push( ComandStream *Stream, const void *ptr){
  address addr;
  assert( Stream != NULL);
  addr = ( address)ptr;
  
  if unlikely( Stream -> push == Stream -> blk_push + Stream -> blk_size){
    Stream -> blk_push = *( address *)( Stream -> blk_push + Stream -> blk_size);
    Stream -> push = Stream -> blk_push;
  }
  
  assert( addr == Stream -> push);
  
  Stream -> push = Stream -> push + Stream -> size;
}


//
// return a pointer to the next element in the stream
//
static inline void *
CS_Releas( ComandStream *Stream){
  address ret = 0;
  assert( Stream != NULL);
  
  if unlikely( Stream -> push == Stream -> releas) goto end;
  
  if unlikely( Stream -> releas == Stream -> blk_releas + Stream -> blk_size){
    Stream -> blk_releas = *( address *)( Stream -> blk_releas + Stream -> blk_size);
    Stream -> releas = Stream -> blk_releas;
  }
  
  ret = Stream -> releas;
  Stream -> releas = Stream -> releas + Stream -> size;
  
 end:
  return ( void *)ret;
}


static inline void
CS_Finish( ComandStream *Stream, const void *ptr){
  address addr;
  assert( Stream != NULL);
  addr = ( address)ptr;
  
  if unlikely( Stream -> finish == Stream -> blk_finish + Stream -> blk_size){
    if unlikely( *( address *)( Stream -> blk_fetch + Stream -> blk_size) != Stream -> blk_finish){
      // lock
      address blk_free =  *( address *)( Stream -> blk_fetch + Stream -> blk_size);
      *( address *)( Stream -> blk_fetch + ( Stream -> blk_size)) = *( address *)( blk_free + Stream -> blk_size);
      // unlock
      MS_FreeFromSize( Stream -> freenode, blk_free, Stream -> blk_size + sizeof( address));
    }
    Stream -> blk_finish = *( address *)( Stream -> blk_finish + Stream -> blk_size);
    Stream -> finish = Stream -> blk_finish;
  }
  
  dassert( addr == Stream -> finish);
  
  Stream -> finish = Stream -> finish + Stream -> size;
}


//
// free all block...
//
static inline void
CS_Free( FreeNode *freenode, ComandStream *Stream){
  if likely( Stream != NULL){
    address addr = Stream -> blk_fetch;
    Stream -> blk_fetch = *( address *)( Stream -> blk_fetch + Stream -> blk_size);
    *( address *)( addr + Stream -> blk_size) = 0;
    
    while( Stream -> blk_fetch != 0){
      addr = Stream -> blk_fetch;
      Stream -> blk_fetch = *( address *)( Stream -> blk_fetch + Stream -> blk_size);
      MS_FreeFromSize( Stream -> freenode, addr, Stream -> blk_size + sizeof( address));
    }
    
    MS_Free( freenode, Stream);
  }
}


#ifdef __cplusplus
}
#endif
#endif
