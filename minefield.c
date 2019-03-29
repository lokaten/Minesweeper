
#include <stdlib.h>
#include <string.h>

#include "MS_util.h"
#include "ComandStream.h"
#include "minefield.h"

static inline MS_element *uncover_element( const MS_field, void *, MS_pos, MS_mstr *);
static inline MS_element *setmine_element( MS_element *, MS_mstr *);
static inline void addelement( const MS_field *, s16, s16);


MS_field *
MF_CreateFieldFromLocal( FreeNode *freenode, const MS_field *proto){
  MS_mstr *mine;
  ComandStream *uncovque;
  MS_element *data;
  u32 truewidth  = proto -> width  + !proto -> global;
  u32 trueheight = proto -> height + !proto -> global;
  
  mine = MS_Create( freenode, MS_mstr,
		    .level = proto -> level,
		    .noelements = proto -> width * proto -> height);
  
  uncovque = CS_CreateStream( freenode, MS_pos);
  
  data = MS_CreateArray( freenode, truewidth * trueheight, MS_element, .count = 14);
  
  return MS_Create( freenode, MS_field,
		    .data = data,
		    .title = proto -> title,
		    .uncovque = uncovque,
		    .mine = mine,
		    .width = truewidth,
		    .width_divobj = gen_divobj( truewidth),
		    .height = trueheight,
		    .height_divobj  = gen_divobj( trueheight),
		    .subwidth  = proto -> width,
		    .subheight = proto -> height,
		    .level = proto -> level,
		    .global = proto -> global,
		    .reseed = proto -> reseed);
}

void
MF_FreeField(  FreeNode *freenode, const MS_field *pminefield){
  const MS_field *minefield = pminefield;
  if( minefield != NULL){
    MS_Free( freenode, minefield, MS_field);
    
    MS_FreeArray( freenode, minefield -> data, minefield -> width * minefield -> height, MS_element);

    CS_Free( freenode, minefield -> uncovque);
    MS_Free( freenode, minefield -> mine, MS_mstr);
  }
}


void
setminefield( const MS_field  *minefield,
	      void *GW){
  u32 i;
  
  i = minefield -> width * minefield -> subheight;
  
  while( i--){
    if( mol_( i, minefield -> width, minefield -> width_divobj) < minefield -> subwidth){
      if( ( minefield -> data + i) -> cover == 0 ||
	  ( minefield -> data + i) -> flag == 1){
	
	if( GW != NULL)
	  drawelement( GW, &( MS_element){ .cover = 1}, mol_(i, minefield -> width, minefield -> width_divobj), div_( i, minefield -> width, minefield -> width_divobj));
      }
      
      *( minefield -> data + i) = ( MS_element){ .count = 15, .cover = 1};
    }
  }
  
  minefield -> mine -> flaged = 0;
  
  minefield -> mine -> uncoverd = 0;
  
  minefield -> mine -> set = 0;
  
  minefield -> mine -> mines = 0;
  
  minefield -> mine -> hit = 0;
  
  minefield -> mine -> seed = MS_rand_seed();
  
  if( minefield -> reseed){
    minefield -> mine -> seed = minefield -> reseed;
  }
}


static inline void
addelement( const MS_field *minefield, s16 x, s16 y){
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
    drawelement( GW, acse( minefield, postion.x, postion.y), postion.x, postion.y);
  
  return acse( minefield, postion.x, postion.y);
}


static inline MS_element *
setmine_element( MS_element *element, MS_mstr *mine){
  assert( mine != NULL);
  assert( element != NULL);
  
  if( !element -> set && element -> count != 14){
    element -> mine = ( ( ( __uint64_t)( mine -> noelements - mine -> set  ) * ( __uint64_t)( mine -> seed = MS_rand( mine -> seed))) <
			( ( __uint64_t)( mine -> level      - mine -> mines) * ( __uint64_t)MS_RAND_MAX));
    
    ++mine -> set;
    
    mine -> mines += element -> mine;
    
    assert( mine -> mines <= mine -> level);
    
    element -> set = 1;
  }
  
  return element;
}


void
uncov_elements( const MS_field *minefield,
		MS_video  vid){
  unsigned long i;
  int x, y;
  assert( minefield != NULL);
  
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
  assert( minefield != NULL);
  
  i = vid.width * vid.height;
  
  while( i--){
    x = ( s32)( i % vid.width) + vid.xdiff;
    y = ( s32)( i / vid.width) + vid.ydiff;
    
    if( !acse( *minefield, x, y) -> set &&
        !acse( *minefield, x, y) -> flag &&
	acse(  *minefield, x, y) -> count == 15){
      acse( *minefield, x, y) -> set = 1;
      ++minefield -> mine ->  set;
    }
    
  }
}
