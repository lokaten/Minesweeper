
#include "MS_util.h"

void *
MS_CreateArrayFromSizeAndLocal( FreeNode *freenode, const size_t num_mem, const size_t size, const void *ptr){
  u32 i = num_mem;
  uintptr_t addr = 0;
  size_t alo_size;
  FreeNode *nf = freenode;
  assert( size);
  assert( num_mem);
  assert( freenode != NULL);
  alo_size = ( num_mem * size + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  while( addr == 0){
    if( nf -> end >= nf -> begining + alo_size){
      addr = nf -> begining;
      nf -> begining += alo_size;
      if( nf -> end == nf -> begining &&
	  nf != freenode){
	MS_Free( freenode, nf, FreeNode);
      }
    }else if( nf -> next == ( uintptr_t)freenode){
      size_t slab_alo_size = ( alo_size + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
      uintptr_t new_slab = ( uintptr_t)MS_CreateSlabFromSize( slab_alo_size);
      FreeNode *ff = MS_CreateLocal( FreeNode,
				     .prev     = ( uintptr_t)nf,
				     .next     = ( uintptr_t)freenode,
				     .begining = new_slab,
				     .end      = new_slab + slab_alo_size);
      addr           = ff -> begining;
      ff -> begining += alo_size;
      if( ff -> end != ff -> begining){
	ff = ( FreeNode *)MS_CreateFromLocal( freenode, FreeNode, ff);
	freenode -> prev = ( uintptr_t)ff;
	nf       -> next = ( uintptr_t)ff;
      }
    }
    
    DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   alo_size: %u  \n", SLAB_SIZE, nf -> end - nf -> begining, alo_size);
    
    nf = ( FreeNode *)nf -> next;
  }
  while( i--){
    memcpy( ( void *)( addr + i * size), ptr, size);
  }
  return ( void *)addr;
}


const void *
MS_FreeFromSize( FreeNode *freenode, const void * vaddr, const size_t size){
  const uintptr_t addr = ( const uintptr_t)vaddr;
  FreeNode *nf = freenode, *ff = NULL;
  size_t alo_size = ( size + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  assert( freenode != NULL);
  assert( addr != 0);
  while( ff == NULL){
    
    if( nf -> end == addr){
      nf -> end += alo_size;
      ff = nf;
    }else if( nf -> begining == addr + alo_size){
      nf -> begining -= alo_size;
      ff = nf;
    }else if( nf -> next == ( uintptr_t)freenode){
      ff = MS_CreateLocal( FreeNode,
			   .prev     = ( uintptr_t)nf,
			   .next     = ( uintptr_t)freenode,
			   .begining = addr,
			   .end      = addr + alo_size);
      if( ff -> end >= ff -> begining + SLAB_SIZE){
	size_t slab_size = ff -> end - ff -> begining;
	slab_size -= slab_size % SLAB_SIZE;
	MS_FreeSlabFromSize( ( void *)ff -> begining, slab_size);
	ff -> begining += slab_size;
      }
      if( ff -> begining != ff -> end){
	ff = ( FreeNode *)MS_CreateFromLocal( freenode, FreeNode, ff);
	nf       -> next = ( uintptr_t)ff;
	freenode -> prev = ( uintptr_t)ff;
      }
    }
    
    nf = ( FreeNode *)nf -> next;
  }
  
  if( ff -> end >= ff -> begining + SLAB_SIZE){
    size_t slab_size = ff -> end - ff -> begining;
    slab_size -= slab_size % SLAB_SIZE;
    MS_FreeSlabFromSize( ( void *)ff -> begining, slab_size);
    ff -> begining += slab_size;
  }
  
  DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   free_size: %u  \n",  SLAB_SIZE, ff -> end - ff -> begining, size);
  
  return NULL;
}
