
#include <SDL2/SDL.h>

#define LOCALE_( name) GW_##name

#include "MS_util.h"
#include "minefield.h"
#include "GW.h"


int MS_OpenImage( SDL_Texture **, SDL_Renderer *, SDL_Rect *, char *, __uint32_t);
int MS_BlitSurface( SDL_Surface *, SDL_Surface *, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
SDL_Texture *drawelement( GraphicWraper *, __uint8_t);
void MS_scale( SDL_Surface *, SDL_Surface *, signed long, signed long, unsigned long, unsigned long);


int
window_scroll( GraphicWraper *GW, MS_diff diff){
  if( !GW -> global){
    diff.x = ( signed long)GW -> logical.realxdiff > diff.x? diff.x: ( signed long)GW -> logical.realxdiff;
    diff.y = ( signed long)GW -> logical.realydiff > diff.y? diff.y: ( signed long)GW -> logical.realydiff;
    diff.x = ( GW -> logical.realxdiff + GW -> logical.realwidth  - diff.x) < ( GW -> mfvid.realwidth ) ? diff.x: -( signed long)( GW -> mfvid.realwidth  - ( GW -> logical.realxdiff + GW -> logical.realwidth ));
    diff.y = ( GW -> logical.realydiff + GW -> logical.realheight - diff.y) < ( GW -> mfvid.realheight) ? diff.y: -( signed long)( GW -> mfvid.realheight - ( GW -> logical.realydiff + GW -> logical.realheight));
  }
  
  if( ( !diff.x) && ( !diff.y)){
    return 0;
  }
  
  GW -> logical.realxdiff = ( GW -> logical.realxdiff + GW -> mfvid.realwidth  - diff.x) % GW -> mfvid.realwidth;
  GW -> logical.realydiff = ( GW -> logical.realydiff + GW -> mfvid.realheight - diff.y) % GW -> mfvid.realheight;
  
  GW -> logical.xdiff = ( GW -> logical.width  * GW -> logical.realxdiff) / GW -> logical.realwidth;
  GW -> logical.ydiff = ( GW -> logical.height * GW -> logical.realydiff) / GW -> logical.realheight;
  
  return 1;
}

int MS_OpenImage( SDL_Texture **tdst, SDL_Renderer *render, SDL_Rect *rec, char *str, __uint32_t c){
  SDL_Surface *img, *dst;
  int ret = 0;
  if unlikely( str != NULL){
    img = IMG_Load( str);
  }else{
    img = NULL;
  }
  dst = SDL_CreateRGBSurface( FALSE, rec -> w, rec -> h, 24, 0xff0000, 0xff00, 0xff, 0x0);
  if unlikely( dst == NULL){
    return -4;
  }
  SDL_FillRect( dst, rec, c);
  if unlikely( img == NULL){
    ret =  -3;
  }
  if( img != NULL) MS_BlitSurface( img, dst, rec -> x, rec -> y, 0, 0, rec -> w, rec -> h);
  *tdst = SDL_CreateTextureFromSurface( render, dst);
  if( img != NULL) SDL_free( img);
  if( dst != NULL) SDL_free( dst);
  return ret;
}

/* 
 * copys a squer from sufer src at loction sx,sy to sufer dst at location dx,dy of size w,h
 */
int
MS_BlitSurface( SDL_Surface *src, SDL_Surface *dst, unsigned long dx, unsigned long dy, unsigned long sx, unsigned long sy, unsigned long w, unsigned long h){
  SDL_Rect srect, drect;
    
  srect.x = sx;
  srect.y = sy;
  srect.w = w;
  srect.h = h;
  drect.x = dx;
  drect.y = dy;
  drect.h = h;
  drect.w = w;
  
  if likely( dst != NULL){
    if likely( src != NULL){
      SDL_BlitSurface( src, &srect, dst, &drect);
      return 0;
    }
    SDL_FillRect( dst, &drect, 0x0);
  }
  return -3;
}

int
draw( GraphicWraper *GW, MS_field minefield){
  MS_pos element, elementsh;
  SDL_Rect srect, drect;
  unsigned long i;
  SDL_Texture *tile;

  MS_video video = GW -> logical; 
  
  i = GW -> logical.width * GW -> logical.height;
  
  while( i--){
    
    element.x = ( video.xdiff + ( i % video.width)) % minefield.width;
    element.y = ( video.ydiff + ( i / video.width)) % minefield.height;
    
    elementsh.x = ( ( video.width  + ( ( minefield.width  + ( element.x) - video.xdiff) % minefield.width )) % video.width );
    elementsh.y = ( ( video.height + ( ( minefield.height + ( element.y) - video.ydiff) % minefield.height)) % video.height);
    
    tile = drawelement( GW, minefield.data[ element.x + element.y * minefield.width]);
    
    srect.x = 0;
    srect.y = 0;
    srect.w = GW -> ewidth;
    srect.h = GW -> eheight;
    drect.x = elementsh.x * GW -> ewidth;
    drect.y = elementsh.y * GW -> eheight;
    drect.w = GW -> ewidth;
    drect.h = GW -> eheight;
    
    SDL_RenderCopy( GW ->  renderer, tile, &srect, &drect);
  }

  SDL_RenderPresent( GW -> renderer);
      
  return 0;
}


SDL_Texture *
drawelement( GraphicWraper *gui, __uint8_t element){

  if( element & EFLAG){
    return ( *gui).flag;
  }
  
  if( element & ECOVER){
    return ( *gui).cover;
  }
  
  if( ( element & EMINE) && ( element & ECOUNT) != ECOUNT){
    return ( *gui).mine;
  }

  switch( ECOUNT & element){
  case 0: return ( *gui).clear;
  case 1: return ( *gui).one;
  case 2: return ( *gui).two;
  case 3: return ( *gui).three;
  case 4: return ( *gui).four;
  case 5: return ( *gui).five;
  case 6: return ( *gui).six;
  case 7: return ( *gui).seven;
  case 8: return ( *gui).eight;
  /**/
  case 0xf:  return ( *gui).clear;
  default:
    return NULL;
  }
  
  return NULL;
}


GraphicWraper *
GW_Create( MS_video rel, MS_video log, unsigned long no_resize, MS_field minefield){
  GraphicWraper *GW = ( GraphicWraper *)malloc( sizeof( GraphicWraper));
  
  if( GW == NULL){
    return NULL;
  }

  GW -> real    = rel;
  GW -> logical = log;

  GW -> ewidth  = 15;
  GW -> eheight = 15;
  
  GW -> logical.realwidth  = GW -> logical.width  * GW -> ewidth;
  GW -> logical.realheight = GW -> logical.height * GW -> eheight;

  GW -> logical.xdiff = 0;
  GW -> logical.ydiff = 0;
  
  GW -> logical.realxdiff = 0;
  GW -> logical.realydiff = 0;
  
  ( void)minefield;
  ( void)no_resize;
    
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER)){
    return NULL;
  }
  
  GW -> window = SDL_CreateWindow( "Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				   GW -> real.realwidth, GW -> real.realheight, SDL_WINDOW_HIDDEN);
  
  GW -> renderer = SDL_CreateRenderer( GW -> window, -1, 0);
  
  if( GW -> window == NULL || GW -> renderer == NULL){
    goto fault;
  }
  
  SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize( GW -> renderer, GW -> logical.realwidth, GW -> logical.realheight);

  if( no_resize){
    SDL_SetWindowResizable( GW ->  window, SDL_FALSE);
  }else{
    SDL_SetWindowResizable( GW ->  window, SDL_TRUE);
  }
  
  {
    SDL_Rect rec;
    rec.x = 0;
    rec.y = 0;
    rec.h = GW -> ewidth;
    rec.w = GW -> eheight;
    
    MS_OpenImage( &GW -> clear, GW -> renderer, &rec, "nola.png",  0xeeeeee);
    MS_OpenImage( &GW -> one  , GW -> renderer, &rec, "etta.png",  0x0000ff);
    MS_OpenImage( &GW -> two  , GW -> renderer, &rec, "tvaa.png",  0x00ff00);
    MS_OpenImage( &GW -> three, GW -> renderer, &rec, "trea.png",  0xff0000);
    MS_OpenImage( &GW -> four , GW -> renderer, &rec, "fyra.png",  0xcccc00);
    MS_OpenImage( &GW -> five , GW -> renderer, &rec, "fema.png",  0xbb0044);
    MS_OpenImage( &GW -> six  , GW -> renderer, &rec, "sexa.png",  0x00ffff);
    MS_OpenImage( &GW -> seven, GW -> renderer, &rec, "sjua.png",  0xbbbbbb);
    MS_OpenImage( &GW -> eight, GW -> renderer, &rec, "ataa.png",  0x666666);
    
    MS_OpenImage( &GW -> mine , GW -> renderer, &rec, "mina.png",  0xffaa77);
    MS_OpenImage( &GW -> cover, GW -> renderer, &rec, "plata.png", 0x888888);
    MS_OpenImage( &GW -> flag , GW -> renderer, &rec, "flaga.png", 0xffff00);
  }
  
  SDL_EventState( SDL_JOYAXISMOTION, SDL_IGNORE);
  SDL_EventState( SDL_JOYBALLMOTION, SDL_IGNORE);
  SDL_EventState( SDL_JOYBUTTONDOWN, SDL_IGNORE);
  SDL_EventState( SDL_JOYBUTTONUP  , SDL_IGNORE);
  SDL_EventState( SDL_USEREVENT    , SDL_IGNORE);
  SDL_EventState( SDL_SYSWMEVENT   , SDL_IGNORE);

  SDL_ShowWindow( GW -> window);
  
  return GW;
 fault:
  SDL_Quit();
  return NULL;
}

void
GW_Free( GraphicWraper *GW){
  if( GW != NULL){
    //free GW -> window
        
    SDL_Quit();
    
    free( GW);
  }
}
