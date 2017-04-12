

#ifdef _OPT_H__
#else
#define _OPT_H__
#ifdef __cplusplus
extern "C" {
#endif
  
#include "MS_util.h"

  typedef enum{
    OPTSW_NUL = 0,
    OPTSW_BO  = 1,
    OPTSW_LU  = 2,
    OPTSW_X   = 3,
    OPTSW_GRP = 4,
  }opttype;
  
  
  typedef struct{
    opttype optsw;
    char *discript;
    char *name;
    char chr;
    unsigned long *data;
  }MS_options;
  
#define OPT_MAX 200
  
  int procopt( MS_stream *, MS_options *, unsigned, char **);
  int help( FILE *, MS_options *);
  
  
#ifdef __cplusplus
}
#endif
#endif
