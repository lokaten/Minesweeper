


#ifdef _MS_GW_H__
#else
#define _MS_GW_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "MS_util.h"
  
typedef struct{
  int x;
  int y;
}MS_diff;

  
typedef struct{
  SDL_Surface *videomode;
  SDL_Surface *vfield;
  SDL_Surface *sfield;
  unsigned long global;
  MS_video mfvid;
  MS_video video;
  MS_video shift;
  MS_video fake;
  unsigned long ewidth;
  unsigned long eheight;
  SDL_Surface **drawfield;
  SDL_Surface *cover;
  SDL_Surface *clear;
  SDL_Surface *flag;
  SDL_Surface *mine;
  SDL_Surface *one;
  SDL_Surface *two;
  SDL_Surface *three;
  SDL_Surface *four;
  SDL_Surface *five;
  SDL_Surface *six;
  SDL_Surface *seven;
  SDL_Surface *eight;
}GraphicWraper;


int drawtemp( GraphicWraper *, MS_video, __uint8_t);
GraphicWraper *GW_Create();
void GW_Free( GraphicWraper *);
int window_scroll( GraphicWraper *, MS_diff);
int draw( GraphicWraper *, MS_field);
int window_resize( GraphicWraper *, MS_video);

#ifdef __cplusplus
}
#endif
#endif
