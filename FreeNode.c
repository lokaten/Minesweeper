
#include <sys/mman.h> // mmap
#include <string.h> // memcpy
#include <unistd.h> // _SC_PAGE_SIZE

#ifndef MAP_ANONYMOUS
#include <stdlib.h> // malloc
#endif

#include "MS_util.h"

#include "debug.h"

#define SLAB_SIZE ( size_t)sysconf( _SC_PAGE_SIZE)
#define ALIGNMENT sizeof( FreeNode)

static inline address MS_CreateSlabFromSize( size_t size);
#define MS_CreateSlab() MS_CreateSlabFromSize( SLAB_SIZE)
static inline address MS_FreeSlabFromSize( address , const size_t);
#define MS_FreeSlab( addr) MS_FreeSlabFromSize( addr, SLAB_SIZE)


FreeNode *
MS_CreateFreeList( void){
  FreeNode *freenode = MS_CreateLocal( FreeNode, 0);
  size_t alo_size = sizeof( FreeNode);
  size_t slab_alo_size = ( alo_size + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
  address new_slab = ( address)MS_CreateEmpty( freenode, FreeNode);
  
  freenode = ( FreeNode *)new_slab;
  freenode -> prev = ( address)freenode;
  freenode -> next = ( address)freenode;
  freenode -> begining = new_slab + alo_size;
  freenode -> end      = new_slab + slab_alo_size;
  
  return freenode;
}

void *
MS_FreeFreeList( FreeNode *freenode){
  FreeNode *ff = MS_CreateLocalFromLocal( FreeNode, freenode);
  ( ( FreeNode *)ff -> next) -> prev = ( address)ff;
  ( ( FreeNode *)ff -> prev) -> next = ( address)ff;
  MS_Free( ff, freenode);
  return NULL;
}

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
	  !( ( address)freenode + sizeof( FreeNode) > nf -> begining &&
	     ( address)freenode                     < nf -> begining + alo_size)){
	ff = nf;
	break;
      }
      nf = ( FreeNode *)nf -> next;
    }while( nf -> next != ( address)freenode);
  }
  
  if( ff == NULL){
    size_t slab_alo_size = ( alo_size + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
    address new_slab = MS_CreateSlabFromSize( slab_alo_size);
    if( slab_alo_size == alo_size){
      ff = MS_CreateLocal( FreeNode, 0);
    }else if( freenode -> end == freenode -> begining){
      ff = freenode;
    }else if( slab_alo_size > alo_size){
      ff = ( FreeNode *)( new_slab + alo_size);
      ff -> next = freenode -> next;
      ff -> prev = ( address)freenode;
      ( ( FreeNode *) freenode -> next) -> prev = ( address)ff;
      freenode -> next = ( address)ff;
    }else if( freenode -> end > freenode -> begining + sizeof( FreeNode)){
      freenode -> begining += sizeof( FreeNode);
      ff = ( FreeNode *)( freenode -> end);
      ff -> next = freenode -> next;
      ff -> prev = ( address)freenode;
      ( ( FreeNode *) freenode -> next) -> prev = ( address)ff;
      freenode -> next = ( address)ff;
    }else{
      // panic
      assert( FALSE && ( _Bool)"we have no idea where to dump the FreeNode");
    }
    ff -> begining = new_slab;
    ff -> end      = new_slab + slab_alo_size;
  }
  
  assert( ff != NULL);
  
  if( ( address)ff + sizeof( FreeNode) > ff -> begining &&
      ( address)ff                     < ff -> begining + alo_size){
    ( ( FreeNode *)ff -> next) -> prev = ff -> prev;
    ( ( FreeNode *)ff -> prev) -> next = ff -> next;
    if( ff -> end > ff -> begining + alo_size){
      FreeNode *nf = ( FreeNode *)( ff -> begining + alo_size);
      nf -> next = freenode -> next;
      nf -> prev = ( address)freenode;
      ( ( FreeNode *) freenode -> next) -> prev = ( address)nf;
      freenode -> next = ( address)nf;
      nf -> begining = ff -> begining + alo_size;
      nf -> end      = ff -> end;
    }
  }
  
  addr           = ff -> begining;
  ff -> begining += alo_size;
  
  DEBUG_PRINT( debug_out, "\rslab: %u  \tleft %u   \talo_size: %u  \n", SLAB_SIZE, ff -> end - ff -> begining, alo_size);
  
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
  FreeNode *ff = freenode;
  size_t alo_size = ( size + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  assert( freenode != NULL);
  assert( addr != 0);
  
  if( ff -> begining == addr + alo_size){
    ff -> begining = addr;
  }else if( ff -> end == addr){
    ff -> end = addr + alo_size;
  }else{
    ff = ( FreeNode *)addr;
    ff -> begining = addr;
    ff -> end      = addr + alo_size;
  }
  
  {
    FreeNode *nf = ( FreeNode*)freenode -> next;
    
    while( nf != freenode){
      if( nf -> begining == ff -> end){
	ff -> end = nf -> end;
      }else if( nf -> end == ff -> begining){
	ff -> begining = nf -> begining;
      }else{
	// do nothing
      }
      if( ( address)nf >= ff -> begining &&
	  ( address)nf <  ff -> end){
	( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	( ( FreeNode *)nf -> prev) -> next = nf -> next;
      }
      nf = ( FreeNode *)nf -> next;
    }
  }
  
  if( ff -> end >= ff -> begining + SLAB_SIZE){
    size_t slab_size = ff -> end - ff -> begining;
    slab_size -= slab_size % SLAB_SIZE;
    
    if( ff != freenode){
      if( ff -> end == ff -> begining + slab_size){
	ff = MS_CreateLocalFromLocal( FreeNode, ff);
      }else if( ff -> end > ff -> begining + slab_size){
	*( FreeNode *)( ff -> begining + slab_size) = *ff;
	ff = ( FreeNode *)( ff -> begining + slab_size);
      }else{
	assert( FALSE);
      }
    }
    
    MS_FreeSlabFromSize( ff -> begining, slab_size);
    ff -> begining += slab_size;
  }
  
  if( ff -> begining != ff ->  end &&
      ff != freenode){
    ff -> next = freenode -> next;
    ff -> prev = ( address)freenode;
    ( ( FreeNode *)freenode -> next) -> prev = ( address)ff;
    freenode -> next = ( address)ff;
  }
  
  DEBUG_PRINT( debug_out, "\rslab: %u  \tleft %u   \tfree_size: %u  \n",  SLAB_SIZE, ff -> end - ff -> begining, alo_size);
  
  return NULL;
}


static inline address
MS_CreateSlabFromSize( size_t size){
  address addr;
  void *ptr;
  size_t alo_size = ( size + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
  assert( alo_size == size);
#ifdef MAP_ANONYMOUS
  ptr = mmap( NULL, alo_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_NORESERVE | MAP_PRIVATE, -1, 0);
  assert( ptr != MAP_FAILED);
#else
  ptr = malloc( alo_size);
  assert( ptr != NULL);
#endif
  addr = ( address)ptr;
  assert( addr % SLAB_SIZE == 0);
  DEBUG_PRINT( debug_out, "\raloc_slab!!    \n");
  return addr;
}


static inline address
MS_FreeSlabFromSize( address addr, const size_t size){
  size_t alo_size = size + SLAB_SIZE - 1;
  alo_size -= alo_size % SLAB_SIZE;
  assert( alo_size == size);
  assert( addr % SLAB_SIZE == 0);
  if( addr != 0){
    munmap( ( void *)addr, alo_size);
    DEBUG_PRINT( debug_out, "\rFree_slab!!    \n");
  }
  return 0;
}

