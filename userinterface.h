


#ifdef MS_GW_H__
#else
#define MS_GW_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
  
#include "MS_util.h"
#include "minefield.h"
  
  typedef struct{
    s32 x;
    s32 y;
  }MS_diff;
  
  
  typedef struct{
    SDL_Window *window;
    SDL_Texture *target;
    SDL_Renderer *renderer;
    MS_video mfvid;
    MS_video real;
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
    u16 global;
    u16 no_resize;
  }GraphicWraper;

  typedef struct{
    int ( *func)( void *);
    void *data;
  }action;
  
  typedef struct{
    const int *argc;
    const char ***argv;
    GraphicWraper *GW;
    MS_field *minefield;
    MS_stream *mss;
    ComandStream *actionque;
    SDL_Event event;
    u64 tutime;
    u64 nextframe;
    u64 gamestart;
    u64 nexttu;
    _Bool gameover;
    MS_diff *diff;
    u32 seed;
    int( *quit)( void *);
  }MS_root;
  
  
  static inline int
  LOCALE_( take_action)( ComandStream *actionque, int ( *func)( void *), void *data){
    int ret = 0;
    action *pact;
    pact = ( action *)CS_Fetch( actionque);
    pact -> func = func;
    pact -> data = data;
    CS_Push( actionque, pact);
    return ret;
  }
#define take_action LOCALE_( take_action)
  
  int event_dispatch( void *);
  int drawtemp( GraphicWraper *, MS_video, __uint8_t);
  GraphicWraper *GW_Init( GraphicWraper *);
  void GW_Free( GraphicWraper *);
  int window_scroll( GraphicWraper *, MS_diff);
  int draw( GraphicWraper *, MS_field);
  
#ifdef __cplusplus
}
#endif
#endif
