
#include <stdlib.h>
#include <string.h>

#include "MS_util.h"
#include "ComandStream.h"
#include "minefield.h"
#include "userinterface.h"

static inline MS_element *uncover_element( const MS_field, void *, MS_pos, MS_mstr *);
static inline MS_element *setmine_element( MS_field, u32, MS_mstr *);
static inline void addelement( const MS_field *, s16, s16);
void uncov_thread_server( void *);

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
  MS_element *data;
  u32 truewidth  = proto -> width  + !proto -> global;
  u32 trueheight = proto -> height + !proto -> global;
  
  mine = MS_Create( freenode, MS_mstr,
		    .level = proto -> level,
		    .noelements = proto -> width * proto -> height);
  
  data = MS_CreateArray( freenode, truewidth * trueheight, MS_element, .count = 14);
  
  uncovque = CS_CreateStream( freenode, MS_pos);
  
  minefield = MS_Create( freenode, MS_field,
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
  
  pthread_mutex_init( &minefield -> mutex_field, NULL);
  
  return minefield;
}

void
MF_FreeField(  FreeNode *freenode, const MS_field *pminefield){
  const MS_field *minefield = MS_CreateLocalFromLocal( MS_field, pminefield);
  if( minefield != NULL){
    MS_Free( freenode, pminefield);
    
    CS_Free( freenode, minefield -> uncovque);
    
    MS_FreeArray( freenode, minefield -> data, minefield -> width * minefield -> height);
    
    MS_Free( freenode, minefield -> mine);
  }
}


void
setminefield_async( void *root){
  pthread_create( NULL, NULL, setminefield, ( void *)root);
}


void
setminefield( void *root){
  const MS_field *minefield = ( ( const MS_root *)root) -> minefield;
  void *GW = ( ( const MS_root *)root) -> GW;
  u32 i;
  
  i = minefield -> width * minefield -> subheight;
  
  pthread_mutex_lock( &minefield -> mutex_field);
  
  while( i--){
    if( ( i < 1864136 ? mol_( i, minefield -> width, minefield -> width_divobj) : ( i % minefield -> width)) < minefield -> subwidth){
      if( ( minefield -> data + i) -> cover == 0 ||
	  ( minefield -> data + i) -> flag == 1){
	
	if( GW  != NULL)
	  drawelement( GW, &( MS_element){ .cover = 1}, ( s32)mol_( i, minefield -> width, minefield -> width_divobj), ( s32)div_( i, minefield -> width, minefield -> width_divobj));
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
  
  pthread_mutex_unlock( &minefield -> mutex_field);
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
uncov_unsafe( void *root){
  const MS_field *minefield = ( ( const MS_root *)root) -> minefield;
  void *GW = ( ( const MS_root *)root) -> GW;
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


void
uncov_async( void *root){
  pthread_create( NULL, NULL, uncov, ( void *)root);
}

void
uncov_thread_server( void *root){
  const MS_field *minefield = ( ( const MS_root *)root) -> minefield;
  u32 numthreads = 8;
  u32 iter = numthreads;
  pthread_t threads[ numthreads];
  pthread_attr_t attr;
  
  pthread_attr_init( &attr);
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE);
  
  pthread_mutex_lock( &minefield -> mutex_field);
  
  while( iter--){
    pthread_create( &threads[ iter], &attr, uncov_unsafe, ( void *)root);
  }
  
  pthread_attr_destroy( &attr);
  
  iter = numthreads;
  
  while( iter--){
    pthread_join( threads[ iter], NULL);
  }
  
  pthread_mutex_unlock( &minefield -> mutex_field);
}


void
uncov( void *root){
  const MS_field *minefield = ( ( const MS_root *)root) -> minefield;
  pthread_mutex_lock( &minefield -> mutex_field);
  uncov_unsafe( root);
  pthread_mutex_unlock( &minefield -> mutex_field);
}


static inline MS_element *
uncover_element( const MS_field minefield, void *GW, MS_pos postion, MS_mstr *mine){
  
  if likely( acse( minefield, postion.x, postion.y) -> count == 15){
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
  }
  
  if( GW != NULL)
    drawelement( GW, acse( minefield, postion.x, postion.y), postion.x, postion.y);
  
  return acse( minefield, postion.x, postion.y);
}


static inline MS_element *
setmine_element( MS_field minefield, u32 index, MS_mstr *mine){
  MS_element *element = minefield.data + index;

  if( !element -> set && index < mine -> noelements + minefield.subheight && index % minefield.width != minefield.subwidth){
    
    //if( !minefield.global){
    //  element -> mine = MS_rand_range( mine -> seed, index - index / minefield.width, mine -> noelements) < mine -> level;
    //}else{
    //  element -> mine = MS_rand_range( mine -> seed, index, mine -> noelements) < mine -> level;
    //}
    
    element -> mine = ( ( ( u64)( mine -> seed = MS_rand( mine -> seed)) * ( mine -> noelements - mine -> set)) >> 32) < ( mine -> level - mine -> mines);
    
    mine -> set += 1;
    
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
  assert( minefield != NULL);
  
  i = vid.width * vid.height;

  pthread_mutex_lock( &minefield -> mutex_field);
  while( i--){
    x = ( s32)( i % vid.width) + vid.xdiff;
    y = ( s32)( i / vid.width) + vid.ydiff;
    
    addelement( minefield, x, y);
  }
  pthread_mutex_unlock( &minefield -> mutex_field);
}


void
setzero( const MS_field *minefield,
	 MS_video  vid){
  unsigned long i;
  int x, y;
  assert( minefield != NULL);
  
  pthread_mutex_lock( &minefield -> mutex_field);
  
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
  
  pthread_mutex_unlock( &minefield -> mutex_field);
}
