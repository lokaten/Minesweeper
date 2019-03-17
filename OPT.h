

#ifdef OPT_H__
#else
#define OPT_H__
#ifdef __cplusplus
extern "C" {
#endif
  
#include "MS_util.h"

  typedef enum{
    OPTSW_NUL = 0,
    OPTSW_LU  = 2,
    OPTSW_X   = 3,
    OPTSW_GRP = 4,
    OPTSW_CPY = 5,
    OPTSW_RAW = 6,
  }opttype;
  
  
  typedef struct{
    const opttype optsw;
    const char *discript;
    const char *name;
    const char chr;
    void *data;
    const void *value;
  }MS_options;
  
#define OPT_MAX 200
  
  int procopt( const MS_stream *, const MS_options *, const int, const char **);
  int help( FILE *, const MS_options *);
  
  
#ifdef __cplusplus
}
#endif
#endif
