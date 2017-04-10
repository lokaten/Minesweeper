


#ifdef _MS_GW_H__
#else
#define _MS_GW_H__
#ifdef __cplusplus
extern "C" {
#endif
  
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
  
#include "MS_util.h"
#include "minefield.h"
  
  typedef struct{
    int x;
    int y;
  }MS_diff;
  
  
  typedef struct{
    SDL_Window *window;
    SDL_Texture *target;
    SDL_Renderer *renderer;
    unsigned long global;
    MS_video mfvid;
    MS_video real;
    MS_video logical;
    unsigned long ewidth;
    unsigned long eheight;
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


  int drawtemp( GraphicWraper *, MS_video, __uint8_t);
  GraphicWraper *GW_Create( MS_video, unsigned long);
  void GW_Free( GraphicWraper *);
  int window_scroll( GraphicWraper *, MS_diff);
  int draw( GraphicWraper *, MS_field);
  
#ifdef __cplusplus
}
#endif
#endif
