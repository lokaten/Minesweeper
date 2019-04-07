
#include "MS_util.h"

void *
MS_CreateArrayFromSizeAndLocal( FreeNode *vfreenode, const size_t num_mem, const size_t size, const void *ptr){
  u32 i = num_mem;
  uintptr_t addr = 0;
  size_t alo_size;
  FreeNode *freenode = vfreenode;
  assert( size);
  assert( num_mem);
  assert( freenode != NULL);
  alo_size = ( num_mem * size + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  while( addr == 0){
    if( freenode -> end >= freenode -> begining + alo_size){
      addr = freenode -> begining;
      freenode -> begining += alo_size;
    }else if( freenode -> next == ( uintptr_t)vfreenode){
      size_t slab_alo_size = ( alo_size + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
      FreeNode *nf = MS_CreateLocal( FreeNode, 0);
      nf -> begining = ( uintptr_t)MS_CreateSlabFromSize( slab_alo_size);
      nf -> end      = nf -> begining + slab_alo_size;
      nf -> prev     = ( uintptr_t)freenode;
      nf -> next     = ( uintptr_t)vfreenode;
      nf = ( FreeNode *)MS_CreateArrayFromSizeAndLocal( freenode, 1, sizeof( FreeNode), nf);
      vfreenode -> prev = ( uintptr_t)nf;
      freenode  -> next = ( uintptr_t)nf;
    }
    
    DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   alo_size: %u  \n", SLAB_SIZE, freenode -> end - freenode -> begining, alo_size);
    
    freenode = ( FreeNode *)freenode -> next;
  }
  assert( addr != 0);
  while( i--){
    memcpy( ( void *)( addr + i * size), ptr, size);
  }
  return ( void *)addr;
}


const void *
MS_FreeFromSize( FreeNode *freenode, const void * vaddr, const size_t vsize){
  const uintptr_t addr = ( const uintptr_t)vaddr;
  FreeNode *nf = freenode, *ff = NULL;
  size_t size = ( vsize + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  assert( freenode != NULL);
  assert( addr != 0);
  while( ff == NULL){
    
    if( nf -> end == addr){
      nf -> end += size;
      ff = nf;
    }else if( nf -> begining == addr + size){
      nf -> begining -= size;
      ff = nf;
    }else if( nf -> next == ( uintptr_t)freenode){
      nf -> next  = ( uintptr_t)MS_Create( freenode, FreeNode,
					   .prev     = ( uintptr_t)nf,
					   .next     = ( uintptr_t)freenode,
					   .begining = addr,
					   .end      = addr);
      freenode -> prev = nf -> next;
    }
    
    if( nf -> end >= nf -> begining + SLAB_SIZE){
      size_t slab_size = nf -> end - nf -> begining;
      slab_size -= slab_size % SLAB_SIZE;
      MS_FreeSlabFromSize( ( void *)nf -> begining, slab_size);
      nf -> begining += slab_size;
    }
    
    {
      uintptr_t nfnext = nf -> next;
      
      if( freenode != nf){
	if( ( nf       -> end == ( uintptr_t)nf) ||
	    ( freenode -> end == ( uintptr_t)nf)){
	  if( nf -> end == nf -> begining){
	    ( ( FreeNode *)nf -> prev) -> next = nf -> next;
	    ( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	  }else if( freenode -> end == nf -> begining){
	    freenode -> end = nf -> end;
	    ( ( FreeNode *)nf -> prev) -> next = nf -> next;
	    ( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	  }else{
	    nf = MS_CreateFromLocal( freenode, FreeNode, nf);
	    ( ( FreeNode *)nf -> prev) -> next = ( uintptr_t)nf;
	    ( ( FreeNode *)nf -> next) -> prev = ( uintptr_t)nf;
	  }
	  if( nf -> end == ( uintptr_t)nf){
	    nf -> end += sizeof( FreeNode);
	    DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   free_size: %u  \n",  SLAB_SIZE, nf -> end - nf -> begining, sizeof( FreeNode));
	  }else if( freenode -> end == ( uintptr_t)nf){
	    freenode -> end += sizeof( FreeNode);
	    DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   free_size: %u  \n",  SLAB_SIZE, freenode -> end - freenode -> begining, sizeof( FreeNode));
	  }
	}else{
	  if( nf -> end == nf -> begining){
	    ( ( FreeNode *)nf -> prev) -> next = nf -> next;
	    ( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	    MS_FreeFromSize( freenode, nf, sizeof( FreeNode));
	  }else if( freenode -> end == nf -> begining){
	    freenode -> end = nf -> end;
	    ( ( FreeNode *)nf -> prev) -> next = nf -> next;
	    ( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	    MS_FreeFromSize( freenode, nf, sizeof( FreeNode));
	  }
	}
      }
      
      
      nf = ( FreeNode *)nfnext;
    }
  }
  
  DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   free_size: %u  \n",  SLAB_SIZE, ff -> end - ff -> begining, size);
  
  return NULL;
}
