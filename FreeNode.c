
#include "MS_util.h"

void *
MS_CreateArrayFromSizeAndLocal( FreeNode *freenode, const size_t num_mem, const size_t size, const void *local){
  address addr = 0;
  FreeNode *ff = NULL;
  size_t alo_size = ( num_mem * size + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  assert( alo_size);
  assert( freenode != NULL);
  if( freenode -> begining != freenode -> end){
    FreeNode *nf = freenode;
    
    do{
      if( nf -> end >= nf -> begining + alo_size){
	ff = nf;
	break;
      }
      nf = ( FreeNode *)nf -> next;
    }while( nf -> next != ( address)freenode);
  }
  
  if( ff == NULL){
    size_t slab_alo_size = ( alo_size + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
    address new_slab = MS_CreateSlabFromSize( slab_alo_size);
    if( freenode -> end == freenode -> begining){
      ff = freenode;
    }else{
      ff = MS_Create( freenode, FreeNode,
		      .next = freenode -> next,
		      .prev = ( address)freenode);
      ( ( FreeNode *) freenode -> next) -> prev = ( address)ff;
      freenode -> next = ( address)ff;
    }
    ff -> begining = new_slab;
    ff -> end      = new_slab + slab_alo_size;
  }
  
  assert( ff != NULL);
  
  addr           = ff -> begining;
  ff -> begining += alo_size;
  
  if( addr == ( address)ff &&
      ff -> end > ff -> begining){
    MS_FreeFromSize( freenode, ( void *)ff -> begining, ff -> end - ff -> begining);
  }
  
  DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   alo_size: %u  \n", SLAB_SIZE, ff -> end - ff -> begining, alo_size);
  
  {
    u32 i = num_mem;
    while( i--){
      memcpy( ( void *)( addr + i * size), local, size);
    }
  }
  return ( void *)addr;
}


const void *
MS_FreeFromSize( FreeNode *freenode, const void * vaddr, const size_t size){
  const address addr = ( const address)vaddr;
  FreeNode *ff = NULL;
  size_t alo_size = ( size + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  assert( freenode != NULL);
  assert( addr != 0);

  {
    FreeNode *nf = freenode;
    
    do{
      if( nf -> begining == addr + alo_size ||
	  nf -> end      == addr){
	ff = nf;
	break;
      }
      nf = ( FreeNode *)nf -> next;
    }while( nf -> next != ( address)freenode);
  }

  if( ff == NULL){
    ff = MS_Create( freenode, FreeNode, 0);
    ff -> next = freenode -> next;
    ff -> prev = ( address)freenode;
    ( ( FreeNode *)freenode -> next) -> prev = ( address)ff;
    freenode -> next = ( address)ff;
    ff -> begining = addr;
    ff -> end      = addr;
  }

  if( ff -> begining == addr + alo_size){
    ff -> begining = addr;
  }else if( ff -> end == addr){
    ff -> end = addr + alo_size;
  }else{
    // panic
    assert( FALSE);
  }
  
  if( ff -> end >= ff -> begining + SLAB_SIZE){
    size_t slab_size = ff -> end - ff -> begining;
    slab_size -= slab_size % SLAB_SIZE;
    MS_FreeSlabFromSize( ff -> begining, slab_size);
    ff -> begining += slab_size;
  }
  
  if( ff -> end == ff -> begining){
    if( ff != freenode){
      ff -> begining = ( address)ff;
      ff -> end      = ( address)ff + sizeof( FreeNode);
    }else if( ff -> next != ( address)freenode){
      // do something
    }else{
      // do nothing
    }
  }
  
  DEBUG_PRINT( stdout, "\rslab: %u  \tleft %u   free_size: %u  \n",  SLAB_SIZE, ff -> end - ff -> begining, alo_size);
  
  return NULL;
}
