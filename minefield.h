

#ifdef MS_MF_H__
#else
#define MS_MF_H__
#ifdef __cplusplus
extern "C" {
#endif
  
#include "ComandStream.h"
  
  typedef struct{
    u32 uncoverd;
    u32 set;
    u32 mines;
    u32 level;
    u32 flaged;
    u32 noelements;
    u32 hit;
    u32 seed;
  }MS_mstr;

  
  typedef struct{
    __uint8_t *data;
    const char *title;
    ComandStream *uncovque;
    MS_mstr *mine;
    u32 width;
    u32 width_divobj;
    u32 height;
    u32 height_divobj;
    u32 subwidth;
    u32 subheight;
    u32 level;
    u32 global;
    u32 reseed;
  }MS_field;
  
  
#ifdef LOCALE_
  INLINE __uint8_t *LOCALE_( acse)( MS_field, int, int);
  
  INLINE __uint8_t *
  LOCALE_( acse)( MS_field field, int x, int y){
    return field.data + ( mol_( (u32)( ( x) + (int)field.width ), field.width , field.width_divobj ) +
			  mol_( (u32)( ( y) + (int)field.height), field.height, field.height_divobj) * field.width);
  }
  
#define acse LOCALE_( acse)
  
#endif

  typedef struct{ MS_field *minefield; MS_video vid;}setzeroargs;
  int setzero( void *args);
  MS_field *MF_Init( MS_field *);
  typedef struct{ MS_field *minefield; MS_stream *mss; MS_video video;}setminefieldargs;
  int setminefield( void *);
  void MF_Free( MS_field *);
  typedef struct{ MS_field *minefield;}uncovargs;
  int uncov( void *);
  typedef struct{ MS_field *minefield;  MS_video vid;}uncov_elementsargs;  
  int uncov_elements( void *args);
  
#ifdef __cplusplus
}
#endif
#endif
