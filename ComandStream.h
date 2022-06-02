

#ifdef MS_CS_H__
#else
#define MS_CS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "MS_util.h"

struct _cs_substream{
  struct _cs_substream *next;
  address blk_push;
  address blk_releas;
  address push;
  address releas;
  pthread_mutex_t mutex_blk;
  pthread_mutex_t mutex_read;
};

struct CS_Block{
  address blk_ptr;
  size_t blk_size;
  size_t size;
};

typedef struct{
  const size_t blk_size;
  const size_t size;
  FreeNode *freenode;
  struct _cs_substream *head;
  pthread_mutex_t mutex_head;
  pthread_mutex_t mutex_signal;
  pthread_cond_t  cond_releas;
  pthread_cond_t  cond_waiting;
  pthread_mutexattr_t attr_mutex_debug;
  u8 waiting;
  u8 reader;
  u8 empty;
  u8 id;
}ComandStream;

static const size_t true_blk_size = 512;
static const u8 _cs_max_stream_id = 64;

extern __thread struct _cs_substream **_cs_substream_bunker;
extern __thread struct _cs_substream **_cs_reading_bunker;

extern u8 id_allocate;

static inline ComandStream *CS_CreateStreamFromSize( FreeNode *, const size_t);
static inline void *CS_Fetch( ComandStream *);
static inline void CS_Push( ComandStream *, const void *);
static inline void CS_Signal( ComandStream *);
static inline void *CS_WaitReleas( ComandStream *);
static inline void CS_Wait( ComandStream *);
static inline void CS_TimedWait( ComandStream *, struct timespec *);
static inline void *CS_Releas( ComandStream *);
static inline int CS_isEmpty( ComandStream *);
static inline void CS_Finish( ComandStream *, const void *);
static inline void CS_Free( FreeNode *, ComandStream *);

static inline ComandStream *
CS_CreateStreamFromSize( FreeNode *freenode, const size_t true_size){
  ComandStream *Stream;
  size_t blk_size;
  size_t size;
  
  assert( true_size);
  
  size = true_size;
  
  blk_size = true_blk_size - sizeof( address);
  blk_size -= blk_size % sizeof( address);
  
  assert( !( blk_size % size));
  
  assert( blk_size >= size);
  
  Stream = MS_Create( freenode, ComandStream,
		      .blk_size = blk_size,
		      .size = size);
  
  Stream -> freenode = freenode;
  
  assert( id_allocate < _cs_max_stream_id);
  
  atomic_fetch_add( &id_allocate, 1);
  
  Stream -> id = id_allocate;
  
  pthread_mutexattr_init( &Stream -> attr_mutex_debug);
#ifdef DEBUG
  pthread_mutexattr_settype( &Stream -> attr_mutex_debug, PTHREAD_MUTEX_ERRORCHECK);
#else
  pthread_mutexattr_settype( &Stream -> attr_mutex_debug, PTHREAD_MUTEX_DEFAULT);
#endif
  
  pthread_mutex_init( &Stream -> mutex_head  , &Stream -> attr_mutex_debug);
  pthread_mutex_init( &Stream -> mutex_signal, &Stream -> attr_mutex_debug);
  pthread_cond_init(  &Stream -> cond_waiting, NULL);
  pthread_cond_init(  &Stream -> cond_releas , NULL);
  
  Stream -> waiting = 0;
  Stream -> reader = 0;
  Stream -> empty = true;
  
  return Stream;
}
#define CS_CreateStream( freenode, type) CS_CreateStreamFromSize( freenode, sizeof( type))


static inline void *
CS_Fetch( ComandStream *Stream){
  address ret = 0;
  struct _cs_substream *_cs_substream;
  
  assert( Stream != NULL);
  
  if unlikely( _cs_substream_bunker == NULL){
    _cs_substream_bunker = MS_CreateArray( Stream -> freenode, _cs_max_stream_id, struct _cs_substream *, 0);
  }
  
  if unlikely( _cs_substream_bunker[ Stream -> id] == NULL){
    address addr = MS_CreateFromSize( Stream -> freenode, true_blk_size);
    
    _cs_substream = MS_Create( Stream -> freenode, struct _cs_substream, 0);
    
    _cs_substream -> blk_push   = addr;
    _cs_substream -> blk_releas = addr;
    
    _cs_substream -> push   = addr;
    _cs_substream -> releas = addr;
    
    *( address *)( _cs_substream -> blk_push + Stream -> blk_size) = addr;
    
    pthread_mutex_init( &_cs_substream -> mutex_blk   , &Stream -> attr_mutex_debug);
    pthread_mutex_init( &_cs_substream -> mutex_read  , &Stream -> attr_mutex_debug);
    
    dassert( pthread_mutex_lock( &Stream -> mutex_head) == 0);
    
    _cs_substream -> next = _cs_substream;
    
    if( Stream -> head == NULL){
      atomic_store( &Stream -> head, _cs_substream);
      goto done;
    }
    
    _cs_substream -> next = Stream -> head -> next;
    atomic_store( &Stream -> head -> next, _cs_substream);
    
  done:
    
    dassert( pthread_mutex_unlock( &Stream -> mutex_head) == 0);
    
    _cs_substream_bunker[ Stream -> id] = _cs_substream;
  }
  
  _cs_substream = _cs_substream_bunker[ Stream -> id];
  
  if unlikely( _cs_substream -> push == _cs_substream -> blk_push + Stream -> blk_size){
    dassert( pthread_mutex_lock( &_cs_substream -> mutex_blk) == 0);
    if unlikely( *( address *)( _cs_substream -> blk_push + Stream -> blk_size) == _cs_substream -> blk_releas){
      address addr = MS_CreateFromSize( Stream -> freenode, true_blk_size);
      *( address *)( addr + Stream -> blk_size) = *( address *)( _cs_substream -> blk_push + Stream -> blk_size);
      *( address *)( _cs_substream -> blk_push + Stream -> blk_size) = addr;
    }
    
    _cs_substream -> blk_push = *( address *)( _cs_substream -> blk_push + Stream -> blk_size);
    
    atomic_store( &_cs_substream -> push, _cs_substream -> blk_push);
    
    dassert( pthread_mutex_unlock( &_cs_substream -> mutex_blk) == 0);
  }
  
  ret = _cs_substream -> push;
  
  return ( void *)ret;
}


static inline void
CS_Push( ComandStream *Stream, const void *ptr){
  address addr;
  struct _cs_substream *_cs_substream;
  
  assert( Stream != NULL);
  assert( _cs_substream_bunker != NULL);
  assert( _cs_substream_bunker[ Stream -> id] != NULL);
  
  _cs_substream = _cs_substream_bunker[ Stream -> id];
  
  addr = ( address)ptr;
  
  assert( addr == _cs_substream -> push);
  
  if( atomic_load( &Stream -> empty) != false)
    atomic_store( &Stream -> empty, false);
  
  atomic_fetch_add( &_cs_substream -> push, Stream -> size);
}

static inline void
CS_Signal( ComandStream *Stream){
  
  assert( Stream != NULL);
  
  pthread_cond_signal( &Stream -> cond_releas);
}


static inline void
CS_iswaiting( ComandStream *Stream, struct timespec tv){
  
  dassert( pthread_mutex_lock( &Stream -> mutex_signal) == 0);
  
  pthread_cond_timedwait( &Stream -> cond_waiting, &Stream -> mutex_signal, &tv);
  
  dassert( pthread_mutex_unlock( &Stream -> mutex_signal) == 0);
  
  return;
}


static inline void *
CS_WaitReleas( ComandStream *Stream){
  address ret = 0;
  
  assert( Stream != NULL);
  
  while( ( ret = ( address)CS_Releas( Stream)) == 0){
    CS_Wait( Stream);
  }
  
  assert( ret != 0);
  return ( void *)ret;
}


static inline void
CS_Wait( ComandStream *Stream){
  CS_TimedWait( Stream, NULL);
}


static inline void
CS_TimedWait( ComandStream *Stream, struct timespec *tv){
  
  assert( Stream != NULL);
  
  dassert( pthread_mutex_lock( &Stream -> mutex_signal) == 0);
  
  Stream -> waiting += 1;

  if( Stream -> waiting >= Stream -> reader){
    pthread_cond_signal( &Stream -> cond_waiting);
  }
  
  if( _cs_reading_bunker != NULL){
    _cs_reading_bunker[ Stream -> id] = NULL;
  }
  
  if( tv != NULL){
    pthread_cond_timedwait( &Stream -> cond_releas, &Stream -> mutex_signal, tv);
  }else{
    pthread_cond_wait( &Stream -> cond_releas, &Stream -> mutex_signal);
  }
  
  Stream -> waiting -= 1;
  
  dassert( pthread_mutex_unlock( &Stream -> mutex_signal) == 0);
}


static inline struct CS_Block
CS_BlockReleas( ComandStream *Stream){
  struct CS_Block block = { 0};
  struct _cs_substream *_cs_substream;
  
  assert( Stream != NULL);
  assert( _cs_reading_bunker != NULL);
  assert( _cs_reading_bunker[ Stream -> id] != NULL);
  
  _cs_substream = _cs_reading_bunker[ Stream -> id];
  
  block.blk_size = Stream -> blk_size;
  block.size = Stream -> size;
  
  dassert( pthread_mutex_lock( &_cs_substream -> mutex_blk) == 0);
  
  if( _cs_substream -> blk_releas != _cs_substream -> blk_push &&
      *( address *)( _cs_substream -> blk_push + Stream -> blk_size) == _cs_substream -> blk_releas){
    
    block.blk_ptr = _cs_substream -> blk_releas;
    
    *( address *)( _cs_substream -> blk_push + Stream -> blk_size) = *( address *)( _cs_substream -> blk_releas + Stream -> blk_size);
    
    _cs_substream -> blk_releas = *( address *)( _cs_substream -> blk_releas + Stream -> blk_size);
    
    _cs_substream -> releas = _cs_substream -> blk_releas;
    
    dassert( pthread_mutex_unlock( &_cs_substream -> mutex_read) == 0);
  }
  
  dassert( pthread_mutex_unlock( &_cs_substream -> mutex_blk) == 0);
  
  return block;
}

//
// return a pointer to the next element in the stream
//
static inline void *
CS_Releas( ComandStream *Stream){
  struct _cs_substream *_cs_substream;
  struct _cs_substream *end_sentinel;
  
  assert( Stream != NULL);
  
  if unlikely( _cs_reading_bunker == NULL){
    _cs_reading_bunker = MS_CreateArray( Stream -> freenode, _cs_max_stream_id, struct _cs_substream *, 0);
  }
  
  _cs_substream = _cs_reading_bunker[ Stream -> id];
  
  if unlikely( _cs_substream == NULL){
    _cs_substream = atomic_load( &Stream -> head);
    
    if( _cs_substream == NULL)
      return NULL;
    
    atomic_fetch_add( &Stream -> reader, 1);
    
    atomic_store( &Stream -> head, Stream -> head -> next);
  }
  
  end_sentinel = _cs_substream;
  
  do{
    _cs_substream = _cs_substream -> next;
    
    if( pthread_mutex_trylock( &_cs_substream -> mutex_read) != 0){
      continue;
    }
    
    if( atomic_load( &_cs_substream -> push) == _cs_substream -> releas){
      dassert( pthread_mutex_unlock( &_cs_substream -> mutex_read) == 0);
      continue;
    }
    
    if unlikely( _cs_substream -> releas == _cs_substream -> blk_releas + Stream -> blk_size){
      dassert( pthread_mutex_lock( &_cs_substream -> mutex_blk) == 0);
      
      _cs_substream -> blk_releas = *( address *)( _cs_substream -> blk_releas + Stream -> blk_size);
      
      if( *( address *)( _cs_substream -> blk_push + Stream -> blk_size) != _cs_substream -> blk_releas){
	address blk_free =  *( address *)( _cs_substream -> blk_push + Stream -> blk_size);
	atomic_store( ( address *)( _cs_substream -> blk_push + ( Stream -> blk_size)), *( address *)( blk_free + Stream -> blk_size));
	MS_FreeFromSize( Stream -> freenode, blk_free, true_blk_size);
      }
      dassert( pthread_mutex_unlock( &_cs_substream -> mutex_blk) == 0);
      
      _cs_substream -> releas = _cs_substream -> blk_releas;
            
      if unlikely( atomic_load( &_cs_substream -> push) == _cs_substream -> releas){
	dassert( pthread_mutex_unlock( &_cs_substream -> mutex_read) == 0);
	continue;
      }
    }
    
    _cs_reading_bunker[ Stream -> id] = _cs_substream;
    
    return ( void *)_cs_substream -> releas;
    
  }while( _cs_substream != end_sentinel);
  
  if( atomic_load( &Stream -> empty) != true)
    atomic_store( &Stream -> empty, true);
  
  _cs_reading_bunker[ Stream -> id] = NULL;
  
  return NULL;
}


static inline int
CS_isEmpty( ComandStream *Stream){
  
  if( Stream == NULL)
    return -1;
  
  if( Stream -> head == NULL)
    return 1;
  
  return atomic_load( &Stream -> empty)? 1: 0;
}


static inline void
CS_Finish( ComandStream *Stream, const void *ptr){
  address addr = ( address)ptr;
  struct _cs_substream *_cs_substream;
  
  assert( Stream != NULL);
  assert( _cs_reading_bunker != NULL);
  assert( _cs_reading_bunker[ Stream -> id] != NULL);
  
  _cs_substream = _cs_reading_bunker[ Stream -> id];
  
  assert( addr == _cs_substream -> releas);
  
  atomic_fetch_add( &_cs_substream -> releas, Stream -> size);
  
  dassert( pthread_mutex_unlock( &_cs_substream -> mutex_read) == 0);
}


//
// free all block...
//
static inline void
CS_Free( FreeNode *freenode, ComandStream *Stream){
  {
    struct _cs_substream *_cs_substream;
    address addr;
    
    assert( Stream != NULL);
    if( _cs_substream_bunker == NULL &&
	_cs_substream_bunker[ Stream -> id] != NULL){
      goto free_readear;
    }
    
    _cs_substream = _cs_substream_bunker[ Stream -> id];
    
    pthread_mutex_lock( &_cs_substream -> mutex_read);
    
    if( _cs_substream -> releas != _cs_substream -> push){
      pthread_mutex_unlock( &_cs_substream -> mutex_read);
      goto free_readear;
    }
    
    addr = _cs_substream -> blk_push;
    _cs_substream -> blk_push = *( address *)( _cs_substream -> blk_push + Stream -> blk_size);
    *( address *)( addr + Stream -> blk_size) = 0;
    
    while( _cs_substream -> blk_push != 0){
      addr = _cs_substream -> blk_push;
      _cs_substream -> blk_push = *( address *)( _cs_substream -> blk_push + Stream -> blk_size);
      MS_FreeFromSize( freenode, addr, true_blk_size);
    }
  }
  {
    struct _cs_substream *end_sentinel;
    
    end_sentinel = atomic_load( &Stream -> head);
    
    do{
      atomic_store( &Stream -> head, Stream -> head -> next);
      
      if( Stream -> head -> next == _cs_substream_bunker[ Stream -> id]){
	atomic_store( &Stream -> head -> next, Stream -> head -> next -> next);
	break;
      }
      
    }while( Stream -> head != end_sentinel);
    
    if( Stream -> head != _cs_substream_bunker[ Stream -> id]){
      MS_Free( freenode, _cs_substream_bunker[ Stream -> id]);
      _cs_substream_bunker[ Stream -> id] = NULL;
    }
  }
  
 free_readear:
  {
    if( _cs_reading_bunker != NULL &&
	_cs_reading_bunker[ Stream -> id] != NULL){
      _cs_reading_bunker[ Stream -> id] = NULL;
      atomic_fetch_sub( &Stream -> reader, 1);
    }
  }
  {
    struct _cs_substream *head;
    
    dassert( pthread_mutex_lock( &Stream -> mutex_head) == 0);
    
    head = atomic_load( &Stream -> head);
    
    if( head != head -> next ||
	atomic_load( &Stream -> reader) != 0){
      dassert( pthread_mutex_unlock( &Stream -> mutex_head) == 0);
      return;
    }
    
    atomic_store( &Stream -> head, NULL);
    
    if( atomic_load( &Stream -> reader) != 0){
      dassert( pthread_mutex_unlock( &Stream -> mutex_head) == 0);
      return;
    }
    
    MS_Free( freenode, head);
    
    _cs_substream_bunker[ Stream -> id] = NULL;
    
    MS_Free( freenode, Stream);
  }
}


#ifdef __cplusplus
}
#endif
#endif
