
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <limits.h>

#define LOCALE_( name) MAIN_##name

#include "MS_util.h"
#include "ComandStream.h"
#include "userinterface.h"
#include "minefield.h"
#include "OPT.h"


void quit( void *);
MS_root *ROOT_Init( const int, const char **);
void ROOT_Free( MS_root *);
static inline void printtime( FILE *, u64);


void
quit( void *data){
  int ret = 0;
  MS_root *root = ( MS_root *)data;
  MS_print( root -> mss -> out, "\rBye!                                \n");
  ROOT_Free( root);
  exit( ret);
}


MS_root *
ROOT_Init( const int argc, const char **argv){
  MS_root *root;
  
  MS_field *field_custom    = MS_Create( MS_field, .title = "custom"   , .width =    0, .height =    0, .level =  0, .global = 0, .reseed = 0);
  MS_field *field_beginner  = MS_Create( MS_field, .title = "beginner" , .width =    9, .height =    9, .level = 10, .global = 0, .reseed = 0);
  MS_field *field_advanced  = MS_Create( MS_field, .title = "advanced" , .width =   16, .height =   16, .level = 40, .global = 0, .reseed = 0);
  MS_field *field_expert    = MS_Create( MS_field, .title = "expert"   , .width =   30, .height =   16, .level = 99, .global = 0, .reseed = 0);
  MS_field *field_benchmark = MS_Create( MS_field, .title = "benchmark", .width = 3200, .height = 1800, .level =  1, .global = 1, .reseed = 0);
  
  MS_stream *very_quiet = MS_Create( MS_stream, .out = NULL  , .err = NULL  , .deb = NULL, .hlp = NULL);
  MS_stream *def_out    = MS_Create( MS_stream, .out = stdout, .err = stderr, .deb = NULL, .hlp = NULL);
  
  unsigned long opt_true  = TRUE;
  unsigned long opt_false = FALSE;

  dassert( opt_true);
  dassert( !opt_false);
  
  root = MS_CreateEmpty( MS_root);
  
  root -> mss = def_out;
  root -> minefield = field_beginner;
  
  {
    MS_options opt[] = {
      { OPTSW_GRP, TERM("Options"                                ), ""               , 0  , NULL                       , NULL},
      { OPTSW_GRP, TERM("Minefield"                              ), ""               , 0  , NULL                       , NULL},
      { OPTSW_LU , TERM("Element wide minefield"                 ), "width"          , 0  , &( field_custom -> width  ), NULL},
      { OPTSW_LU , TERM("Element high minefield"                 ), "height"         , 0  , &( field_custom -> height ), NULL},
      { OPTSW_LU , TERM("Number of mines"                        ), "level"          , 0  , &( field_custom -> level  ), NULL},
#ifdef DEBUG
      { OPTSW_X  , TERM("Generate Minefield based on this seed"  ), "seed"           , 0  , &( field_custom -> reseed ), NULL},
      { OPTSW_CPY, TERM(""                                       ), "global"         , 'g', &( field_custom -> global ), &opt_true},
#endif
      { OPTSW_GRP, TERM("Video"                                  ), ""               , 0  , NULL                        , NULL},
      { OPTSW_LU , TERM(""                                       ), "window-width"   , 0  , &root -> real.realwidth     , NULL},
      { OPTSW_LU , TERM(""                                       ), "window-height"  , 0  , &root -> real.realheight    , NULL},
      { OPTSW_LU , TERM(""                                       ), "element-width"  , 0  , &root -> real.element_width , NULL},
      { OPTSW_LU , TERM(""                                       ), "element-height" , 0  , &root -> real.element_height, NULL},
      { OPTSW_CPY, TERM("Resize don't work well with all system" ), "no-resize"      , 'R', &root -> no_resize          , &opt_true},
      { OPTSW_GRP, TERM("Mode"                                   ), ""               , 0  , NULL                       , NULL},
      { OPTSW_CPY, TERM("customaise your own"                    ), "custom"         , 'b', &root -> minefield         , field_custom   },
      { OPTSW_CPY, TERM("Mimic windows minesweeper beginner mode"), "beginner"       , 'b', &root -> minefield         , field_beginner },
      { OPTSW_CPY, TERM("Mimic windows minesweeper advanced mode"), "advanced"       , 'a', &root -> minefield         , field_advanced },
      { OPTSW_CPY, TERM("Mimic windows minesweeper expert mode"  ), "expert"         , 'e', &root -> minefield         , field_expert   },
      { OPTSW_CPY, TERM(""                                       ), "benchmark"      , 'B', &root -> minefield         , field_benchmark},
#ifndef NO_TERM
      { OPTSW_GRP, TERM("Output"                                 ), ""               , 0  , NULL                       , NULL},
      { OPTSW_CPY, TERM("Print generic help mesage"              ), "help"           , 'h', &( def_out -> hlp         ), stdout},
      { OPTSW_CPY, TERM("Supres reguler output"                  ), "quiet"          , 'q', &( def_out -> out         ), NULL},
      { OPTSW_CPY, TERM("Supres all output"                      ), "very-quiet"     , 'Q', &root -> mss               , very_quiet},
#ifdef DEBUG
      { OPTSW_CPY, TERM("Debug data"                             ), "debug"          , 'd', &( def_out -> deb         ), stdout},
#endif
#endif
      { OPTSW_NUL, TERM(""/* Last elemnt is a NULL termination */), ""               , 0  , NULL                       , NULL}};
    
    root -> real = (MS_video){ .element_width = 15, .element_height = 15};
    
    if( procopt( root -> mss, opt, argc, argv)){
      MS_print( root -> mss -> err, "\rWRong or broken input, pleas refer to --help\n");
    }
#ifndef NO_TERM
    if( root -> mss -> hlp){
      help( root -> mss -> hlp, opt);
      quit( root);
    }
#endif
  }
  
  root -> minefield -> reseed = field_custom -> reseed;
  
  root -> quit = quit;
  
  if( root -> minefield -> level >= ( root -> minefield -> width * root -> minefield -> height)){
    root -> minefield -> level = ( root -> minefield -> width * root -> minefield -> height + 1) / 3;
    MS_print( root -> mss -> err, "\rMore mines then elments!\n");
  }
  
  
  DEBUG_PRINT( root -> mss -> deb, "\rseed is printed when setminefield is called so that you can re run spcific minefield whit help of --seed\n");
  DEBUG_PRINT( root -> mss -> deb, "\rNOTE: user input changes how the minfield is generated.\n");
  
  MS_print( root -> mss -> out, "\rMode: %s\n", root -> minefield -> title);
  
  DEBUG_PRINT( root -> mss -> deb, "\rwidth: %lu   ", root -> minefield -> width);
  DEBUG_PRINT( root -> mss -> deb, "\r\t\theight: %lu   ", root -> minefield -> height);
  DEBUG_PRINT( root -> mss -> deb, "\r\t\t\t\tlevel: %lu   \n", root -> minefield -> level);
  
  if( root -> minefield != field_custom   ) MS_Free( field_custom   );
  if( root -> minefield != field_beginner ) MS_Free( field_beginner );
  if( root -> minefield != field_advanced ) MS_Free( field_advanced );
  if( root -> minefield != field_expert   ) MS_Free( field_expert   );
  if( root -> minefield != field_benchmark) MS_Free( field_benchmark);
  
  if( root -> mss != very_quiet) MS_Free( very_quiet);
  if( root -> mss != def_out   ) MS_Free( def_out   );
  
  return root;
}


void
ROOT_Free( MS_root *root){
  if( root != NULL){
    MF_Free( root -> minefield);
    GW_Free( root -> GW);
    MS_Free( root -> mss);
    CS_Free( root -> actionque);
    MS_Free( root);
  }
}


int
main( const int argc, const char** argv){
  MS_root *root;
  
  root = ROOT_Init( argc, argv);
  root -> minefield = MF_Init( root -> minefield);
  root -> GW = GW_Init( root);
  
  root -> seed = MS_rand_seed();
  
  root -> tutime    = getnanosec();
  root -> gamestart = root -> tutime;
  root -> nextframe = root -> tutime;
    
  root -> gameover = FALSE;
  
  if( strstr( root -> minefield -> title, "benchmark")){
    setzero( root -> minefield, ( MS_video){ .xdiff = -1, .ydiff = -1, .width  = 3, .height = 3});
    uncov_elements( root -> minefield, ( MS_video){ .xdiff =  0, .ydiff =  0, .width  = 1, .height = 1});
    uncov( root -> minefield, root -> GW);
    quit( root);
  }
  
  draw( root -> GW, *root -> minefield);
  
  setminefield( root -> minefield, root -> GW, root -> mss,
		( MS_video){ .width = root -> minefield -> subwidth, .height = root -> minefield -> subheight});
  
  while( TRUE){
    {
      event_dispatch( root);
      
      dassert( root -> minefield -> mine -> mines <= root -> minefield -> mine -> level);
      dassert( root -> minefield -> mine -> set   <= root -> minefield -> mine -> noelements);
      
      draw( root -> GW, *root -> minefield);
    }
    
    {
#ifndef NO_TERM
      MS_stream     *mss       = root -> mss;
      MS_field      *minefield = root -> minefield;
      
#ifdef DEBUG
      if( mss -> deb != NULL){
	__uint64_t mytime = getnanosec() - root -> tutime;
	
	DEBUG_PRINT( mss -> deb, "\r\t\t\t\t\t\t\t %lu.%09lu      ", ( unsigned long)( ( mytime) / 1000000000), ( unsigned long)( ( mytime) % 1000000000));
      }
#endif
      
      if( !root -> gameover){
	if unlikely( minefield -> mine -> hit){
	  printtime( mss -> out, ( root -> tutime - root -> gamestart) / 1000000);
	  MS_print( mss -> out, "\r\t\t\t Mine!!               \n");
	  root -> gameover = TRUE;
	}else if unlikely( ( minefield -> mine -> uncoverd == ( minefield -> mine -> noelements - minefield -> mine -> level))){
	  printtime( mss -> out, ( root -> tutime - root -> gamestart) / 1000000);
	  MS_print( mss -> out, "\r\t\t\t Win!!         \n");
	  root -> gameover = TRUE;
	}
	
	if( !root -> gameover){
	  MS_print( mss -> out, "\r\t\t\t %lu of %lu      ", minefield -> mine -> flaged, minefield -> mine -> level);
	}
      }
      
      printtime( mss -> out, ( root -> tutime - root -> gamestart) / 1000000);
#endif
    }
  }
  
  ROOT_Free( root);
  fprintf( stdout, "\r"); /* we never want this line to be optimazie out */
  return -1;
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
