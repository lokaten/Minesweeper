

#ifdef MS_MF_H__
#else
#define MS_MF_H__
#ifdef __cplusplus
extern "C" {
#endif
  
#include "ComandStream.h"
  
  typedef struct{
    u8 count:4;
    u8 mine:1;
    u8 cover:1;
    u8 flag:1;
    u8 set:1;
  }MS_element;
  
  
  typedef struct{
    s16 x;
    s16 y;
  }MS_pos;

  
  typedef struct{
    u32 uncoverd;
    u32 set;
    u32 mines;
    u32 level;
    u32 flaged;
    const u32 noelements;
    u32 hit;
    u32 seed;
  }MS_mstr;

  
  typedef struct{
    MS_element *data;
    const char *title;
    ComandStream *uncovque;
    MS_mstr *mine;
    const u32 width;
    const u32 width_divobj;
    const u32 height;
    const u32 height_divobj;
    const u32 subwidth;
    const u32 subheight;
    const u32 level;
    const u32 global;
    const u32 reseed;
  }MS_field;
  
  
#ifdef LOCALE_
  static inline MS_element *LOCALE_( acse)( const MS_field, int, int);
  
  static inline MS_element *
  LOCALE_( acse)( const MS_field field, int x, int y){
    return field.data + ( mol_( (u32)( x + (int)field.width ), field.width , field.width_divobj ) +
			  mol_( (u32)( y + (int)field.height), field.height, field.height_divobj) * field.width);
  }
  
#define acse LOCALE_( acse)
  
#endif
  
  void setzero( const MS_field *, MS_video);
  MS_field *MF_CreateFieldFromRef( const MS_field *);
#define MF_CreateField( ...) MF_CreateFieldFromRef( &( MS_field){__VA_ARGS__})
  void setminefield( const MS_field *, void*, const MS_stream *, MS_video);
  void MF_FreeFieldPartial( const MS_field *);
  void MF_FreeField( MS_field *);
  void uncov( const MS_field *, void *);
  void uncov_elements( const MS_field *,  MS_video);
  
  /**/
  void drawelement( void *, const MS_field *, s16, s16);
  
#ifdef __cplusplus
}
#endif
#endif
