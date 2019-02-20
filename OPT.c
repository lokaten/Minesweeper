
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define LOCALE_( name) OPT_##name

#include "MS_util.h"
#include "OPT.h"

int
procopt( MS_stream *mss, MS_options *opt, const int argc, const char **argv){
  int ret = 0;
  int i = 0;
  char *lr = NULL;
  
  while( ++i < argc){
    if( strstr( argv[ i], "--") == argv[ i]){
      signed long j = -1;
      while( ++j < OPT_MAX){
        if( opt[ j].optsw){
          if( strcmp( argv[ i] + 2, opt[ j].name) == 0){
            errno = 0;
            switch( opt[ j].optsw){
            case OPTSW_LU:
              if( ++i < argc){
                *( unsigned long *)( opt[ j].data) = strtoul( argv[ i], &lr, 10);
                if( *lr != 0 || ( errno != 0)){
                  ret = -1;
                }
              }
              break;
            case OPTSW_X:
              if( ++i < argc){
                *( unsigned long *)( opt[ j].data) = strtoul( argv[ i], &lr, 16);
                if( *lr != 0 || ( errno != 0)){
                  ret = -1;
                }
              }
              break;
            case OPTSW_CPY:
              *( void **)( opt[ j].data) = ( void *)( opt[ j].value);
              break;
            case OPTSW_RAW:
              /* *( void **)( opt[ j].data) = ( void *)argv[ i]; */
              break;
	    default:
	      ret = -1;
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
          if( opt[ j].optsw){
            if( *( argv[ i] + k) &&
                ( *( argv[ i] + k) == opt[ j].chr)){
              errno = 0;
              switch( opt[ j].optsw){
              case OPTSW_LU:
                if( ( i + 1) < argc){
                  *( unsigned long *)( opt[ j].data) = strtoul( argv[ i + 1], &lr, 10);
                  if( *lr != 0 || ( errno != 0)){
                    ret = -1;
                  }
                }
                break;
              case OPTSW_CPY:
                *( void **)( opt[ j].data) = ( void *)( opt[ j].value);
                break;
	      default:
		ret = -1;
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


int
help( FILE *stream, MS_options *opt){
  int ret = 0;
#ifdef NO_TERM
  ( void) stream;
  ( void) opt;
#else
  int j = -1;
  
  while( ++j < OPT_MAX){
    switch( opt[ j].optsw){
    case OPTSW_GRP:
      MS_print( stream, "\r%s: \n", opt[ j].discript);
      break;
    case OPTSW_LU:
      MS_print( stream, "\r\t --%s ", opt[ j].name);
      MS_print( stream, "\r\t\t\t\t\t <int> ");
      MS_print( stream, "\r\t\t\t\t\t\t %s \n", opt[ j].discript);
      break;
    case OPTSW_X:
      MS_print( stream, "\r\t --%s ", opt[ j].name);
      MS_print( stream, "\r\t\t\t\t\t <hex> ");
      MS_print( stream, "\r\t\t\t\t\t\t %s \n", opt[ j].discript);
      break;
    case OPTSW_CPY:
      MS_print( stream, "\r\t --%s ", opt[ j].name);
      MS_print( stream, "\r\t\t\t\t %c ", opt[ j].chr);
      MS_print( stream, "\r\t\t\t\t\t\t %s \n", opt[ j].discript);
      break;
    case OPTSW_NUL:
      return ret;
    default:
      ret = -1;
    }
  }
#endif
  return ret;
}

