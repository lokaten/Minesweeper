
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <limits.h>

#define LOCALE_( name) MAIN_##name

#include "MS_util.h"
#include "ComandStream.h"
#include "GW.h"
#include "minefield.h"
#include "OPT.h"

void quit( void);

void
quit( void){
  SDL_Event key = ( SDL_Event){ .key = ( SDL_KeyboardEvent){ .type = SDL_QUIT}};
  SDL_PushEvent( &key);
}

typedef struct{
  const int *argv;
  const char ***argc;
  GraphicWraper *GW;
  MS_field *minefield;
  MS_stream *mss;
}MS_root;

int readincmdline( MS_root *);
int mainloop( MS_stream *, MS_field *, GraphicWraper *);
int keypressevent( SDL_Event, MS_field *, MS_stream *, MS_video, MS_diff *);
int keyreleasevent( SDL_Event, MS_diff *);
int swap_flag( MS_field *, int, int);
int pointerpressevent( SDL_Event, MS_field *, MS_video);
int pointerreleasevent( SDL_Event, MS_field *, MS_stream *, MS_video, __uint64_t, __uint64_t);
int pointermoveevent( SDL_Event, MS_field *, MS_video);
void printtime( FILE *, unsigned long);

int
readincmdline( MS_root *root){
  
  MS_field *field_custom    = MS_Create( MS_field, ( ( MS_field){ .title = "custom"   , .width =    9, .height =    9, .level = 10, .global = 0, .reseed = 0}));
  MS_field *field_beginner  = MS_Create( MS_field, ( ( MS_field){ .title = "beginner" , .width =    9, .height =    9, .level = 10, .global = 0, .reseed = 0}));
  MS_field *field_advanced  = MS_Create( MS_field, ( ( MS_field){ .title = "advanced" , .width =   16, .height =   16, .level = 40, .global = 0, .reseed = 0}));
  MS_field *field_expert    = MS_Create( MS_field, ( ( MS_field){ .title = "expert"   , .width =   30, .height =   16, .level = 99, .global = 0, .reseed = 0}));
  MS_field *field_benchmark = MS_Create( MS_field, ( ( MS_field){ .title = "benchmark", .width = 3200, .height = 1800, .level =  1, .global = 1, .reseed = 0}));
  
  unsigned long xscale = 15;
  unsigned long yscale = 15;
  
  unsigned long opt_true  = TRUE;
  unsigned long opt_false = FALSE;
  
  root -> GW = MS_CreateEmpty( GraphicWraper);
  
  MS_options opt[] = {
    { OPTSW_GRP, ""                                       , "Options"        , 0  , NULL                       , NULL},
    { OPTSW_GRP, ""                                       , "Minefield"      , 0  , NULL                       , NULL},
    { OPTSW_LU , "Element wide minefield"                 , "width"          , 0  , &( field_custom -> width  ), NULL},
    { OPTSW_LU , "Element high minefield"                 , "height"         , 0  , &( field_custom -> height ), NULL},
    { OPTSW_LU , "Number of mines"                        , "level"          , 0  , &( field_custom -> level  ), NULL},
#ifdef DEBUG
    { OPTSW_X  , "Generate Minefield based on this seed"  , "seed"           , 0  , &( field_custom -> reseed ), NULL},
    { OPTSW_CPY, ""                                       , "global"         , 'g', &( field_custom -> global ), &opt_true},
#endif
    { OPTSW_GRP, ""                                       , "Video"          , 0  , NULL                             , NULL},
    { OPTSW_LU , "Pixel wide window"                      , "video-width"    , 0  , &( root -> GW -> real.realwidth ), NULL},
    { OPTSW_LU , "Pixel high window"                      , "video-height"   , 0  , &( root -> GW -> real.realheight), NULL},
    { OPTSW_LU , "Pixel wide Element"                     , "element-width"  , 0  , &( xscale                       ), NULL},
    { OPTSW_LU , "Pixel high Element"                     , "element-height" , 0  , &( yscale                       ), NULL},
    { OPTSW_CPY, "Resize don't work well with all system" , "no-resize"      , 'R', &( root -> GW -> no_resize      ), &opt_true},
    { OPTSW_GRP, ""                                       , "Mode"           , 0  , NULL                       , NULL},
    { OPTSW_CPY, "Mimic windows minesweeper beginner mode", "beginner"       , 'b', &root -> minefield         , field_beginner },
    { OPTSW_CPY, "Mimic windows minesweeper advanced mode", "advanced"       , 'a', &root -> minefield         , field_advanced },
    { OPTSW_CPY, "Mimic windows minesweeper expert mode"  , "expert"         , 'e', &root -> minefield         , field_expert   },
#ifdef DEBUG
    { OPTSW_CPY, ""                                       , "benchmark"      , 'B', &root -> minefield         , field_benchmark},
#endif
    { OPTSW_GRP, ""                                       , "Output"         , 0  , NULL                       , NULL},
    { OPTSW_CPY, "Print generic help mesage"              , "help"           , 'h', &root -> mss -> hlp        , stdout},
    { OPTSW_CPY, "Supres reguler output"                  , "quiet"          , 'q', &root -> mss -> out        , NULL},
    { OPTSW_CPY, "Supres all output"                      , "very-quiet"     , 'Q', &root -> mss -> err        , NULL},
#ifdef DEBUG
    { OPTSW_CPY, "Debug data"                             , "debug"          , 'd', &root -> mss -> deb        , stdout},
#endif
    { OPTSW_NUL, "Last elemnt is a NULL termination"      , ""               , 0  , NULL                       , NULL}};
      
  root -> GW -> real = ( MS_video){ .width = 0, .height = 0, .realwidth = 0, .realheight = 0};
  
  *root -> mss = ( MS_stream){ .out = stdout, .err = stderr, .deb = NULL, .hlp = NULL};
  
  root -> minefield = field_custom;
  
  if( procopt( root -> mss, opt, *root -> argv, *root -> argc)){
    MS_print( root -> mss -> err, "\rWRong or broken input, pleas refer to --help\n");
  }
  
  root -> mss -> out = root -> mss -> err? root -> mss -> out: NULL;
  
  root -> GW -> real = root -> minefield == field_benchmark? ( MS_video){ .width = 1,  .height = 1}: root -> GW -> real;
  
  root -> GW -> real.width  = root -> GW -> real.width ? root -> GW -> real.width : root -> minefield -> width;
  root -> GW -> real.height = root -> GW -> real.height? root -> GW -> real.height: root -> minefield -> height;
  
  root -> GW -> real.realwidth  = root -> GW -> real.realwidth ? root -> GW -> real.realwidth : root -> GW -> real.width  * xscale;
  root -> GW -> real.realheight = root -> GW -> real.realheight? root -> GW -> real.realheight: root -> GW -> real.height * yscale;
  
  root -> GW -> real.width  = root -> GW -> real.width  * xscale <= root -> GW -> real.realwidth ? root -> GW -> real.width : ( root -> GW -> real.realwidth  + ( xscale - 1)) / xscale;
  root -> GW -> real.height = root -> GW -> real.height * yscale <= root -> GW -> real.realheight? root -> GW -> real.height: ( root -> GW -> real.realheight + ( yscale - 1)) / yscale;
  
  if( root -> minefield -> level >= ( root -> minefield -> width * root -> minefield -> height)){
    root -> minefield -> level = ( root -> minefield -> width * root -> minefield -> height + 1) / 3;
    MS_print( root -> mss -> err, "\rMore mines then elments!\n");
  }

  if( root -> mss -> hlp){
    help( root -> mss -> hlp, opt);
    exit( 0);
  }
  
  MS_print( root -> mss -> deb, "\rseed is printed when setminefield is called so that you can re run spcific minefield whit help of --seed\n");
  MS_print( root -> mss -> deb, "\rNOTE: user input changes how the minfield is generated.\n");
  
  MS_print( root -> mss -> out, "\rMode: %s\n", root -> minefield -> title);
  
  MS_print( root -> mss -> deb, "\rwidth: %lu   ", root -> minefield -> width);
  MS_print( root -> mss -> deb, "\r\t\theight: %lu   ", root -> minefield -> height);
  MS_print( root -> mss -> deb, "\r\t\t\t\tlevel: %lu   \n", root -> minefield -> level);

  if( root -> minefield != field_beginner ) free( field_beginner );
  if( root -> minefield != field_advanced ) free( field_advanced );
  if( root -> minefield != field_expert   ) free( field_expert   );
  if( root -> minefield != field_benchmark) free( field_benchmark);
  
  return 0;
}

int
main( const int argv, const char** argc){
  int ret = -1;
  MS_root *root = MS_CreateEmpty( MS_root);
  
  root -> argv = &argv;
  root -> argc = &argc;
  root -> mss = MS_CreateEmpty( MS_stream);
  
  {
    MS_video video;
        
    unsigned long no_resize = 0;
    unsigned long benchmark = 0;
    
    readincmdline( root);
    
    if( ( root -> GW = GW_Init( root -> GW)) == NULL){
      goto fault;
    }
    
#ifdef __cplusplus
#else
    if( root -> minefield -> title == ( const char *)"benchmark"){
      SDL_PushEvent( &( SDL_Event){ .button = ( SDL_MouseButtonEvent){ .type = SDL_MOUSEBUTTONDOWN, .button = SDL_BUTTON_LEFT, .x = 0, .y = 0}});
      SDL_PushEvent( &( SDL_Event){ .button = ( SDL_MouseButtonEvent){ .type = SDL_MOUSEBUTTONUP  , .button = SDL_BUTTON_LEFT, .x = 0, .y = 0}});
      SDL_PushEvent( &( SDL_Event){ .key = ( SDL_KeyboardEvent){ .type = SDL_QUIT}});
    }
#endif
    
    root -> GW -> global = root -> minefield -> global;
    
    root -> GW -> mfvid.width  = root -> minefield -> width;
    root -> GW -> mfvid.height = root -> minefield -> height;
    root -> GW -> mfvid.xdiff  = 0;
    root -> GW -> mfvid.ydiff  = 0;
    
    root -> GW -> mfvid.realwidth  = ( root -> GW -> mfvid.width  * root -> GW -> real.realwidth ) / root -> GW -> real.width ;
    root -> GW -> mfvid.realheight = ( root -> GW -> mfvid.height * root -> GW -> real.realheight) / root -> GW -> real.height;
    
    if( ( root -> minefield = MF_Init( root -> minefield)) == NULL){
      goto fault;
    }
        
    setminefield( root -> minefield, root -> mss, root -> GW -> mfvid);
  }
  
  ret = mainloop( root -> mss, root -> minefield, root -> GW);

 fault:
  MF_Free( root -> minefield);
  
  GW_Free( root -> GW);
  
  MS_print( root -> mss -> out, "\nBye!\n");
  
  if( root -> mss -> out == NULL){
    MS_print( root -> mss -> deb, "\n");
  }
  
  free( root -> mss);

  free( root);
  
  exit( ret);
  
  return ret;
}




int
mainloop( MS_stream *mss, MS_field *minefield, GraphicWraper *GW){
  int ret = 0;
  
  SDL_Event event;
  
  __uint64_t tutime, nextframe, gamestart, nexttu;
  
  MS_diff *diff = ( MS_diff *)malloc( sizeof( MS_diff));
  
  unsigned long seed = MS_rand_seed();
  
  int err = 0, e;
    
  diff -> x = 0;
  diff -> y = 0;
  
  tutime = getnanosec();
  gamestart = tutime;
  nextframe = tutime;
  nexttu = tutime;
      
  while( TRUE){
    
    if( nexttu > tutime){
      e = SDL_WaitEventTimeout( &event, ( nexttu - tutime) / 1000000);
      
      tutime = getnanosec();
      if( !minefield -> mine -> uncoverd){
        gamestart = tutime;
      }
      
      if( e){
        switch( expect( event.type, SDL_MOUSEBUTTONDOWN)){
        case SDL_QUIT:
          ret = 0;
          goto bail;
        case SDL_KEYDOWN:
          if( ( err = keypressevent( event, minefield, mss, GW -> mfvid, diff)) > 0){
            nextframe = tutime;
          }
          break;
        case SDL_KEYUP:
          if( ( err = keyreleasevent( event, diff)) > 0){
            nextframe = tutime;
          }
          if( diff -> x || diff -> y){
            nextframe = tutime;
          }
          break;
        case SDL_MOUSEBUTTONDOWN:
          if( ( err = pointerpressevent( event, minefield, GW -> real)) > 0){
            nextframe = tutime;
          }
          break;
        case SDL_MOUSEBUTTONUP:
          if( ( err = pointerreleasevent( event, minefield, mss, GW -> real, tutime, gamestart)) > 0){
            nextframe = tutime;
          }
          break;
        case SDL_MOUSEMOTION:
          if( ( err = pointermoveevent( event, minefield, GW -> real)) > 0){
            nextframe = tutime;
          }
          break;
        default:
          break;
        }
      
        if unlikely( err == -2){
          MS_print( mss -> err, "\r\t\t\t\t\t\t\t\t\t alloc limet!     ");
          err = 0;
        }
      }
    }else{
      if( minefield -> mine -> uncoverd && !minefield -> mine -> hit && minefield -> mine -> uncoverd < ( minefield -> mine -> noelements - minefield -> mine -> level)){
	MS_print( mss -> out, "\r\t\t\t %lu of %lu      ", minefield -> mine -> flaged, minefield -> mine -> level);
      }else{
	gamestart = tutime;
      }
      
      printtime( mss -> out, ( tutime - gamestart) / 1000000);
      
      nexttu = getnanosec();
      
      /* to make sure the time looks like it updatet consistanly we randomaize
       * the time we wait betwen updating it, with max time betwen update beigen 150ms
       */
      nexttu += 50000000lu + ( ( ( __uint64_t)( seed = MS_rand( seed)) * 100000000lu) / MS_RAND_MAX);
    }
    
    
    if( ( nextframe == tutime) || ( ( nextframe < tutime) && ( diff -> x || diff -> y))){
      if( window_scroll( GW, *diff)){
        nextframe += 1000000000 / 30;
      }
      
      assert( !( ( minefield -> mine -> mines > minefield -> mine -> level) || ( minefield -> mine -> set > ( minefield -> mine -> noelements))));
      
      assert( !( ( minefield -> mine -> set >= minefield -> mine -> noelements) && ( minefield -> mine -> mines < minefield -> mine -> level)));
      
      if unlikely( draw( GW, *minefield) == -3){
        MS_print( mss -> err, "\r\t\t\t\t\t\t\t\t\t inval data   ");
      }

      nexttu = getnanosec();
      
      if( mss -> deb != NULL){
        __uint64_t mytime = getnanosec() - tutime;
        
        MS_print( mss -> deb, "\r\t\t\t\t\t\t\t %lu.%09lu      ", ( unsigned long)( ( mytime) / 1000000000), ( unsigned long)( ( mytime) % 1000000000));
      }
    }
  }

 bail:
  
  if( diff != NULL)free( diff);
  
  return ret;
}


int
keypressevent( SDL_Event event,
               MS_field *minefield,
               MS_stream *mss,
               MS_video mfvid,
               MS_diff *diff){
  int ret = 0;
  unsigned e =  event.key.keysym.sym;
    
  switch( e){
  case SDLK_ESCAPE:
    quit();
    return 0;
  case SDLK_F2:
    if( minefield -> mine -> uncoverd || minefield -> mine -> flaged){
      setminefield( minefield, mss, mfvid);
      ret = 1;
    }
    break;
  case SDLK_F3:
    if( minefield -> mine -> uncoverd < ( minefield -> mine -> noelements - minefield -> mine -> flaged)){
      ret = uncov_elements( *minefield, minefield -> uncovque, mfvid, minefield -> mine);
    }
    if unlikely( uncov( minefield)){
      ret = -2;
    }
    break;
  default:
    break;
  }
  
  if( e == SDLK_h || e == SDLK_LEFT){
    diff -> x -= 3;
    diff -> x = diff -> x < -3? -3: diff -> x;
    ret = 1;
  }
  if( e == SDLK_j || e == SDLK_DOWN){
    diff -> y += 3;
    diff -> y = ( *diff).y > 3? 3: diff -> y;
    ret = 1;
  }
  if( e == SDLK_k || e == SDLK_UP){
    diff -> y -= 3;
    diff -> y = diff -> y < -3? -3: diff -> y;
    ret =  1;
  }
  if( e == SDLK_l || e == SDLK_RIGHT){
    diff -> x += 3;
    diff -> x = ( *diff).x > 3? 3: diff -> x;
    ret = 1;
  }

  return ret;
}


int
keyreleasevent( SDL_Event event,
                MS_diff *diff){
  int ret = 0;
  unsigned e =  event.key.keysym.sym;
      
  if( e == SDLK_h || e == SDLK_LEFT){
    diff -> x += 3;
    diff -> x = diff -> x < 0? 0: diff -> x;
  }
  if( e == SDLK_j || e == SDLK_DOWN){
    diff -> y -= 3;
    diff -> y = diff -> y > 0? 0: diff -> y;
  }
  if( e == SDLK_k || e == SDLK_UP){
    diff -> y += 3;
    diff -> y = diff -> y < 0? 0: diff -> y;
  }
  if( e == SDLK_l || e == SDLK_RIGHT){
    diff -> x -= 3;
    diff -> x = diff -> x > 0? 0: diff -> x;
  }

  return ret;
}


int
swap_flag( MS_field *minefield, int x, int y){
  int ret = 0;
  __uint8_t *element = acse( *minefield, x, y);
  if( *element & EFLAG){
    *element &= ~EFLAG;
    --minefield -> mine -> flaged;
    ret = 1;
  }else if( *element & ECOVER){
    *element|= EFLAG;
    ++minefield -> mine -> flaged;
    ret = 1;
  }
  
  return ret;
}


int
pointerpressevent( SDL_Event event,
                   MS_field *minefield,
                   MS_video video){
  int ret = 0;
  MS_pos postion;
  MS_video vid;
  
  postion.x = ( ( unsigned long)( ( ( event.button.x + video.realxdiff) * video.width ) / video.realwidth )) % minefield -> width;
  postion.y = ( ( unsigned long)( ( ( event.button.y + video.realydiff) * video.height) / video.realheight)) % minefield -> height;
  
  switch( event.button.button){
  case SDL_BUTTON_LEFT:
  case SDL_BUTTON_MIDDLE:
    vid = ( MS_video){ .xdiff = postion.x, .ydiff = postion.y, .width  = 1, .height = 1};
    
    if( event.button.button == SDL_BUTTON_MIDDLE){
      vid = ( MS_video){ .xdiff = postion.x - 1, .ydiff = postion.y - 1, .width  = 3, .height = 3};
    }
    
    ret = uncov_elements( *minefield, minefield -> uncovque, vid, minefield -> mine);
    break;
  case SDL_BUTTON_RIGHT:
    ret = swap_flag( minefield, postion.x, postion.y);
    break;
  default:
    break;
  }
  
  return ret;
}


int
pointerreleasevent( SDL_Event event,
                    MS_field *minefield,
                    MS_stream *mss,
                    MS_video video,
                    __uint64_t tutime,
                    __uint64_t gamestart){
  int ret = 0;
  MS_pos postion, *el;
  MS_video vid;
  
  MS_video mfvid = { .xdiff = 0, .ydiff = 0, .width  = minefield -> subwidth, .height = minefield -> subheight};
  
  postion.x = ( ( unsigned long)( ( ( event.button.x + video.realxdiff) * video.width ) / video.realwidth )) % minefield -> width;
  postion.y = ( ( unsigned long)( ( ( event.button.y + video.realydiff) * video.height) / video.realheight)) % minefield -> height;
  
  switch( event.button.button){
  case SDL_BUTTON_LEFT:
  case SDL_BUTTON_MIDDLE:
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
      setzero( *minefield, minefield -> mine, vid);
    }
    
    ret += uncov_elements( *minefield, minefield -> uncovque, vid, minefield -> mine);
    
    if unlikely( uncov( minefield)){
      ret = -2;
    }
    
    if unlikely( minefield -> mine -> hit){
      printtime( mss -> out, ( tutime - gamestart) / 1000000);
      MS_print( mss -> out, "\r\t\t\t Mine!!               \n");
      uncov_elements( *minefield, minefield -> uncovque, mfvid, minefield -> mine);
      
      if unlikely( uncov( minefield)){
	ret = -2;
      }
    }
    
    if unlikely( !minefield -> mine -> hit && ( minefield -> mine -> uncoverd == ( minefield -> mine -> noelements - minefield -> mine -> level))){
      printtime( mss -> out, ( tutime - gamestart) / 1000000);
      MS_print( mss -> out, "\r\t\t\t Win!!         \n");
    }
    
  default:
    break;
  }
  return ret;
}


int
pointermoveevent( SDL_Event event,
                  MS_field *minefield,
                  MS_video video){
  int ret = 0;
  MS_pos postion, prv_pos;
  
  unsigned long pos, pps;
  
  MS_video vid;
    
  postion.x = ( ( unsigned long)( ( ( event.motion.x + video.realxdiff) * video.width ) / video.realwidth )) % minefield -> width;
  postion.y = ( ( unsigned long)( ( ( event.motion.y + video.realydiff) * video.height) / video.realheight)) % minefield -> height;
  
  prv_pos.x = ( ( unsigned long)( ( ( event.motion.x - event.motion.xrel + video.realxdiff) * video.width ) / video.realwidth )) % minefield -> width;
  prv_pos.y = ( ( unsigned long)( ( ( event.motion.y - event.motion.yrel + video.realydiff) * video.height) / video.realheight)) % minefield -> height;
  
  pos = postion.x + postion.y * minefield -> width;
  
  pps = prv_pos.x + prv_pos.y * minefield -> width;
  
  if( pos != pps){
    
    if( event.motion.state & SDL_BUTTON_LMASK){
      vid = ( MS_video){ .xdiff = postion.x, .ydiff = postion.y, .width  = 1, .height = 1};
      ret += uncov_elements( *minefield, minefield -> uncovque, vid, minefield -> mine);
    }
    
    if( event.motion.state & SDL_BUTTON_MMASK){
      vid = ( MS_video){ .xdiff = postion.x - 1, .ydiff = postion.y - 1, .width  = 3, .height = 3};
      ret += uncov_elements( *minefield, minefield -> uncovque, vid, minefield -> mine);
    }
    
    if( event.motion.state & SDL_BUTTON_RMASK){
      ret += swap_flag( minefield, postion.x, postion.y);
      ret += swap_flag( minefield, prv_pos.x, prv_pos.y);
    }
  }
  
  return ret;
}


void
printtime( FILE * stream, unsigned long time){
  if( time > 3600000lu){
    MS_print( stream, "\r%lu:",    ( ( time) / 3600000)       );
    MS_print( stream, "%02lu:",    ( ( time) / 60000  ) % 60  );
    MS_print( stream, "%02lu.",    ( ( time) / 1000   ) % 60  );
    MS_print( stream, "%03lu    ", ( ( time)          ) % 1000);
  }else{
    if( time > 60000lu){
      MS_print( stream, "\r%lu:",    ( ( time) / 60000  ) % 60  );
      MS_print( stream, "%02lu.",    ( ( time) / 1000   ) % 60  );
      MS_print( stream, "%03lu    ", ( ( time) / 1      ) % 1000);
    }else{
      MS_print( stream, "\r%lu.",    ( ( time) / 1000   ) % 60  );
      MS_print( stream, "%03lu    ", ( ( time) / 1      ) % 1000);
    }
  }
  return;
}
