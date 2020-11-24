

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
    const u32 level;
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
    pthread_mutex_t mutex_field;
    pthread_mutexattr_t attr_mutex;
  }MS_field;
  
  static inline MS_element *acse( const MS_field, int, int);
  
  static inline MS_element *
  acse( const MS_field field, int x, int y){
    return field.data + ( mol_( (u32)( x + (int)field.width ), field.width , field.width_divobj ) +
			  mol_( (u32)( y + (int)field.height), field.height, field.height_divobj) * field.width);
  }
  
  void setzero( MS_field *, MS_video);
  MS_field *MF_CreateFieldFromLocal( FreeNode *, const MS_field *);
#define MF_CreateField( freenode, ...) MF_CreateFieldFromLocal( freenode, &( MS_field){__VA_ARGS__})
  void *setminefield( void *);
  void MF_FreeField( FreeNode *, const MS_field *);
  void *uncov_workthread( void *);
  void *uncov( void *);
  void uncov_elements( void *,  MS_video);
  
#ifdef __cplusplus
}
#endif
#endif
