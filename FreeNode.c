
#include "MS_util.h"

void *
MS_CreateArrayFromSizeAndLocal( FreeNode *freenode, const size_t num_mem, const size_t size, const void *ptr){
  uintptr_t addr = 0;
  FreeNode *ff = NULL;
  size_t alo_size = ( num_mem * size + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  assert( alo_size);
  assert( freenode != NULL);
  if( freenode -> end >= freenode -> begining + alo_size){
    ff = freenode;
  }else{
    {
      FreeNode *nf = ( FreeNode *)freenode -> next;
      
      while( nf != freenode){
	if( nf -> end >= nf -> begining + alo_size){
	  ff = MS_CreateLocal( FreeNode,
			       .begining = nf -> begining,
			       .end      = nf -> end);
	  ( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	  ( ( FreeNode *)nf -> prev) -> next = nf -> next;
	  MS_Free( freenode, nf, FreeNode);
	  break;
	}
	
	nf = ( FreeNode *)nf -> next;
      }
    }
    
    if( ff == NULL){
      size_t slab_alo_size = ( alo_size + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
      uintptr_t new_slab = ( uintptr_t)MS_CreateSlabFromSize( slab_alo_size);
      if( freenode -> end == freenode -> begining){
	ff = freenode;
      }else{
	ff = MS_CreateLocal( FreeNode, 0);
      }
      ff -> begining = new_slab;
      ff -> end      = new_slab + slab_alo_size;
    }
  }
  
  addr           = ff -> begining;
  ff -> begining += alo_size;
  
  if( ff -> end != ff -> begining &&
      ff != freenode){
    ff -> next = ( uintptr_t)ff;
    ff -> prev = ( uintptr_t)ff;
    if( freenode -> end >= freenode -> begining + sizeof( FreeNode)){
      ff = MS_CreateFromLocal( freenode, FreeNode, ff);
    }else{
      // FIXME: we shold probably look a bit harder for free heap space too alocate the freenode...
      assert( ff -> end >= ff -> begining + sizeof( FreeNode));
      ff = MS_CreateFromLocal( ff, FreeNode, ff);
    }
    ff -> next = freenode -> next;
    ff -> prev = ( uintptr_t)freenode;
    ( ( FreeNode *)freenode -> next) -> prev = ( uintptr_t)ff;
    freenode -> next = ( uintptr_t)ff;
  }
  
  DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   alo_size: %u  \n", SLAB_SIZE, ff -> end - ff -> begining, alo_size);
  
  {
    u32 i = num_mem;
    while( i--){
      memcpy( ( void *)( addr + i * size), ptr, size);
    }
  }
  return ( void *)addr;
}


const void *
MS_FreeFromSize( FreeNode *freenode, const void * vaddr, const size_t size){
  const uintptr_t addr = ( const uintptr_t)vaddr;
  FreeNode *ff;
  size_t alo_size = ( size + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  assert( freenode != NULL);
  assert( addr != 0);
  
  ff = MS_CreateLocal( FreeNode,
		       .begining = addr,
		       .end      = addr + alo_size);
  
  if( ff -> begining == freenode -> end){
    freenode -> end = ff -> end;
    ff = freenode;
  }else if( ff -> end == freenode -> begining){
    freenode -> begining = ff -> begining;
    ff = freenode;
  }

  {
    FreeNode *nf = ( FreeNode *)freenode -> next;
    
    while( nf != freenode){
      if( nf -> end == ff -> begining){
	ff -> begining = nf -> begining;
	( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	( ( FreeNode *)nf -> prev) -> next = nf -> next;
	MS_Free( freenode, nf, FreeNode);
      }else if( nf -> begining == ff -> end){
	ff -> end = nf -> end;
	( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	( ( FreeNode *)nf -> prev) -> next = nf -> next;
	MS_Free( freenode, nf, FreeNode);
      }
      
      nf = ( FreeNode *)nf -> next;
    }
  }

  if( ff -> end >= ff -> begining + SLAB_SIZE){
    size_t slab_size = ff -> end - ff -> begining;
    slab_size -= slab_size % SLAB_SIZE;
    MS_FreeSlabFromSize( ( void *)ff -> begining, slab_size);
    ff -> begining += slab_size;
  }
  
  if( ff -> end != ff -> begining &&
      ff != freenode){
    ff = MS_CreateFromLocal( freenode, FreeNode, ff);
    ff -> next = freenode -> next;
    ff -> prev = ( uintptr_t)freenode;
    ( ( FreeNode *)freenode -> next) -> prev = ( uintptr_t)ff;
    freenode -> next = ( uintptr_t)ff;
  }
  
  DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   free_size: %u  \n",  SLAB_SIZE, ff -> end - ff -> begining, alo_size);
  
  return NULL;
}
