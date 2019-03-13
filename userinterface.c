
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define LOCALE_( name) GW_##name

#include "MS_util.h"
#include "minefield.h"
#include "userinterface.h"

typedef struct{
  s32 x;
  s32 y;
}MS_diff;

typedef struct{
  MS_video mfvid;
  MS_video real;
  u16 global;
  u16 no_resize;
  SDL_Window *window;
  SDL_Texture *target;
  SDL_Renderer *renderer;
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

SDL_Texture *MS_OpenImage( SDL_Renderer *, const char *);
int MS_BlitTex(  SDL_Renderer *, SDL_Texture *, int, int, int, int, int, int);
int MS_BlitTile( SDL_Renderer *, SDL_Texture *, int, int, int, int);
void mousebuttondown( MS_root *, SDL_Event);

void *
GW_Init( MS_root *root){
  GraphicWraper *GW = MS_CreateEmpty( GraphicWraper);
  
  GW -> global = root -> minefield -> global;
  
  GW -> real = root -> real;
  
  GW -> mfvid.width  = root -> minefield -> subwidth;
  GW -> mfvid.height = root -> minefield -> subheight;
  
  GW -> real.realwidth  = root -> real.realwidth ? root -> real.realwidth : GW -> mfvid.width  * GW -> real.element_width;
  GW -> real.realheight = root -> real.realheight? root -> real.realheight: GW -> mfvid.height * GW -> real.element_height;
  
  GW -> real.width  = ( GW -> real.realwidth ) / GW -> real.element_width ;
  GW -> real.height = ( GW -> real.realheight) / GW -> real.element_height;
  
  GW -> mfvid.realwidth  = GW -> mfvid.width  * GW -> real.element_width;
  GW -> mfvid.realheight = GW -> mfvid.height * GW -> real.element_height;
  
  assert( !SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER));
  
  GW -> window = SDL_CreateWindow( root -> minefield -> title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				   (int)GW -> real.realwidth, (int)GW -> real.realheight, SDL_WINDOW_HIDDEN);
  
  assert( GW -> window != NULL);
  
  GW -> renderer = SDL_CreateRenderer( GW -> window, -1, 0);
  
  assert( GW -> renderer != NULL);
  
  SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize( GW -> renderer, (int)GW -> real.realwidth, (int)GW -> real.realheight);
  SDL_SetRenderDrawColor( GW -> renderer, 0, 0xff, 0, 0xff);
  
  SDL_SetWindowResizable( GW ->  window, (SDL_bool)!root -> no_resize);
  
  GW -> target = SDL_CreateTexture( GW -> renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, (int)GW -> real.realwidth, (int)GW -> real.realheight);
  
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

  return GW;
}

void
event_dispatch( MS_root *root){
  MS_field      *minefield = root -> minefield;
  GraphicWraper *GW        = ( GraphicWraper *)root -> GW;
  SDL_Event event;
  
  if unlikely( GW == NULL) return;
    
  if( SDL_WaitEventTimeout( &event, 1000)){
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
	  root -> gameover = FALSE;
	  if( minefield -> mine -> uncoverd || minefield -> mine -> flaged){
	    setminefield( minefield, GW, root -> mss, GW -> mfvid);
	  }
	  break;
	case SDLK_F3:
	case 'e':
	  if( minefield -> mine -> uncoverd < ( minefield -> mine -> noelements - minefield -> mine -> flaged)){
	    uncov_elements( minefield, GW -> mfvid);
	  }
	  uncov( minefield, GW);
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
    
    root -> nextframe = root -> tutime;
  }
}


void mousebuttondown( MS_root * root,
		      SDL_Event event){
  
  MS_field      *minefield = root -> minefield;
  GraphicWraper *GW        = ( GraphicWraper *)root -> GW;
  
  MS_pos postion;
  
  {
    MS_video video = ( ( GraphicWraper *)( root -> GW)) -> real;
    postion.x = (s16)( ( ( (u16)( event.button.x + video.realxdiff) * video.width ) / video.realwidth ) % minefield -> width );
    postion.y = (s16)( ( ( (u16)( event.button.y + video.realydiff) * video.height) / video.realheight) % minefield -> height);
  }
  
  switch( event.button.button){
  case SDL_BUTTON_LEFT:
  case SDL_BUTTON_MIDDLE:
    {
      MS_video vid;
      MS_pos *el;
      
      while( ( el = ( MS_pos *)CS_Releas( minefield -> uncovque)) != NULL){
	*acse( *minefield, el -> x, el -> y) |= ECOVER;
	CS_Finish( minefield -> uncovque, el);
      }
      
      if( ( minefield -> mine -> uncoverd == ( minefield -> mine -> noelements - minefield -> mine -> level)) || ( minefield -> mine -> hit)){
	break;
      }
      
      vid = ( MS_video){ .xdiff = postion.x, .ydiff = postion.y, .width  = 1, .height = 1};
      
      if( event.button.button == SDL_BUTTON_MIDDLE){
	vid = ( MS_video){ .xdiff = postion.x - 1, .ydiff = postion.y - 1, .width  = 3, .height = 3};
      }
      
      if( minefield -> mine -> set == 0){
	/*let's play "Texas Sharpshooter"*/
	setzero( minefield, vid);
      }
      
      uncov_elements( minefield, vid);
      
      uncov( minefield, GW);
      break;
    }
  case SDL_BUTTON_RIGHT:
    {
      __uint8_t *element = acse( *minefield, postion.x, postion.y);
      if( *element & EFLAG){
	*element &= ~EFLAG;
	--minefield -> mine -> flaged;
      }else if( *element & ECOVER){
	*element|= EFLAG;
	++minefield -> mine -> flaged;
      }
      
      drawelement( GW, minefield, postion.x, postion.y);
    }
  default:
    break;
  }
}

void
draw( void *gw_void, MS_field minefield){
  GraphicWraper *GW = gw_void;
  (void)minefield;
  
  {
    int
      ax = (int)( (u16)( GW -> real.realxdiff + (s16)GW -> real.realwidth ) % GW -> real.realwidth ),
      ay = (int)( (u16)( GW -> real.realydiff + (s16)GW -> real.realheight) % GW -> real.realheight),
      cx = (int)GW -> real.realwidth  - ax,
      cy = (int)GW -> real.realheight - ay;
    
    MS_BlitTex( GW -> renderer, GW -> target, 0 , 0 , ax, ay, cx, cy);
    MS_BlitTex( GW -> renderer, GW -> target, ax, ay, cx, cy, 0 , 0 );
  }
  
  SDL_RenderPresent( GW -> renderer);
  
  SDL_ShowWindow( GW -> window);
}

void
drawelement( void *VGW, MS_field *minefield, s16 w, s16 h){
  GraphicWraper *GW = ( GraphicWraper *)VGW;
  SDL_Texture *tile = NULL;
  __uint8_t element = *acse( *minefield, w, h);
  
  if( element & EFLAG){
    tile =  GW -> flag;
  }
  
  if( tile == NULL && ( element & ECOVER)){
    tile =  GW -> cover;
  }
  
  if( tile == NULL && ( element & EMINE) && ( element & ECOUNT) != ECOUNT){
    tile =  GW -> mine;
  }

  if( tile == NULL){
    switch( ECOUNT & element){
    case 0: tile =  GW -> clear; break;
    case 1: tile =  GW -> one; break;
    case 2: tile =  GW -> two; break;
    case 3: tile =  GW -> three; break;
    case 4: tile =  GW -> four; break;
    case 5: tile =  GW -> five; break;
    case 6: tile =  GW -> six; break;
    case 7: tile =  GW -> seven; break;
    case 8: tile =  GW -> eight; break;
    case 0xf:  tile =  GW -> clear;break;
    default:
      break;
    }
  }

  SDL_SetRenderTarget( GW -> renderer, GW -> target);
  
  MS_BlitTile( GW -> renderer, tile,
	       ( ( w - GW -> real.xdiff)) * (s16)GW -> real.element_width  - (int)( (u16)( GW -> real.realxdiff + (s16)GW -> real.realwidth ) % GW -> real.element_width),
	       ( ( h - GW -> real.ydiff)) * (s16)GW -> real.element_height - (int)( (u16)( GW -> real.realydiff + (s16)GW -> real.realheight) % GW -> real.element_height),
	       (int)GW -> real.element_width,
	       (int)GW -> real.element_height);
  
  SDL_SetRenderTarget( GW -> renderer, NULL);
}

void
GW_Free( void *GW){
  if( GW != NULL){
    SDL_Quit();
    
    free( GW);
  }
}

SDL_Texture *
MS_OpenImage( SDL_Renderer *render, const char *str){
  SDL_Texture *tex = NULL;
  SDL_Surface *img = NULL;
  assert( render != NULL);
  assert(    str != NULL);
  if unlikely( ( img = IMG_Load( str                            )) == NULL) goto bail;
  if unlikely( ( tex = SDL_CreateTextureFromSurface( render, img)) == NULL) goto bail;
 bail:
  if( img != NULL) SDL_free( img);
  return tex;
}

int
MS_BlitTex( SDL_Renderer *renderer, SDL_Texture *tex, int dx, int dy, int w, int h, int sx, int sy){
  dassert( renderer != NULL);
  dassert(      tex != NULL);
  return SDL_RenderCopyEx( renderer, tex, &( SDL_Rect){ .x = sx, .y = sy, .w = w, .h = h}, &( SDL_Rect){ .x = dx, .y = dy, .w = w, .h = h}, 0, NULL, SDL_FLIP_NONE);
}

int
MS_BlitTile( SDL_Renderer *renderer, SDL_Texture *tile, int dx, int dy, int w, int h){
  dassert( renderer != NULL);
  dassert(     tile != NULL);
  return SDL_RenderCopyEx( renderer, tile, NULL, &( SDL_Rect){ .x = dx, .y = dy, .w = w, .h = h}, 0, NULL, SDL_FLIP_NONE);
}


