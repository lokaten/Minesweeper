
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include <limits.h>

#include "MS_util.h"
#include "userinterface.h"
#include "minefield.h"
#include "OPT.h"

#include "debug.h"

static inline const MS_root * ROOT_FreeRoot( const MS_root *);
MS_root *ROOT_Init( const int, const char **);
static inline void printtime( FILE *, u64);

FILE *debug_out;

FUNC_DEF( void, FUNC_quit){
  int ret = 0;
  FILE *f;
  assert( parm -> root != NULL);
  assert( parm -> root -> mss != NULL);
  f = parm -> root -> mss -> out;
  ROOT_FreeRoot( parm -> root);
  MS_print( f, TERM("\rBye!                                \n"));
  exit( ret);
}

static inline const MS_root *
ROOT_FreeRoot( const MS_root *proot){
  const MS_root *root;
  assert( proot != NULL);
  root = MS_CreateLocalFromLocal( MS_root, proot);
  GW_Free( root -> freenode, root -> GW);
  MS_Free( root -> freenode, proot);
  MS_Free( root -> freenode, root -> mss);
  MF_FreeField( root -> freenode, root -> minefield);
  MS_FreeFreeList( root -> freenode);
  return NULL;
}

MS_root *
ROOT_Init( const int argc, const char **argv){
  MS_root *root;
  FreeNode *freenode;
  
  MS_video real = { 0};
  MS_field *minefield;
  MS_stream *mss;
  bool no_resize = 0;
  
  u32 custom = FALSE; // procopt will overflow _Bool
  u32 custom_global = FALSE;
  char *custom_title = "Custom";
  u32 custom_reseed = 0;
  u16 custom_width = 0;
  u16 custom_height = 0;
  u32 custom_level = 0;
  
  u32 benchmark = FALSE;
  
  MS_field *field_beginner  = MS_CreateLocal( MS_field, .title = "Beginner" , .width =     9, .height =     9, .level =     10, .global = 0, .reseed = 0);
  MS_field *field_advanced  = MS_CreateLocal( MS_field, .title = "Advanced" , .width =    16, .height =    16, .level =     40, .global = 0, .reseed = 0);
  MS_field *field_expert    = MS_CreateLocal( MS_field, .title = "Expert"   , .width =    30, .height =    16, .level =     99, .global = 0, .reseed = 0);
  MS_field *field_extrem    = MS_CreateLocal( MS_field, .title = "Extrem"   , .width =    32, .height =    18, .level =    127, .global = 0, .reseed = 0);
  MS_field *field_fractal   = MS_CreateLocal( MS_field, .title = "Fractal"  , .width =  2000, .height =  1200, .level = 215000, .global = 0, .reseed = 0);
  MS_field *field_benchmark = MS_CreateLocal( MS_field, .title = "Benchmark", .width = 32000, .height = 18000, .level =    127, .global = 0, .reseed = 0);
  
#ifndef NO_TERM
  MS_stream *very_quiet = MS_CreateLocal( MS_stream, .out = NULL  , .err = NULL  , .deb = NULL, .hlp = NULL);
#endif
  
  MS_stream *def_out    = MS_CreateLocal( MS_stream, .out = stdout, .err = stderr, .deb = NULL, .hlp = NULL);
  
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
      { OPTSW_STR, TERM("Window Title"                           ), "title"          , 0  , &custom_title               , NULL},
      { OPTSW_LU , TERM("Element high minefield"                 ), "height"         , 0  , &custom_height              , NULL},
      { OPTSW_LU , TERM("Element wide minefield"                 ), "width"          , 0  , &custom_width               , NULL},
      { OPTSW_LU , TERM("Number of mines"                        ), "level"          , 0  , &custom_level               , NULL},
      { OPTSW_X  , TERM("seed"                                   ), "seed"           , 0  , &custom_reseed              , NULL},
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
      { OPTSW_CPY, TERM("customaise your own"                    ), "custom"         , 'c', &custom                     , &opt_true},
      { OPTSW_CPY, TERM("Mimic windows minesweeper beginner mode"), "beginner"       , 'b', &minefield                  , field_beginner },
      { OPTSW_CPY, TERM("Mimic windows minesweeper advanced mode"), "advanced"       , 'a', &minefield                  , field_advanced },
      { OPTSW_CPY, TERM("Mimic windows minesweeper expert mode"  ), "expert"         , 'e', &minefield                  , field_expert   },
      { OPTSW_CPY, TERM(""                                       ), "extrem"         , 'x', &minefield                  , field_extrem   },
      { OPTSW_CPY, TERM(""                                       ), "fractal"        , 'f', &minefield                  , field_fractal  },
      { OPTSW_CPY, TERM(""                                       ), "benchmark"      , 'B', &benchmark                  , &opt_true      },
#ifndef NO_TERM
      { OPTSW_GRP, TERM("Output"                                 ), ""               , 0  , NULL                       , NULL},
      { OPTSW_CPY, TERM("Print generic help mesage"              ), "help"           , 'h', &( def_out -> hlp         ), stdout},
      { OPTSW_CPY, TERM("Supres reguler output"                  ), "quiet"          , 'q', &( def_out -> out         ), NULL},
      { OPTSW_CPY, TERM("Supres all output"                      ), "very-quiet"     , 'Q', &(mss                     ), very_quiet},
      { OPTSW_CPY, TERM("Debug data"                             ), "debug"          , 'd', &( def_out -> deb         ), stdout},
#endif
      { OPTSW_NUL, TERM(""/* Last elemnt is a NULL termination */), NULL             , 0  , NULL                       , NULL}};
    
    if( procopt( mss, opt, argc, argv) < 0){
      MS_print( mss -> err, TERM("\rWrong or broken input, pleas refer to --help\n"));
    }
#ifndef NO_TERM
    if( mss -> hlp){
      help( mss -> hlp, opt);
      
      quit( &( const MS_root){ .mss = mss});
    }
#endif
  }
  
#ifdef DEBUG
  debug_out = mss -> deb;
#endif
  
  freenode = MS_CreateFreeList();
  
  if( benchmark){
    minefield = field_benchmark;
  }
  
  if( minefield == field_fractal){
    real.element_width  = real.element_width ? real.element_width : 1;
    real.element_height = real.element_height? real.element_height: 1;
  }else{
    real.element_width  = real.element_width ? real.element_width : 15;
    real.element_height = real.element_height? real.element_height: 15;
  }
  
  if( custom){
    custom_width  = custom_width  ? custom_width : minefield -> width;
    custom_height = custom_height ? custom_height: minefield -> height;
    custom_level  = custom_level  ? custom_level : minefield -> level;
    
    if( custom_level >= ( u32)( custom_width * custom_height)){
      custom_level = ( custom_width * custom_height + 1) / 3;
      MS_print( mss -> err, TERM("\rMore mines then elments!\n"));
    }
    
    minefield = MF_CreateField( freenode, .title = custom_title, .width = custom_width, .height = custom_height, .level = custom_level, .global = custom_global, .reseed = custom_reseed);
  }else{
    minefield = MF_CreateFieldFromLocal( freenode, minefield);
  }
  
  {
    real.realwidth  = real.realwidth ? real.realwidth : minefield -> width  * real.element_width;
    real.realheight = real.realheight? real.realheight: minefield -> height * real.element_height;
    
    real.width  = real.realwidth  / real.element_width ;
    real.height = real.realheight / real.element_height;
  }
  
  mss = MS_CreateFromLocal( freenode, MS_stream, mss);
  
  assert( minefield -> title != NULL);
  
  MS_print( mss -> out, "\rMode: %s\n", minefield -> title);
  
  if( custom || mss -> deb != NULL){
    MS_print( mss -> out, "\rWidth: %lu   ", minefield -> subwidth);
    MS_print( mss -> out, "\r\t\tHeight: %lu   ", minefield -> subheight);
    MS_print( mss -> out, "\r\t\t\t\tLevel: %lu   \n", minefield -> level);
  }
  
  root = MS_Create( freenode, MS_root,
		    .freenode = freenode,
		    .real = real,
		    .minefield = minefield,
		    .mss = mss,
		    .no_resize = no_resize);
  
  pthread_create( NULL, NULL, uncov_workthread, ( void *)root);
  
  setminefield( root);
  
  if( benchmark){
    setzero( root, ( MS_video){ .xdiff = 320, .ydiff = 180, .width  = 3, .height = 3});
    uncov_elements( root, ( MS_video){ .xdiff =  321, .ydiff =  181, .width  = 1, .height = 1});
    uncov( root);
    
    {
      u64 tutime;
      u64 gamestart;
      
      tutime    = getnanosec();
      gamestart = tutime;
      
      while( TRUE){
	struct timespec tv;
	
	clock_gettime( CLOCK_REALTIME, &tv);
	
	tv.tv_nsec += 150000000;
	
	CS_iswaiting( root -> minefield -> uncovque, tv, 1);
	
	tutime = getnanosec();
	
	printtime( root -> mss -> out, ( tutime - gamestart) / 1000000);
	
	assert( root -> minefield -> mine -> mines <= root -> minefield -> mine -> level);
	assert( root -> minefield -> mine -> set   <= root -> minefield -> mine -> noelements);
	
	if( ( root -> minefield -> mine -> uncoverd == ( root -> minefield -> mine -> noelements - root -> minefield -> mine -> level))){
	  MS_print( root -> mss -> out, TERM( "\r\t\t\t Win!!         \n"));
	  break;
	}else{
	  MS_print( root -> mss -> out, TERM( "\r\t\t\t %lu of %lu      "), root -> minefield -> mine -> uncoverd, root -> minefield -> mine -> noelements);
	}
	
      }
    }
    
    quit( root);
  }
  
  // minefield test for drawque before submiting to drawque
  root -> drawque = CS_CreateStream( freenode, DrawComand);
  
  root -> GW = GW_Init( root -> freenode, root);
  
  draw( root);
  
  return root;
}


int
main( const int argc, const char** argv){
  MS_root *root;
  
  bool gameover;
  u64 tutime;
#ifdef DEBUG
  u64 wtime = 0, wtutime = 0, ftime[ 16] = { 0}, fos = 0;
#endif
  u64 gamestart;
  
  root = ROOT_Init( argc, argv);
  
  gameover  = FALSE;
  tutime    = getnanosec();
  gamestart = tutime;
  
  root -> idle = 0;
  
  while( TRUE){
    
    event_dispatch( root);
    
#ifdef DEBUG
    ftime[ fos] = getmicrosec();
#endif
    
    tutime = getnanosec();
    
    if( !root -> minefield -> mine -> uncoverd || gameover){
      gamestart = tutime;
    }
    
    if( !root -> idle){
      
      
      assert( root -> minefield -> mine -> mines <= root -> minefield -> mine -> level);
      assert( root -> minefield -> mine -> set   <= root -> minefield -> mine -> noelements);
      
      if( !gameover){
	if( root -> minefield -> mine -> hit){
	  pthread_create( NULL, NULL, uncov_field, ( void *)root);
	  
	  MS_print( root -> mss -> out, TERM( "\r\t\t\t Mine!!        "));
	  gameover = TRUE;
	}else if( ( root -> minefield -> mine -> uncoverd == ( root -> minefield -> mine -> noelements - root -> minefield -> mine -> level))){
	  printtime( root -> mss -> out, ( tutime - gamestart) / 1000000);
	  MS_print( root -> mss -> out, TERM( "\r\t\t\t Win!!         \n"));
	  gameover = TRUE;
	}else{
	  MS_print( root -> mss -> out, TERM( "\r\t\t\t %lu of %lu    "), root -> minefield -> mine -> flaged, root -> minefield -> mine -> level);
	}
      }else if( !root -> minefield -> mine -> uncoverd){
	gameover = FALSE;
      }
    }
    
    draw( root);
    
#ifdef DEBUG
    if( root -> mss -> deb != NULL){
      u64 mytime = getnanosec() - tutime;
      
      if( mytime > wtime ||
	  getnanosec() - wtutime > 2000000000){
	wtime = mytime;
	wtutime = getnanosec();
      }
      
      fos = ( fos + 1) % 16;
      
      DEBUG_PRINT( root -> mss -> deb, "\r\t\t\t\t\t\t\t %lu.%09lu      %lu.%09lu      fps: %lf      ", ( u64)( ( mytime) / 1000000000), ( u64)( ( mytime) % 1000000000), ( u64)( ( wtime) / 1000000000), ( u64)( ( wtime) % 1000000000), ( double)16000000 / ( double)( getmicrosec() - ftime[ fos]));
    }
#endif
    
    printtime( root -> mss -> out, ( tutime - gamestart) / 1000000);
  }
}


static inline void
printtime( FILE * stream, u64 time){
#ifdef NO_TERM
  ( void) stream;
  ( void) time;
#else
  if( time > U64C( 60000)){
    if( time > U64C( 3600000)){
      MS_print( stream, "\r%lu:", ( time / U64C( 3600000))            );
      MS_print( stream, "%02lu:", ( time / U64C( 60000  )) % U64C( 60));
    }else{
      MS_print( stream, "\r%lu:", ( time / U64C( 60000  )) % U64C( 60));
    }
    MS_print( stream, "%02lu.", ( time / U64C( 1000   )) % U64C( 60));
  }else{
    MS_print( stream, "\r%lu.", ( time / U64C( 1000   )) % U64C( 60));
  }
  MS_print( stream, "%03lu                 ", time % U64C( 1000));
#endif
  return;
}
