
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
window_scroll( GraphicWraper *window, MS_diff diff){
  if( !( *window).global){
    diff.x = ( signed long)( *window).video.realxdiff > diff.x? diff.x: ( signed long)( *window).video.realxdiff;
    diff.y = ( signed long)( *window).video.realydiff > diff.y? diff.y: ( signed long)( *window).video.realydiff;
    diff.x = ( ( *window).video.realxdiff + ( *window).fake.realwidth  - diff.x) < ( ( *window).mfvid.realwidth ) ? diff.x: -( signed long)( ( *window).mfvid.realwidth  - ( ( *window).video.realxdiff + ( *window).fake.realwidth ));
    diff.y = ( ( *window).video.realydiff + ( *window).fake.realheight - diff.y) < ( ( *window).mfvid.realheight) ? diff.y: -( signed long)( ( *window).mfvid.realheight - ( ( *window).video.realydiff + ( *window).fake.realheight));
  }

  if( ( !diff.x) && ( !diff.y)){
    return 0;
  }
  
  ( *window).video.realxdiff = ( ( *window).video.realxdiff + ( *window).mfvid.realwidth  - diff.x) % ( *window).mfvid.realwidth;
  ( *window).video.realydiff = ( ( *window).video.realydiff + ( *window).mfvid.realheight - diff.y) % ( *window).mfvid.realheight;
  
  ( *window).video.xdiff = ( ( *window).video.width  * ( *window).video.realxdiff) / ( *window).video.realwidth;
  ( *window).video.ydiff = ( ( *window).video.height * ( *window).video.realydiff) / ( *window).video.realheight;
  
  ( *window).shift.realxdiff = ( ( *window).shift.realxdiff + ( *window).video.realwidth  - diff.x) % ( *window).video.realwidth;
  ( *window).shift.realydiff = ( ( *window).shift.realydiff + ( *window).video.realheight - diff.y) % ( *window).video.realheight;
  
  ( *window).shift.xdiff = ( ( *window).video.width  * ( *window).shift.realxdiff) / ( *window).video.realwidth;
  ( *window).shift.ydiff = ( ( *window).video.height * ( *window).shift.realydiff) / ( *window).video.realheight;

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

  MS_video video = GW -> video; 
  
  i = GW -> shift.width * GW -> shift.height;
  
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
GW_Create( MS_video video, MS_video fake, unsigned long no_resize, MS_field minefield){
  GraphicWraper *GW = ( GraphicWraper *)malloc( sizeof( GraphicWraper));
  
  if( GW == NULL){
    return NULL;
  }

  ( void)no_resize;
  
  ( *GW).shift.xdiff = 0;
  ( *GW).shift.ydiff = 0;
  ( *GW).shift.realxdiff = 0;
  ( *GW).shift.realydiff = 0;
  
  ( *GW).shift.width  = video.width  + 1;
  ( *GW).shift.height = video.height + 1;
  ( *GW).shift.realwidth  = ( ( *GW).shift.width  * video.realwidth ) / video.width ;
  ( *GW).shift.realheight = ( ( *GW).shift.height * video.realheight) / video.height;
  
  video.xdiff = 0;
  video.ydiff = 0;
  video.realxdiff = 0;
  video.realydiff = 0;
  
  if( video.width < minefield.width){
    video.width  = ( *GW).shift.width;
    video.realwidth  = ( *GW).shift.realwidth;
  }
  
  if( video.height < minefield.height){
    video.height = ( *GW).shift.height;
    video.realheight = ( *GW).shift.realheight;
  }
  
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER)){
    return NULL;
  }
  
  ( *GW).ewidth  = 15;
  ( *GW).eheight = 15;
  
  GW -> window = SDL_CreateWindow( "Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				   fake.realwidth, fake.realheight, 0);
  
  GW -> renderer = SDL_CreateRenderer( GW -> window, -1, 0);
  
  if( GW -> window == NULL || GW -> renderer == NULL){
    goto fault;
  }
  
  SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize( GW -> renderer, GW -> shift.width * GW -> ewidth, GW -> shift.height * GW -> eheight);
  
  {
    SDL_Rect rec;
    rec.x = 0;
    rec.y = 0;
    rec.h = ( *GW).ewidth;
    rec.w = ( *GW).eheight;
    
    MS_OpenImage( &( *GW).clear, GW -> renderer, &rec, "nola.png",  0xeeeeee);
    MS_OpenImage( &( *GW).one  , GW -> renderer, &rec, "etta.png",  0x0000ff);
    MS_OpenImage( &( *GW).two  , GW -> renderer, &rec, "tvaa.png",  0x00ff00);
    MS_OpenImage( &( *GW).three, GW -> renderer, &rec, "trea.png",  0xff0000);
    MS_OpenImage( &( *GW).four , GW -> renderer, &rec, "fyra.png",  0xcccc00);
    MS_OpenImage( &( *GW).five , GW -> renderer, &rec, "fema.png",  0xbb0044);
    MS_OpenImage( &( *GW).six  , GW -> renderer, &rec, "sexa.png",  0x00ffff);
    MS_OpenImage( &( *GW).seven, GW -> renderer, &rec, "sjua.png",  0xbbbbbb);
    MS_OpenImage( &( *GW).eight, GW -> renderer, &rec, "ataa.png",  0x666666);
    
    MS_OpenImage( &( *GW).mine , GW -> renderer, &rec, "mina.png",  0xffaa77);
    MS_OpenImage( &( *GW).cover, GW -> renderer, &rec, "plata.png", 0x888888);
    MS_OpenImage( &( *GW).flag , GW -> renderer, &rec, "flaga.png", 0xffff00);
  }
  
  SDL_EventState( SDL_JOYAXISMOTION, SDL_IGNORE);
  SDL_EventState( SDL_JOYBALLMOTION, SDL_IGNORE);
  SDL_EventState( SDL_JOYBUTTONDOWN, SDL_IGNORE);
  SDL_EventState( SDL_JOYBUTTONUP  , SDL_IGNORE);
  SDL_EventState( SDL_USEREVENT    , SDL_IGNORE);
  SDL_EventState( SDL_SYSWMEVENT   , SDL_IGNORE);
  
  ( *GW).video = video;
  ( *GW).fake  = fake;
  
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
