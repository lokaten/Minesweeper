
#include "MS_util.h"

#include "debug.h"

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
      if( nf -> end >= nf -> begining + alo_size &&
	  nf -> begining != ( address)freenode){
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
      ff -> end      = new_slab + slab_alo_size;
    }else if( slab_alo_size > alo_size){
      ff = ( FreeNode *)( new_slab + slab_alo_size - sizeof( FreeNode));
      ff -> next = freenode -> next;
      ff -> prev = ( address)freenode;
      ( ( FreeNode *) freenode -> next) -> prev = ( address)ff;
      freenode -> next = ( address)ff;
      ff -> end      = ( address)ff;
    }else if( freenode -> end > freenode -> begining + sizeof( FreeNode)){
      freenode -> begining += sizeof( FreeNode);
      ff = ( FreeNode *)( freenode -> end);
      ff -> next = freenode -> next;
      ff -> prev = ( address)freenode;
      ( ( FreeNode *) freenode -> next) -> prev = ( address)ff;
      freenode -> next = ( address)ff;
      ff -> end      = new_slab + slab_alo_size;
    }else{
      // we have no idea where to dump the FreeNode
      // panic
      assert( FALSE);
    }
    ff -> begining = new_slab;
  }
  
  assert( ff != NULL);
  
  if( ( address)ff <= ff -> end - sizeof( FreeNode) &&
      ( address)ff >= ff -> begining){
    ( ( FreeNode *)ff -> next) -> prev = ff -> prev;
    ( ( FreeNode *)ff -> prev) -> next = ff -> next;
    if( ff -> end > ff -> begining + alo_size){
      FreeNode *nf = ( FreeNode *)( ff -> begining + alo_size);
      ff -> next = freenode -> next;
      ff -> prev = ( address)freenode;
      ( ( FreeNode *) freenode -> next) -> prev = ( address)nf;
      freenode -> next = ( address)nf;
      nf -> begining = ff -> begining;
      nf -> end      = ff -> end;
      ff = nf;
    }
  }
  
  addr           = ff -> begining;
  ff -> begining += alo_size;
  
  if( addr == ( address)ff &&
      ff -> end > ff -> begining){
    MS_FreeFromSize( freenode, ( void *)ff -> begining, ff -> end - ff -> begining);
  }
  
  DEBUG_PRINT( debug_out, "\rslab: %u  \tleft %u   alo_size: %u  \n", SLAB_SIZE, ff -> end - ff -> begining, alo_size);
  
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
      if( freenode -> end == nf -> begining){
	freenode -> end = nf -> end;
	if( ( address)nf <= nf -> end - sizeof( FreeNode) &&
	    ( address)nf >= nf -> begining){
	  ( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	  ( ( FreeNode *)nf -> prev) -> next = nf -> next;
	}else{
	  nf -> begining = ( address)nf;
	  nf -> end      = ( address)nf + sizeof( FreeNode);
	}
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
    if( ( address)ff <= ff -> end - sizeof( FreeNode) &&
	( address)ff >= ff -> begining){
      assert( ff != freenode);
      ( ( FreeNode *)ff -> next) -> prev = ff -> prev;
      ( ( FreeNode *)ff -> prev) -> next = ff -> next;
      ff = MS_CreateLocalFromLocal( FreeNode, ff);
    }else{
      size_t slab_size = ff -> end - ff -> begining;
      slab_size -= slab_size % SLAB_SIZE;
      MS_FreeSlabFromSize( ff -> begining, slab_size);
      ff -> begining += slab_size;
    }
  }
  
  if( ff -> end == ff -> begining){
    ff -> begining = ( address)ff;
    ff -> end      = ( address)ff + sizeof( FreeNode);
  }
  
  DEBUG_PRINT( debug_out, "\rslab: %u  \tleft %u   free_size: %u  \n",  SLAB_SIZE, ff -> end - ff -> begining, alo_size);
  
  return NULL;
}
