
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

static inline void drawelement_unqued( GraphicWraper *, const MS_element *, s16, s16);
static inline SDL_Texture *MS_OpenImage( SDL_Renderer *, const char *);
static inline int MS_BlitTarget( SDL_Renderer *, SDL_Texture *, s32, s32, s32, s32, int, int, int, int);
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
  
  GW -> real.width  = GW -> real.realwidth  / GW -> real.element_width ;
  GW -> real.height = GW -> real.realheight / GW -> real.element_height;
  
  assert( !SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER));
  
  GW -> window = SDL_CreateWindow( root -> minefield -> title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				   (int)GW -> real.realwidth, (int)GW -> real.realheight, SDL_WINDOW_HIDDEN);
  
  assert( GW -> window != NULL);
  
  GW -> renderer = SDL_CreateRenderer( GW -> window, -1, SDL_RENDERER_TARGETTEXTURE);
  
  assert( GW -> renderer != NULL);
  
  SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_SetRenderDrawColor( GW -> renderer, 0, 0xff, 0, 0xff);
  
  SDL_SetWindowResizable( GW ->  window, ( SDL_bool)!root -> no_resize);
  
  GW -> renderer_info = MS_CreateEmptyLocal( SDL_RendererInfo);
  
  assert( SDL_GetRendererInfo( GW -> renderer, GW -> renderer_info) == 0);
  
  GW -> logical.element_width  = GW -> renderer_info -> max_texture_width  / GW -> mfvid.width;
  GW -> logical.element_height = GW -> renderer_info -> max_texture_height / GW -> mfvid.height;
  
  GW -> logical.element_width  = GW -> logical.element_width  < 120? GW -> logical.element_width : GW -> real.element_width;
  GW -> logical.element_height = GW -> logical.element_height < 120? GW -> logical.element_height: GW -> real.element_height;
  
  GW -> mfvid.realwidth  = GW -> logical.element_width  * GW -> mfvid.width;
  GW -> mfvid.realheight = GW -> logical.element_height * GW -> mfvid.height;
  
  GW -> logical.realwidth  = GW -> real.realwidth  * GW -> logical.element_width  / GW -> real.element_width;
  GW -> logical.realheight = GW -> real.realheight * GW -> logical.element_height / GW -> real.element_height;
  
  GW -> logical.width  = GW -> logical.realwidth  / GW -> logical.element_width  + 1;
  GW -> logical.height = GW -> logical.realheight / GW -> logical.element_height + 1;
  
  GW -> target = SDL_CreateTexture( GW -> renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, GW -> mfvid.realwidth, GW -> mfvid.realheight);
  
  SDL_SetRenderTarget( GW -> renderer, GW -> target);
  
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
  
  SDL_ShowWindow( GW -> window);
  
  {
    s32 h = GW -> real.height + 1;
    
    while( h--){
      s32 w = GW -> real.width + 1;
      
      while( w--){
	u8 cover = GW -> global || !( w + ( s32)GW -> mfvid.xdiff == 0 || h + ( s32)GW -> logical.ydiff == 0 || w + GW -> logical.xdiff == ( s32)GW -> mfvid.width || h + GW -> logical.ydiff == ( s32)GW -> mfvid.height);
	
	drawelement_unqued( GW, &( MS_element){ .cover = cover}, GW -> logical.xdiff + w, GW -> logical.ydiff + h);
      }
    }
  }
  
  root -> idle = 0;
  
  root -> tutime = getnanosec();
      
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
  
  if( SDL_WaitEventTimeout( &event, root -> idle? U64C( 60): 0)){
    switch( expect( event.type, SDL_MOUSEBUTTONDOWN)){
    case SDL_QUIT:
      quit( root);
    case SDL_WINDOWEVENT:
      switch( event.window.event){
      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED:
	
	GW -> real.realwidth  = event.window.data1;
	GW -> real.realheight = event.window.data2;
	
	GW -> real.width  = GW -> real.realwidth  / GW -> real.element_width ;
	GW -> real.height = GW -> real.realheight / GW -> real.element_height;
	
	GW -> logical.realwidth  = GW -> real.realwidth  * GW -> logical.element_width  / GW -> real.element_width;
	GW -> logical.realheight = GW -> real.realheight * GW -> logical.element_height / GW -> real.element_height;
	
	GW -> logical.width  = GW -> logical.realwidth  / GW -> logical.element_width  + 1;
	GW -> logical.height = GW -> logical.realheight / GW -> logical.element_height + 1;
	
	//fallthrough
      case SDL_WINDOWEVENT_TAKE_FOCUS:
      case SDL_WINDOWEVENT_RESTORED:
	
	{
	  s32 h = GW -> real.height + 1;
	  
	  while( h--){
	    s32 w = GW -> real.width + 1;
	    
	    while( w--){
	      drawelement_unqued( GW, acse( *root -> minefield, GW -> logical.xdiff + w, GW -> logical.ydiff + h), GW -> logical.xdiff + w, GW -> logical.ydiff + h);
	    }
	  }
	}
	
	root -> idle = 0;
	
	break;
      default:
	break;
      }
      break;
    case SDL_KEYDOWN:
      {
	switch( event.key.keysym.sym){
	case SDLK_ESCAPE:
	  quit( root);
	case SDLK_F2:
	case 'r':
	  	  
	  {
	    s32 h = GW -> real.height + 1;
	    
	    while( h--){
	      s32 w = GW -> real.width + 1;
	      
	      while( w--){
		u8 cover = GW -> global || !( ( s32)GW -> logical.xdiff + w == 0 || ( s32)GW -> logical.ydiff + h == 0 || GW -> logical.xdiff + w == ( s32)GW -> mfvid.width || GW -> logical.ydiff + h == ( s32)GW -> mfvid.height);

		if( acse( *root -> minefield, GW -> logical.xdiff + w, GW -> logical.ydiff + h) -> cover != cover)
		  drawelement_unqued( GW, &( MS_element){ .cover = cover}, GW -> logical.xdiff + w, GW -> logical.ydiff + h);
	      }
	    }
	  }
	  
	  setminefield( root);
	  
	  root -> idle = 0;
	  
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
	  
	  GW -> logical.realxdiff = ( GW -> logical.realxdiff + GW -> mfvid.realwidth) % GW -> mfvid.realwidth;
	  
	  GW -> real.realxdiff = ( GW -> logical.realxdiff * ( s32)GW -> real.element_width ) / ( s32)GW -> logical.element_width;
	  
	  GW -> logical.xdiff = GW -> logical.realxdiff / GW -> logical.element_width;
	  
	  {
	    s32 w = 0;
	    s32 h = GW -> real.height + 1;
	    
	    while( h--){
	      drawelement_unqued( GW, acse( *root -> minefield, GW -> logical.xdiff + w, GW -> logical.ydiff + h), GW -> logical.xdiff + w, GW -> logical.ydiff + h);
	    }
	  }
	  
	  root -> idle = 0;
	  
	  break;
	case SDLK_DOWN:
	case 'j':
	  GW -> logical.realydiff = GW -> logical.realydiff + ( s32)GW -> logical.element_height;
	  if( !GW -> global)
	    GW -> logical.realydiff = GW -> logical.realydiff <= ( s32)( GW -> mfvid.realheight - GW -> logical.realheight)?
	      GW -> logical.realydiff: ( s32)( GW -> mfvid.realheight - GW -> logical.realheight);
	  
	  GW -> logical.realydiff = ( GW -> logical.realydiff + GW -> mfvid.realheight) % GW -> mfvid.realheight;
	  
	  GW -> real.realydiff = ( GW -> logical.realydiff * ( s32)GW -> real.element_height) / ( s32)GW -> logical.element_height;
	  
	  GW -> logical.ydiff = GW -> logical.realydiff / GW -> logical.element_height;
	  
	  {
	    s32 w = GW -> real.width + 1;
	    s32 h = GW -> real.height;
	    
	    while( w--){
	      drawelement_unqued( GW, acse( *root -> minefield, GW -> logical.xdiff + w, GW -> logical.ydiff + h), GW -> logical.xdiff + w, GW -> logical.ydiff + h);
	    }
	  }
	  
	  root -> idle = 0;
	  
	  break;
	case SDLK_UP:
	case 'k':
	  GW -> logical.realydiff = GW -> logical.realydiff - ( s32)GW -> logical.element_height;
	  if( !GW -> global)
	    GW -> logical.realydiff = GW -> logical.realydiff >= 0? GW -> logical.realydiff: 0;
	  
	  GW -> logical.realydiff = ( GW -> logical.realydiff + GW -> mfvid.realheight) % GW -> mfvid.realheight;
	  
	  GW -> real.realydiff = ( GW -> logical.realydiff * ( s32)GW -> real.element_height) / ( s32)GW -> logical.element_height;
	  
	  GW -> logical.ydiff = GW -> logical.realydiff / GW -> logical.element_height;
	  
	  {
	    s32 w = GW -> real.width + 1;
	    s32 h = 0;
	    
	    while( w--){
	      drawelement_unqued( GW, acse( *root -> minefield, GW -> logical.xdiff + w, GW -> logical.ydiff + h), GW -> logical.xdiff + w, GW -> logical.ydiff + h);
	    }
	  }
	  
	  root -> idle = 0;
	  
	  break;
	case SDLK_RIGHT:
	case 'l':
	  GW -> logical.realxdiff = GW -> logical.realxdiff + ( s32)GW -> logical.element_width;
	  if( !GW -> global)
	    GW -> logical.realxdiff = GW -> logical.realxdiff <= ( s32)( GW -> mfvid.realwidth - GW -> logical.realwidth)?
	      GW -> logical.realxdiff: ( s32)( GW -> mfvid.realwidth - GW -> logical.realwidth);
	  
	  GW -> logical.realxdiff = ( GW -> logical.realxdiff + GW -> mfvid.realwidth) % GW -> mfvid.realwidth;
	  
	  GW -> real.realxdiff = ( GW -> logical.realxdiff * ( s32)GW -> real.element_width ) / ( s32)GW -> logical.element_width;
	  
	  GW -> logical.xdiff = GW -> logical.realxdiff / GW -> logical.element_width;
	  
	  {
	    s32 w = GW -> real.width;
	    s32 h = GW -> real.height + 1;
	    
	    while( h--){
	      drawelement_unqued( GW, acse( *root -> minefield, GW -> logical.xdiff + w, GW -> logical.ydiff + h), GW -> logical.xdiff + w, GW -> logical.ydiff + h);
	    }
	  }
	  
	  root -> idle = 0;
	  
	  break;
	default:
	  break;
	}
	break;
      }
    case SDL_MOUSEBUTTONDOWN:
      if( root -> idle){
	mousebuttondown( root, event);
      }
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
  
  postion.x = ( ( event.button.x + GW -> real.realxdiff) / GW -> real.element_width ) % GW -> mfvid.width;
  postion.y = ( ( event.button.y + GW -> real.realydiff) / GW -> real.element_height) % GW -> mfvid.height;
  
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
	setzero( root, vid);
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
	
	drawelement( root -> drawque, &( MS_element){ .cover = 1}, postion.x, postion.y);
      }else if( element -> cover){
	element -> flag = 1;
	++minefield -> mine -> flaged;
	
	drawelement( root -> drawque, &( MS_element){ .flag = 1}, postion.x, postion.y);
      }
    }
  default:
    break;
  }
}

void
draw( MS_root *root){
  GraphicWraper *GW = ( GraphicWraper *)root -> GW;
  DrawComand *dc = NULL;
  address com;
  u64 mytime, cutime;
  
  SDL_SetRenderTarget( GW -> renderer, GW -> target);
  
  if( root -> idle){
    root -> tutime = getnanosec();
  }
    
  root -> tutime += 1000000000 / 45;
  
  while( ( void *)( com = ( address)CS_Releas( root -> drawque)) != NULL){ 
    struct CS_Block block;
    
    root -> idle = 0;
    
    block = CS_BlockReleas( root -> drawque);
    
    dc = MS_CreateLocalFromLocal( DrawComand, ( DrawComand *)com);
    
    if( !block.blk_ptr){
      CS_Finish( root -> drawque, ( DrawComand *)com);
    }
    
    do{
      MS_element *element = &( dc -> element);
      s16 w = dc -> pos.x;
      s16 h = dc -> pos.y;
      
      if( !( ( w < GW -> logical.xdiff && GW -> logical.xdiff + GW -> real.width < GW -> mfvid.width) ||
	     w > GW -> logical.xdiff + ( s32)GW -> real.width  + 1 ||
	     ( h < GW -> logical.ydiff && GW -> logical.ydiff + GW -> real.height < GW -> mfvid.height) ||
	     h > GW -> logical.ydiff + ( s32)GW -> real.height + 1)){
	drawelement_unqued( GW, element, w, h);
      }
      
      com += block.size;
      
      dc = ( DrawComand *)com;
      
    }while( block.blk_ptr && com != block.blk_ptr + block.blk_size);
    
    if( block.blk_ptr){
      MS_FreeFromSize( root -> freenode, block.blk_ptr, true_blk_size);
    }
    
    if( getnanosec() > root -> tutime - root -> fliptime){
      break;
    }
  }
  
  mytime = getnanosec();
  
  SDL_SetRenderTarget( GW -> renderer, NULL);
  
  MS_BlitTarget( GW -> renderer, GW -> target,
		 0,
		 0,
		 GW -> mfvid.realwidth,
		 GW -> mfvid.realheight,
		 -GW -> real.realxdiff,
		 -GW -> real.realydiff,
		 GW -> real.element_width  * GW -> mfvid.width,
		 GW -> real.element_height * GW -> mfvid.height);
  
  if( GW -> global){
    MS_BlitTarget( GW -> renderer, GW -> target,
		   0,
		   0,
		   GW -> mfvid.realwidth,
		   GW -> mfvid.realheight,
		   -GW -> real.realxdiff + GW -> real.element_width  * GW -> mfvid.width,
		   -GW -> real.realydiff,
		   GW -> real.element_width  * GW -> mfvid.width,
		   GW -> real.element_height * GW -> mfvid.height);
    
    MS_BlitTarget( GW -> renderer, GW -> target,
		   0,
		   0,
		   GW -> mfvid.realwidth,
		   GW -> mfvid.realheight,
		   -GW -> real.realxdiff,
		   -GW -> real.realydiff + GW -> real.element_height * GW -> mfvid.height,
		   GW -> real.element_width  * GW -> mfvid.width,
		   GW -> real.element_height * GW -> mfvid.height);
    
    MS_BlitTarget( GW -> renderer, GW -> target,
		   0,
		   0,
		   GW -> mfvid.realwidth,
		   GW -> mfvid.realheight,
		   -GW -> real.realxdiff + GW -> real.element_width  * GW -> mfvid.width,
		   -GW -> real.realydiff + GW -> real.element_height * GW -> mfvid.height,
		   GW -> real.element_width  * GW -> mfvid.width,
		   GW -> real.element_height * GW -> mfvid.height);
  }
  
  SDL_SetRenderTarget( GW -> renderer, GW -> target);
  
  if( !root -> idle){
    
    root -> idle = !com;
    
    mytime = getnanosec() - mytime;
    
    cutime = getnanosec() + 5000000;
    
    if( root -> tutime > cutime){
      nanosleep( &( struct timespec){ .tv_nsec = root -> tutime - cutime}, NULL);
    }
    
    root -> fliptime = mytime;
    root -> fliptime = 300000 < root -> fliptime? root -> fliptime: 300000;
    
    SDL_RenderPresent( GW -> renderer);
  }
}

static inline void
drawelement_unqued( GraphicWraper *GW, const MS_element *element, s16 w, s16 h){
  SDL_Texture *tile = NULL;
  
  if( GW == NULL) return;
  
  if( element -> flag){
    tile =  GW -> flag;
  }else if( element -> cover){
    tile =  GW -> cover;
  }else if( element -> mine){
    tile =  GW -> mine;
  }else switch( element -> count){
    case  0: tile =  GW -> clear; break;
    case  1: tile =  GW -> one; break;
    case  2: tile =  GW -> two; break;
    case  3: tile =  GW -> three; break;
    case  4: tile =  GW -> four; break;
    case  5: tile =  GW -> five; break;
    case  6: tile =  GW -> six; break;
    case  7: tile =  GW -> seven; break;
    case  8: tile =  GW -> eight; break;
    case 15: tile =  GW -> clear; break;
    default:
      return;
      break;
  }
  
  assert( tile != NULL);
  
  MS_BlitTile( GW -> renderer, tile,
	       (int)w * ( int)GW -> logical.element_width ,
	       (int)h * ( int)GW -> logical.element_height,
	       (int)GW -> logical.element_width,
	       (int)GW -> logical.element_height);
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
MS_BlitTarget( SDL_Renderer *renderer, SDL_Texture *target, s32 sx, s32 sy, s32 sw, s32 sh, int dx, int dy, int dw, int dh){
  assert( renderer != NULL);
  assert(   target != NULL);
  
  return SDL_RenderCopyEx( renderer, target,
			   &( SDL_Rect){  .x = sx,
					  .y = sy,
					  .w = sw,
					  .h = sh},
			   &( SDL_Rect){  .x = dx,
					  .y = dy,
					  .w = dw,
					  .h = dh},
			   0, NULL, 0);
}

static inline int
MS_BlitTile( SDL_Renderer *renderer, SDL_Texture *tile, int dx, int dy, int w, int h){
  assert( renderer != NULL);
  assert(     tile != NULL);
  return SDL_RenderCopyEx( renderer, tile, NULL, &( SDL_Rect){ .x = dx, .y = dy, .w = w, .h = h}, 0, NULL, 0);
}


