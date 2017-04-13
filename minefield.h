

#ifdef _MS_MF_H__
#else
#define _MS_MF_H__
#ifdef __cplusplus
extern "C" {
#endif
  
#include "ComandStream.h"
  
  typedef struct{
    unsigned long uncoverd;
    unsigned long set;
    unsigned long mines;
    unsigned long level;
    unsigned long flaged;
    unsigned long noelements;
    unsigned long hit;
    __uint32_t seed;
  }MS_mstr;

  
  typedef struct{
    __uint8_t *data;
    char *title;
    ComandStream *uncovque;
    MS_mstr *mine;
    unsigned long width;
    unsigned long width_divobj;
    unsigned long height;
    unsigned long height_divobj;
    unsigned long subwidth;
    unsigned long subheight;
    unsigned long global;
    unsigned long reseed;
    unsigned long level;
  }MS_field;
  
  
#ifdef LOCALE_
  INLINE __uint8_t *LOCALE_( acse)( MS_field, int, int);
  /*
  INLINE __uint8_t *
  LOCALE_( acse)( MS_field field, int x, int y){
    return field.data + ( ( ( ( x) + field.width ) % field.width ) +
			  ( ( ( y) + field.height) % field.height) * field.width);
  }
#define acse LOCALE_( acse)
  */
  
  INLINE __uint8_t *
  LOCALE_( acse)( MS_field field, int x, int y){
    return field.data + ( mol_( ( ( x) + field.width ), field.width , field.width_divobj ) +
			  mol_( ( ( y) + field.height), field.height, field.height_divobj) * field.width);
  }
#define acse LOCALE_( acse)
  
#endif
  
  void setzero( MS_field, MS_mstr *, MS_video);
  MS_field *MF_Create( MS_field);
  void setminefield( MS_field *, MS_stream *, MS_video);
  void MF_Free( MS_field *);
  int uncov( MS_field, ComandStream *, MS_mstr *);
  int uncov_elements( MS_field, ComandStream *, MS_video, MS_mstr *);
  
#ifdef __cplusplus
}
#endif
#endif
