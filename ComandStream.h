

#ifdef _MS_CS_H__
#else
#define _MS_CS_H__
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
  __uint16_t efetch;
  __uint16_t epush;
  __uint16_t ereleas;
  __uint16_t efinish;
}ComandStream;


INLINE ComandStream *LOCALE_( CS_Create)( size_t);
INLINE void *LOCALE_( CS_Fetch)( ComandStream *);
INLINE void LOCALE_( CS_Push)( ComandStream *, void *);
INLINE void *LOCALE_( CS_Releas)( ComandStream *);
INLINE void LOCALE_( CS_Finish)( ComandStream *, void *);
INLINE void LOCALE_( CS_Free)( ComandStream *);

_Pragma("GCC diagnostic ignored \"-Wpointer-arith\"")
_Pragma("GCC diagnostic ignored \"-Wcast-align\"")

#define NC 4096

ComandStream *
LOCALE_( CS_Create)( size_t size){
  ComandStream *CS = ( ComandStream *)malloc( sizeof( ComandStream));
  char *ptr;
  
  if( CS == NULL){
    return NULL;
  }
  
  ( *CS).blk_size = NC * size;
    
  if( !( *CS).blk_size){
    free( CS);
    return NULL;
  }
  
  ptr = malloc( ( *CS).blk_size + sizeof( char *));
  
  if( ptr == NULL){
    free( CS);
    return NULL;
  }
  
  *( char **)( ptr + ( *CS).blk_size) = ptr;
    
  ( *CS).size = size;
  
  ( *CS).blk_fetch  = ptr;
  ( *CS).blk_push   = ptr;
  ( *CS).blk_releas = ptr;
  ( *CS).blk_finish = ptr;
    
  ( *CS).fetch  = ptr;
  ( *CS).push   = ptr;
  ( *CS).releas = ptr;
  ( *CS).finish = ptr;

  ( *CS).efetch  = 0;
  ( *CS).epush   = 0;
  ( *CS).ereleas = 0;
  ( *CS).efinish = 0;
      
  return CS;
}
#define CS_Create LOCALE_( CS_Create)


INLINE void *
LOCALE_( CS_Fetch)( ComandStream *CS){
  void *ret = NULL;
  if likely( CS != NULL){
    if unlikely( ( *CS).efetch >= NC){
      if unlikely( *( char **)( ( *CS).blk_fetch + ( *CS).blk_size) == ( *CS).blk_finish){
        char *ptr = malloc( ( *CS).blk_size + sizeof( char *));
        if unlikely( ptr == NULL){
	  goto bail;
        }
        /*lock*/
        *( char **)( ptr + ( *CS).blk_size) = *( char **)( ( *CS).blk_fetch + ( *CS).blk_size);
        *( char **)( ( *CS).blk_fetch + ( *CS).blk_size) = ptr;
        /*unlock*/
      }
      ( *CS).blk_fetch = *( char **)( ( *CS).blk_fetch + ( *CS).blk_size);
      ( *CS).fetch  = ( *CS).blk_fetch;
      ( *CS).efetch = 0;
    }
        
    ret = ( *CS).fetch;
    ( *CS).fetch = ( *CS).fetch + ( *CS).size;
    ++( *CS).efetch;
  }
  
 bail:
    
  return ret;
}
#define CS_Fetch LOCALE_( CS_Fetch)


INLINE void
LOCALE_( CS_Push)( ComandStream *CS, void *ptr){
  if likely( CS != NULL){
    if unlikely( ( *CS).epush >= NC){
      ( *CS).blk_push = *( char **)( ( *CS).blk_push + ( *CS).blk_size);
      ( *CS).push = ( *CS).blk_push;
      ( *CS).epush = 0;
      }
    
    if likely( ( *CS).push == ptr){
      ( *CS).push = ( *CS).push + ( *CS).size;
      ++( *CS).epush;
    }
  }
}
#define CS_Push LOCALE_( CS_Push)

  
/* 
 * return a pointer to the next element in the stream
 */
INLINE void *
LOCALE_( CS_Releas)( ComandStream *CS){
  void *ret = NULL;
  
  if likely( CS != NULL){
    if unlikely( ( *CS).push != ( *CS).releas){
      if( ( *CS).ereleas >= NC){
        ( *CS).blk_releas = *( char **)( ( *CS).blk_releas + ( *CS).blk_size);
        ( *CS).releas = ( *CS).blk_releas;
        ( *CS).ereleas = 0;
      }
      
      ret = ( *CS).releas;
      ( *CS).releas = ( *CS).releas + ( *CS).size;
      ++( *CS).ereleas;
    }
  }
    
  return ret;
}
#define CS_Releas LOCALE_( CS_Releas)
  

INLINE void
LOCALE_( CS_Finish)( ComandStream *CS, void *ptr){
  if likely( CS != NULL){
    if unlikely( ( *CS).efinish >= NC){
      if unlikely( *( char **)( ( *CS).blk_fetch + ( *CS).blk_size) != ( *CS).blk_finish){
	/*lock*/
	char *lptr =  *( char **)( ( *CS).blk_fetch + ( *CS).blk_size);
	*( char **)( ( *CS).blk_fetch + ( ( *CS).blk_size)) = *( char **)( lptr + ( *CS).blk_size);
	/*unlock*/
	free( lptr);
      }
      ( *CS).blk_finish = *( char **)( ( *CS).blk_finish + ( *CS).blk_size);
      ( *CS).finish = ( *CS).blk_finish;
      ( *CS).efinish = 0;
    }
    
    if likely( ( *CS).finish == ptr){
      ( *CS).finish = ( *CS).finish + ( *CS).size;
      ++( *CS).efinish;
    }
  }
}
#define CS_Finish LOCALE_( CS_Finish)

  
/*
 * free all block...
 */
void
LOCALE_( CS_Free)( ComandStream *CS){
  if likely( CS != NULL){
    char *ptr = ( *CS).blk_fetch;
    ( *CS).blk_fetch = *( char **)( ( *CS).blk_fetch + ( *CS).blk_size);
    *( char **)( ptr + ( *CS).blk_size) = NULL;
   
    while( ( *CS).blk_fetch != NULL){
      ptr = ( *CS).blk_fetch;
      ( *CS).blk_fetch = *( char **)( ( *CS).blk_fetch + ( *CS).blk_size);
      free( ptr);
    }
    
    free( CS);
  }
}
#define CS_Free LOCALE_( CS_Free)
  
#ifdef __cplusplus
}
#endif
#endif
