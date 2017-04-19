
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
    
    GW -> real.xdiff = ( GW -> real.width  * GW -> real.realxdiff) / GW -> real.realwidth;
    GW -> real.ydiff = ( GW -> real.height * GW -> real.realydiff) / GW -> real.realheight;
    ret = 1;
  }
  
  return ret;
}

int MS_OpenImage( SDL_Texture **tdst, SDL_Renderer *render, SDL_Rect *rec, char *str, __uint32_t c){
  int ret = 0;
  SDL_Surface *img, *dst;
  if unlikely( str != NULL){
    img = IMG_Load( str);
  }else{
    img = NULL;
  }
  dst = SDL_CreateRGBSurface( FALSE, rec -> w, rec -> h, 24, 0xff0000, 0xff00, 0xff, 0x0);
  if unlikely( dst == NULL){
    ret = -4;
  }else{
    SDL_FillRect( dst, rec, c);
    if unlikely( img == NULL){
      ret =  -3;
    }
    if( img != NULL) MS_BlitSurface( img, dst, rec -> x, rec -> y, 0, 0, rec -> w, rec -> h);
    *tdst = SDL_CreateTextureFromSurface( render, dst);
    if( img != NULL) SDL_free( img);
    if( dst != NULL) SDL_free( dst);
  }
  return ret;
}

/* 
 * copys a squer from sufer src at loction sx,sy to sufer dst at location dx,dy of size w,h
 */
int
MS_BlitSurface( SDL_Surface *src, SDL_Surface *dst, unsigned long dx, unsigned long dy, unsigned long sx, unsigned long sy, unsigned long w, unsigned long h){
  int ret = 0;
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
      ret = 0;
    }else{
      SDL_FillRect( dst, &drect, 0x0);
      ret = -3;
    }
  }
  return ret;
}

int
draw( GraphicWraper *GW, MS_field minefield){
  int ret = 0;
  MS_pos element, elementsh;
  SDL_Rect srect, drect;
  unsigned long i;
  SDL_Texture *tile;
  
  GW -> logical.realxdiff = ( GW -> logical.realwidth  * GW -> real.realxdiff) / GW -> real.realwidth ;
  GW -> logical.realydiff = ( GW -> logical.realheight * GW -> real.realydiff) / GW -> real.realheight;
  
  GW -> logical.realxdiff -= GW -> logical.realxdiff % GW -> logical.realwidth  + GW -> logical.element_width;
  GW -> logical.realydiff -= GW -> logical.realydiff % GW -> logical.realheight + GW -> logical.element_height;
  
  GW -> logical.xdiff = ( GW -> logical.width  * GW -> logical.realxdiff) / GW -> logical.realwidth;
  GW -> logical.ydiff = ( GW -> logical.height * GW -> logical.realydiff) / GW -> logical.realheight;
  
  SDL_SetRenderTarget( GW -> renderer, GW -> target);
  
  i = GW -> logical.width * GW -> logical.height;
  
  while( i--){
    
    {
      element.x = ( GW -> real.xdiff + i % GW -> logical.width);
      element.y = ( GW -> real.ydiff + i / GW -> logical.width);
      
      tile = drawelement( GW, *acse( minefield, element.x, element.y));
      
      if( tile == NULL){
        ret = -3;
        continue;
      }
    }
    
    elementsh.x = ( element.x + 1) % GW -> logical.width ;
    elementsh.y = ( element.y + 1) % GW -> logical.height;
    
    srect.x = 0;
    srect.y = 0;
    srect.w = GW -> logical.element_width;
    srect.h = GW -> logical.element_height;
    drect.x = elementsh.x * GW -> logical.element_width ;
    drect.y = elementsh.y * GW -> logical.element_height;
    drect.w = GW -> logical.element_width;
    drect.h = GW -> logical.element_height;
    
    SDL_RenderCopy( GW -> renderer, tile, &srect, &drect);
  }

  SDL_SetRenderTarget( GW -> renderer, NULL);
  
  {
    unsigned long ax, ay, bx, by, adx, ady, bdx, bdy, cx, cy;
    
    SDL_RenderClear( GW -> renderer);  
    
    ax = ( ( GW -> logical.realwidth  * GW -> real.realxdiff) / GW -> real.realwidth ) % GW -> logical.realwidth ;
    ay = ( ( GW -> logical.realheight * GW -> real.realydiff) / GW -> real.realheight) % GW -> logical.realheight;
    bx = ax + GW -> logical.element_width;
    by = ay + GW -> logical.element_height;
    cx = GW -> logical.realwidth  - bx;
    cy = GW -> logical.realheight - by;
    
    adx = ( ax * GW -> real.realwidth ) / ( ax + cx);
    ady = ( ay * GW -> real.realheight) / ( ay + cy);
    bdx = GW -> real.realwidth  - adx;
    bdy = GW -> real.realheight - ady;
    
    srect = ( SDL_Rect){ .x = bx , .y = by , .w = cx , .h = cy };
    drect = ( SDL_Rect){ .x = 0  , .y = 0  , .w = bdx, .h = bdy};
    
    SDL_RenderCopyEx( GW -> renderer, GW -> target, &srect, &drect, 0, NULL, SDL_FLIP_NONE);
    
    srect = ( SDL_Rect){ .x = 0  , .y = 0  , .w = ax , .h = ay };
    drect = ( SDL_Rect){ .x = bdx, .y = bdy, .w = adx, .h = ady};
    
    SDL_RenderCopyEx( GW -> renderer, GW -> target, &srect, &drect, 0, NULL, SDL_FLIP_NONE);
    
    srect = ( SDL_Rect){ .x = 0  , .y = by , .w = ax , .h = cy };
    drect = ( SDL_Rect){ .x = bdx, .y = 0  , .w = adx, .h = bdy};
    
    SDL_RenderCopyEx( GW -> renderer, GW -> target, &srect, &drect, 0, NULL, SDL_FLIP_NONE);
    
    srect = ( SDL_Rect){ .x = bx , .y = 0  , .w = cx , .h = ay };
    drect = ( SDL_Rect){ .x = 0  , .y = bdy, .w = bdx, .h = ady};
    
    SDL_RenderCopyEx( GW -> renderer, GW -> target, &srect, &drect, 0, NULL, SDL_FLIP_NONE);
  }
  
  SDL_RenderPresent( GW -> renderer);
      
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
  
  if( GW == NULL){
    goto fault;
  }
  
  GW -> real.width  = GW -> real.width ? GW -> real.width : GW -> mfvid.width;
  GW -> real.height = GW -> real.height? GW -> real.height: GW -> mfvid.height;
  
  GW -> real.realwidth  = GW -> real.realwidth ? GW -> real.realwidth : GW -> real.width  * GW -> real.element_width;
  GW -> real.realheight = GW -> real.realheight? GW -> real.realheight: GW -> real.height * GW -> real.element_height;
  
  GW -> real.width  = ( ( GW -> real.width  * GW -> real.element_width ) <= GW -> real.realwidth )? GW -> real.width : ( GW -> real.realwidth ) / GW -> real.element_width;
  GW -> real.height = ( ( GW -> real.height * GW -> real.element_height) <= GW -> real.realheight)? GW -> real.height: ( GW -> real.realheight) / GW -> real.element_height;
  
  GW -> logical = GW -> real;
  
  GW -> logical.element_width  = 15;
  GW -> logical.element_height = 15;
  
  GW -> logical.width  += 1;
  GW -> logical.height += 1;
  
  GW -> logical.realwidth  = GW -> logical.width  * GW -> logical.element_width;
  GW -> logical.realheight = GW -> logical.height * GW -> logical.element_height;
  
  GW -> mfvid.realwidth  = ( GW -> mfvid.width  * GW -> real.realwidth ) / GW -> real.width ;
  GW -> mfvid.realheight = ( GW -> mfvid.height * GW -> real.realheight) / GW -> real.height;
  
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER)){
    goto fault;
  }
  
  GW -> window = SDL_CreateWindow( "Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				   GW -> real.realwidth, GW -> real.realheight, SDL_WINDOW_HIDDEN);
  
  GW -> renderer = SDL_CreateRenderer( GW -> window, -1, 0);
  
  if( GW -> window == NULL || GW -> renderer == NULL){
    goto fault;
  }
  
  SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize( GW -> renderer, GW -> real.realwidth, GW -> real.realheight);
  SDL_SetRenderDrawColor( GW -> renderer, 0, 0xff, 0, 0xff);
  
  if( GW -> no_resize){
    SDL_SetWindowResizable( GW ->  window, SDL_FALSE);
  }else{
    SDL_SetWindowResizable( GW ->  window, SDL_TRUE);
  }
  
  GW -> target = SDL_CreateTexture( GW -> renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, GW -> logical.realwidth, GW -> logical.realheight);
  
  {
    SDL_Rect rec;
    rec.x = 0;
    rec.y = 0;
    rec.h = GW -> logical.element_width;
    rec.w = GW -> logical.element_height;
    
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
  
  ret = GW;
  fault:
  if( ret == NULL) GW_Free( GW);
  return ret;
}

void
GW_Free( GraphicWraper *GW){
  if( GW != NULL){
    SDL_Quit();
    
    free( GW);
  }
}
