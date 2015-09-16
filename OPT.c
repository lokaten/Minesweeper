
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define LOCALE_( name) OPT_##name

#include "MS_util.h"
#include "OPT.h"

int
procopt( MS_stream mss, MS_options *opt, unsigned argc, char **argv){
  unsigned long i = 0;
  unsigned long ret = 0;
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
                *( opt[ j].data) = strtoul( argv[ i], &lr, 10);
                if( *lr != 0 || ( errno != 0)){
                  ret = -1;
                }
              }
              break;
            case OPTSW_X:
              if( ++i < argc){
                *( opt[ j].data) = strtoul( argv[ i], &lr, 16);
                if( *lr != 0 || ( errno != 0)){
                  ret = -1;
                }
              }
              break;
            case OPTSW_BO:
              *( opt[ j].data) = TRUE;
              break;
            }
            break;
          }
        }else{
          MS_print( mss.err, "\rOption \'%s\' dosen't exist\n", argv[ i]);
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
                  *( opt[ j].data) = strtoul( argv[ i + 1], &lr, 10);
                  if( *lr != 0 || ( errno != 0)){
                    ret = -1;
                  }
                }
                break;
              case OPTSW_BO:
                *( opt[ j].data) = TRUE;
                break;
              }
              break;
            }
          }else{
            MS_print( mss.err, "\rShort option \'%c\' dosen't exist\n", *( argv[ i] + k));
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


void
help( MS_stream mss, MS_options *opt){
  
  signed long j = -1;

  while( ++j < OPT_MAX){
    switch( opt[ j].optsw){
    case OPTSW_GRP:
      MS_print( mss.out, "\r%s: \n", opt[ j].name);
      break;
    case OPTSW_LU:
      MS_print( mss.out, "\r\t --%s ", opt[ j].name);
      MS_print( mss.out, "\r\t\t\t\t\t <int> ");
      MS_print( mss.out, "\r\t\t\t\t\t\t %s \n", opt[ j].discript);
      break;
    case OPTSW_X:
      MS_print( mss.out, "\r\t --%s ", opt[ j].name);
      MS_print( mss.out, "\r\t\t\t\t\t <hex> ");
      MS_print( mss.out, "\r\t\t\t\t\t\t %s \n", opt[ j].discript);
      break;
    case OPTSW_BO:
      MS_print( mss.out, "\r\t --%s ", opt[ j].name);
      MS_print( mss.out, "\r\t\t\t\t %c ", opt[ j].chr);
      MS_print( mss.out, "\r\t\t\t\t\t\t %s \n", opt[ j].discript);
      break;
    case OPTSW_NUL:
      return;
    }
  }
  
  return;
}

