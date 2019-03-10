

#ifdef MS_CS_H__
#else
#define MS_CS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
  
#include "MS_util.h"

typedef struct{
  size_t blk_size;
  size_t size;
  char *blk_fetch;
  char *blk_push;
  char *blk_releas;
  char *blk_finish;
  char *fetch;
  char *push;
  char *releas;
  char *finish;
}ComandStream;


static inline ComandStream *LOCALE_( CS_Create)( size_t);
static inline void *LOCALE_( CS_Fetch)( ComandStream *);
static inline void LOCALE_( CS_Push)( ComandStream *, void *);
static inline void *LOCALE_( CS_Releas)( ComandStream *);
static inline void LOCALE_( CS_Finish)( ComandStream *, void *);
static inline void LOCALE_( CS_Free)( ComandStream *);

_Pragma("GCC diagnostic ignored \"-Wpointer-arith\"")
_Pragma("GCC diagnostic ignored \"-Wcast-align\"")

#define NC 4096

static inline ComandStream *
LOCALE_( CS_Create)( size_t size){
  ComandStream *ret = NULL;
  ComandStream *Stream = MS_CreateEmpty( ComandStream);
  char *ptr;
  
  if( Stream == NULL) goto end;
  
  Stream -> blk_size = NC * size;
  
  assert( Stream -> blk_size);
  
  ptr = ( char *)malloc( Stream -> blk_size + sizeof( char *));
  
  if( ptr == NULL) goto end;
  
  *( char **)( ptr + Stream -> blk_size) = ptr;
    
  Stream -> size = size;
  
  Stream -> blk_fetch  = ptr;
  Stream -> blk_push   = ptr;
  Stream -> blk_releas = ptr;
  Stream -> blk_finish = ptr;
    
  Stream -> fetch  = ptr;
  Stream -> push   = ptr;
  Stream -> releas = ptr;
  Stream -> finish = ptr;

  ret = Stream;
 end:
  if( ret != Stream) LOCALE_( CS_Free)( Stream);
  return ret;
}
#define CS_Create( type) LOCALE_( CS_Create)( sizeof( type))


static inline void *
LOCALE_( CS_Fetch)( ComandStream *Stream){
  void *ret = NULL;
  assert( Stream != NULL);
  if unlikely( Stream -> fetch == Stream -> blk_fetch + Stream -> blk_size){
    if unlikely( *( char **)( Stream -> blk_fetch + Stream -> blk_size) == Stream -> blk_finish){
      char *ptr = ( char *)malloc( Stream -> blk_size + sizeof( char *));
      if unlikely( ptr == NULL) goto end;
      /*lock*/
      *( char **)( ptr + Stream -> blk_size) = *( char **)( Stream -> blk_fetch + Stream -> blk_size);
      *( char **)( Stream -> blk_fetch + Stream -> blk_size) = ptr;
      /*unlock*/
    }
    Stream -> blk_fetch = *( char **)( Stream -> blk_fetch + Stream -> blk_size);
    Stream -> fetch  = Stream -> blk_fetch;
  }
  
  ret = Stream -> fetch;
  Stream -> fetch = Stream -> fetch + Stream -> size;
  
 end:
  return ret;
}
#define CS_Fetch LOCALE_( CS_Fetch)


static inline void
LOCALE_( CS_Push)( ComandStream *Stream, void *ptr){
  assert( Stream != NULL);
  
  if unlikely( Stream -> push == Stream -> blk_push + Stream -> blk_size){
    Stream -> blk_push = *( char **)( Stream -> blk_push + Stream -> blk_size);
    Stream -> push = Stream -> blk_push;
  }
  
  assert( ptr == Stream -> push);
  
  Stream -> push = Stream -> push + Stream -> size;
}
#define CS_Push LOCALE_( CS_Push)

  
/* 
 * return a pointer to the next element in the stream
 */
static inline void *
LOCALE_( CS_Releas)( ComandStream *Stream){
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
#define CS_Releas LOCALE_( CS_Releas)
  

static inline void
LOCALE_( CS_Finish)( ComandStream *Stream, void *ptr){
  assert( Stream != NULL);
  
  if unlikely( Stream -> finish == Stream -> blk_finish + Stream -> blk_size){
    if unlikely( *( char **)( Stream -> blk_fetch + Stream -> blk_size) != Stream -> blk_finish){
      /*lock*/
      char *lptr =  *( char **)( Stream -> blk_fetch + Stream -> blk_size);
      *( char **)( Stream -> blk_fetch + ( Stream -> blk_size)) = *( char **)( lptr + Stream -> blk_size);
      /*unlock*/
      free( lptr);
    }
    Stream -> blk_finish = *( char **)( Stream -> blk_finish + Stream -> blk_size);
    Stream -> finish = Stream -> blk_finish;
  }
  
  assert( ptr == Stream -> finish);
  
  Stream -> finish = Stream -> finish + Stream -> size;
}
#define CS_Finish LOCALE_( CS_Finish)

  
/*
 * free all block...
 */
static inline void
LOCALE_( CS_Free)( ComandStream *Stream){
  if likely( Stream != NULL){
    char *ptr = Stream -> blk_fetch;
    Stream -> blk_fetch = *( char **)( Stream -> blk_fetch + Stream -> blk_size);
    *( char **)( ptr + Stream -> blk_size) = NULL;
   
    while( Stream -> blk_fetch != NULL){
      ptr = Stream -> blk_fetch;
      Stream -> blk_fetch = *( char **)( Stream -> blk_fetch + Stream -> blk_size);
      free( ptr);
    }
    
    free( Stream);
  }
}
#define CS_Free LOCALE_( CS_Free)
  
#ifdef __cplusplus
}
#endif
#endif
