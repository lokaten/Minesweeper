

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
    s32 x;
    s32 y;
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
  
  static inline MS_element *
  acse( const MS_field field, s32 x, s32 y){
    
    if( x < 0 || y < 0 || x >= ( s32)field.width || y >= ( s32)field.height){
      x = ( x + field.width ) % field.width;
      y = ( y + field.height) % field.height;
    }
    
    return field.data + x + y * field.width;
  }
  
  void setzero( MS_video);
  MS_field *MF_CreateFieldFromLocal( FreeNode *, const MS_field *);
#define MF_CreateField( freenode, ...) MF_CreateFieldFromLocal( freenode, &( MS_field){__VA_ARGS__})
  void *setminefield( void);
  void MF_FreeField( FreeNode *, const MS_field *);
  void *uncov_workthread( void *);
  void setmine_elements( MS_field *,  MS_video);
  void uncov_elements( MS_field *, MS_video);
  void uncov_field( MS_field *);
  
#ifdef __cplusplus
}
#endif
#endif
