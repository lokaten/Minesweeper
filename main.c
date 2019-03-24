
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <limits.h>

#include "MS_util.h"
#include "userinterface.h"
#include "minefield.h"
#include "OPT.h"


MS_root *ROOT_Init( const int, const char **);
static inline void printtime( FILE *, u64);


void
quit( const MS_root *root){
  int ret = 0;
  GW_Free( root -> GW);
  MF_FreeFieldPartial( root -> minefield);
  MS_print( root -> mss -> out, TERM("\rBye!                                \n"));
  exit( ret);
}


MS_root *
ROOT_Init( const int argc, const char **argv){
  MS_root *root;
  
  MS_video real;
  MS_field *minefield;
  MS_stream *mss;
  bool no_resize;

  bool custom = FALSE;
  bool custom_global = FALSE;
  u16 custom_width;
  u16 custom_height;
  u16 custom_level;

#define CreateLocal( type, ...) &( type){__VA_ARGS__}
  
  MS_field *field_beginner  = CreateLocal( MS_field, .title = "beginner" , .width =    9, .height =    9, .level = 10, .global = 0, .reseed = 0);
  MS_field *field_advanced  = CreateLocal( MS_field, .title = "advanced" , .width =   16, .height =   16, .level = 40, .global = 0, .reseed = 0);
  MS_field *field_expert    = CreateLocal( MS_field, .title = "expert"   , .width =   30, .height =   16, .level = 99, .global = 0, .reseed = 0);
  MS_field *field_benchmark = CreateLocal( MS_field, .title = "benchmark", .width = 3200, .height = 1800, .level =  1, .global = 1, .reseed = 0);
  
  MS_stream *very_quiet = CreateLocal( MS_stream, .out = NULL  , .err = NULL  , .deb = NULL, .hlp = NULL);
  MS_stream *def_out    = CreateLocal( MS_stream, .out = stdout, .err = stderr, .deb = NULL, .hlp = NULL);
  
  const unsigned long opt_true  = TRUE;
  const unsigned long opt_false = FALSE;
  
  dassert( opt_true);
  dassert( !opt_false);
  
  mss = def_out;
  minefield = field_beginner;
  
  {
    MS_options opt[] = {
      { OPTSW_GRP, TERM("Options"                                ), ""               , 0  , NULL                        , NULL},
      { OPTSW_GRP, TERM("Custom"                                 ), ""               , 0  , NULL                        , NULL},
      { OPTSW_LU , TERM("Element wide minefield"                 ), "width"          , 0  , &custom_width               , NULL},
      { OPTSW_LU , TERM("Element high minefield"                 ), "height"         , 0  , &custom_height              , NULL},
      { OPTSW_LU , TERM("Number of mines"                        ), "level"          , 0  , &custom_level               , NULL},
#ifdef DEBUG
      { OPTSW_CPY, TERM(""                                       ), "global"         , 'g', &custom_global              , &opt_true},
#endif
      { OPTSW_GRP, TERM("Video"                                  ), ""               , 0  , NULL                        , NULL},
      { OPTSW_LU , TERM(""                                       ), "window-width"   , 0  , &real.realwidth             , NULL},
      { OPTSW_LU , TERM(""                                       ), "window-height"  , 0  , &real.realheight            , NULL},
      { OPTSW_LU , TERM(""                                       ), "element-width"  , 0  , &real.element_width         , NULL},
      { OPTSW_LU , TERM(""                                       ), "element-height" , 0  , &real.element_height        , NULL},
      { OPTSW_CPY, TERM("Resize don't work well with all system" ), "no-resize"      , 'R', &no_resize                  , &opt_true},
      { OPTSW_GRP, TERM("Mode"                                   ), ""               , 0  , NULL                        , NULL},
      { OPTSW_CPY, TERM("customaise your own"                    ), "custom"         , 0  , &custom                     , &opt_true},
      { OPTSW_CPY, TERM("Mimic windows minesweeper beginner mode"), "beginner"       , 'b', &minefield                  , field_beginner },
      { OPTSW_CPY, TERM("Mimic windows minesweeper advanced mode"), "advanced"       , 'a', &minefield                  , field_advanced },
      { OPTSW_CPY, TERM("Mimic windows minesweeper expert mode"  ), "expert"         , 'e', &minefield                  , field_expert   },
      { OPTSW_CPY, TERM(""                                       ), "benchmark"      , 'B', &minefield                  , field_benchmark},
#ifndef NO_TERM
      { OPTSW_GRP, TERM("Output"                                 ), ""               , 0  , NULL                       , NULL},
      { OPTSW_CPY, TERM("Print generic help mesage"              ), "help"           , 'h', &( def_out -> hlp         ), stdout},
      { OPTSW_CPY, TERM("Supres reguler output"                  ), "quiet"          , 'q', &( def_out -> out         ), NULL},
      { OPTSW_CPY, TERM("Supres all output"                      ), "very-quiet"     , 'Q', &(mss                     ), very_quiet},
#ifdef DEBUG
      { OPTSW_CPY, TERM("Debug data"                             ), "debug"          , 'd', &( def_out -> deb         ), stdout},
#endif
#endif
      { OPTSW_NUL, TERM(""/* Last elemnt is a NULL termination */), ""               , 0  , NULL                       , NULL}};
    
    real = (MS_video){ .element_width = 15, .element_height = 15};
    
    if( procopt( mss, opt, argc, argv)){
      MS_print( mss -> err, TERM("\rWRong or broken input, pleas refer to --help\n"));
    }
#ifndef NO_TERM
    if( mss -> hlp){
      help( mss -> hlp, opt);
      
      quit( &( const MS_root){ .mss = mss});
    }
#endif
  }
  
  if( custom){
    if( custom_level >= ( custom_width * custom_height)){
      custom_level = ( custom_width * custom_height + 1) / 3;
      MS_print( mss -> err, TERM("\rMore mines then elments!\n"));
    }
    
    minefield  = MF_CreateField( .title = "custom" , .width = custom_width, .height = custom_height, .level = custom_level, .global = custom_global, .reseed = 0);
  }else{
    minefield = MF_CreateFieldFromLocal( minefield);
  }
  
  mss = MS_CreateFromLocal( MS_stream, mss);
  
  assert( minefield -> title != NULL);
  
  MS_print( mss -> out, "\rMode: %s\n", minefield -> title);
  
  if( custom){
    MS_print( mss -> out, "\rwidth: %lu   ", minefield -> width);
    MS_print( mss -> out, "\r\t\theight: %lu   ", minefield -> height);
    MS_print( mss -> out, "\r\t\t\t\tlevel: %lu   \n", minefield -> level);
  }
  
  if( strstr( minefield -> title, "benchmark")){
    setminefield( minefield, NULL);
    setzero( minefield, ( MS_video){ .xdiff = -1, .ydiff = -1, .width  = 3, .height = 3});
    uncov_elements( minefield, ( MS_video){ .xdiff =  0, .ydiff =  0, .width  = 1, .height = 1});
    uncov( minefield, NULL);
    MF_FreeField( minefield);
    MS_Free( mss);
    quit( NULL);
  }else{
    root = MS_Create( MS_root,
		      .real = real,
		      .minefield = minefield,
		      .mss = mss,
		      .no_resize = no_resize);
    
    root -> GW = GW_Init( root);
    
    draw( root -> GW, *root -> minefield);
    
    setminefield( root -> minefield, root -> GW);
  }
  
  return root;
}


int
main( const int argc, const char** argv){
  const MS_root *root = ROOT_Init( argc, argv);

  bool gameover;
  u64 tutime;
  u64 gamestart;
  
  gameover  = FALSE;
  tutime    = getnanosec();
  gamestart = tutime;
  
  while( TRUE){
    {
      tutime = getnanosec();
      if( !root -> minefield -> mine -> uncoverd || gameover){
	gamestart = tutime;
      }
      
      event_dispatch( root);
      
      if unlikely( root -> minefield -> mine -> hit){
	MS_video mfvid = { .xdiff = 0, .ydiff = 0, .width  = root -> minefield -> subwidth, .height = root -> minefield -> subheight};
	uncov_elements( root -> minefield, mfvid);
	
	uncov( root -> minefield, root -> GW);
      }
      
      dassert( root -> minefield -> mine -> mines <= root -> minefield -> mine -> level);
      dassert( root -> minefield -> mine -> set   <= root -> minefield -> mine -> noelements);
      
      if( !gameover){
	if unlikely( root -> minefield -> mine -> hit){
	  MS_print( root -> mss -> out, TERM( "\r\t\t\t Mine!!               "));
	  gameover = TRUE;
	}else if unlikely( ( root -> minefield -> mine -> uncoverd == ( root -> minefield -> mine -> noelements - root -> minefield -> mine -> level))){
	  printtime( root -> mss -> out, ( tutime - gamestart) / 1000000);
	  MS_print( root -> mss -> out, TERM( "\r\t\t\t Win!!         \n"));
	  gameover = TRUE;
	}else{
	  printtime( root -> mss -> out, ( tutime - gamestart) / 1000000);
	  MS_print( root -> mss -> out, TERM( "\r\t\t\t %lu of %lu      "), root -> minefield -> mine -> flaged, root -> minefield -> mine -> level);
	}
      }else if( !root -> minefield -> mine -> set){
	gameover = FALSE;
      }
      
      draw( root -> GW, *root -> minefield);
      
#ifdef DEBUG
      if( root -> mss -> deb != NULL){
	__uint64_t mytime = getnanosec() - tutime;
	
	DEBUG_PRINT( root -> mss -> deb, "\r\t\t\t\t\t\t\t %lu.%09lu      ", ( unsigned long)( ( mytime) / 1000000000), ( unsigned long)( ( mytime) % 1000000000));
      }
#endif
    }
  }
}


static inline void
printtime( FILE * stream, u64 time){
#ifdef NO_TERM
  ( void) stream;
  ( void) time;
#else
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
#endif
  return;
}
