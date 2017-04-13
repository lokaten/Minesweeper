
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


int readincmdline( int, char **, MS_field *, MS_video *, MS_stream *, unsigned long *, unsigned long *);
int mainloop( MS_stream *, MS_field *, GraphicWraper *);
int keypressevent( SDL_Event, MS_field *, MS_stream *, MS_video, MS_diff *);
int keyreleasevent( SDL_Event, MS_diff *);
int swap_flag( MS_field *, int, int);
int pointerpressevent( SDL_Event, MS_field *, MS_video);
int pointerreleasevent( SDL_Event, MS_field *, MS_stream *, MS_video, __uint64_t, __uint64_t);
int pointermoveevent( SDL_Event, MS_field *, MS_video);
void printtime( FILE *, unsigned long);

int
readincmdline( int argv,
               char **argc,
               MS_field *minefield,
               MS_video *video,
               MS_stream *mss,
               unsigned long *no_resize,
               unsigned long *benchmark){
  
  MS_field field_custom    = { .title = "custom"   , .width =    9, .height =    9, .level = 10, .global = 0, .reseed = 0};
  MS_field field_beginner  = { .title = "beginner" , .width =    9, .height =    9, .level = 10, .global = 0, .reseed = 0};
  MS_field field_advanced  = { .title = "advanced" , .width =   16, .height =   16, .level = 40, .global = 0, .reseed = 0};
  MS_field field_expert    = { .title = "expert"   , .width =   30, .height =   16, .level = 99, .global = 0, .reseed = 0};
  MS_field field_benchmark = { .title = "benchmark", .width = 3200, .height = 1800, .level =  1, .global = 1, .reseed = 0};
    
  unsigned long helpopt = 0;
  
  unsigned long xscale = 15;
  unsigned long yscale = 15;
  
  unsigned long quiet = 0;
  unsigned long vquiet = 0;
  
  unsigned long debug = 0;
  unsigned long force  = 0;
  
  MS_field *mfvid;
  
  unsigned long opt_true  = TRUE;
  unsigned long opt_false = FALSE;
  
  mfvid = &field_custom;
  
  MS_options opt[ OPT_MAX] = {
    { OPTSW_GRP, ""                                       , "Options"        , 0  , NULL                       , NULL},
#ifdef DEBUG
    { OPTSW_CPY, "ignore validation of options"           , "force"          , 'f', &( force                  ), &opt_true},
#endif
    { OPTSW_GRP, ""                                       , "Minefield"      , 0  , NULL                       , NULL},
    { OPTSW_LU , "Element wide minefield"                 , "width"          , 0  , &( field_custom.width     ), NULL},
    { OPTSW_LU , "Element high minefield"                 , "height"         , 0  , &( field_custom.height    ), NULL},
    { OPTSW_LU , "Number of mines"                        , "level"          , 0  , &( field_custom.level     ), NULL},
#ifdef DEBUG
    { OPTSW_X  , "Generate Minefield based on this seed"  , "seed"           , 0  , &( field_custom.reseed    ), NULL},
#endif
    { OPTSW_CPY, ""                                       , "global"         , 'g', &( field_custom.global    ), &opt_true},
    { OPTSW_GRP, ""                                       , "Video"          , 0  , NULL                       , NULL},
    { OPTSW_LU , "Pixel wide window"                      , "video-width"    , 0  , &( video -> realwidth     ), NULL},
    { OPTSW_LU , "Pixel high window"                      , "video-height"   , 0  , &( video -> realheight    ), NULL},
    { OPTSW_LU , "Pixel wide Element"                     , "element-width"  , 0  , &( xscale                 ), NULL},
    { OPTSW_LU , "Pixel high Element"                     , "element-height" , 0  , &( yscale                 ), NULL},
    { OPTSW_CPY, "Resize don't work well with all system" , "no-resize"      , 0  , ( no_resize               ), &opt_true},
    { OPTSW_GRP, ""                                       , "Mode"           , 0  , NULL                       , NULL},
    { OPTSW_CPY, "Mimic windows minesweeper beginner mode", "beginner"       , 'b', &mfvid                     , &field_beginner },
    { OPTSW_CPY, "Mimic windows minesweeper advanced mode", "advanced"       , 'a', &mfvid                     , &field_advanced },
    { OPTSW_CPY, "Mimic windows minesweeper expert mode"  , "expert"         , 'e', &mfvid                     , &field_expert   },
#ifdef DEBUG
    { OPTSW_CPY, ""                                       , "benchmark"      , 'B', benchmark                  , &opt_true},
#endif
    { OPTSW_GRP, ""                                       , "Output"         , 0  , NULL                       , NULL},
    { OPTSW_CPY, "Print generic help mesage"              , "help"           , 'h', &( helpopt                ), &opt_true},
    { OPTSW_CPY, "Supres reguler output"                  , "quiet"          , 'q', &( quiet                  ), &opt_true},
    { OPTSW_CPY, "Supres all output"                      , "very-quiet"     , 'Q', &( vquiet                 ), &opt_true},
#ifdef DEBUG
    { OPTSW_CPY, "Debug data"                             , "debug"          , 'd', &( debug                  ), &opt_true},
#endif
    { OPTSW_NUL, "Last elemnt is a NULL termination"      , ""               , 0  , NULL                       , NULL}};
      
  *video = ( MS_video){ .width = 0, .height = 0, .realwidth = 0, .realheight = 0};
    
  if( procopt( mss, opt, argv, argc)){
    if( !vquiet) MS_print( stderr, "\rWRong or broken input, pleas refer to --help\n");
    helpopt = 2;
  }
    
  mss -> out = !quiet && !vquiet? stdout: NULL;
  mss -> err =           !vquiet? stderr: NULL;
  mss -> deb = debug  && !vquiet? stdout: NULL;
      
  if( *benchmark){
    *mfvid = field_benchmark;
    video -> width      = 1;
    video -> height     = 1;
  }
  
  video -> width  = video -> width ? video -> width : mfvid -> width;
  video -> height = video -> height? video -> height: mfvid -> height;
  
  video -> realwidth  = video -> realwidth ? video -> realwidth : video -> width  * xscale;
  video -> realheight = video -> realheight? video -> realheight: video -> height * yscale;
  
  video -> width  = video -> width  * xscale <= video -> realwidth ? video -> width : ( video -> realwidth  + ( xscale - 1)) / xscale;
  video -> height = video -> height * yscale <= video -> realheight? video -> height: ( video -> realheight + ( yscale - 1)) / yscale;
  
  if( mfvid -> level >= ( mfvid -> width * mfvid -> height)){
    mfvid -> level = ( mfvid -> width * mfvid -> height + 1) / 3;
    MS_print( mss -> err, "\rMore mines then elments!\n");
    helpopt = 2;
  }
      
  if( helpopt){
    if( helpopt == 2){
      if( !force){
        help( mss -> out, opt);
        exit( 1);
      }
    }else{
      help( mss -> out, opt);
      exit( 0);
    }
  }
  
  MS_print( mss -> deb, "\rseed is printed when setminefield is called so that you can re run spcific minefield whit help of --seed\n");
  MS_print( mss -> deb, "\rNOTE: user input changes how the minfield is generated.\n");
  
  *minefield = *mfvid;

  MS_print( mss -> out, "\rMode: %s\n", mfvid -> title);
  
  MS_print( mss -> deb, "\rwidth: %lu   ", minefield -> width);
  MS_print( mss -> deb, "\r\t\theight: %lu   ", minefield -> height);
  MS_print( mss -> deb, "\r\t\t\t\tlevel: %lu   \n", minefield -> level);
    
  return 0;
}

int
main( int argv, char** argc){
  int ret;
  GraphicWraper *GW;
  MS_field *minefield;
  MS_stream *mss = ( MS_stream *)malloc( sizeof( MS_stream));
    
  {
    MS_field *mfvid = ( MS_field *)malloc( sizeof( MS_field));
    MS_video video;
        
    unsigned long no_resize = 0;
    unsigned long benchmark = 0;
    
    readincmdline( argv, argc, mfvid, &video, mss, &no_resize, &benchmark);
        
    GW = GW_Create( video, no_resize);
    
    if( GW == NULL){
      exit( 1);
    }
    
#ifdef __cplusplus
#else
    if( benchmark){
      SDL_PushEvent( &( SDL_Event){ .button = ( SDL_MouseButtonEvent){ .type = SDL_MOUSEBUTTONDOWN, .button = SDL_BUTTON_LEFT, .x = 0, .y = 0}});
      SDL_PushEvent( &( SDL_Event){ .button = ( SDL_MouseButtonEvent){ .type = SDL_MOUSEBUTTONUP  , .button = SDL_BUTTON_LEFT, .x = 0, .y = 0}});
      SDL_PushEvent( &( SDL_Event){ .key = ( SDL_KeyboardEvent){ .type = SDL_QUIT}});
    }
#endif
    
    GW -> global = mfvid -> global;
    
    GW -> mfvid.width  = mfvid -> width;
    GW -> mfvid.height = mfvid -> height;
    GW -> mfvid.xdiff  = 0;
    GW -> mfvid.ydiff  = 0;
    
    GW -> mfvid.realwidth  = mfvid -> width  * GW -> ewidth;
    GW -> mfvid.realheight = mfvid -> height * GW -> eheight;
    
    minefield = MF_Create( *mfvid);
        
    setminefield( minefield, mss, GW -> mfvid);
  }
  
  ret = mainloop( mss, minefield, GW);
  
  MF_Free( minefield);
  
  GW_Free( GW);
  
  MS_print( mss -> out, "\nBye!\n");
  
  if( mss -> out == NULL){
    MS_print( mss -> deb, "\n");
  }
  
  exit( ret);
  
  return ret;
}




int
mainloop( MS_stream *mss, MS_field *minefield, GraphicWraper *GW){
  int ret = 0;
  
  SDL_Event event;
  
  __uint64_t tutime, nextframe, gamestart, nexttu;
  
  MS_diff *diff = ( MS_diff *)malloc( sizeof( MS_diff));
  
  __uint32_t seed = MS_rand_seed();
  
  int err = 0;
  
  int e = 0;
  
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
    }else{
      if( minefield -> mine -> uncoverd && !minefield -> mine -> hit && minefield -> mine -> uncoverd < ( minefield -> mine -> noelements - minefield -> mine -> level)){
	MS_print( mss -> out, "\r\t\t\t %lu of %lu      ", minefield -> mine -> flaged, minefield -> mine -> level);
      }else{
	gamestart = tutime;
      }
      
      printtime( mss -> out, ( tutime - gamestart) / 1000000);
      
      /* to make sure the time looks like it updatet consistanly we randomaize
       * the time we wait betwen updating it, with max time betwen update beigen 150ms
       */
      nexttu += 50000000lu + ( ( ( __uint64_t)( seed = MS_rand( seed)) * 100000000lu) / MS_RAND_MAX);
    }
    
    if( e){
      err = 0;
      
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
        if( ( err = pointerpressevent( event, minefield, GW -> logical)) > 0){
          nextframe = tutime;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        if( ( err = pointerreleasevent( event, minefield, mss, GW -> logical, tutime, gamestart)) > 0){
          nextframe = tutime;
        }
        nextframe = tutime;
        break;
      case SDL_MOUSEMOTION:
        if( ( err = pointermoveevent( event, minefield, GW -> logical)) > 0){
          nextframe = tutime;
        }
        break;
      default:
        break;
      }
      
      if unlikely( err == -2){
        MS_print( mss -> err, "\r\t\t\t\t\t\t\t\t\t alloc limet!     \n");
      }
    }
    
    if( nextframe <= tutime && ( ( nextframe == tutime) || ( diff -> x || diff -> y))){
      if( window_scroll( GW, *diff)){
        nextframe += 1000000000 / 30;
      }
                  
      assert( !( ( minefield -> mine -> mines > minefield -> mine -> level) || ( minefield -> mine -> set > ( minefield -> mine -> noelements))));
      
      assert( !( ( minefield -> mine -> set >= minefield -> mine -> noelements) && ( minefield -> mine -> mines < minefield -> mine -> level)));
      
      if unlikely( draw( GW, *minefield) == -3){
        MS_print( mss -> err, "\r\t\t\t\t\t\t\t\t\t inval data \n");
      }
      
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
    swap_flag( minefield, postion.x, postion.y);
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
