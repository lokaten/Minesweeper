
#include <stdlib.h>
#include <string.h>

#include "MS_util.h"
#include "ComandStream.h"
#include "minefield.h"
#include "userinterface.h"

static inline MS_element *uncover_element( MS_field *, MS_pos, MS_mstr *);
static inline MS_element *setmine_element( MS_field *, u32, MS_mstr *);
static inline void addelement( MS_field *, s32, s32);

static inline u32
acse_u( const MS_field *field, int x, int y){
  return y * field -> width + x;
}

static inline MS_element *
acse_f( const MS_field *field, int x, int y){
  return field -> data + acse_u( field, x, y);
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
		    .noelements = proto -> width * proto -> height,
		    .seed = MS_rand_seed());
  
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
    
    pthread_mutex_lock( &minefield -> mutex_field);
    
    MS_Free( freenode, pminefield);
    
    CS_Free( freenode, minefield -> uncovque);
    
    MS_FreeArray( freenode, minefield -> data, minefield -> width * minefield -> height);
    
    MS_Free( freenode, minefield -> mine);
  }
}


void *
setminefield( void){
  MS_field *minefield = ( ( MS_root *)root) -> minefield;
  u32 w, h;
  
  pthread_mutex_lock( &minefield -> mutex_field);
  
  if( minefield -> data == NULL){
    minefield -> data = MS_CreateArray( ( ( MS_root *) root) -> freenode, minefield -> width * minefield -> height, MS_element, .set = 1);
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
  
  MS_print( ( ( MS_root *)root) -> mss -> out, "\r\t\t\t\t\tseed: %x   ", minefield -> mine -> seed);
  
  h = -1;
  
  while( ++h < minefield -> height){
    w = minefield -> width;
    
    while( w--){
      u8 cover = minefield -> global || ( w > 0 && w < minefield -> subwidth + 1 && h > 0 && h < minefield -> subheight + 1);
      
      *acse_f( minefield, w, h) = ( MS_element){ .count = 15, .cover = cover, .set = !cover};
    }
  }
  
  minefield -> mine -> flaged = 0;
  
  minefield -> mine -> uncoverd = 0;
  
  minefield -> mine -> hit = 0;
  
  pthread_mutex_unlock( &minefield -> mutex_field);
  
  ( ( MS_root *) root) -> idle = 0;
  
  return NULL;
}


static inline void
addelement( MS_field *minefield, s32 x, s32 y){
  MS_pos lpos;
  
  assert( minefield -> data != NULL);
  
  lpos.x = x;
  lpos.y = y;
  
  if( lpos.x < 0 || lpos.y < 0 || lpos.x > ( s32)minefield -> width || lpos.y > ( s32)minefield -> height){
    lpos.x = (s32)mol_( (u32)( x + (s32)minefield -> width ), minefield -> width , minefield -> width_divobj );
    lpos.y = (s32)mol_( (u32)( y + (s32)minefield -> height), minefield -> height, minefield -> height_divobj);
  }
  
  if( acse_f( minefield, lpos.x, lpos.y) -> cover &&
      !acse_f( minefield, lpos.x, lpos.y) -> flag){
    MS_pos *pos = ( MS_pos *)CS_Fetch( minefield -> uncovque);
    *pos = lpos;
    if( acse_f( minefield, lpos.x, lpos.y) -> cover){
      acse_f( minefield, lpos.x, lpos.y) -> cover = 0;
      minefield -> mine -> uncoverd += 1;
    }
    CS_Push( minefield -> uncovque, pos);
  }
}


void *
uncov_workthread( void *v_ptr){
  MS_field *minefield = root -> minefield;
  MS_pos *pos;
  address com;
  MS_element *element;
  assert( minefield != NULL);
  
  assert( v_ptr == NULL);
  
  while( TRUE){
    struct CS_Block block = { 0};
    
    com = ( address)CS_WaitReleas( minefield -> uncovque);
    
    assert( ( MS_pos *)com != NULL);
    
    block = CS_BlockReleas( minefield -> uncovque);
    
    pos = MS_CreateLocalFromLocal( MS_pos, ( MS_pos *)com);
    
    if( !block.blk_ptr){
      CS_Finish( minefield -> uncovque, ( void *)com);
    }
    
    if( minefield -> mine -> set < minefield -> mine -> noelements){
      pthread_mutex_lock( &minefield -> mutex_field);
      if( minefield -> mine -> set < minefield -> mine -> noelements){
	
	setmine_elements( minefield, ( MS_video){ .width = minefield -> width, .height = minefield -> height});
      }
      pthread_mutex_unlock( &minefield -> mutex_field);
    }
    
    do{
      
      element = uncover_element( minefield, *pos, minefield -> mine);
      
      if( root -> drawque != NULL){
	drawelement( root -> drawque, element, pos -> x, pos -> y);
      }
      
      if likely( element -> count == 0){
	addelement( minefield, pos -> x - 1, pos -> y + 1);
	addelement( minefield, pos -> x    , pos -> y + 1);
	addelement( minefield, pos -> x + 1, pos -> y + 1);
	
	addelement( minefield, pos -> x - 1, pos -> y - 1);
	addelement( minefield, pos -> x    , pos -> y - 1);
	addelement( minefield, pos -> x + 1, pos -> y - 1);
	
	addelement( minefield, pos -> x - 1, pos -> y    );
	addelement( minefield, pos -> x + 1, pos -> y    );
      }
      
      com += block.size;
      
      pos = ( MS_pos *)com;
      
    }while( block.blk_ptr && com != block.blk_ptr + block.blk_size);
    
    if( root -> drawque != NULL){
      CS_Signal( root -> drawque);
    }
    
    if( block.blk_ptr){
      MS_FreeFromSize( root -> freenode, block.blk_ptr, true_blk_size);
    }
  }
  
  return NULL;
}

void *
uncov( void){
  CS_Signal( ( ( MS_root *)root) -> minefield -> uncovque);
  
  ( ( MS_root *) root) -> idle = 0;
  
  return NULL;
}


static inline MS_element *
uncover_element( MS_field *minefield, MS_pos postion, MS_mstr *mine){
  MS_pos *pos = &postion;
  
  mine -> hit += acse_f( minefield, pos -> x, pos -> y) -> mine;
  
  acse_f( minefield, postion.x, postion.y) -> count = 0;
  
  acse_f( minefield, postion.x, postion.y) -> count += acse( *minefield, postion.x - 1, postion.y + 1) -> mine;
  acse_f( minefield, postion.x, postion.y) -> count += acse( *minefield, postion.x    , postion.y + 1) -> mine;
  acse_f( minefield, postion.x, postion.y) -> count += acse( *minefield, postion.x + 1, postion.y + 1) -> mine;
  
  acse_f( minefield, postion.x, postion.y) -> count += acse( *minefield, postion.x - 1, postion.y - 1) -> mine;
  acse_f( minefield, postion.x, postion.y) -> count += acse( *minefield, postion.x    , postion.y - 1) -> mine;
  acse_f( minefield, postion.x, postion.y) -> count += acse( *minefield, postion.x + 1, postion.y - 1) -> mine;
  
  acse_f( minefield, postion.x, postion.y) -> count += acse( *minefield, postion.x - 1, postion.y    ) -> mine;
  acse_f( minefield, postion.x, postion.y) -> count += acse( *minefield, postion.x + 1, postion.y    ) -> mine;
  
  return acse_f( minefield, postion.x, postion.y);
}


static inline MS_element *
setmine_element( MS_field *minefield, u32 index, MS_mstr *mine){
  MS_element *element = minefield -> data + index;
  
  // call MS_rand() for every cell to keep an uniqe seed per cell.
  // this means that that when we sellect which cells becomes mines,
  // the decision will only depend on thier postion and the orginal seed set in setminefield.
  mine -> seed = MS_rand( mine -> seed);
  
  if( !element -> set){
    element -> mine = ( ( ( u64)( mine -> seed) * ( mine -> noelements - mine -> set)) >> 32) < ( mine -> level - mine -> mines);
    
    mine -> set += 1;
    
    mine -> mines += element -> mine;
    
    element -> set = 1;
  }
  
  return element;
}

void
setmine_elements( MS_field *minefield,
		  MS_video vid){
  s32 x, y;
  MS_pos postion;
  
  assert( minefield -> data != NULL);
  
  vid.xdiff = vid.xdiff < 0 ? 0: vid.xdiff;
  vid.ydiff = vid.ydiff < 0 ? 0: vid.ydiff;
  
  vid.width  = vid.xdiff + vid.width  > minefield -> width  ? minefield -> width  - vid.xdiff: vid.width;
  vid.height = vid.ydiff + vid.height > minefield -> height ? minefield -> height - vid.ydiff: vid.height;
  
  y = vid.height;
  
  pthread_mutex_lock( &minefield -> mutex_field);
  
  while( y--){
    x = vid.width;
    
    while( x--){
      postion.x = vid.xdiff + x;
      postion.y = vid.ydiff + y;
      
      setmine_element( minefield, acse_u( minefield, postion.x, postion.y), minefield -> mine);
    }
  }
  
  pthread_mutex_unlock( &minefield -> mutex_field);
}


void
uncov_elements( MS_video vid){
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


void *
uncov_field( void *v_ptr){
  MS_video vid;
  assert( root != NULL);
  
  assert( v_ptr == NULL);
  
  vid  = ( MS_video){ .xdiff = 0, .ydiff = 0,
		      .width  = ( ( MS_root *) root) -> minefield -> width,
		      .height = ( ( MS_root *) root) -> minefield -> height};
  
  uncov_elements( vid);
  
  uncov();
  
  return NULL;
}


void
setzero( MS_video  vid){
  MS_field *minefield = ( ( MS_root *) root) -> minefield;
  unsigned long i;
  int x, y;
  assert( minefield != NULL);
  
  i = vid.width * vid.height;
  
  pthread_mutex_lock( &minefield -> mutex_field);
  
  if( minefield -> data == NULL){
    minefield -> data = MS_CreateArray( ( ( MS_root *) root) -> freenode, minefield -> width * minefield -> height, MS_element, .set = 1);
  }
  
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
