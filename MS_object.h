

#ifdef _MS_OBJ_H__
#else
#define _MS_OBJ_H__
#ifdef __cplusplus
extern "C" {
#endif
  
  typedef struct{
    void *func_ptr;
    char *name;
    void *varg;
  }MS_object;
  
#ifdef __cplusplus
}
#endif
#endif
