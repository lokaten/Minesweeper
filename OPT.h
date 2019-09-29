

#ifdef OPT_H__
#else
#define OPT_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "MS_util.h"


typedef struct{
  uintptr_t func;
  const char *discript;
  const char *name;
  const char chr;
  void *data;
  const void *value;
}MS_options;


int arg_too_ul( char **, void *);
int arg_too_x( char **, void *);
int arg_too_cpy( char **, void *);


#define OPTSW_NUL 0
#define OPTSW_LU  ( uintptr_t)&arg_too_ul
#define OPTSW_X   ( uintptr_t)&arg_too_x
#define OPTSW_GRP 4
#define OPTSW_CPY ( uintptr_t)&arg_too_cpy
#define OPTSW_RAW 6

#define OPT_MAX 200

static inline int procopt( const MS_stream *, const MS_options *, const int, const char **);
static inline int help( FILE *, const MS_options *);

int
arg_too_ul( char **arg, void *popt){
  int ret = 1;
  char *lr = NULL;
  MS_options opt = *(MS_options *)popt;
  *( unsigned long *)( opt.data) = strtoul( arg[ ret], &lr, 10);
  if( *lr != 0 || ( errno != 0)){
    ret = -1;
  }
  return ret;
}

int
arg_too_x( char **arg, void *popt){
  int ret = 1;
  char *lr = NULL;
  MS_options opt = *(MS_options *)popt;
  *( unsigned long *)( opt.data) = strtoul( arg[ ret], &lr, 10);
  if( *lr != 0 || ( errno != 0)){
    ret = -1;
  }
  return ret;
}

int
arg_too_cpy( char **arg, void *popt){
  int ret = 0;
  MS_options opt = *(MS_options *)popt;
  ( void)arg;
  *( const void **)( opt.data) = ( const void *)( opt.value);
  return ret;
}

static inline int
procopt( const MS_stream *mss, const MS_options *opt, const int argc, const char **argv){
  int ret = 0;
  int i = 0;
  
  while( ++i < argc){
    if( strstr( argv[ i], "--") == argv[ i]){
      signed long j = -1;
      while( ++j < OPT_MAX){
        if( ( uintptr_t)opt[ j].func){
          if( strcmp( argv[ i] + 2, opt[ j].name) == 0){
            errno = 0;
	    {
	      int ( *func)( const char **, const void *) = ( int( *)( const char **, const void *))opt[j].func;
	      ret = func( &argv[ i], ( const void *)&opt[ j]);
	      if( ret > 0){
		i += ret;
		ret = 0;
	      }
	    }
            break;
          }
        }else{
          MS_print( mss -> err, "\rOption \'%s\' dosen't exist\n", argv[ i]);
          ret = -1;
          break;
        }
      }
      continue;
    }
    
    if( strstr( argv[ i], "-") == argv[ i]){
      unsigned long k = 0;
      while( ++k < strlen( argv[ i])){
        signed long j = -1;
        while( ++j < OPT_MAX){
          if( ( uintptr_t)opt[ j].func){
            if( *( argv[ i] + k) &&
                ( *( argv[ i] + k) == opt[ j].chr)){
              errno = 0;
	      {
		int ( *func)( const char **, const void *) = ( int( *)( const char **, const void *))opt[j].func;
		ret = func( &argv[ i], ( const void *)&opt[ j]);
		if( ret > 0){
		  i += ret;
		  ret = 0;
		}
	      }
	      break;
	    }
          }else{
            MS_print( mss -> err, "\rShort option \'%c\' dosen't exist\n", *( argv[ i] + k));
            ret = -1;
            break;
          }
        }
      }
      continue;
    }
    
    ret = -1;
  }

  return ret;
}


static inline int
help( FILE *stream, const MS_options *opt){
  int ret = 0;
#ifdef NO_TERM
  ( void) stream;
  ( void) opt;
#else
  int j = -1;
  
  while( ++j < OPT_MAX &&
	 opt[ j].func != 0){
    if( ( uintptr_t)opt[ j].func == OPTSW_GRP){
      MS_print( stream, "\r%s: \n", opt[ j].discript);
    }else if( ( uintptr_t)opt[ j].func == OPTSW_X){
      MS_print( stream, "\r\t --%s ", opt[ j].name);
      MS_print( stream, "\r\t\t\t\t\t <int> ");
      MS_print( stream, "\r\t\t\t\t\t\t %s \n", opt[ j].discript);
    }else if( ( uintptr_t)opt[ j].func == OPTSW_LU){
      MS_print( stream, "\r\t --%s ", opt[ j].name);
      MS_print( stream, "\r\t\t\t\t\t <hex> ");
      MS_print( stream, "\r\t\t\t\t\t\t %s \n", opt[ j].discript);
    }else if( ( uintptr_t)opt[ j].func == OPTSW_CPY){
      MS_print( stream, "\r\t --%s ", opt[ j].name);
      MS_print( stream, "\r\t\t\t\t %c ", opt[ j].chr);
      MS_print( stream, "\r\t\t\t\t\t\t %s \n", opt[ j].discript);
    }
  }
#endif
  return ret;
}


#ifdef __cplusplus
}
#endif
#endif
