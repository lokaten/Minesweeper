
#include <stdlib.h>
#include <string.h>

#define LOCALE_( name) MF_##name

#include "MS_util.h"
#include "ComandStream.h"
#include "minefield.h"

INLINE __uint8_t uncover_element( MS_field, MS_pos, MS_mstr *);
INLINE __uint8_t setmine_element( __uint8_t *, MS_mstr *);
INLINE int addelement( MS_field *, signed long, signed long);


MS_field *
MF_Init( MS_field *minefield){
  MS_field *ret = NULL;
  
  if( minefield == NULL){
    goto end;
  }
  
  if( ( minefield -> mine     = MS_CreateEmpty( MS_mstr   )) == NULL) goto end;
  if( ( minefield -> uncovque = CS_Create( sizeof( MS_pos))) == NULL) goto end;
  
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
  
  if( minefield -> data == NULL) goto end;
  
  if( !minefield -> global){
    memset( minefield -> data, ESET, minefield -> width * minefield -> height);
  }
  
  ret = minefield;
 end:
  if( ret == NULL)MF_Free( minefield);
  return ret;
}


void
setminefield( MS_field *minefield, MS_stream *mss, MS_video video){
  unsigned long i;
  unsigned long x;
  
  i = video.height;
  
  x = minefield -> subwidth - video.xdiff;
  x = x < video.width? x: video.width;
  
  if( x) while( i--){
    memset( minefield -> data + ( video.xdiff + ( ( video.ydiff + i) % minefield -> subheight) * minefield -> width), ENUT, x);
  }
  
  i = video.height;
  
  if( video.width - x) while( i--){
    memset( minefield -> data + ( ( video.ydiff + i) % minefield -> subheight) * minefield -> width, ENUT, video.width - x);
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
    if( minefield -> data != NULL) free( minefield -> data);
    if( minefield -> mine != NULL)free( minefield -> mine);
    
    CS_Free( minefield -> uncovque);
  }
}

INLINE int
addelement( MS_field *minefield, signed long x, signed long y){
  int ret = 0;
  /* check that ECOVER is true, to not uncover the same element twice, and also skip if EFLAG is true
   */
  if( ( *acse( *minefield, x, y) & ECOVER) && ( *acse( *minefield, x, y) & EFLAG) ^ EFLAG){
    MS_pos *pos = ( MS_pos *)CS_Fetch( minefield -> uncovque);
    if likely( pos != NULL){
      pos -> x = mol_( ( x + minefield -> width ), minefield -> width , minefield -> width_divobj );
      pos -> y = mol_( ( y + minefield -> height), minefield -> height, minefield -> height_divobj);
      *acse( *minefield, x, y) &= ~ECOVER;
      CS_Push( minefield -> uncovque, pos);
    }else{
      ret = -2;
    }
  }
  
  return ret;
}



int
uncov( MS_field *minefield){
  int ret = 0;
  MS_pos *element;
  
  while likely( ( element = ( MS_pos *)CS_Releas( minefield -> uncovque)) != NULL){
      
    /* check if elemnt has no suronding mines and if that is the case continue whit uncovering the neigburing elemnts
     */
    if( ( uncover_element( *minefield, *element, minefield -> mine) & ECOUNT) == 0){
      
      ret = addelement( minefield, element -> x + 1, element -> y + 1);
      ret = addelement( minefield, element -> x - 1, element -> y + 1);
      ret = addelement( minefield, element -> x    , element -> y + 1);
      
      ret = addelement( minefield, element -> x + 1, element -> y - 1);
      ret = addelement( minefield, element -> x - 1, element -> y - 1);
      ret = addelement( minefield, element -> x    , element -> y - 1);
      
      ret = addelement( minefield, element -> x + 1, element -> y    );
      ret = addelement( minefield, element -> x - 1, element -> y    );
    }
    
    CS_Finish( minefield -> uncovque, element);
  }
      
  return ret;
}

INLINE __uint8_t
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


INLINE __uint8_t
setmine_element( __uint8_t *element, MS_mstr *mine){
  if( !( ( *element) & ESET)){
    __uint8_t u;  
    
    /* this is probrbly the third or fourth atampte to detect 32bit machins hoepe fully it works*/
    if( sizeof( unsigned long) == 8){
      u = ( ( ( __uint64_t)( ( *mine).noelements - ( *mine).set  ) * ( __uint64_t)( ( *mine).seed = MS_rand( ( *mine).seed))) <
            ( ( __uint64_t)( ( *mine).level      - ( *mine).mines) * ( __uint64_t)MS_RAND_MAX));
    }else{
      if( ( *mine).noelements > ( *mine).set){
        u = ( __uint32_t)( ( *mine).seed = MS_rand( ( *mine).seed)) < ( ( MS_RAND_MAX / ( ( ( *mine).noelements - ( *mine).set))) * ( ( *mine).level - ( *mine).mines));
      }else{
        u = 0;
      }
    }
    
    ++( *mine).set;
    
    ( *element) |= u << SMINE;
    ( *mine).mines += u;
        
    ( *element) |= ESET;
  }
  
  return *element;
}


int
uncov_elements( MS_field minefield, ComandStream *CS, MS_video vid, MS_mstr *mine){
  unsigned long i, x, y;
  signed long ret;
  
  ( void)mine;
  
  ret = 0;
  
  i = vid.width * vid.height;
  
  while( i--){
    x = ( ( i % vid.width) + vid.xdiff) % minefield.width;
    y = ( ( i / vid.width) + vid.ydiff) % minefield.height;
  
    if unlikely( addelement( &minefield, x, y) == -2) ret = -2;
  }

  if( CS -> push != CS -> releas){
    ret = 1;
  }
  
  return ret;
}


void
setzero( MS_field minefield, MS_mstr *mine, MS_video vid){
  unsigned long i, x, y;
    
  i = vid.width * vid.height;
  
  while( i--){
    x = ( ( i % vid.width) + vid.xdiff) % minefield.width;
    y = ( ( i / vid.width) + vid.ydiff) % minefield.height;

    if( !( *acse( minefield, x, y) & ESET ) &&
        !( *acse( minefield, x, y) & EFLAG)){
      *acse( minefield, x, y) |= ESET;
      ++( *mine).set;
    }
    
  }
    
  return;
}
