
#include <stdlib.h>
#include <string.h>

#define LOCALE_( name) MF_##name

#include "MS_util.h"
#include "ComandStream.h"
#include "minefield.h"

static inline MS_element *uncover_element( const MS_field, void *, MS_pos, MS_mstr *);
static inline MS_element *setmine_element( MS_element *, MS_mstr *);
static inline void addelement( const MS_field *, s16, s16);


const MS_field *
MF_Init( MS_field *proto){
  MS_field *minefield;
  assert( proto != NULL);
  
  minefield = MS_Create( MS_field,
			 .width = proto -> width + !proto -> global,
			 .width_divobj = gen_divobj( proto -> width + !proto -> global),
			 .height = proto -> height + !proto -> global,
			 .height_divobj  = gen_divobj( proto -> height + !proto -> global),
			 .subwidth  = proto -> width,
			 .subheight = proto -> height,
			 .level = proto -> level,
			 .global = proto -> global,
			 .reseed = 0);
  
  minefield -> mine = MS_CreateEmpty( MS_mstr);
  minefield -> uncovque = CS_Create( MS_pos);
  
  minefield -> mine -> noelements = minefield -> width * minefield -> height;
  
  minefield -> data = ( MS_element *)malloc( sizeof( MS_element) * minefield -> width * minefield -> height);
  
  assert( minefield -> data != NULL);
  
  if( !minefield -> global){
    int i = (int)( minefield -> width * minefield -> height);
    
    while( i--){
      ( minefield -> data + i) -> set  = 1;
      ( minefield -> data + i) -> mine = 0;
    }
  }
  
  return minefield;
}


void
setminefield( const MS_field  *minefield,
	      void *GW,
	      const MS_stream *mss,
	      MS_video   video){
  
  u32 i;
  
  MS_pos element;
  
  u16 w = video.width;
  u16 h = video.height;
  
  video.xdiff = ( int)( ( u32)( video.xdiff + ( int)video.width ) % video.width );
  video.ydiff = ( int)( ( u32)( video.ydiff + ( int)video.height) % video.height);
  
  i = w * h;
  
  while( i--){
    
    {
      element.x = (s16)( (u16)video.xdiff + i % w);
      element.y = (s16)( (u16)video.ydiff + i / w);
    }
    
    if( acse( *minefield, element.x, element.y) -> count != 15){
      
      *acse( *minefield, element.x, element.y) = (MS_element){ .count = 15, .cover = 1};
      
      if( GW != NULL)
	drawelement( GW, minefield, element.x, element.y);
    }
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
MF_Free( const MS_field *minefield){
  if( minefield != NULL){
    MS_Free( minefield -> data);
    MS_Free( minefield -> mine);
    
    CS_Free( minefield -> uncovque);
  }
}

static inline void
addelement( const MS_field *minefield, s16 x, s16 y){
  /* check that ECOVER is true, to not uncover the same element twice, and also skip if EFLAG is true
   */
  if( acse( *minefield, x, y) -> cover && !acse( *minefield, x, y) -> flag){
    MS_pos *pos = ( MS_pos *)CS_Fetch( minefield -> uncovque);
    pos -> x = (s32)mol_( (u32)( x + (s32)minefield -> width ), minefield -> width , minefield -> width_divobj );
    pos -> y = (s32)mol_( (u32)( y + (s32)minefield -> height), minefield -> height, minefield -> height_divobj);
    acse( *minefield, x, y) -> cover = 0;
    CS_Push( minefield -> uncovque, pos);
  }
}


void
uncov( const MS_field *minefield, void *GW){
  MS_pos *element;
  assert( minefield != NULL);
  
  while likely( ( element = ( MS_pos *)CS_Releas( minefield -> uncovque)) != NULL){
    /* check if elemnt has no suronding mines and if that is the case continue whit uncovering the neigburing elemnts
     */
    if likely( uncover_element( *minefield, GW, *element, minefield -> mine) -> count == 0){
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

static inline MS_element *
uncover_element( const MS_field minefield, void *GW, MS_pos postion, MS_mstr *mine){
  
  if likely( acse( minefield, postion.x, postion.y) -> count == 15){
    mine -> uncoverd += 1;
    
    acse( minefield, postion.x, postion.y) -> count = 0;
    
    mine -> hit += setmine_element( acse( minefield, postion.x, postion.y), mine) -> mine;
    
    acse( minefield, postion.x, postion.y) -> count += setmine_element( acse( minefield, postion.x + 1, postion.y + 1), mine) -> mine;
    acse( minefield, postion.x, postion.y) -> count += setmine_element( acse( minefield, postion.x - 1, postion.y + 1), mine) -> mine;
    acse( minefield, postion.x, postion.y) -> count += setmine_element( acse( minefield, postion.x    , postion.y + 1), mine) -> mine;
    
    acse( minefield, postion.x, postion.y) -> count += setmine_element( acse( minefield, postion.x + 1, postion.y - 1), mine) -> mine;
    acse( minefield, postion.x, postion.y) -> count += setmine_element( acse( minefield, postion.x - 1, postion.y - 1), mine) -> mine;
    acse( minefield, postion.x, postion.y) -> count += setmine_element( acse( minefield, postion.x    , postion.y - 1), mine) -> mine;
    
    acse( minefield, postion.x, postion.y) -> count += setmine_element( acse( minefield, postion.x + 1, postion.y    ), mine) -> mine;
    acse( minefield, postion.x, postion.y) -> count += setmine_element( acse( minefield, postion.x - 1, postion.y    ), mine) -> mine;
  }

  if( GW != NULL)
    drawelement( GW, &minefield, postion.x, postion.y);
  
  return acse( minefield, postion.x, postion.y);
}


static inline MS_element *
setmine_element( MS_element *element, MS_mstr *mine){
  if( !element -> set){
    element -> mine = ( ( ( __uint64_t)( ( *mine).noelements - ( *mine).set  ) * ( __uint64_t)( ( *mine).seed = MS_rand( ( *mine).seed))) <
			( ( __uint64_t)( ( *mine).level      - ( *mine).mines) * ( __uint64_t)MS_RAND_MAX));
    
    ++mine -> set;
    
    mine -> mines += element -> mine;

    element -> set = 1;
  }
  
  return element;
}


void
uncov_elements( const MS_field *minefield,
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
setzero( const MS_field *minefield,
	 MS_video  vid){
  
  unsigned long i;
  int x, y;
  
  i = vid.width * vid.height;
  
  while( i--){
    x = ( s32)( i % vid.width) + vid.xdiff;
    y = ( s32)( i / vid.width) + vid.ydiff;

    if( !acse( *minefield, x, y) -> set &&
        !acse( *minefield, x, y) -> flag){
      acse( *minefield, x, y) -> set = 1;
      ++minefield -> mine ->  set;
    }
    
  }
}
