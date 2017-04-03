

#ifdef _OPT_H__
#else
#define _OPT_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "MS_util.h"

typedef struct{
  unsigned long optsw;
  char *discript;
  char *name;
  char chr;
  unsigned long *data;
}MS_options;


#define OPTSW_NUL  0
#define OPTSW_BO   1
#define OPTSW_LU   2
#define OPTSW_X    3
#define OPTSW_GRP  4

#define OPT_MAX 200

int procopt( MS_stream, MS_options *, unsigned, char **);
void help( MS_stream, MS_options *);
  

#ifdef __cplusplus
}
#endif
#endif
