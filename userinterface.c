
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "MS_util.h"
#include "minefield.h"
#include "userinterface.h"
#include "ComandStream.h"

typedef struct{
  s32 x;
  s32 y;
}MS_diff;

typedef struct{
  MS_video mfvid;
  MS_video real;
  MS_video logical;
  u16 global;
  u16 no_resize;
  SDL_Window *window;
  SDL_Texture *target;
  SDL_Renderer *renderer;
  SDL_RendererInfo *renderer_info;
  SDL_Texture *cover;
  SDL_Texture *clear;
  SDL_Texture *flag;
  SDL_Texture *mine;
  SDL_Texture *one;
  SDL_Texture *two;
  SDL_Texture *three;
  SDL_Texture *four;
  SDL_Texture *five;
  SDL_Texture *six;
  SDL_Texture *seven;
  SDL_Texture *eight;
}GraphicWraper;

static inline SDL_Texture *MS_OpenImage( SDL_Renderer *, const char *);
static inline int MS_BlitTile( SDL_Renderer *, SDL_Texture *, int, int, int, int);
static inline void mousebuttondown( MS_root *, SDL_Event);

void *
GW_Init( FreeNode *freenode, MS_root *root){
  GraphicWraper *GW = MS_CreateEmpty( freenode, GraphicWraper);
  
  GW -> global = root -> minefield -> global;
  
  GW -> real = root -> real;
  
  GW -> mfvid.width  = root -> minefield -> width;
  GW -> mfvid.height = root -> minefield -> height;
  
  GW -> real.realwidth  = root -> real.realwidth ? root -> real.realwidth : GW -> mfvid.width  * GW -> real.element_width;
  GW -> real.realheight = root -> real.realheight? root -> real.realheight: GW -> mfvid.height * GW -> real.element_height;
  
  GW -> real.width  = ( GW -> real.realwidth ) / GW -> real.element_width ;
  GW -> real.height = ( GW -> real.realheight) / GW -> real.element_height;
  
  GW -> logical.width  = GW -> real.width;
  GW -> logical.height = GW -> real.height;
  
  assert( !SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER));
  
  GW -> window = SDL_CreateWindow( root -> minefield -> title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				   (int)GW -> real.realwidth, (int)GW -> real.realheight, SDL_WINDOW_HIDDEN);
  
  assert( GW -> window != NULL);
  
  GW -> renderer = SDL_CreateRenderer( GW -> window, -1, SDL_RENDERER_TARGETTEXTURE);
  
  assert( GW -> renderer != NULL);
  
  SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize( GW -> renderer, (int)GW -> real.realwidth, (int)GW -> real.realheight);
  SDL_SetRenderDrawColor( GW -> renderer, 0, 0xff, 0, 0xff);
  
  SDL_SetWindowResizable( GW ->  window, ( SDL_bool)!root -> no_resize);
  
  GW -> renderer_info = MS_CreateEmptyLocal( SDL_RendererInfo);
  
  assert( SDL_GetRendererInfo( GW -> renderer, GW -> renderer_info) == 0);
  
  GW -> logical.element_width  = GW -> renderer_info -> max_texture_width  / GW -> mfvid.width;
  GW -> logical.element_height = GW -> renderer_info -> max_texture_height / GW -> mfvid.height;
  
  GW -> logical.element_width  = GW -> logical.element_width  < 120? GW -> logical.element_width : 120;
  GW -> logical.element_height = GW -> logical.element_height < 120? GW -> logical.element_height: 120;
  
  GW -> mfvid.realwidth  = GW -> logical.element_width  * GW -> mfvid.width;
  GW -> mfvid.realheight = GW -> logical.element_height * GW -> mfvid.height;
  
  GW -> logical.realwidth  = GW -> logical.width  * GW -> logical.element_width;
  GW -> logical.realheight = GW -> logical.height * GW -> logical.element_height;
  
  GW -> target = SDL_CreateTexture( GW -> renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, (int)GW -> mfvid.realwidth, (int)GW -> mfvid.realheight);
  
  {
    // confirm size of GW -> target
    int w, h;
    SDL_QueryTexture( GW -> target, NULL, NULL, &w, &h);
    assert( w == ( int)GW -> mfvid.realwidth);
    assert( h == ( int)GW -> mfvid.realheight);
  }
  
  GW -> clear = MS_OpenImage( GW -> renderer, "nola.png");
  GW -> one   = MS_OpenImage( GW -> renderer, "etta.png");
  GW -> two   = MS_OpenImage( GW -> renderer, "tvaa.png");
  GW -> three = MS_OpenImage( GW -> renderer, "trea.png");
  GW -> four  = MS_OpenImage( GW -> renderer, "fyra.png");
  GW -> five  = MS_OpenImage( GW -> renderer, "fema.png");
  GW -> six   = MS_OpenImage( GW -> renderer, "sexa.png");
  GW -> seven = MS_OpenImage( GW -> renderer, "sjua.png");
  GW -> eight = MS_OpenImage( GW -> renderer, "ataa.png");
  
  GW -> mine  = MS_OpenImage( GW -> renderer, "mina.png");
  GW -> cover = MS_OpenImage( GW -> renderer, "plata.png");
  GW -> flag  = MS_OpenImage( GW -> renderer, "flaga.png");
  
  SDL_EventState( SDL_JOYAXISMOTION, SDL_IGNORE);
  SDL_EventState( SDL_JOYBALLMOTION, SDL_IGNORE);
  SDL_EventState( SDL_JOYBUTTONDOWN, SDL_IGNORE);
  SDL_EventState( SDL_JOYBUTTONUP  , SDL_IGNORE);
  SDL_EventState( SDL_USEREVENT    , SDL_IGNORE);
  SDL_EventState( SDL_SYSWMEVENT   , SDL_IGNORE);
  
  SDL_EventState( SDL_MOUSEWHEEL   , SDL_IGNORE);
  SDL_EventState( SDL_FINGERDOWN   , SDL_IGNORE);
  SDL_EventState( SDL_FINGERUP     , SDL_IGNORE);
  
  SDL_EventState( SDL_MOUSEMOTION  , SDL_IGNORE);
  
  root -> idle = 0;
  
  root -> tutime = getnanosec();
  
  SDL_ShowWindow( GW -> window);
  
  GW -> logical.realydiff = ( s32)( GW -> mfvid.realheight - GW -> logical.realheight);
  GW -> real.realydiff = ( GW -> logical.realydiff * ( s32)GW -> real.realheight) / ( s32)GW -> logical.realheight;
  
  GW -> logical.ydiff = GW -> logical.realydiff / GW -> logical.element_height;
  
  return GW;
}

void
GW_Free( FreeNode *freenode, void *GW){
  if( GW != NULL){
    SDL_Quit();
    
    MS_Free( freenode, ( GraphicWraper *)GW);
  }
}

void
event_dispatch( MS_root *root){
  GraphicWraper *GW   = ( GraphicWraper *)root -> GW;
  SDL_Event event;
  
  assert( GW);
    
  if( SDL_WaitEventTimeout( &event, root -> idle? U64C( 1000): 0)){ // U64C( 75) + ( ( ( u64)MS_rand( MS_rand_seed()) * U64C( 100)) >> 32))){
    switch( expect( event.type, SDL_MOUSEBUTTONDOWN)){
    case SDL_QUIT:
      quit( root);
    case SDL_KEYDOWN:
      {
	switch( event.key.keysym.sym){
	case SDLK_ESCAPE:
	  quit( root);
	case SDLK_F2:
	case 'r':
	  pthread_create( NULL, NULL, setminefield, ( void *)root);
	  
	  break;
	case SDLK_F3:
	case 'e':
	  pthread_create( NULL, NULL, uncov_field, ( void *)root);
	  
	  break;
	case SDLK_LEFT:
	case 'h':
	  GW -> logical.realxdiff = GW -> logical.realxdiff - ( s32)GW -> logical.element_width;
	  if( !GW -> global)
	    GW -> logical.realxdiff = GW -> logical.realxdiff >= 0? GW -> logical.realxdiff: 0;
	  GW -> real.realxdiff = ( GW -> logical.realxdiff * ( s32)GW -> real.realwidth ) / ( s32)GW -> logical.realwidth;
	  
	  GW -> logical.xdiff = GW -> logical.realxdiff / GW -> logical.element_width;
	  {
	    u32 i = GW -> real.height + 2;
	    
	    while( i--){
	      s32 h = GW -> logical.ydiff + i - 1;
	      
	      drawelement( root -> drawque, acse( *root -> minefield, GW -> logical.xdiff - 1, h), GW -> logical.xdiff - 1, h);
	    }
	  }
	  break;
	case SDLK_DOWN:
	case 'j':
	  GW -> logical.realydiff = GW -> logical.realydiff - ( s32)GW -> logical.element_height;
	  if( !GW -> global)
	    GW -> logical.realydiff = GW -> logical.realydiff >= 0? GW -> logical.realydiff: 0;
	  GW -> real.realydiff = ( GW -> logical.realydiff * ( s32)GW -> real.realheight) / ( s32)GW -> logical.realheight;
	  
	  GW -> logical.ydiff = GW -> logical.realydiff / GW -> logical.element_height;
	  {
	    u32 i = GW -> real.width + 2;
	    
	    while( i--){
	      s32 w = GW -> logical.xdiff + i - 1;
	      
	      drawelement( root -> drawque, acse( *root -> minefield, w, GW -> logical.ydiff - 1), w, GW -> logical.ydiff - 1);
	    }
	  }
	  break;
	case SDLK_UP:
	case 'k':
	  GW -> logical.realydiff = GW -> logical.realydiff + ( s32)GW -> logical.element_height;
	  if( !GW -> global)
	    GW -> logical.realydiff = GW -> logical.realydiff <= ( s32)( GW -> mfvid.realheight - GW -> logical.realheight)?
	      GW -> logical.realydiff: ( s32)( GW -> mfvid.realheight - GW -> logical.realheight);
	  GW -> real.realydiff = ( GW -> logical.realydiff * ( s32)GW -> real.realheight) / ( s32)GW -> logical.realheight;
	  
	  GW -> logical.ydiff = GW -> logical.realydiff / GW -> logical.element_height;
	  { 
	    u32 i = GW -> real.width + 2;
	    
	    while( i--){
	      s32 w = GW -> logical.xdiff + i - 1;
	      
	      drawelement( root -> drawque, acse( *root -> minefield, w, GW -> logical.ydiff + GW -> real.height), w, GW -> logical.ydiff + GW -> real.height);
	    }
	  }
	  break;
	case SDLK_RIGHT:
	case 'l':
	  GW -> logical.realxdiff = GW -> logical.realxdiff + ( s32)GW -> logical.element_width;
	  if( !GW -> global)
	    GW -> logical.realxdiff = GW -> logical.realxdiff <= ( s32)( GW -> mfvid.realwidth - GW -> logical.realwidth)?
	      GW -> logical.realxdiff: ( s32)( GW -> mfvid.realwidth - GW -> logical.realwidth);
	  GW -> real.realxdiff = ( GW -> logical.realxdiff * ( s32)GW -> real.realwidth ) / ( s32)GW -> logical.realwidth;
	  
	  GW -> logical.xdiff = GW -> logical.realxdiff / GW -> logical.element_width;
	  { 
	    u32 i = GW -> real.height + 2;
	    
	    while( i--){
	      s32 h = GW -> logical.ydiff + i - 1;
	      
	      drawelement( root -> drawque, acse( *root -> minefield, GW -> logical.xdiff + GW -> real.width , h), GW -> logical.xdiff + GW -> real.width , h);
	    }
	  }
	  break;
	default:
	  break;
	}
	break;
      }
    case SDL_MOUSEBUTTONDOWN:
      mousebuttondown( root, event);
      break;
    default:
      break;
    }
  }
}


static inline void
mousebuttondown( MS_root * root,
		 SDL_Event event){
  
  MS_field *minefield = root -> minefield;
  GraphicWraper *GW   = ( GraphicWraper *)root -> GW;
  
  MS_pos postion;
  
  postion.x = ( s16)( ( ( u16)( (                         event.button.x + GW -> real.realxdiff) / (int)GW -> real.element_width ) + GW -> mfvid.width ) % GW -> mfvid.width );
  postion.y = ( s16)( ( ( u16)( ( GW -> real.realheight - event.button.y + GW -> real.realydiff) / (int)GW -> real.element_height) + GW -> mfvid.height) % GW -> mfvid.height);
  
  switch( event.button.button){
  case SDL_BUTTON_LEFT:
  case SDL_BUTTON_MIDDLE:
    {
      MS_video vid;
      
      vid = ( MS_video){ .xdiff = postion.x, .ydiff = postion.y, .width  = 1, .height = 1};
      
      if( event.button.button == SDL_BUTTON_MIDDLE){
	vid = ( MS_video){ .xdiff = postion.x - 1, .ydiff = postion.y - 1, .width  = 3, .height = 3};
      }
      
      if( minefield -> mine -> uncoverd == 0){
	//let's play "Texas Sharpshooter"
	setzero( minefield, vid);
      }
      
      uncov_elements( root, vid);
      
      uncov( root);
      break;
    }
  case SDL_BUTTON_RIGHT:
    {
      MS_element *element = acse( *minefield, postion.x, postion.y);
      
      if( element -> flag){
	element -> flag = 0;
	--minefield -> mine -> flaged;
      }else if( element -> cover){
	element -> flag = 1;
	++minefield -> mine -> flaged;
      }
      
      drawelement( root -> drawque, element, postion.x, postion.y);
    }
  default:
    break;
  }
}

void
draw( MS_root *root){
  GraphicWraper *GW = ( GraphicWraper *)root -> GW;
  DrawComand *dc = NULL;
  
  SDL_SetRenderTarget( GW -> renderer, GW -> target);
  
  if( root -> tutime + 20000000 > getnanosec()){
    root -> tutime = getnanosec();
  }
  
  root -> idle = 1;
    
  while( ( dc = ( DrawComand *)CS_Releas( root -> drawque)) != NULL){ 
    SDL_Texture *tile = NULL;
    MS_element *element = &( dc -> element);
    s16 w = mol_( ( dc -> pos.x + root -> minefield -> width ), root -> minefield -> width , root -> minefield -> width_divobj);
    s16 h = mol_( ( dc -> pos.y + root -> minefield -> height), root -> minefield -> height, root -> minefield -> height_divobj);
    
    root -> idle -= !!root -> idle;
    
    if( w < GW -> logical.xdiff - 1 ||
	w > GW -> logical.xdiff + ( int)GW -> real.width ||
	h < GW -> logical.ydiff - 1 ||
	h > GW -> logical.ydiff + ( int)GW -> real.height){
      goto finish;
    }
    
    if( element -> flag){
      tile =  GW -> flag;
    }else if( element -> cover){
      tile =  GW -> cover;
    }else if( element -> mine){
      tile =  GW -> mine;
    }else switch( element -> count){
    case 0: tile =  GW -> clear; break;
    case 1: tile =  GW -> one; break;
    case 2: tile =  GW -> two; break;
    case 3: tile =  GW -> three; break;
    case 4: tile =  GW -> four; break;
    case 5: tile =  GW -> five; break;
    case 6: tile =  GW -> six; break;
    case 7: tile =  GW -> seven; break;
    case 8: tile =  GW -> eight; break;
    default:
      goto finish;
      break;
    }
    
    assert( tile != NULL);
    
    MS_BlitTile( GW -> renderer, tile,
		 (int)w * ( int)GW -> logical.element_width ,
		 (int)h * ( int)GW -> logical.element_height,
		 (int)GW -> logical.element_width,
		 (int)GW -> logical.element_height);
    
  finish:
    CS_Finish( root -> drawque, dc);
    
    if( getnanosec() > root -> tutime + 15000000){
      break;
    }
  }
  
  if( dc != NULL){
    root -> idle = 0;
  }
  
  root -> tutime = getnanosec();
  
  SDL_SetRenderTarget( GW -> renderer, NULL);
  
  SDL_RenderCopyEx( GW -> renderer, GW -> target, &( SDL_Rect){ .x = GW -> logical.realxdiff, .y = GW -> logical.realydiff, .w = (int)GW -> logical.realwidth, .h = (int)GW -> logical.realheight},
		    &( SDL_Rect){ .x = 0, .y = 0, .w = (int)GW -> real.realwidth, .h = (int)GW -> real.realheight}, 0, NULL, SDL_FLIP_VERTICAL);
  
  SDL_RenderPresent( GW -> renderer);
}

void
drawelement( ComandStream *drawque, const MS_element *element, s16 w, s16 h){
  DrawComand *dc = ( DrawComand *)CS_Fetch( drawque);
  
  dc -> pos.x = w;
  dc -> pos.y = h;
  dc -> element = *element;
  
  CS_Push( drawque, dc);
}

static inline SDL_Texture *
MS_OpenImage( SDL_Renderer *render, const char *str){
  SDL_Texture *tex = NULL;
  SDL_Surface *img = NULL;
  assert( render != NULL);
  assert(    str != NULL);
  img = IMG_Load( str);
  assert( img != NULL);
  tex = SDL_CreateTextureFromSurface( render, img);
  assert( tex != NULL);
  SDL_free( img);
  return tex;
}

static inline int
MS_BlitTile( SDL_Renderer *renderer, SDL_Texture *tile, int dx, int dy, int w, int h){
  assert( renderer != NULL);
  assert(     tile != NULL);
  return SDL_RenderCopyEx( renderer, tile, NULL, &( SDL_Rect){ .x = dx, .y = dy, .w = w, .h = h}, 0, NULL, SDL_FLIP_VERTICAL);
}


