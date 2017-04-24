

#ifdef _MS_MF_H__
#else
#define _MS_MF_H__
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
    u32 global;
    u32 reseed;
    u32 level;
  }MS_field;
  
  
#ifdef LOCALE_
  INLINE __uint8_t *LOCALE_( acse)( MS_field, int, int);
    
  INLINE __uint8_t *
  LOCALE_( acse)( MS_field field, int x, int y){
    return field.data + ( mol_( ( ( x) + field.width ), field.width , field.width_divobj ) +
			  mol_( ( ( y) + field.height), field.height, field.height_divobj) * field.width);
  }
#define acse LOCALE_( acse)
  
#endif
  
  void setzero( MS_field, MS_mstr *, MS_video);
  MS_field *MF_Init( MS_field *);
  void setminefield( MS_field *, MS_stream *, MS_video);
  void MF_Free( MS_field *);
  int uncov( MS_field *);
  int uncov_elements( MS_field, ComandStream *, MS_video, MS_mstr *);
  
#ifdef __cplusplus
}
#endif
#endif
