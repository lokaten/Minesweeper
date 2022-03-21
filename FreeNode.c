
#include <stdlib.h> // malloc
#include <string.h> // memcpy

#include "MS_util.h"

FreeNode *
MS_CreateFreeList( void){
  return NULL;
}

void *
MS_FreeFreeList( FreeNode *freenode){
  ( void)freenode;
  return NULL;
}

address
MS_CreateArrayFromSizeAndLocal( FreeNode *freenode, const size_t num_mem, const size_t size, const void *local){
  address addr = ( address)calloc( num_mem, size);
  
  ( void)freenode;
    
  {
    u32 i = num_mem;
    while( i--){
      memcpy( ( void *)( addr + i * size), local, size);
    }
  }
   
   
  return addr;
}


address
MS_FreeFromSize( FreeNode *freenode, const address addr, const size_t size){
  ( void)freenode;
  ( void)size;
  if( addr){
    free( ( void *)addr);
  }
  return 0;
}


