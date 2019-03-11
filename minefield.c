
#include <stdlib.h>
#include <string.h>

#define LOCALE_( name) MF_##name

#include "MS_util.h"
#include "ComandStream.h"
#include "minefield.h"

static inline __uint8_t uncover_element( MS_field, MS_pos, MS_mstr *);
static inline __uint8_t setmine_element( __uint8_t *, MS_mstr *);
static inline void addelement( MS_field *, signed long, signed long);


MS_field *
MF_Init( MS_field *minefield){
  
  dassert( minefield);
  
  minefield -> mine = MS_CreateEmpty( MS_mstr);
  minefield -> uncovque = CS_Create( MS_pos);
  
  minefield -> mine -> noelements = minefield -> width * minefield -> height;
  
  minefield -> subwidth  = minefield -> width;
  minefield -> subheight = minefield -> height;
  
  if( !minefield -> global){
    minefield -> width  += 1;
    minefield -> height += 1;
  }
  
  minefield -> width_divobj  = gen_divobj( minefield -> width );
  minefield -> height_divobj = gen_divobj( minefield -> height);
  
  minefield -> data = ( __uint8_t *)malloc( sizeof( __uint8_t) * minefield -> width * minefield -> height);
  
  assert( minefield -> data != NULL);
  
  if( !minefield -> global){
    memset( minefield -> data, ESET, minefield -> width * minefield -> height);
  }
  
  return minefield;
}


void
setminefield( MS_field  *minefield,
	      MS_stream *mss,
	      MS_video   video){
  
  int i;
  u32 x;
  
  i = ( int)video.height;
  
  video.xdiff = ( int)( ( u32)( video.xdiff + ( int)video.width ) % video.width );
  video.ydiff = ( int)( ( u32)( video.ydiff + ( int)video.height) % video.height);
  
  x = minefield -> subwidth - ( u32)video.xdiff;
  x = x < video.width? x: video.width;
  
  if( x) while( i--){
    memset( minefield -> data + ( video.xdiff + ( int)( ( ( u32)( video.ydiff + i) % minefield -> subheight) * minefield -> width)), ENUT, x);
  }
  
  i = ( int)video.height;
  
  if( video.width - x) while( i--){
    memset( minefield -> data + ( ( u32)( video.ydiff + i) % minefield -> subheight) * minefield -> width, ENUT, video.width - x);
  }
  
  minefield -> mine -> level = minefield -> level;
  
  minefield -> mine -> flaged = 0;
  
  minefield -> mine -> uncoverd = 0;
  
  minefield -> mine -> set = 0;
  
  minefield -> mine -> mines = 0;
  
  minefield -> mine -> hit = 0;
  
  minefield -> mine -> seed = MS_rand_seed();
  
  if( minefield -> reseed){
    minefield -> mine -> seed = minefield -> reseed;
  }
  
  MS_print( mss -> deb, "\rSeed: %08x   \n", minefield -> mine -> seed);
}


void
MF_Free( MS_field *minefield){
  if( minefield != NULL){
    MS_Free( minefield -> data);
    MS_Free( minefield -> mine);
    
    CS_Free( minefield -> uncovque);
    MS_Free( minefield);
  }
}

static inline void
addelement( MS_field *minefield, signed long x, signed long y){
  /* check that ECOVER is true, to not uncover the same element twice, and also skip if EFLAG is true
   */
  if( ( *acse( *minefield, x, y) & ECOVER) && ( *acse( *minefield, x, y) & EFLAG) ^ EFLAG){
    MS_pos *pos = ( MS_pos *)CS_Fetch( minefield -> uncovque);
    pos -> x = (s32)mol_( (u32)( x + (s32)minefield -> width ), minefield -> width , minefield -> width_divobj );
    pos -> y = (s32)mol_( (u32)( y + (s32)minefield -> height), minefield -> height, minefield -> height_divobj);
    *acse( *minefield, x, y) &= ~ECOVER;
    CS_Push( minefield -> uncovque, pos);
  }
}



void
uncov( MS_field *minefield){
  MS_pos *element;
  
  dassert( minefield != NULL);
  
  while likely( ( element = ( MS_pos *)CS_Releas( minefield -> uncovque)) != NULL){
    
    /* check if elemnt has no suronding mines and if that is the case continue whit uncovering the neigburing elemnts
     */
    if likely( ( uncover_element( *minefield, *element, minefield -> mine) & ECOUNT) == 0){
      addelement( minefield, element -> x + 1, element -> y + 1);
      addelement( minefield, element -> x - 1, element -> y + 1);
      addelement( minefield, element -> x    , element -> y + 1);
      
      addelement( minefield, element -> x + 1, element -> y - 1);
      addelement( minefield, element -> x - 1, element -> y - 1);
      addelement( minefield, element -> x    , element -> y - 1);
      
      addelement( minefield, element -> x + 1, element -> y    );
      addelement( minefield, element -> x - 1, element -> y    );
    }
    
    CS_Finish( minefield -> uncovque, element);
  }
}

static inline __uint8_t
uncover_element( MS_field minefield, MS_pos postion, MS_mstr *mine){
  
  /* chech that it hasnt been uncover yet, becuse elements are set to ECOVER | ECOUNT and ECOVER alaredy is down;
   */
  if likely( ( *acse( minefield, postion.x, postion.y) & ECOUNT) == ECOUNT){
    ( *mine).uncoverd += 1;
    
    *acse( minefield, postion.x, postion.y) &= ~ECOUNT;
    
    ( *mine).hit += ( ( *acse( minefield, postion.x, postion.y) = setmine_element( acse( minefield, postion.x, postion.y), mine)) & EMINE) >> SMINE; 
    
    *acse( minefield, postion.x, postion.y) += ( setmine_element( acse( minefield, postion.x + 1, postion.y + 1), mine) & EMINE) >> SMINE;
    *acse( minefield, postion.x, postion.y) += ( setmine_element( acse( minefield, postion.x - 1, postion.y + 1), mine) & EMINE) >> SMINE;
    *acse( minefield, postion.x, postion.y) += ( setmine_element( acse( minefield, postion.x    , postion.y + 1), mine) & EMINE) >> SMINE;
    
    *acse( minefield, postion.x, postion.y) += ( setmine_element( acse( minefield, postion.x + 1, postion.y - 1), mine) & EMINE) >> SMINE;
    *acse( minefield, postion.x, postion.y) += ( setmine_element( acse( minefield, postion.x - 1, postion.y - 1), mine) & EMINE) >> SMINE;
    *acse( minefield, postion.x, postion.y) += ( setmine_element( acse( minefield, postion.x    , postion.y - 1), mine) & EMINE) >> SMINE;
    
    *acse( minefield, postion.x, postion.y) += ( setmine_element( acse( minefield, postion.x + 1, postion.y    ), mine) & EMINE) >> SMINE;
    *acse( minefield, postion.x, postion.y) += ( setmine_element( acse( minefield, postion.x - 1, postion.y    ), mine) & EMINE) >> SMINE;
  }
  
  return *acse( minefield, postion.x, postion.y);
}


static inline __uint8_t
setmine_element( __uint8_t *element, MS_mstr *mine){
  if( !( ( *element) & ESET)){
    __uint8_t u;
    
    u = ( ( ( __uint64_t)( ( *mine).noelements - ( *mine).set  ) * ( __uint64_t)( ( *mine).seed = MS_rand( ( *mine).seed))) <
	  ( ( __uint64_t)( ( *mine).level      - ( *mine).mines) * ( __uint64_t)MS_RAND_MAX));
    
    ++( *mine).set;
    
    ( *element) |= u << SMINE;
    ( *mine).mines += u;
    
    ( *element) |= ESET;
  }
  
  return *element;
}


void
uncov_elements( MS_field *minefield,
		MS_video  vid){
  
  unsigned long i;
  int x, y;
  
  i = vid.width * vid.height;
  
  while( i--){
    x = ( s32)( i % vid.width) + vid.xdiff;
    y = ( s32)( i / vid.width) + vid.ydiff;
    
    addelement( minefield, x, y);
  }
}


void
setzero( MS_field *minefield,
	 MS_video  vid){
  
  unsigned long i;
  int x, y;
  
  i = vid.width * vid.height;
  
  while( i--){
    x = ( s32)( i % vid.width) + vid.xdiff;
    y = ( s32)( i / vid.width) + vid.ydiff;

    if( !( *acse( *minefield, x, y) & ESET ) &&
        !( *acse( *minefield, x, y) & EFLAG)){
      *acse( *minefield, x, y) |= ESET;
      ++minefield -> mine ->  set;
    }
    
  }
}
