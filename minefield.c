
#include <stdlib.h>
#include <string.h>

#include "MS_util.h"
#include "ComandStream.h"
#include "minefield.h"
#include "userinterface.h"

static inline MS_element *uncover_element( MS_field, ComandStream *, MS_pos, MS_mstr *);
static inline MS_element *setmine_element( MS_field, u32, MS_mstr *);
static inline void addelement( MS_field *, s16, s16);

static inline u32
acse_u( const MS_field field, int x, int y){
  return ( u32)( mol_( (u32)( x + (int)field.width ), field.width , field.width_divobj ) +
		 mol_( (u32)( y + (int)field.height), field.height, field.height_divobj) * field.width);
}

MS_field *
MF_CreateFieldFromLocal( FreeNode *freenode, const MS_field *proto){
  MS_field *minefield;
  MS_mstr *mine;
  ComandStream *uncovque;
  u32 truewidth  = proto -> width  + !proto -> global * 2;
  u32 trueheight = proto -> height + !proto -> global * 2;
  
  mine = MS_Create( freenode, MS_mstr,
		    .level = proto -> level,
		    .noelements = proto -> width * proto -> height);
  
  uncovque = CS_CreateStream( freenode, MS_pos);
  
  minefield = MS_Create( freenode, MS_field,
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
  
  minefield -> data = NULL;
  
  pthread_mutexattr_init( &minefield -> attr_mutex);
  pthread_mutexattr_settype( &minefield -> attr_mutex, PTHREAD_MUTEX_RECURSIVE);
  
  pthread_mutex_init( &minefield -> mutex_field, &minefield -> attr_mutex);
  
  return minefield;
}

void
MF_FreeField(  FreeNode *freenode, const MS_field *pminefield){
  MS_field *minefield = MS_CreateLocalFromLocal( MS_field, pminefield);
  if( minefield != NULL){
    
    MS_Free( freenode, pminefield);
    
    CS_Free( freenode, minefield -> uncovque);
    
    MS_FreeArray( freenode, minefield -> data, minefield -> width * minefield -> height);
    
    MS_Free( freenode, minefield -> mine);
  }
}


void *
setminefield( void *root){
  MS_field *minefield = ( ( MS_root *)root) -> minefield;
  u32 i;
  
  i = minefield -> width * ( minefield -> subheight + 1);
  
  if( minefield -> data == NULL){
    minefield -> data = MS_CreateArray( ( ( MS_root *) root) -> freenode, minefield -> width * minefield -> height, MS_element, .set = 1);
  }
  
  pthread_mutex_lock( &minefield -> mutex_field);
  
  minefield -> mine -> flaged = 0;
  
  minefield -> mine -> uncoverd = 0;
  
  minefield -> mine -> set = 0;
  
  minefield -> mine -> mines = 0;
  
  minefield -> mine -> hit = 0;
  
  minefield -> mine -> seed = MS_rand_seed();
  
  if( minefield -> reseed){
    minefield -> mine -> seed = minefield -> reseed;
  }
  
  while( i-- > minefield -> width){
    u32 w = i < 1864136 ? mol_( i, minefield -> width, minefield -> width_divobj) : ( i % minefield -> width);
    if( w > 0 && w < minefield -> subwidth + 1){
      if( !( MS_element *)( uintptr_t)( minefield -> data + i) -> cover ||
	   ( MS_element *)( uintptr_t)( minefield -> data + i) -> flag){
	drawelement( ( ( MS_root *)root) -> drawque, &( MS_element){ .cover = 1}, ( s32)w, ( s32)div_( i, minefield -> width, minefield -> width_divobj));
      }
      
      *( minefield -> data + i) = ( MS_element){ .count = 15, .cover = 1};
    }
  }
  
  i = minefield -> width * minefield -> height;
  
  while( i--){
    u32 w = i < 1864136 ? mol_( i, minefield -> width, minefield -> width_divobj) : ( i % minefield -> width);
    u32 h = div_( i, minefield -> width, minefield -> width_divobj);
    
    if( !w || !h || w == minefield -> width - 1 || h == minefield -> height - 1){
      *( minefield -> data + i) = ( MS_element){ .count = 15, .set = 1};
      
      uncover_element( *minefield, ( ( MS_root *)root) -> drawque, ( MS_pos){ .x = w, .y = h}, minefield -> mine);
    }
  }
  
  minefield -> mine -> flaged = 0;
  
  minefield -> mine -> uncoverd = 0;
  
  minefield -> mine -> hit = 0;
  
  pthread_mutex_unlock( &minefield -> mutex_field);
  
  return NULL;
}


static inline void
addelement( MS_field *minefield, s16 x, s16 y){
  
  if( acse( *minefield, x, y) -> cover       &&
      acse( *minefield, x, y) -> count == 15 &&
      !acse( *minefield, x, y) -> flag){
    MS_pos *pos = ( MS_pos *)CS_Fetch( minefield -> uncovque);
    if( pos != NULL){
      pos -> x = (s32)mol_( (u32)( x + (s32)minefield -> width ), minefield -> width , minefield -> width_divobj );
      pos -> y = (s32)mol_( (u32)( y + (s32)minefield -> height), minefield -> height, minefield -> height_divobj);
      acse( *minefield, pos -> x, pos -> y) -> cover = 0;
      CS_Push( minefield -> uncovque, pos);
    }
  }
}


void *
uncov_workthread( void *root){
  MS_field *minefield = ( ( MS_root *)root) -> minefield;
  MS_pos *pos;
  MS_element *element;
  assert( minefield != NULL);
  
  while( TRUE){
    
    pos = ( MS_pos *)CS_WaitReleas( minefield -> uncovque);
    
    assert( pos != NULL);
    
    element = uncover_element( *minefield, ( ( MS_root *)root) -> drawque, *pos, minefield -> mine);
    
    CS_Finish( minefield -> uncovque, pos);
    
    if likely( element -> count == 0){
      addelement( minefield, pos -> x + 1, pos -> y + 1);
      addelement( minefield, pos -> x - 1, pos -> y + 1);
      addelement( minefield, pos -> x    , pos -> y + 1);
      
      addelement( minefield, pos -> x + 1, pos -> y - 1);
      addelement( minefield, pos -> x - 1, pos -> y - 1);
      addelement( minefield, pos -> x    , pos -> y - 1);
      
      addelement( minefield, pos -> x + 1, pos -> y    );
      addelement( minefield, pos -> x - 1, pos -> y    );
    }
  }
  
  return NULL;
}

void *
uncov( void *root){
  CS_Signal( ( ( MS_root *)root) -> minefield -> uncovque);
  
  return NULL;
}


static inline MS_element *
uncover_element( MS_field minefield, ComandStream *drawque, MS_pos postion, MS_mstr *mine){
  
  if likely( acse( minefield, postion.x, postion.y) -> count == 15){

    pthread_mutex_lock( &minefield.mutex_field);
    
    mine -> uncoverd += 1;
    
    acse( minefield, postion.x, postion.y) -> count = 0;
    
    mine -> hit += setmine_element( minefield, acse_u( minefield, postion.x, postion.y), mine) -> mine;
    
    acse( minefield, postion.x, postion.y) -> count += setmine_element( minefield, acse_u( minefield, postion.x + 1, postion.y + 1), mine) -> mine;
    acse( minefield, postion.x, postion.y) -> count += setmine_element( minefield, acse_u( minefield, postion.x - 1, postion.y + 1), mine) -> mine;
    acse( minefield, postion.x, postion.y) -> count += setmine_element( minefield, acse_u( minefield, postion.x    , postion.y + 1), mine) -> mine;
    
    acse( minefield, postion.x, postion.y) -> count += setmine_element( minefield, acse_u( minefield, postion.x + 1, postion.y - 1), mine) -> mine;
    acse( minefield, postion.x, postion.y) -> count += setmine_element( minefield, acse_u( minefield, postion.x - 1, postion.y - 1), mine) -> mine;
    acse( minefield, postion.x, postion.y) -> count += setmine_element( minefield, acse_u( minefield, postion.x    , postion.y - 1), mine) -> mine;
    
    acse( minefield, postion.x, postion.y) -> count += setmine_element( minefield, acse_u( minefield, postion.x + 1, postion.y    ), mine) -> mine;
    acse( minefield, postion.x, postion.y) -> count += setmine_element( minefield, acse_u( minefield, postion.x - 1, postion.y    ), mine) -> mine;
    
    drawelement( drawque, acse( minefield, postion.x, postion.y), postion.x, postion.y);
    
    pthread_mutex_unlock( &minefield.mutex_field);
  }
  
  return acse( minefield, postion.x, postion.y);
}


static inline MS_element *
setmine_element( MS_field minefield, u32 index, MS_mstr *mine){
  MS_element *element = minefield.data + index;
  
  if( !element -> set){
    element -> mine = ( ( ( u64)( mine -> seed = MS_rand( mine -> seed)) * ( mine -> noelements - mine -> set)) >> 32) < ( mine -> level - mine -> mines);
    
    mine -> set += 1;
    
    mine -> mines += element -> mine;
    
    element -> set = 1;
  }
  
  return element;
}



void
uncov_elements( void *root,
		MS_video  vid){
  unsigned long i;
  MS_pos postion;
  assert( root != NULL);
  
  i = vid.width * vid.height;
    
  while( i--){
    postion.x = ( s32)( i % vid.width) + vid.xdiff;
    postion.y = ( s32)( i / vid.width) + vid.ydiff;

    addelement( ( ( MS_root *) root) -> minefield, postion.x, postion.y);
  }
}


void
setzero( MS_field *minefield,
	 MS_video  vid){
  unsigned long i;
  int x, y;
  assert( minefield != NULL);
  
  i = vid.width * vid.height;
  
  pthread_mutex_lock( &minefield -> mutex_field);
  
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

  pthread_mutex_unlock( &minefield -> mutex_field);
}
