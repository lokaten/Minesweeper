
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <time.h>
#include <limits.h>

#define LOCALE_( name) MAIN_##name

#include "MS_util.h"
#include "ComandStream.h"
#include "GW.h"
#include "minefield.h"
#include "OPT.h"



void
quit(){
  SDL_PushEvent( &( SDL_Event){ .key = ( SDL_KeyboardEvent){ .type = SDL_QUIT}});
}



int mainloop( MS_stream, MS_field, GraphicWraper *, ComandStream *, MS_mstr);
int keypressevent( SDL_Event, MS_field, MS_video, MS_diff *, ComandStream *, MS_mstr *);
int keyreleasevent( SDL_Event, MS_field, MS_video, MS_diff *, ComandStream *, MS_mstr *);
int pointerpressevent( SDL_Event, GraphicWraper *,MS_video, MS_field, ComandStream *, MS_mstr *);
int pointerreleasevent( SDL_Event, MS_stream, MS_video, MS_field, ComandStream *, MS_mstr *, __uint64_t, __uint64_t);
int pointermoveevent( SDL_Event, GraphicWraper *,MS_video, MS_field, ComandStream *, MS_mstr *);
void printtime( FILE *, unsigned long);


/*Beginner*/
#define WIDTH   9
#define HEIGHT  9


int
main( int argc, char** argv){
  
  MS_field minefield;
  MS_video video;
  MS_video fake;
  MS_mstr mine;
    
  unsigned long expert = 0;
  unsigned long advanced = 0;
  unsigned long beginner = 0;
  unsigned long benchmark = 0;
  
  unsigned long helpopt = 0;
  
  unsigned long xscale = 0;
  unsigned long yscale = 0;
  
  unsigned long quiet = 0;
  unsigned long vquiet = 0;
  
  unsigned long debug = 0;
  unsigned long no_resize = 0;

  unsigned long force = 0;
  
  GraphicWraper *window;
  
  
  MS_stream mss;
  
  /* put all comand line option in an array ( C99?)
   */
  MS_options opt[] = {
    { OPTSW_GRP, ""                                       , "Options"      , 0  , NULL                },
#ifdef DEBUG
    { OPTSW_BO , "ignore validation of options"             , "force"        , 'f', &( force           )},
#endif
    { OPTSW_GRP, ""                                       , "Minefield"    , 0  , NULL                },
    { OPTSW_LU , "Element wide minefield"                 , "width"        , 0  , &( minefield.width )},
    { OPTSW_LU , "Element high minefield"                 , "height"       , 0  , &( minefield.height)},
    { OPTSW_LU , "Number of mines"                        , "level"        , 0  , &( mine.level      )},
#ifdef DEBUG
    { OPTSW_X  , "Generate Minefield based on this seed"  , "seed"         , 0  , &( mine.reseed     )},
#endif
    { OPTSW_BO , ""                                       , "global"       , 'g', &( minefield.global)},
    { OPTSW_GRP, ""                                       , "Video"        , 0  , NULL                },
    { OPTSW_LU , "Element wide window"                    , "video-width"  , 0  , &( video.width     )},
    { OPTSW_LU , "Element high window"                    , "video-height" , 0  , &( video.height    )},
    { OPTSW_LU , "Pixel wide window"                      , "real-width"   , 0  , &( video.realwidth )},
    { OPTSW_LU , "Pixel high window"                      , "real-height"  , 0  , &( video.realheight)},
    { OPTSW_LU , "Pixel wide Element"                     , "scale-width"  , 0  , &( xscale          )},
    { OPTSW_LU , "Pixel high Element"                     , "scale-height" , 0  , &( yscale          )},
    { OPTSW_BO , "Resize don't work well with all system" , "no-resize"    , 0  , &( no_resize       )},
    { OPTSW_GRP, ""                                       , "Mode"         , 0  , NULL                },
    { OPTSW_BO , "Mimic windows minesweeper beginner mode", "beginner"     , 'b', &( beginner        )},
    { OPTSW_BO , "Mimic windows minesweeper advanced mode", "advanced"     , 'a', &( advanced        )},
    { OPTSW_BO , "Mimic windows minesweeper expert mode"  , "expert"       , 'e', &( expert          )},
    { OPTSW_BO , ""                                       , "benchmark"    , 'B', &( benchmark       )},
    { OPTSW_GRP, ""                                       , "Output"       , 0  , NULL                },
    { OPTSW_BO , "Print generic help mesage"              , "help"         , 'h', &( helpopt         )},
    { OPTSW_BO , "Supres reguler output"                  , "quiet"        , 'q', &( quiet           )},
    { OPTSW_BO , "Supres all output"                      , "very-quiet"   , 'Q', &( vquiet          )},
#ifdef DEBUG
    { OPTSW_BO , "Debug data"                             , "debug"        , 'd', &( debug           )},
#endif
    { OPTSW_NUL, "Last elemnt is a NULL termination"      , ""             , 0  , NULL                }};
  
  
  mss.err = stderr;
  mss.out = stdout;
#ifdef DEBUG
  mss.deb = stdout;
#else
  mss.deb = NULL;
#endif
  
  minefield.width  = 0;
  minefield.height = 0;
  mine.level       = 0;
  mine.reseed      = 0;
  
  video.width  = 0;
  video.height = 0;
  video.realwidth  = 0;
  video.realheight = 0;
  
  fake.realwidth  = 0;
  fake.realheight = 0;
  
  minefield.realwidth  = 0;
  minefield.realheight = 0;
  
  minefield.global = 0;
  

  procopt( mss, opt, argc, argv);
  
  
  if( !debug){
    mss.deb = NULL;
  }
  
  if( vquiet){
    mss.out = NULL;
    mss.err = NULL;
  }else{
    if( debug){
      mss.deb = stdout;
    }
  }

  
  if( quiet){
    mss.out = NULL;
  }
  
  if( ( beginner || expert || advanced) &&
      ( minefield.width || minefield.height ||
        mine.level || minefield.global || mine.reseed)){
    MS_print( mss.err, "\rThe \"Minefield\" options are not compatible with the \"Mode\" options.\n");
    helpopt = 2;
  }
  
  
#ifdef DEBUG
  if( mss.deb != NULL){
    /* print seed so that you can re run spcific minefield
     * NOTE: user input changes how the minfield is generated.
     */
    
    MS_print( mss.deb, "\rSeed: %08x   \n", mine.seed);
    MS_print( mss.deb, "\rNOTE: user input changes how the minfield is generated.\n", mine.seed);
    MS_print( mss.deb, "\rTODO: print new seed when setminefield is called.\n", mine.seed);
  }
#endif

  if( !mine.reseed){
    mine.reseed = MS_rand_seed();
  }
  
  srand( ( unsigned)time( ( void *)NULL));
  
  if( xscale || yscale || video.realwidth || video.realheight || !no_resize){
    MS_print( mss.err, "\rTODO: resize is broken\n");
  }
  
  if( !minefield.width != !minefield.height){
    MS_print( mss.err, "\rspecified --width or --height with out the other isn't recomended.\n");
    helpopt = 2;
  }
  
  if( !xscale != !yscale){
    MS_print( mss.err, "\rspecified --scale-width or --scale-height with out the other isn't recomended.\n");
    helpopt = 2;
  }
  
  if( !video.width != !video.height){
    MS_print( mss.err, "\rspecified --video-width or --video-height with out the other isn't recomended.\n");
    helpopt = 2;
  }
  
  if( !video.realwidth != !video.realheight){
    MS_print( mss.err, "\rspecified --real-width or --real-height with out the other isn't recomended.\n");
    helpopt = 2;
  }
    
  if( beginner){
    if( advanced || expert){
      MS_print( mss.err, "\rOpptions --expert, --beginner and --advanced are not compatible with each other.\n");
      helpopt = 2;
    }
    minefield.width  = minefield.width ? minefield.width :  9;
    minefield.height = minefield.height? minefield.height:  9;
    mine.level       = mine.level      ? mine.level      : 10;
  }
  
  if( advanced){
    if( beginner || expert){
      MS_print( mss.err, "\rOpptions --expert, --beginner and --advanced are not compatible with each other.\n");
      helpopt = 2;
    }
    minefield.width  = minefield.width ? minefield.width : 16;
    minefield.height = minefield.height? minefield.height: 16;
    mine.level       = mine.level      ? mine.level      : 40;
  }
  
  if( expert){
    if( beginner || advanced){
      MS_print( mss.err, "\rOpptions --expert, --beginner and --advanced are not compatible with each other.\n");
      helpopt = 2;
    }
    minefield.width  = minefield.width ? minefield.width : 30;
    minefield.height = minefield.height? minefield.height: 16;
    mine.level       = mine.level      ? mine.level      : 99;
  }
  
  if( helpopt){
    if( helpopt == 2){
      if( !force){
        help( mss, opt);
        exit( 1);
      }
    }else{
      help( mss, opt);
      exit( 0);
    }
  }
  
  
  if( benchmark){
    minefield.width  = minefield.width ? minefield.width : 3200;
    minefield.height = minefield.height? minefield.height: 1800;
    mine.level       = mine.level      ? mine.level      : 1;
    video.width      = video.width     ? video.width     : 1;
    video.height     = video.height    ? video.height    : 1;
    video.realwidth  = video.realwidth ? video.realwidth : 15;
    video.realheight = video.realheight? video.realheight: 15;
    xscale           = xscale          ? xscale          : 15;
    yscale           = yscale          ? yscale          : 15;
    no_resize = 1;
  }

  minefield.width  = minefield.width ? minefield.width : WIDTH;
  minefield.height = minefield.height? minefield.height: HEIGHT;
  
    
  if( mine.level >= ( minefield.width * minefield.height)){
    mine.level = ( minefield.width * minefield.height + 1) / 3;
    MS_print( mss.err, "\rMore mine then elments, reset mine cout to: %lu\n", mine.level);
  }
  
  if( mine.level == 0){
    mine.level = ( minefield.width * minefield.height + 4) / 8;
  }
  
  if( xscale && video.width  && ( video.realwidth  >= ( video.width  * xscale))){
    video.realwidth = video.width * xscale;
  }
  
  if( yscale && video.height && ( video.realheight >= ( video.height * yscale))){
    video.realheight = video.height * yscale;
  }
  
  if( video.realwidth  && xscale && ( video.realwidth  != video.width  * xscale)){
    video.width = ( video.realwidth + xscale - 1) / xscale;
    if( video.width > minefield.width){
      video.width  = minefield.width;
    }
    fake.realwidth = video.realwidth <= ( video.width * xscale)? video.realwidth: video.width * xscale;
    video.realwidth = video.width * xscale;
  }
  
  if( video.realheight && yscale && ( video.realheight != video.height * yscale)){
    video.height = ( video.realheight + yscale - 1) / yscale;
    if( video.height > minefield.height){
      video.height = minefield.height;
    }
    fake.realheight = video.realheight <= ( video.height * yscale)? video.realheight: video.height * yscale;
    video.realheight = video.height * yscale;
  }
  
  if( ( video.width  == 0) || ( !force && ( video.width  > minefield.width))){
    video.width  = minefield.width;
  }
  
  if( ( video.height == 0) || ( !force && ( video.height > minefield.height))){
    video.height = minefield.height;
  }
  
  if( ( video.realwidth  == 0) && xscale){
    video.realwidth  = video.width  * xscale;
  }
  
  if( ( video.realheight == 0) && yscale){
    video.realheight = video.height * yscale;
  }
  
  if( video.realwidth  == 0){
    video.realwidth  = video.width  * 15;
    minefield.realwidth  = minefield.width  * 15;
  }
  
  if( video.realheight == 0){
    video.realheight = video.height * 15;
    minefield.realheight = minefield.height * 15;
  }
  
  if( minefield.realwidth  == 0){
    minefield.realwidth  = ( minefield.width  * video.realwidth ) / video.width;
  }
  
  if( minefield.realheight == 0){
    minefield.realheight = ( minefield.height * video.realheight) / video.height;
  }
  
  fake.width  = video.width;
  fake.height = video.height;
  
  if( fake.realwidth  == 0){
    fake.realwidth  = video.realwidth;
  }
  
  if( fake.realheight == 0){
    fake.realheight = video.realheight;
  }
  
  if( !minefield.global && !force){
    if( minefield.width == 9 && minefield.height == 9 && mine.level == 10){
      MS_print( mss.out, "\rMode: beginner \n");
    }
    
    if( minefield.width == 16 && minefield.height == 16 && mine.level == 40){
      MS_print( mss.out, "\rMode: advanced \n");
    }
    
    if( minefield.width == 30 && minefield.height == 16 && mine.level == 99){
      MS_print( mss.out, "\rMode: expert \n");
    }
  }
  
  window = GW_Create( video, fake, no_resize, minefield);

  if( window == NULL){
    exit( 1);
  }

  if( benchmark){
    SDL_PushEvent( &( SDL_Event){ .button = ( SDL_MouseButtonEvent){ .type = SDL_MOUSEBUTTONDOWN, .button = SDL_BUTTON_LEFT, .x = 0, .y = 0}});
    SDL_PushEvent( &( SDL_Event){ .button = ( SDL_MouseButtonEvent){ .type = SDL_MOUSEBUTTONUP  , .button = SDL_BUTTON_LEFT, .x = 0, .y = 0}});
    SDL_PushEvent( &( SDL_Event){ .key = ( SDL_KeyboardEvent){ .type = SDL_QUIT}});
  }
  
  ( *window).global = minefield.global;

  {
    int ret;
    ComandStream *uncovque = CS_Create( sizeof( MS_pos));
    
    minefield.subwidth  = minefield.width;
    minefield.subheight = minefield.height;
    mine.noelements = minefield.subwidth * minefield.subheight;
    
    if( !minefield.global){
      minefield.width  += 1;
      minefield.height += 1;
    }
    
    minefield.data = ( __uint8_t *)malloc( sizeof( __uint8_t) * minefield.width * minefield.height);
    
    if( minefield.data == NULL){
      fprintf( stderr, "\rlimet-mem \n");
      exit( 1);
    }
    
    if( !minefield.global){
      memset( minefield.data, ESET, minefield.width * minefield.height);
    }
    
    ( *window).mfvid.width  = minefield.width;
    ( *window).mfvid.height = minefield.height;
    ( *window).mfvid.realwidth  = minefield.realwidth;
    ( *window).mfvid.realheight = minefield.realheight;
    
    ret = mainloop( mss, minefield, window, uncovque, mine);
    
    free( minefield.data);
    
    CS_Free( uncovque);
    
    GW_Free( window);
    
    MS_print( mss.out, "\nBye!\n");
    
    exit( ret);
  }
  
  return 1;
}



int
mainloop( MS_stream mss, MS_field minefield, GraphicWraper *window, ComandStream *uncovque, MS_mstr mine){
  SDL_Event event;
    
  __uint64_t tutime, nextframe, gamestart, nexttu;
  
  MS_video mfvid;
  
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
  
  mfvid.xdiff = 0;
  mfvid.ydiff = 0;
  
  mfvid.width  = minefield.subwidth;
  mfvid.height = minefield.subheight;
  
  setminefield( mfvid, minefield, &mine);
  
  while( TRUE){
    
    if( nexttu <= tutime){
      if( mine.uncoverd && !mine.hit && mine.uncoverd < ( mine.noelements - mine.level)){
        MS_print( mss.out, "\r\t\t\t %lu of %lu      ", mine.flaged, mine.level);
      }else{
        gamestart = tutime;
      }
      
      printtime( mss.out, ( tutime - gamestart) / 1000000);
      
      /* to make sure the time looks like it updatet consistanly we randomaize
       * the time we wait betwen updating it, with max time betwen update beigen 150ms
       */
      nexttu += 50000000llu + ( ( ( __uint64_t)( seed = MS_rand( seed)) * 100000000llu) / MS_RAND_MAX);
    }
    
    if( e){
      err = 0;
      
      switch( expect( event.type, SDL_MOUSEBUTTONDOWN)){
      case SDL_QUIT:
        return 0;
      case SDL_VIDEOEXPOSE:
        nextframe = tutime;
        break;
      case SDL_VIDEORESIZE:
        if( window_resize( window, ( MS_video){ .width = event.resize.w, .height = event.resize.h})){
          MS_print( mss.err, "\rresize faild!                                             \n");
          return -1;
        }
        nextframe = tutime;
        break;
      case SDL_KEYDOWN:
        if( ( err = keypressevent( event, minefield, mfvid, &diff, uncovque, &mine)) > 0){
          nextframe = tutime;
        }
        break;
      case SDL_KEYUP:
        if( ( err = keyreleasevent( event, minefield, mfvid, &diff, uncovque, &mine)) > 0){
          nextframe = tutime;
        }
        if( diff.x || diff.y){
          nextframe = tutime;
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        if( ( err = pointerpressevent( event, window, ( *window).video, minefield, uncovque, &mine)) > 0){
          nextframe = tutime;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        if( ( err = pointerreleasevent( event, mss, ( *window).video, minefield, uncovque, &mine, tutime, gamestart)) > 0){
          nextframe = tutime;
        }
        nextframe = tutime;
        break;
      case SDL_MOUSEMOTION:
        if( ( err = pointermoveevent( event, window, ( *window).video, minefield, uncovque, &mine)) > 0){
          nextframe = tutime;
        }
        break;
      default:
        break;
      }
      
      if unlikely( err == -2){
        MS_print( mss.err, "\r\t\t\t\t\t\t\t\t\t alloc limet!     \n");
      }
    }
    
    if( nextframe <= tutime && ( ( nextframe == tutime) || ( diff.x || diff.y))){
      if( window_scroll( window, diff)){
        nextframe += 1000000000 / 30;
      }
                  
      if unlikely( ( mine.mines > mine.level) || ( mine.set > ( mine.noelements))){
        MS_print( mss.err, "\rIn expliciv error: %lu of %lu, %lu of %lu \n", mine.mines, mine.level, mine.set, mine.noelements);
      }
      
      if unlikely( ( mine.set >= mine.noelements) && ( mine.mines < mine.level)){
        MS_print( mss.err, "\rmine count fall short, curetn count: %lu of %lu \n", mine.mines, mine.level);
      }
      
      if unlikely( draw( window, minefield) == -3){
        MS_print( mss.err, "\r\t\t\t\t\t\t\t\t\t inval data \n");
      }
      
      if( mss.deb != NULL){
        __uint64_t mytime = getnanosec() - tutime;
        
        MS_print( mss.deb, "\r\t\t\t\t\t\t\t %lu.%09lu      ", ( unsigned long)( ( mytime) / 1000000000), ( unsigned long)( ( mytime) % 1000000000));
      }
    }
    
    if likely( !( e = SDL_PollEvent( &event))){
      tutime = getnanosec();
      if( !mine.uncoverd){
        gamestart = tutime;
      }
      SDL_Delay( 1);
    }
  }
  
  return 1;
}


int
keypressevent( SDL_Event event, MS_field minefield, MS_video mfvid, MS_diff *diff, ComandStream *uncovque, MS_mstr *mine){
  int ret = 0;
  switch( event.key.keysym.sym){
  case SDLK_ESCAPE:
    quit();
    return 0;
  case SDLK_F2:
    if( ( *mine).uncoverd || ( *mine).flaged){
      setminefield( mfvid, minefield, mine);
      return 1;
    }
    return 0;
  case SDLK_F3:
    if( ( *mine).uncoverd < ( ( *mine).noelements - ( *mine).flaged)){
      ret = uncov_elements( minefield, uncovque, mfvid, mine);
    }
    if unlikely( uncov( minefield, uncovque, mine)){
      ret = -2;
    }
    return ret;
  case SDLK_h:
  case SDLK_LEFT:
    ( *diff).x -= 3;
    ( *diff).x = ( *diff).x < -3? -3: ( *diff).x;
    return 1;
  case SDLK_j:
  case SDLK_DOWN:
    ( *diff).y += 3;
    ( *diff).y = ( *diff).y > 3? 3: ( *diff).y;
    return 1;
  case SDLK_k:
  case SDLK_UP:
    ( *diff).y -= 3;
    ( *diff).y = ( *diff).y < -3? -3: ( *diff).y;
    return 1;
  case SDLK_l:
  case SDLK_RIGHT:
    ( *diff).x += 3;
    ( *diff).x = ( *diff).x > 3? 3: ( *diff).x;
    return 1;
  default:
    return 0;
  }
}


int
keyreleasevent( SDL_Event event, MS_field minefield, MS_video mfvid, MS_diff *diff, ComandStream *uncovque, MS_mstr *mine){
  ( void) mine;
  ( void) uncovque;
  ( void) minefield;
  ( void) mfvid;
  switch( event.key.keysym.sym){
  case SDLK_h:
  case SDLK_LEFT:
    ( *diff).x += 3;
    ( *diff).x = ( *diff).x < 0? 0: ( *diff).x;
    return 0;
  case SDLK_j:
  case SDLK_DOWN:
    ( *diff).y -= 3;
    ( *diff).y = ( *diff).y > 0? 0: ( *diff).y;
    return 0;
  case SDLK_k:
  case SDLK_UP:
    ( *diff).y += 3;
    ( *diff).y = ( *diff).y < 0? 0: ( *diff).y;
    return 0;
  case SDLK_l:
  case SDLK_RIGHT:
    ( *diff).x -= 3;
    ( *diff).x = ( *diff).x > 0? 0: ( *diff).x;
    return 0;
  default:
    return 0;
  }
}


int
pointerpressevent( SDL_Event event, GraphicWraper *gw, MS_video video, MS_field minefield, ComandStream *uncovque, MS_mstr *mine){
  MS_pos postion;
  unsigned long pos;
  int ret = 0;
  MS_video vid;
  
  ( void)gw;
  ( void)uncovque;
  
  postion.x = ( ( unsigned long)( ( ( event.button.x + video.realxdiff) * video.width ) / video.realwidth )) % minefield.width;
  postion.y = ( ( unsigned long)( ( ( event.button.y + video.realydiff) * video.height) / video.realheight)) % minefield.height;
  
  pos = postion.x + postion.y * minefield.width;
  
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
    if( minefield.data[ pos] & EFLAG){
      minefield.data[ pos] &= ~EFLAG;
      --( *mine).flaged;
      ret = 1;
      break;
    }

    if( ( minefield.data[ pos] & ECOVER) && !( minefield.data[ pos] & EFLAG)){
      minefield.data[ pos] |= EFLAG;
      ++( *mine).flaged;
      ret = 1;
    }
    break;
  }
  
  return ret;
}



int
pointerreleasevent( SDL_Event event, MS_stream mss, MS_video video, MS_field minefield, ComandStream *uncovque, MS_mstr *mine, __uint64_t tutime, __uint64_t gamestart){
  MS_pos postion;
  int ret = 0;
  MS_video vid;

  MS_video mfvid;
  
  unsigned long pos;

  mfvid.xdiff = 0;
  mfvid.ydiff = 0;
  mfvid.width  = minefield.subwidth;
  mfvid.height = minefield.subheight;
  
  postion.x = ( ( unsigned long)( ( ( event.button.x + video.realxdiff) * video.width ) / video.realwidth )) % minefield.width;
  postion.y = ( ( unsigned long)( ( ( event.button.y + video.realydiff) * video.height) / video.realheight)) % minefield.height;

  pos = postion.x + postion.y * minefield.width;
  
  switch( event.button.button){
  case SDL_BUTTON_LEFT:
  case SDL_BUTTON_MIDDLE:

    if( ( mine -> uncoverd == ( mine -> noelements - mine -> level))
        || ( mine -> hit)){
      MS_pos *el;
      while( ( el = CS_Releas( uncovque)) != NULL){
        *acse( minefield, el -> x, el -> y) |= ECOVER;
        CS_Finish( uncovque, el);
      }
      break;
    }
    
    vid.xdiff = postion.x;
    vid.ydiff = postion.y;
    vid.width  = 1;
    vid.height = 1;
    
    if( event.button.button == SDL_BUTTON_MIDDLE){
      if( ( ( minefield.data[ pos] & ECOVER) && !( minefield.data[ pos] & EFLAG))){
        MS_pos *el;
        while( ( el = CS_Releas( uncovque)) != NULL){
          *acse( minefield, el -> x, el -> y) |= ECOVER;
          CS_Finish( uncovque, el);
        }
        break;
      }
      vid.xdiff = ( minefield.width  + vid.xdiff - 1) % minefield.width;
      vid.ydiff = ( minefield.height + vid.ydiff - 1) % minefield.height;
      vid.width  = 3;
      vid.height = 3;
    }
    
    if( ( *mine).set == 0){
      /*let's play "Texas Sharpshooter"*/
      setzero( minefield, mine, vid);
    }

    ret = 1;
    
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
  case SDL_BUTTON_RIGHT:
    break;
  }
  return ret;
}


int
pointermoveevent( SDL_Event event, GraphicWraper *gw, MS_video video, MS_field minefield, ComandStream *uncovque, MS_mstr *mine){
  ( void)event;
  ( void)gw;
  ( void)video;
  ( void)minefield;
  ( void)mine;
  ( void)uncovque;
  return 0;
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
