
#include <SDL2/SDL.h>

#define LOCALE_( name) GW_##name

#include "MS_util.h"
#include "minefield.h"
#include "userinterface.h"


SDL_Texture *MS_OpenImage( SDL_Renderer *, const char *);
int MS_BlitTex(  SDL_Renderer *, SDL_Texture *, int, int, int, int, int, int);
int MS_BlitTile( SDL_Renderer *, SDL_Texture *, int, int, int, int);
SDL_Texture *drawelement( GraphicWraper *, __uint8_t);
void MS_scale( SDL_Surface *, SDL_Surface *, signed long, signed long, unsigned long, unsigned long);


int
event_dispatch( void *data){
  int ret = 1;
  
  MS_root       *root      = data;
  MS_field      *minefield = root -> minefield;
  GraphicWraper *GW        = root -> GW;
    
  root -> tutime = getnanosec();
  if( !minefield -> mine -> uncoverd || root -> gameover){
    root -> gamestart = root -> tutime;
  }
  
  if unlikely( minefield -> mine -> hit){
    MS_video mfvid = { .xdiff = 0, .ydiff = 0, .width  = minefield -> subwidth, .height = minefield -> subheight};
    take_action( root -> actionque,  uncov_elements,  MS_Create( uncov_elementsargs, minefield, mfvid));
    
    take_action( root -> actionque,  uncov,  MS_Create( uncovargs, minefield));
  }
  
  if( SDL_WaitEventTimeout( &root -> event, 1)){
    switch( expect( root -> event.type, SDL_MOUSEBUTTONDOWN)){
    case SDL_QUIT:            take_action( root -> actionque, root -> quit, root); goto end;
    case SDL_KEYDOWN:
      {
	SDL_Event event = root -> event;
	
	MS_diff *diff = root -> diff;
		
	switch( root -> event.key.keysym.sym){
	case SDLK_ESCAPE:
	  take_action( root -> actionque, root -> quit, ( void *)root);
	  break;
	case SDLK_F2:
	case 'r':
	  root -> gameover = FALSE;
	  if( minefield -> mine -> uncoverd || minefield -> mine -> flaged){
	    take_action( root -> actionque, setminefield, MS_Create( setminefieldargs, minefield, root -> mss, GW -> mfvid));
	    ret = 1;
	  }
	  break;
	case SDLK_F3:
	case 'e':
	  if( minefield -> mine -> uncoverd < ( minefield -> mine -> noelements - minefield -> mine -> flaged)){
	    take_action( root -> actionque,  uncov_elements,  MS_Create( uncov_elementsargs, minefield, GW -> mfvid));
	  }
	  take_action( root -> actionque,  uncov,  MS_Create( uncovargs, minefield));
	  break;
	default:
	  break;
	}
	break;
      }
    case SDL_MOUSEBUTTONDOWN:
      {
	MS_pos postion;
	
	{
	  MS_video video = root -> GW -> real;
	  postion.x = ( ( unsigned long)( ( ( root -> event.button.x + video.realxdiff) * video.width ) / video.realwidth )) % minefield -> width;
	  postion.y = ( ( unsigned long)( ( ( root -> event.button.y + video.realydiff) * video.height) / video.realheight)) % minefield -> height;
	}
	
	switch( root -> event.button.button){
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
	    
	    if( root -> event.button.button == SDL_BUTTON_MIDDLE){
	      vid = ( MS_video){ .xdiff = postion.x - 1, .ydiff = postion.y - 1, .width  = 3, .height = 3};
	    }
	    
	    if( minefield -> mine -> set == 0){
	      /*let's play "Texas Sharpshooter"*/
	      take_action( root -> actionque,  setzero,  MS_Create( setzeroargs, minefield, vid));
	    }
	    
	    take_action( root -> actionque,  uncov_elements,  MS_Create( uncov_elementsargs, minefield, vid));
	    
	    take_action( root -> actionque,  uncov,  MS_Create( uncovargs, minefield));
	    break;
	  }
	case SDL_BUTTON_RIGHT:
	  {
	    __uint8_t *element = acse( *minefield, postion.x, postion.y);
	    if( *element & EFLAG){
	      *element &= ~EFLAG;
	      --minefield -> mine -> flaged;
	      ret = 1;
	    }else if( *element & ECOVER){
	      *element|= EFLAG;
	      ++minefield -> mine -> flaged;
	      ret = 1;
	    }
	  }
	default:
	  break;
	}
	break;
      }
    default: break;
    }
    
    root -> nextframe = root -> tutime;
  }
  
  assert( data != NULL);
  
  take_action( root -> actionque, event_dispatch, root);
 end:
  return ret;
}


int
window_scroll( GraphicWraper *GW, MS_diff diff){
  int ret = 0;
  if( !GW -> global){
    diff.x = ( signed long)GW -> real.realxdiff > diff.x? diff.x: ( signed long)GW -> real.realxdiff;
    diff.y = ( signed long)GW -> real.realydiff > diff.y? diff.y: ( signed long)GW -> real.realydiff;
    diff.x = ( GW -> real.realxdiff + GW -> real.realwidth  - diff.x) < ( GW -> mfvid.realwidth ) ? diff.x: -( signed long)( GW -> mfvid.realwidth  - ( GW -> real.realxdiff + GW -> real.realwidth ));
    diff.y = ( GW -> real.realydiff + GW -> real.realheight - diff.y) < ( GW -> mfvid.realheight) ? diff.y: -( signed long)( GW -> mfvid.realheight - ( GW -> real.realydiff + GW -> real.realheight));
  }
  
  if( ( diff.x) || ( diff.y)){
    GW -> real.realxdiff = ( GW -> real.realxdiff + GW -> mfvid.realwidth  - diff.x) % GW -> mfvid.realwidth ;
    GW -> real.realydiff = ( GW -> real.realydiff + GW -> mfvid.realheight - diff.y) % GW -> mfvid.realheight;
    
    GW -> real.xdiff = GW -> real.realxdiff / GW -> real.element_width;
    GW -> real.ydiff = GW -> real.realydiff / GW -> real.element_height;
    ret = 1;
  }
  
  return ret;
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
  assert( renderer != NULL);
  assert(      tex != NULL);
  return SDL_RenderCopyEx( renderer, tex, &( SDL_Rect){ .x = sx, .y = sy, .w = w, .h = h}, &( SDL_Rect){ .x = dx, .y = dy, .w = w, .h = h}, 0, NULL, SDL_FLIP_NONE);
}

int
MS_BlitTile( SDL_Renderer *renderer, SDL_Texture *tile, int dx, int dy, int w, int h){
  assert( renderer != NULL);
  assert(     tile != NULL);
  return SDL_RenderCopyEx( renderer, tile, NULL, &( SDL_Rect){ .x = dx, .y = dy, .w = w, .h = h}, 0, NULL, SDL_FLIP_NONE);
}


int
draw( GraphicWraper *GW, MS_field minefield){
  int ret = 0;
  MS_pos element;
  unsigned long i;
  SDL_Texture *tile;

  int w = GW -> real.realwidth  / GW -> real.element_width  + 1;
  int h = GW -> real.realheight / GW -> real.element_height + 1;

  SDL_SetRenderTarget( GW -> renderer, GW -> target);
  
  i = w * h;
  
  while( i--){
    
    {
      element.x = ( GW -> real.xdiff + i % w);
      element.y = ( GW -> real.ydiff + i / w);
      
      tile = drawelement( GW, *acse( minefield, element.x, element.y));
      
      if( tile == NULL){
        ret = -3;
        continue;
      }
    }
    
    MS_BlitTile( GW -> renderer, tile,
                 i % w * GW -> real.element_width  - GW -> real.realxdiff % GW -> real.element_width,
                 i / w * GW -> real.element_height - GW -> real.realydiff % GW -> real.element_height,
                 GW -> real.element_width,
                 GW -> real.element_height);
  }
  
  SDL_SetRenderTarget( GW -> renderer, NULL);

  {
    int ax = GW -> real.realxdiff % GW -> real.realwidth, ay = GW -> real.realydiff % GW -> real.realheight, cx = GW -> real.realwidth - ax, cy = GW -> real.realheight - ay;
    
    MS_BlitTex( GW -> renderer, GW -> target, 0 , 0 , ax, ay, cx, cy);
    MS_BlitTex( GW -> renderer, GW -> target, ax, ay, cx, cy, 0 , 0 );
  }
  
  SDL_RenderPresent( GW -> renderer);
  
  SDL_ShowWindow( GW -> window);
  
  return ret;
}


SDL_Texture *
drawelement( GraphicWraper *GW, __uint8_t element){
  SDL_Texture *tile = NULL;
  
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
  
  return tile;
}


GraphicWraper *
GW_Init( GraphicWraper *GW){
  GraphicWraper *ret = NULL;
  
  if unlikely( GW == NULL) goto end;
  
  GW -> real.realwidth  = GW -> real.realwidth ? GW -> real.realwidth : GW -> mfvid.width  * GW -> real.element_width;
  GW -> real.realheight = GW -> real.realheight? GW -> real.realheight: GW -> mfvid.height * GW -> real.element_height;
  
  GW -> real.width  = ( GW -> real.realwidth ) / GW -> real.element_width ;
  GW -> real.height = ( GW -> real.realheight) / GW -> real.element_height;
  
  GW -> mfvid.realwidth  = GW -> mfvid.width  * GW -> real.element_width;
  GW -> mfvid.realheight = GW -> mfvid.height * GW -> real.element_height;
  
  if unlikely( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER)) goto end;
  
  GW -> window = SDL_CreateWindow( "Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				   GW -> real.realwidth, GW -> real.realheight, SDL_WINDOW_HIDDEN);
  
  if unlikely( GW -> window   == NULL) goto end;
  
  GW -> renderer = SDL_CreateRenderer( GW -> window, -1, 0);
  
  if unlikely( GW -> renderer == NULL) goto end;
  
  SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize( GW -> renderer, GW -> real.realwidth, GW -> real.realheight);
  SDL_SetRenderDrawColor( GW -> renderer, 0, 0xff, 0, 0xff);
  
  SDL_SetWindowResizable( GW ->  window, (SDL_bool)!GW -> no_resize);
  
  GW -> target = SDL_CreateTexture( GW -> renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, GW -> real.realwidth, GW -> real.realheight);
  
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
  
  ret = GW;
 end:
  if unlikely( ret != GW) GW_Free( GW);
  return ret;
}

void
GW_Free( GraphicWraper *GW){
  if( GW != NULL){
    SDL_Quit();
    
    free( GW);
  }
}