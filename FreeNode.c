
#include "MS_util.h"

void *
MS_CreateArrayFromSizeAndLocal( FreeNode *freenode, const size_t num_mem, const size_t size, const void *ptr){
  address addr = 0;
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
	  MS_Free( freenode, nf);
	  break;
	}
	
	nf = ( FreeNode *)nf -> next;
      }
    }
    
    if( ff == NULL){
      size_t slab_alo_size = ( alo_size + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
      address new_slab = MS_CreateSlabFromSize( slab_alo_size);
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
    ff -> next = ( address)ff;
    ff -> prev = ( address)ff;
    if( freenode -> end >= freenode -> begining + sizeof( FreeNode)){
      ff = MS_CreateFromLocal( freenode, FreeNode, ff);
    }else{
      // FIXME: we shold probably look a bit harder for free heap space too alocate the freenode...
      assert( ff -> end >= ff -> begining + sizeof( FreeNode));
      ff = MS_CreateFromLocal( ff, FreeNode, ff);
    }
    ff -> next = freenode -> next;
    ff -> prev = ( address)freenode;
    ( ( FreeNode *)freenode -> next) -> prev = ( address)ff;
    freenode -> next = ( address)ff;
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
  const address addr = ( const address)vaddr;
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
	nf -> end = ff -> end;
	ff -> begining = ff -> end;
      }else if( nf -> begining == ff -> end){
	nf -> begining = ff -> begining;
	ff -> end = ff -> begining;
      }
      
      if( freenode -> begining == ( address)nf + sizeof( FreeNode) &&
	  nf -> end >= nf -> begining + sizeof( FreeNode)){
	memcpy( ( FreeNode *)nf -> begining, nf, sizeof( FreeNode));
	nf = ( FreeNode *)nf -> begining;
	nf -> begining += sizeof( FreeNode);
	( ( FreeNode *)nf -> next) -> prev = ( address)nf;
	( ( FreeNode *)nf -> prev) -> next = ( address)nf;
	freenode -> begining -= sizeof( FreeNode);
      }

      {
	FreeNode *hf = nf;
	nf = MS_CreateLocalFromLocal( FreeNode, nf);
      
      if( freenode -> begining == nf -> end){
	freenode -> begining = nf -> begining;
	( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	( ( FreeNode *)nf -> prev) -> next = nf -> next;
	MS_Free( freenode, hf); // FIXME: recursion is a bad idea
      }
      
      if( nf -> end >= nf -> begining + SLAB_SIZE){
	size_t slab_size = nf -> end - nf -> begining;
	slab_size -= slab_size % SLAB_SIZE;
	MS_FreeSlabFromSize( nf -> begining, slab_size);
	nf -> begining += slab_size;
	if( nf -> end == nf -> begining){
	  ( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	  ( ( FreeNode *)nf -> prev) -> next = nf -> next;
	  MS_Free( freenode, hf); // FIXME: recursion is a bad idea
	}
      }
      
      DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   free_size: %u  \n",  SLAB_SIZE, nf -> end - nf -> begining, alo_size);
      
      nf = ( FreeNode *)nf -> next;
      }
    }
  }
  
  if( ff -> end >= ff -> begining + SLAB_SIZE){
    size_t slab_size = ff -> end - ff -> begining;
    slab_size -= slab_size % SLAB_SIZE;
    MS_FreeSlabFromSize( ff -> begining, slab_size);
    ff -> begining += slab_size;
  }
  
  if( ff -> end != ff -> begining &&
      ff != freenode){
    ff = MS_CreateFromLocal( freenode, FreeNode, ff);
    ff -> next = freenode -> next;
    ff -> prev = ( address)freenode;
    ( ( FreeNode *)freenode -> next) -> prev = ( address)ff;
    freenode -> next = ( address)ff;
    DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   free_size: %u  \n",  SLAB_SIZE, ff -> end - ff -> begining, alo_size);
  }else if( ff == freenode){
    DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   free_size: %u  \n",  SLAB_SIZE, ff -> end - ff -> begining, alo_size);
  }
  
  return NULL;
}
