
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
int keypressevent( SDL_Event, MS_stream, MS_field, MS_video, MS_diff *, ComandStream *, MS_mstr *);
int keyreleasevent( SDL_Event, MS_field, MS_video, MS_diff *, ComandStream *, MS_mstr *);
int swap_flag( MS_field, int, int, MS_mstr *);
int pointerpressevent( SDL_Event, GraphicWraper *,MS_video, MS_field, ComandStream *, MS_mstr *);
int pointerreleasevent( SDL_Event, MS_stream, MS_video, MS_field, ComandStream *, MS_mstr *, __uint64_t, __uint64_t);
int pointermoveevent( SDL_Event, GraphicWraper *,MS_video, MS_field, ComandStream *, MS_mstr *);
void printtime( FILE *, unsigned long);

int
readincmdline( int argv,
               char **argc,
               MS_field *mfvid,
               MS_video *video,
               MS_stream *mss,
               unsigned long *no_resize,
               unsigned long *benchmark){
  
  unsigned long expert = 0;
  unsigned long advanced = 0;
  unsigned long beginner = 0;
  
  unsigned long helpopt = 0;
  
  unsigned long xscale = 15;
  unsigned long yscale = 15;
  
  unsigned long quiet = 0;
  unsigned long vquiet = 0;
  
  unsigned long debug = 0;
  unsigned long force  = 0;
  
  /* put all comand line option in an array ( C99?) 
   */
  MS_options opt[ OPT_MAX] = {
    { OPTSW_GRP, ""                                       , "Options"        , 0  , NULL                   },
#ifdef DEBUG
    { OPTSW_BO , "ignore validation of options"           , "force"          , 'f', &( force              )},
#endif
    { OPTSW_GRP, ""                                       , "Minefield"      , 0  , NULL                   },
    { OPTSW_LU , "Element wide minefield"                 , "width"          , 0  , &( mfvid -> width     )},
    { OPTSW_LU , "Element high minefield"                 , "height"         , 0  , &( mfvid -> height    )},
    { OPTSW_LU , "Number of mines"                        , "level"          , 0  , &( mfvid -> level     )},
#ifdef DEBUG
    { OPTSW_X  , "Generate Minefield based on this seed"  , "seed"           , 0  , &( mfvid -> reseed    )},
#endif
    { OPTSW_BO , ""                                       , "global"         , 'g', &( mfvid -> global    )},
    { OPTSW_GRP, ""                                       , "Video"          , 0  , NULL                   },
    { OPTSW_LU , "Pixel wide window"                      , "video-width"    , 0  , &( video -> realwidth )},
    { OPTSW_LU , "Pixel high window"                      , "video-height"   , 0  , &( video -> realheight)},
    { OPTSW_LU , "Pixel wide Element"                     , "element-width"  , 0  , &( xscale             )},
    { OPTSW_LU , "Pixel high Element"                     , "element-height" , 0  , &( yscale             )},
    { OPTSW_BO , "Resize don't work well with all system" , "no-resize"      , 0  , ( no_resize           )},
    { OPTSW_GRP, ""                                       , "Mode"           , 0  , NULL                   },
    { OPTSW_BO , "Mimic windows minesweeper beginner mode", "beginner"       , 'b', &( beginner           )},
    { OPTSW_BO , "Mimic windows minesweeper advanced mode", "advanced"       , 'a', &( advanced           )},
    { OPTSW_BO , "Mimic windows minesweeper expert mode"  , "expert"         , 'e', &( expert             )},
#ifdef DEBUG
    { OPTSW_BO , ""                                       , "benchmark"      , 'B',  benchmark             },
#endif
    { OPTSW_GRP, ""                                       , "Output"         , 0  , NULL                   },
    { OPTSW_BO , "Print generic help mesage"              , "help"           , 'h', &( helpopt            )},
    { OPTSW_BO , "Supres reguler output"                  , "quiet"          , 'q', &( quiet              )},
    { OPTSW_BO , "Supres all output"                      , "very-quiet"     , 'Q', &( vquiet             )},
#ifdef DEBUG
    { OPTSW_BO , "Debug data"                             , "debug"          , 'd', &( debug              )},
#endif
    { OPTSW_NUL, "Last elemnt is a NULL termination"      , ""               , 0  , NULL                   }};
  
  
  mss -> err = stderr;
  mss -> out = stdout;
  mss -> deb = NULL;
  
  
  mfvid -> width  = 9;
  mfvid -> height = 9;
    
  mfvid -> level  = 10;
  mfvid -> global = 0;
  mfvid -> reseed = 0;
  
  video -> width  = 0;
  video -> height = 0;
  video -> realwidth  = 0;
  video -> realheight = 0;
  
  if( procopt( mss, opt, argv, argc)){
    if( !vquiet) MS_print( mss -> err, "\rWRong or broken input, pleas refer to --help\n");
    helpopt = 2;
  }
  
  
  if( !debug){
    mss -> deb = NULL;
  }
  
  if( vquiet){
    mss -> out = NULL;
    mss -> err = NULL;
  }else{
    if( debug){
      mss -> deb = stdout;
    }
  }
  
  
  if( quiet){
    mss -> out = NULL;
  }

  if( beginner){
    if( advanced || expert){
      MS_print( mss -> err, "\rOpptions --expert, --beginner and --advanced are not compatible with each other.\n");
      helpopt = 2;
    }
    mfvid -> width  =  9;
    mfvid -> height =  9;
    mfvid -> level  = 10;
  }
  
  if( advanced){
    if( beginner || expert){
      MS_print( mss -> err, "\rOpptions --expert, --beginner and --advanced are not compatible with each other.\n");
      helpopt = 2;
    }
    mfvid -> width  = 16;
    mfvid -> height = 16;
    mfvid -> level  = 40;
  }
  
  if( expert){
    if( beginner || advanced){
      MS_print( mss -> err, "\rOpptions --expert, --beginner and --advanced are not compatible with each other.\n");
      helpopt = 2;
    }
    mfvid -> width  = 30;
    mfvid -> height = 16;
    mfvid -> level  = 99;
  }
    
  if( *benchmark){
    mfvid -> width      = 3200;
    mfvid -> height     = 1800;
    mfvid -> level      = 1;
    video -> width      = 1;
    video -> height     = 1;
    *no_resize = 1;
  }
  
  video -> width  = video -> width ? video -> width : mfvid -> width;
  video -> height = video -> height? video -> height: mfvid -> height;
  
  video -> realwidth  = video -> realwidth ? video -> realwidth : video -> width  * xscale;
  video -> realheight = video -> realheight? video -> realheight: video -> height * yscale;
  
  video -> width  = video -> width  * xscale <= video -> realwidth ? video -> width : ( video -> realwidth  + ( xscale - 1)) / xscale;
  video -> height = video -> height * yscale <= video -> realheight? video -> height: ( video -> realheight + ( yscale - 1)) / yscale;
  
  if( mfvid -> level >= ( mfvid -> width * mfvid -> height)){
    mfvid -> level = ( mfvid -> width * mfvid -> height + 1) / 3;
    MS_print( mss -> err, "\rMore mine then elments!\n", mfvid -> level);
    helpopt = 2;
  }
    
  if( ( !mfvid -> global && !force) && ( mfvid -> width == 9 && mfvid -> height == 9 && mfvid -> level == 10)){
    MS_print( mss -> out, "\rMode: beginner \n");
  }else{
    if( beginner){
      MS_print( mss -> err, "\rThe \"Minefield\" options are not compatible with the \"Mode\" options.\n");
      helpopt = 2;
    }
  }
  
  if( ( !mfvid -> global && !force) && ( mfvid -> width == 16 && mfvid -> height == 16 && mfvid -> level == 40)){
    MS_print( mss -> out, "\rMode: advanced \n");
  }else{
    if( advanced){
      MS_print( mss -> err, "\rThe \"Minefield\" options are not compatible with the \"Mode\" options.\n");
      helpopt = 2;
    }
  }

  if( ( !mfvid -> global && !force) && ( mfvid -> width == 30 && mfvid -> height == 16 && mfvid -> level == 99)){
    MS_print( mss -> out, "\rMode: expert \n");
  }else{
    if( expert){
      MS_print( mss -> err, "\rThe \"Minefield\" options are not compatible with the \"Mode\" options.\n");
      helpopt = 2;
    }
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
  
  MS_print( mss -> deb, "\rwidth: %lu   ", mfvid -> width);
  MS_print( mss -> deb, "\r\t\theight: %lu   ", mfvid -> height);
  MS_print( mss -> deb, "\r\t\t\t\tlevel: %lu   \n", mfvid -> level);
    
  return 0;
}

int
main( int argv, char** argc){
  int ret;
  GraphicWraper *GW;
  MS_field *minefield;
  MS_stream *mss = ( MS_stream *)malloc( sizeof( MS_stream));
    
  {
    MS_field mfvid;
    MS_video video;
        
    unsigned long no_resize = 0;
    unsigned long benchmark = 0;
    
    readincmdline( argv, argc, &mfvid, &video, mss, &no_resize, &benchmark);
    
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
    
    GW -> global = mfvid.global;
    
    GW -> mfvid.width  = mfvid.width;
    GW -> mfvid.height = mfvid.height;
    GW -> mfvid.xdiff  = 0;
    GW -> mfvid.ydiff  = 0;
    
    GW -> mfvid.realwidth  = mfvid.width  * GW -> ewidth;
    GW -> mfvid.realheight = mfvid.height * GW -> eheight;
    
    minefield = MF_Create( mss, mfvid, GW -> mfvid);
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
  SDL_Event event;
  
  __uint64_t tutime, nextframe, gamestart, nexttu;
  
  MS_diff diff;
  
  __uint32_t seed = MS_rand_seed();
  
  int err = 0;
  
  int e = 0;
  
  diff.x = 0;
  diff.y = 0;
  
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
        return 0;
      case SDL_KEYDOWN:
        if( ( err = keypressevent( event, *mss, *minefield, GW -> mfvid, &diff, minefield -> uncovque, minefield -> mine)) > 0){
          nextframe = tutime;
        }
        break;
      case SDL_KEYUP:
        if( ( err = keyreleasevent( event, *minefield, GW -> mfvid, &diff, minefield -> uncovque, minefield -> mine)) > 0){
          nextframe = tutime;
        }
        if( diff.x || diff.y){
          nextframe = tutime;
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        if( ( err = pointerpressevent( event, GW, GW -> logical, *minefield, minefield -> uncovque, minefield -> mine)) > 0){
          nextframe = tutime;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        if( ( err = pointerreleasevent( event, *mss, GW -> logical, *minefield, minefield -> uncovque,  minefield -> mine, tutime, gamestart)) > 0){
          nextframe = tutime;
        }
        nextframe = tutime;
        break;
      case SDL_MOUSEMOTION:
        if( ( err = pointermoveevent( event, GW, GW -> logical, *minefield, minefield -> uncovque, minefield -> mine)) > 0){
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
    
    if( nextframe <= tutime && ( ( nextframe == tutime) || ( diff.x || diff.y))){
      if( window_scroll( GW, diff)){
        nextframe += 1000000000 / 30;
      }
                  
      if unlikely( ( minefield -> mine -> mines > minefield -> mine -> level) || ( minefield -> mine -> set > ( minefield -> mine -> noelements))){
        MS_print( mss -> err, "\rIn expliciv error: %lu of %lu, %lu of %lu \n", minefield -> mine -> mines, minefield -> mine -> level, minefield -> mine -> set, minefield -> mine -> noelements);
      }
      
      if unlikely( ( minefield -> mine -> set >= minefield -> mine -> noelements) && ( minefield -> mine -> mines < minefield -> mine -> level)){
        MS_print( mss -> err, "\rmine count fall short, curetn count: %lu of %lu \n", minefield -> mine -> mines, minefield -> mine -> level);
      }
      
      if unlikely( draw( GW, *minefield) == -3){
        MS_print( mss -> err, "\r\t\t\t\t\t\t\t\t\t inval data \n");
      }
      
      if( mss -> deb != NULL){
        __uint64_t mytime = getnanosec() - tutime;
        
        MS_print( mss -> deb, "\r\t\t\t\t\t\t\t %lu.%09lu      ", ( unsigned long)( ( mytime) / 1000000000), ( unsigned long)( ( mytime) % 1000000000));
      }
    }
  }
  
  return 1;
}


int
keypressevent( SDL_Event event, MS_stream mss, MS_field minefield, MS_video mfvid, MS_diff *diff, ComandStream *uncovque, MS_mstr *mine){
  int ret = 0;
  unsigned e =  event.key.keysym.sym;
  ( void)mss;
  
  switch( e){
  case SDLK_ESCAPE:
    quit();
    return 0;
  case SDLK_F2:
    if( mine -> uncoverd || mine -> flaged){
      setminefield( &minefield, &mss, mfvid);
      ret = 1;
    }
    break;
  case SDLK_F3:
    if( mine -> uncoverd < ( mine -> noelements - mine -> flaged)){
      ret = uncov_elements( minefield, uncovque, mfvid, mine);
    }
    if unlikely( uncov( minefield, uncovque, mine)){
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
keyreleasevent( SDL_Event event, MS_field minefield, MS_video mfvid, MS_diff *diff, ComandStream *uncovque, MS_mstr *mine){
  int ret = 0;
  unsigned e =  event.key.keysym.sym;
  ( void) mine;
  ( void) uncovque;
  ( void) minefield;
  ( void) mfvid;
    
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
swap_flag( MS_field minefield, int x, int y, MS_mstr *mine){
  int ret = 0;
  __uint8_t *element = acse( minefield, x, y);
  if( *element & EFLAG){
    *element &= ~EFLAG;
    --mine -> flaged;
    ret = 1;
  }else if( *element & ECOVER){
    *element|= EFLAG;
    ++mine -> flaged;
    ret = 1;
  }
  
  return ret;
}


int
pointerpressevent( SDL_Event event, GraphicWraper *gw, MS_video video, MS_field minefield, ComandStream *uncovque, MS_mstr *mine){
  int ret = 0;
  MS_pos postion;
  MS_video vid;
  
  ( void)gw;
  ( void)uncovque;
  
  postion.x = ( ( unsigned long)( ( ( event.button.x + video.realxdiff) * video.width ) / video.realwidth )) % minefield.width;
  postion.y = ( ( unsigned long)( ( ( event.button.y + video.realydiff) * video.height) / video.realheight)) % minefield.height;
  
  switch( event.button.button){
  case SDL_BUTTON_LEFT:
  case SDL_BUTTON_MIDDLE:
    vid.xdiff = postion.x;
    vid.ydiff = postion.y;
    vid.width  = 1;
    vid.height = 1;
    
    if( event.button.button == SDL_BUTTON_MIDDLE){
      vid.xdiff = ( minefield.width  + vid.xdiff - 1) % minefield.width;
      vid.ydiff = ( minefield.height + vid.ydiff - 1) % minefield.height;
      vid.width  = 3;
      vid.height = 3;
    }
    
    ret = uncov_elements( minefield, uncovque, vid, mine);
    break;
  case SDL_BUTTON_RIGHT:
    swap_flag( minefield, postion.x, postion.y, mine);
    break;
  default:
    break;
  }
  
  return ret;
}


int
pointerreleasevent( SDL_Event event, MS_stream mss, MS_video video, MS_field minefield, ComandStream *uncovque, MS_mstr *mine, __uint64_t tutime, __uint64_t gamestart){
  MS_pos postion, *el;
  int ret = 0;
  MS_video vid;
  
  MS_video mfvid;
  
  mfvid.xdiff = 0;
  mfvid.ydiff = 0;
  mfvid.width  = minefield.subwidth;
  mfvid.height = minefield.subheight;
  
  postion.x = ( ( unsigned long)( ( ( event.button.x + video.realxdiff) * video.width ) / video.realwidth )) % minefield.width;
  postion.y = ( ( unsigned long)( ( ( event.button.y + video.realydiff) * video.height) / video.realheight)) % minefield.height;
  
  switch( event.button.button){
  case SDL_BUTTON_LEFT:
  case SDL_BUTTON_MIDDLE:
    while( ( el = ( MS_pos *)CS_Releas( uncovque)) != NULL){
      *acse( minefield, el -> x, el -> y) |= ECOVER;
      CS_Finish( uncovque, el);
    }
    
    if( ( mine -> uncoverd == ( mine -> noelements - mine -> level)) || ( mine -> hit)){
      break;
    }
    
    vid.xdiff = postion.x;
    vid.ydiff = postion.y;
    vid.width  = 1;
    vid.height = 1;
    
    if( event.button.button == SDL_BUTTON_MIDDLE){
      vid.xdiff = ( minefield.width  + vid.xdiff - 1) % minefield.width;
      vid.ydiff = ( minefield.height + vid.ydiff - 1) % minefield.height;
      vid.width  = 3;
      vid.height = 3;
    }
    
    if( ( *mine).set == 0){
      /*let's play "Texas Sharpshooter"*/
      setzero( minefield, mine, vid);
    }
    
    ret += uncov_elements( minefield, uncovque, vid, mine);
    
    if unlikely( uncov( minefield, uncovque, mine)){
      ret = -2;
    }
    
    if likely( gamestart != tutime){
      if unlikely( mine -> hit){
	printtime( mss.out, ( tutime - gamestart) / 1000000);
        MS_print( mss.out, "\r\t\t\t Mine!!               \n");
        uncov_elements( minefield, uncovque, mfvid, mine);
        
        if unlikely( uncov( minefield, uncovque, mine)){
	  ret = -2;
	}
      }
      
      if unlikely( !mine -> hit && ( mine -> uncoverd == ( mine -> noelements - mine -> level))){
	printtime( mss.out, ( tutime - gamestart) / 1000000);
        MS_print( mss.out, "\r\t\t\t Win!!         \n");
      }
    }
  default:
    break;
  }
  return ret;
}


int
pointermoveevent( SDL_Event event, GraphicWraper *gw, MS_video video, MS_field minefield, ComandStream *uncovque, MS_mstr *mine){
  int ret = 0;
  MS_pos postion, prv_pos;
  
  unsigned long pos, pps;
  
  MS_video vid;
  
  ( void)gw;
  ( void)video;
  
  postion.x = ( ( unsigned long)( ( ( event.motion.x + video.realxdiff) * video.width ) / video.realwidth )) % minefield.width;
  postion.y = ( ( unsigned long)( ( ( event.motion.y + video.realydiff) * video.height) / video.realheight)) % minefield.height;
  
  prv_pos.x = ( ( unsigned long)( ( ( event.motion.x - event.motion.xrel + video.realxdiff) * video.width ) / video.realwidth )) % minefield.width;
  prv_pos.y = ( ( unsigned long)( ( ( event.motion.y - event.motion.yrel + video.realydiff) * video.height) / video.realheight)) % minefield.height;
  
  pos = postion.x + postion.y * minefield.width;
  
  pps = prv_pos.x + prv_pos.y * minefield.width;
  
  if( pos != pps){
    if( event.motion.state & SDL_BUTTON_LMASK){
      vid.xdiff = postion.x;
      vid.ydiff = postion.y;
      vid.width  = 1;
      vid.height = 1;
      ret += uncov_elements( minefield, uncovque, vid, mine);
    }
    
    if( event.motion.state & SDL_BUTTON_MMASK){
      vid.xdiff = ( minefield.width  + postion.x - 1) % minefield.width;
      vid.ydiff = ( minefield.height + postion.y - 1) % minefield.height;
      vid.width  = 3;
      vid.height = 3;
      ret += uncov_elements( minefield, uncovque, vid, mine);
    }
    
    if( event.motion.state & SDL_BUTTON_RMASK){
      ret += swap_flag( minefield, postion.x, postion.y, mine);
      ret += swap_flag( minefield, prv_pos.x, prv_pos.y, mine);
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
