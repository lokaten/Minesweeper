
#define LOCALE_( name) GW_##name

#include "MS_util.h"
#include "GW.h"


int MS_OpenImage( SDL_Surface **, SDL_Rect *, char *, __uint32_t);
int MS_BlitSurface( SDL_Surface *, SDL_Surface *, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
SDL_Surface *drawelement( GraphicWraper *, __uint8_t);
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

int MS_OpenImage( SDL_Surface **dst, SDL_Rect *rec, char *str, __uint32_t c){
  SDL_Surface *img;
  if unlikely( str != NULL){
    img = IMG_Load( str);
  }else{
    img = NULL;
  }
  *dst = SDL_CreateRGBSurface( FALSE, ( *rec).h, ( *rec).w, 24, 0xff0000, 0xff00, 0xff, 0x0);
  if unlikely( *dst == NULL){
    return -4;
  }
  SDL_FillRect( *dst, rec, c);
  if unlikely( img == NULL){
    return -3;
  }
  MS_BlitSurface( img, *dst, ( *rec).x, ( *rec).y, 0, 0, ( *rec).w, ( *rec).h);
  SDL_free( img);
  return 0;
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
draw( GraphicWraper *gw, MS_field minefield){
  MS_pos element, elementsh;
  unsigned long bx, by, b2x, b2y;

  SDL_Surface **gui = ( *gw).gui;

  MS_video video = gw -> video;
  MS_video shift = gw -> shift;
  
  SDL_Surface *videomode = ( *gw).videomode;
  SDL_Surface **drawfield = ( *gw).drawfield;
  
  SDL_Surface *sur = gui[ 1];
  
  int ret = 0;
  
  unsigned long i;

  SDL_Surface *tile;
    
  i = shift.width * shift.height;
  
  while( i--){
    
    element.x = ( video.xdiff + ( i % video.width)) % minefield.width;
    element.y = ( video.ydiff + ( i / video.width)) % minefield.height;
    
    elementsh.x = ( ( video.width  + ( ( minefield.width  + ( element.x) - video.xdiff) % minefield.width ) + shift.xdiff) % video.width );
    elementsh.y = ( ( video.height + ( ( minefield.height + ( element.y) - video.ydiff) % minefield.height) + shift.ydiff) % video.height);
    
    tile = drawelement( gw, minefield.data[ element.x + element.y * minefield.width]);
    
    if( drawfield[ elementsh.x + elementsh.y * shift.width] != tile){
      
      MS_BlitSurface( tile, gui[ 0], elementsh.x * ( *gw).ewidth , elementsh.y * ( *gw).eheight, 0, 0, ( *gw).ewidth, ( *gw).eheight);
      
      drawfield[ elementsh.x + elementsh.y * shift.width] = tile;
      
      if( sur != gui[ 0]){
        MS_scale( gui[ 0], gui[ 1],
                  ( elementsh.x * shift.realwidth ) / shift.width , ( elementsh.y * shift.realheight) / shift.height,
                  ( shift.realwidth + shift.width) / shift.width, ( shift.realheight + shift.height) / shift.height);
      }
    }
  }
  
  b2x = shift.realxdiff;
  b2y = shift.realydiff;
  
  bx = video.realwidth  - b2x;
  by = video.realheight - b2y;
  
  MS_BlitSurface( sur, videomode,   0,   0, b2x, b2y, bx , by );
  MS_BlitSurface( sur, videomode,   0,  by, b2x,   0, bx , b2y);
  MS_BlitSurface( sur, videomode,  bx,   0,   0, b2y, b2x, by );
  MS_BlitSurface( sur, videomode,  bx,  by,   0,   0, b2x, b2y);
  
  SDL_Flip( videomode);
    
  return ret;
}


SDL_Surface *
drawelement( GraphicWraper *gui, __uint8_t element){

  if( element & EFLAG){
    return ( *gui).flag;
  }
  
  if( element & ECOVER){
    return ( *gui).cover;
  }
  
  if( element & EMINE){
    return ( *gui).mine;
  }

  switch( ECOUNT & element){
  case EC_ZERO : return ( *gui).clear;
  case EC_ONE  : return ( *gui).one;
  case EC_TWO  : return ( *gui).two;
  case EC_THERE: return ( *gui).three;
  case EC_FOUR : return ( *gui).four;
  case EC_FIVE : return ( *gui).five;
  case EC_SIX  : return ( *gui).six;
  case EC_SEVEN: return ( *gui).seven;
  case EC_EIGHT: return ( *gui).eight;
  /**/
  case 0xf:  return ( *gui).clear;
  default:
    return NULL;
  }
  
  return NULL;
}



void
MS_scale( SDL_Surface *src, SDL_Surface *dst, signed long x, signed long y, unsigned long w, unsigned long h){
  unsigned long sampel = 4;
  unsigned long dw = dst -> w;
  unsigned long dh = dst -> h;
  unsigned long sw = src -> w;
  unsigned long sh = src -> h;
  unsigned long pd = dst -> pitch;
  unsigned long ps = src -> pitch;
  unsigned long i, j;
  unsigned long xd, yd;
  unsigned long xa[ sampel], ya[ sampel];
  unsigned long Bs = src -> format -> BytesPerPixel;
  unsigned long Bd = dst -> format -> BytesPerPixel;
  __uint64_t r, g, b;
  __uint8_t *spixel = ( __uint8_t *)src -> pixels;
  __uint8_t *dpixel = ( __uint8_t *)dst -> pixels;
  x = x > 0? x: 0;
  y = y > 0? y: 0;
  w = w > 1? w: 1;
  h = h > 1? h: 1;
  i = w * h;
  SDL_LockSurface( src);
  SDL_LockSurface( dst);
  while( i--){
    xd = ( x + ( i % w)) % dw;
    yd = ( y + ( i / w)) % dh;
    
    r = 0;
    g = 0;
    b = 0;
        
    j = sampel;
    
    while( j--){
      xa[ j] = ( ( xd * sampel + j) * sw) / ( dw * sampel);
      ya[ j] = ( ( yd * sampel + j) * sh) / ( dh * sampel);
    }
    
    j = sampel * sampel;
    
    while( j--){
      r += spixel[ ( xa[ j % sampel] * Bs + ya[ j / sampel] * ps) + 0];
      g += spixel[ ( xa[ j % sampel] * Bs + ya[ j / sampel] * ps) + 1];
      b += spixel[ ( xa[ j % sampel] * Bs + ya[ j / sampel] * ps) + 2];
    }
    
    dpixel[ ( xd * Bd + yd * pd) + 0] = r / ( sampel * sampel);
    dpixel[ ( xd * Bd + yd * pd) + 1] = g / ( sampel * sampel);
    dpixel[ ( xd * Bd + yd * pd) + 2] = b / ( sampel * sampel);
  }
  SDL_UnlockSurface( src);
  SDL_UnlockSurface( dst);
}


int
window_resize( GraphicWraper *window, MS_video new_video){
  int ret = 0;
  
  if( ( *window).gui[ 1] != ( *window).vfield){
    SDL_FreeSurface( ( *window).gui[ 1]);
  }
  
  ( *window).video.realwidth  = ( ( *window).video.realwidth  * new_video.width ) / ( *window).fake.realwidth;
  ( *window).video.realheight = ( ( *window).video.realheight * new_video.height) / ( *window).fake.realheight;
  
  ( *window).mfvid.realwidth  = ( ( *window).mfvid.realwidth  * new_video.width ) / ( *window).fake.realwidth;
  ( *window).mfvid.realheight = ( ( *window).mfvid.realheight * new_video.height) / ( *window).fake.realheight;
  
  ( *window).video.realxdiff = ( ( *window).video.realxdiff * new_video.width ) / ( *window).fake.realwidth;
  ( *window).video.realydiff = ( ( *window).video.realydiff * new_video.height) / ( *window).fake.realheight;
  
  ( *window).video.xdiff = ( ( *window).video.width  * ( *window).video.realxdiff) / ( *window).video.realwidth;
  ( *window).video.ydiff = ( ( *window).video.height * ( *window).video.realydiff) / ( *window).video.realheight;
  
  ( *window).shift.realwidth  = ( ( *window).shift.realwidth  * new_video.width ) / ( *window).fake.realwidth;
  ( *window).shift.realheight = ( ( *window).shift.realheight * new_video.height) / ( *window).fake.realheight;
  
  ( *window).shift.realxdiff = ( ( *window).shift.realxdiff * new_video.width ) / ( *window).fake.realwidth;
  ( *window).shift.realydiff = ( ( *window).shift.realydiff * new_video.height) / ( *window).fake.realheight;
  
  ( *window).shift.xdiff = ( ( *window).video.width  * ( *window).shift.realxdiff) / ( *window).video.realwidth;
  ( *window).shift.ydiff = ( ( *window).video.height * ( *window).shift.realydiff) / ( *window).video.realheight;
  
  ( *window).fake.realwidth  = new_video.width ;
  ( *window).fake.realheight = new_video.height;
  
  SDL_FreeSurface( ( *window).videomode);
  
  ( *window).videomode = SDL_SetVideoMode( ( *window).fake.realwidth, ( *window).fake.realheight, 24, SDL_DOUBLEBUF | SDL_RESIZABLE);
  
  if( ( *window).videomode == NULL){
    return -2;
  }
  
  if( ( *window).video.realwidth  == ( ( *window).video.width  * 15) &&
      ( *window).video.realheight == ( ( *window).video.height * 15)){
    ( *window).gui[ 1] = ( *window).vfield;
  }else{
    ( *window).gui[ 1] = SDL_CreateRGBSurface( FALSE, ( *window).video.realwidth, ( *window).video.realheight, 24, 0xff0000, 0xff00, 0xff, 0);
  }
  
  if( ( *window).gui[ 1] == NULL){
    return -2;
  }
  
  MS_scale( ( *window).gui[ 0], ( *window).gui[ 1], 0, 0, ( *window).video.realwidth, ( *window).video.realheight);
  
  return ret;
}


GraphicWraper *
GW_Create( MS_video video, MS_video fake, unsigned long no_resize, MS_field minefield){
  GraphicWraper *GW = ( GraphicWraper *)malloc( sizeof( GraphicWraper));

  if( GW == NULL){
    return NULL;
  }

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
  
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTTHREAD)){
    return NULL;
  }
  
  ( *GW).ewidth  = 15;
  ( *GW).eheight = 15;
  
  {
    SDL_Rect rec;
    rec.x = 0;
    rec.y = 0;
    rec.h = ( *GW).ewidth;
    rec.w = ( *GW).eheight;
    
    MS_OpenImage( &( *GW).clear, &rec, "nola.png",  0xeeeeee);
    MS_OpenImage( &( *GW).one  , &rec, "etta.png",  0x0000ff);
    MS_OpenImage( &( *GW).two  , &rec, "tvaa.png",  0x00ff00);
    MS_OpenImage( &( *GW).three, &rec, "trea.png",  0xff0000);
    MS_OpenImage( &( *GW).four , &rec, "fyra.png",  0xcccc00);
    MS_OpenImage( &( *GW).five , &rec, "fema.png",  0xbb0044);
    MS_OpenImage( &( *GW).six  , &rec, "sexa.png",  0x00ffff);
    MS_OpenImage( &( *GW).seven, &rec, "sjua.png",  0xbbbbbb);
    MS_OpenImage( &( *GW).eight, &rec, "ataa.png",  0x666666);
    
    MS_OpenImage( &( *GW).mine , &rec, "mina.png",  0xffaa77);
    MS_OpenImage( &( *GW).cover, &rec, "plata.png", 0x888888);
    MS_OpenImage( &( *GW).flag , &rec, "flaga.png", 0xffff00);
  }
  
  
  ( *GW).vfield = SDL_CreateRGBSurface( FALSE, video.width * ( *GW).ewidth, video.height * ( *GW).eheight, 24, 0xff0000, 0xff00, 0xff, 0);
  
  if( ( *GW).vfield == NULL){
    goto fault;
  }
  
  ( *GW).gui[ 0] = ( *GW).vfield;
  
  if( video.realwidth  == ( video.width  * ( *GW).ewidth ) &&
      video.realheight == ( video.height * ( *GW).eheight)){
    ( *GW).gui[ 1] = ( *GW).vfield;
  }else{
    ( *GW).gui[ 1] = SDL_CreateRGBSurface( FALSE, video.realwidth, video.realheight, 24, 0xff0000, 0xff00, 0xff, 0);
  }
  
  if( ( *GW).gui[ 1] == NULL){
    goto fault;
  }
    
  ( *GW).drawfield = ( SDL_Surface **)malloc( sizeof( SDL_Surface *) * ( *GW).shift.width * ( *GW).shift.height);
  
  if( ( *GW).drawfield == NULL){
    goto fault;
  }
  
  memset( ( *GW).drawfield, 0, ( *GW).shift.width * ( *GW).shift.height * sizeof( SDL_Surface *));
  
  if( no_resize){
    ( *GW).videomode = SDL_SetVideoMode( fake.realwidth, fake.realheight, 24, SDL_DOUBLEBUF);
  }else{
    ( *GW).videomode = SDL_SetVideoMode( fake.realwidth, fake.realheight, 24, SDL_DOUBLEBUF | SDL_RESIZABLE);
  }
  
  if( ( *GW).videomode == NULL){
    goto fault;
  }
  
  SDL_EventState( SDL_ACTIVEEVENT  , SDL_IGNORE);
  SDL_EventState( SDL_JOYAXISMOTION, SDL_IGNORE);
  SDL_EventState( SDL_JOYBALLMOTION, SDL_IGNORE);
  SDL_EventState( SDL_JOYBUTTONDOWN, SDL_IGNORE);
  SDL_EventState( SDL_JOYBUTTONUP  , SDL_IGNORE);
  SDL_EventState( SDL_USEREVENT    , SDL_IGNORE);
  SDL_EventState( SDL_SYSWMEVENT   , SDL_IGNORE);
  
  if( no_resize) SDL_EventState( SDL_VIDEORESIZE, SDL_IGNORE);

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
    SDL_FreeSurface( ( *GW).videomode);
    
    SDL_FreeSurface( ( *GW).gui[ 0]);
    if( ( *GW).gui[ 0] != ( *GW).gui[ 1]){
      SDL_FreeSurface( ( *GW).gui[ 1]);
    }
    SDL_FreeSurface( ( *GW).clear);
    SDL_FreeSurface( ( *GW).one  );
    SDL_FreeSurface( ( *GW).two  );
    SDL_FreeSurface( ( *GW).three);
    SDL_FreeSurface( ( *GW).four );
    SDL_FreeSurface( ( *GW).five );
    SDL_FreeSurface( ( *GW).six  );
    SDL_FreeSurface( ( *GW).seven);
    SDL_FreeSurface( ( *GW).eight);
    SDL_FreeSurface( ( *GW).mine );
    SDL_FreeSurface( ( *GW).cover);
    SDL_FreeSurface( ( *GW).flag );
    
    SDL_Quit();
    
    free( GW);
  }
}
