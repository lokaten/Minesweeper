

#ifdef MS_CS_H__
#else
#define MS_CS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <pthread.h>

#include "MS_util.h"

typedef struct{
  const size_t blk_size;
  const size_t size;
  FreeNode *freenode;
  address blk_push;
  address blk_releas;
  address push;
  address releas;
  pthread_mutex_t mutex_signal;
  pthread_mutex_t mutex_blk;
  pthread_mutex_t mutex_push;
  pthread_mutex_t mutex_write;
  pthread_mutex_t mutex_read;
  pthread_cond_t  cond_releas;
  pthread_cond_t  cond_waiting;
  pthread_mutexattr_t attr_mutex_debug;
  u8 waiting;
  u8 worker;
}ComandStream;

static const size_t true_blk_size = 512;

static inline ComandStream *CS_CreateStreamFromSize( FreeNode *, const size_t);
static inline void *CS_Fetch( ComandStream *);
static inline void CS_Push( ComandStream *, const void *);
static inline void CS_Signal( ComandStream *);
static inline void *CS_WaitReleas( ComandStream *);
static inline void *CS_Releas( ComandStream *);
static inline void CS_Finish( ComandStream *, const void *);
static inline void CS_Free( FreeNode *, ComandStream *);

static inline ComandStream *
CS_CreateStreamFromSize( FreeNode *freenode, const size_t true_size){
  ComandStream *Stream;
  size_t blk_size;
  size_t size;
  address addr;
  assert( true_size);
  
  size = true_size + sizeof( address) - 1;
  size -= size % sizeof( address);
  
  blk_size = true_blk_size - sizeof( address);
  blk_size -= blk_size % sizeof( address);
  
  assert( blk_size >= size);
  
  Stream = MS_Create( freenode, ComandStream,
		      .blk_size = blk_size,
		      .size = size);
  
  addr = MS_CreateFromSize( freenode, true_blk_size);
  
  Stream -> freenode = freenode;
  
  *( address *)( addr + Stream -> blk_size) = addr;
  
  Stream -> blk_push   = addr;
  Stream -> blk_releas = addr;
  
  Stream -> push   = addr;
  Stream -> releas = addr;
  
  pthread_mutexattr_init( &Stream -> attr_mutex_debug);
#ifdef DEBUG
  pthread_mutexattr_settype( &Stream -> attr_mutex_debug, PTHREAD_MUTEX_ERRORCHECK);
#else
  pthread_mutexattr_settype( &Stream -> attr_mutex_debug, PTHREAD_MUTEX_DEFAULT);
#endif
  pthread_mutex_init( &Stream -> mutex_signal, &Stream -> attr_mutex_debug);
  pthread_mutex_init( &Stream -> mutex_blk   , &Stream -> attr_mutex_debug);
  pthread_mutex_init( &Stream -> mutex_push  , &Stream -> attr_mutex_debug);
  pthread_mutex_init( &Stream -> mutex_write , &Stream -> attr_mutex_debug);
  pthread_mutex_init( &Stream -> mutex_read  , &Stream -> attr_mutex_debug);
  pthread_cond_init(  &Stream -> cond_waiting, NULL);
  pthread_cond_init(  &Stream -> cond_releas , NULL);
  
  Stream -> waiting = 0;
  Stream -> worker = 0;
  
  return Stream;
}
#define CS_CreateStream( freenode, type) CS_CreateStreamFromSize( freenode, sizeof( type))


static inline void *
CS_Fetch( ComandStream *Stream){
  address ret = 0;
  assert( Stream != NULL);
  
  dassert( pthread_mutex_lock( &Stream -> mutex_write) == 0);
  
  dassert( pthread_mutex_lock( &Stream -> mutex_blk) == 0);
  if unlikely( Stream -> push == Stream -> blk_push + Stream -> blk_size){
    if unlikely( *( address *)( Stream -> blk_push + Stream -> blk_size) == Stream -> blk_releas){
      address addr = MS_CreateFromSize( Stream -> freenode, true_blk_size);
      *( address *)( addr + Stream -> blk_size) = *( address *)( Stream -> blk_push + Stream -> blk_size);
      *( address *)( Stream -> blk_push + Stream -> blk_size) = addr;
    }
    Stream -> blk_push = *( address *)( Stream -> blk_push + Stream -> blk_size);
    dassert( pthread_mutex_lock( &Stream -> mutex_push) == 0);
    Stream -> push  = Stream -> blk_push;
    dassert( pthread_mutex_unlock( &Stream -> mutex_push) == 0);
  }
  dassert( pthread_mutex_unlock( &Stream -> mutex_blk) == 0);
  
  ret = Stream -> push;
  
  return ( void *)ret;
}


static inline void
CS_Push( ComandStream *Stream, const void *ptr){
  address addr;
  assert( Stream != NULL);
  addr = ( address)ptr;
  
  assert( addr == Stream -> push);
  
  dassert( pthread_mutex_lock( &Stream -> mutex_push) == 0);
  Stream -> push = Stream -> push + Stream -> size;
  dassert( pthread_mutex_unlock( &Stream -> mutex_push) == 0);
  
  if( Stream -> push == Stream -> blk_push + Stream -> blk_size){
    CS_Signal( Stream);
  }
  
  dassert( pthread_mutex_unlock( &Stream -> mutex_write) == 0);
}

static inline void
CS_Signal( ComandStream *Stream){
  assert( Stream != NULL);
  
  if( pthread_mutex_trylock( &Stream -> mutex_signal) == 0){
    if( Stream -> waiting){
      dassert( pthread_mutex_unlock( &Stream -> mutex_signal) == 0);
      
      pthread_cond_signal( &Stream -> cond_releas);
    }else{
      dassert( pthread_mutex_unlock( &Stream -> mutex_signal) == 0);
    }
  }
}


static inline void
CS_iswaiting( ComandStream *Stream, struct timespec tv, u8 worker){
    
  dassert( pthread_mutex_lock( &Stream -> mutex_signal) == 0);
  
  Stream -> worker = worker;
  
  pthread_cond_timedwait( &Stream -> cond_waiting, &Stream -> mutex_signal, &tv);
  
  Stream -> worker = 0;
  
  dassert( pthread_mutex_unlock( &Stream -> mutex_signal) == 0);
  
  return;
}


static inline void *
CS_WaitReleas( ComandStream *Stream){
  address ret = 0;
  assert( Stream != NULL);
  
  while( ( ret = ( address)CS_Releas( Stream)) == 0){
    
    if( pthread_mutex_trylock( &Stream -> mutex_signal) == 0){
      
      Stream -> waiting += 1;
      
      if( Stream -> waiting >= Stream -> worker){
	pthread_cond_signal( &Stream -> cond_waiting);
      }
      
      pthread_cond_wait( &Stream -> cond_releas, &Stream -> mutex_signal);
      
      Stream -> waiting -= 1;
      
      dassert( pthread_mutex_unlock( &Stream -> mutex_signal) == 0);
    }
  }
  
  assert( ret != 0);
  return ( void *)ret;
}

//
// return a pointer to the next element in the stream
//
static inline void *
CS_Releas( ComandStream *Stream){
  address ret = 0;
  assert( Stream != NULL);
  
  if( pthread_mutex_trylock( &Stream -> mutex_read) != 0){
    goto end;
  }
  
  dassert( pthread_mutex_lock( &Stream -> mutex_push) == 0);
  if unlikely( Stream -> push == Stream -> releas){
    dassert( pthread_mutex_unlock( &Stream -> mutex_push) == 0);
    pthread_mutex_unlock( &Stream -> mutex_read);
    goto end;
  }
  dassert( pthread_mutex_unlock( &Stream -> mutex_push) == 0);
  
  dassert( pthread_mutex_lock( &Stream -> mutex_blk) == 0);
  if unlikely( Stream -> releas == Stream -> blk_releas + Stream -> blk_size){
    if unlikely( *( address *)( Stream -> blk_push + Stream -> blk_size) != Stream -> blk_releas){
      address blk_free =  *( address *)( Stream -> blk_push + Stream -> blk_size);
      *( address *)( Stream -> blk_push + ( Stream -> blk_size)) = *( address *)( blk_free + Stream -> blk_size);
      MS_FreeFromSize( Stream -> freenode, blk_free, true_blk_size);
    }
    Stream -> blk_releas = *( address *)( Stream -> blk_releas + Stream -> blk_size);
    Stream -> releas = Stream -> blk_releas;
    
    dassert( pthread_mutex_lock( &Stream -> mutex_push) == 0);
    if unlikely( Stream -> push == Stream -> releas){
      dassert( pthread_mutex_unlock( &Stream -> mutex_push) == 0);
      dassert( pthread_mutex_unlock( &Stream -> mutex_blk) == 0);
      pthread_mutex_unlock( &Stream -> mutex_read);
      goto end;
    }
    dassert( pthread_mutex_unlock( &Stream -> mutex_push) == 0);
  }
  dassert( pthread_mutex_unlock( &Stream -> mutex_blk) == 0);
  
  ret = Stream -> releas;
  
 end:
  return ( void *)ret;
}


static inline void
CS_Finish( ComandStream *Stream, const void *ptr){
  address addr;
  assert( Stream != NULL);
  addr = ( address)ptr;
  
  assert( ptr != NULL);
  
  dassert( addr == Stream -> releas);
  
  Stream -> releas = Stream -> releas + Stream -> size;
  
  dassert( pthread_mutex_unlock( &Stream -> mutex_read) == 0);
}


//
// free all block...
//
static inline void
CS_Free( FreeNode *freenode, ComandStream *Stream){
  if likely( Stream != NULL){
    address addr;
    
    pthread_mutex_lock( &Stream -> mutex_read);
    pthread_mutex_lock( &Stream -> mutex_write);
    addr = Stream -> blk_push;
    Stream -> blk_push = *( address *)( Stream -> blk_push + Stream -> blk_size);
    *( address *)( addr + Stream -> blk_size) = 0;
    
    while( Stream -> blk_push != 0){
      addr = Stream -> blk_push;
      Stream -> blk_push = *( address *)( Stream -> blk_push + Stream -> blk_size);
      MS_FreeFromSize( Stream -> freenode, addr, true_blk_size);
    }
    
    MS_Free( freenode, Stream);
  }
}


#ifdef __cplusplus
}
#endif
#endif
