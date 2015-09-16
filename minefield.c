
#include <stdlib.h>
#include <string.h>

#define LOCALE_( name) MF_##name

#include "MS_util.h"
#include "ComandStream.h"

#include "minefield.h"

__uint8_t uncover_element( MS_field, MS_pos, MS_mstr *);
__uint8_t setmine_element( __uint8_t *, MS_mstr *);
INLINE int addelement( ComandStream *, MS_field, signed long, signed long);


void
setminefield( MS_video video, MS_field minefield, MS_mstr *mine){
  unsigned long i;
  unsigned long x;
  
  i = video.height;
    
  x = minefield.subwidth - video.xdiff;
  x = x < video.width? x: video.width;
  
  if( x) while( i--){
    memset( minefield.data + ( video.xdiff + ( ( video.ydiff + i) % minefield.subheight) * minefield.width), ENUT, x);
  }
  
  i = video.height;
  
  if( video.width - x) while( i--){
    memset( minefield.data + ( ( video.ydiff + i) % minefield.subheight) * minefield.width, ENUT, video.width - x);
  }
  
  ( *mine).flaged = 0;
  
  ( *mine).uncoverd = 0;
  
  ( *mine).set = 0;
  
  ( *mine).mines = 0;
  
  ( *mine).hit = 0;

  if( ( *mine).reseed <= MS_RAND_MAX){
    ( *mine).seed = ( *mine).reseed;
  }
  
  return;
}


INLINE int
addelement( ComandStream *uncovque, MS_field minefield, signed long x, signed long y){
  
  /* check that ECOVER is true, to not uncover the same element twice, and also skip if EFLAG is true
   */
  if( ( *acse( minefield, x, y) & ECOVER) && ( *acse( minefield, x, y) & EFLAG) ^ EFLAG){
    MS_pos *pos = CS_Fetch( uncovque);
    if likely( pos != NULL){
      ( *pos).x = ( x + minefield.width ) % minefield.width;
      ( *pos).y = ( y + minefield.height) % minefield.height;
      *acse( minefield, x, y) &= ~ECOVER;
      CS_Push( uncovque, pos);
      return 0;
    }
    return -2;
  }
  
  return 0;
}



int
uncov( MS_field minefield, ComandStream *uncovque, MS_mstr *mine){
  int ret = 0;
  MS_pos *element;
  
  while likely( ( element = ( MS_pos *)CS_Releas( uncovque)) != NULL){
      
    /* check if elemnt has no suronding mines and if that is the case continue whit uncovering the neigburing elemnts
     */
    if( ( uncover_element( minefield, *element, mine) & ECOUNT) == EC_ZERO){
      if unlikely( addelement( uncovque, minefield, ( *element).x + 1, ( *element).y + 1) == -2) ret = -2;
      if unlikely( addelement( uncovque, minefield, ( *element).x - 1, ( *element).y + 1) == -2) ret = -2;
      if unlikely( addelement( uncovque, minefield, ( *element).x    , ( *element).y + 1) == -2) ret = -2;
      
      if unlikely( addelement( uncovque, minefield, ( *element).x + 1, ( *element).y - 1) == -2) ret = -2;
      if unlikely( addelement( uncovque, minefield, ( *element).x - 1, ( *element).y - 1) == -2) ret = -2;
      if unlikely( addelement( uncovque, minefield, ( *element).x    , ( *element).y - 1) == -2) ret = -2;
      
      if unlikely( addelement( uncovque, minefield, ( *element).x + 1, ( *element).y    ) == -2) ret = -2;
      if unlikely( addelement( uncovque, minefield, ( *element).x - 1, ( *element).y    ) == -2) ret = -2;
    }
    
    CS_Finish( uncovque, element);
  }
      
  return ret;
}

__uint8_t
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


__uint8_t
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
  
    if unlikely( addelement( CS, minefield, x, y) == -2) ret = -2;
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
