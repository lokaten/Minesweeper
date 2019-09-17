
#include <sys/mman.h> // mmap
#include <string.h> // memcpy
#include <unistd.h> // _SC_PAGE_SIZE

#ifndef MAP_ANONYMOUS
#include <stdlib.h> // malloc
#endif

#include "MS_util.h"

#include "debug.h"


#define SLAB_SIZE ( size_t)sysconf( _SC_PAGE_SIZE)
#define ALIGNMENT sizeof( address)
#define MIN_ALO_SIZE sizeof( FreeNode)

static inline address MS_CreateSlabFromSize( const size_t size);
#define MS_CreateSlab() MS_CreateSlabFromSize( SLAB_SIZE)
static inline address MS_FreeSlabFromSize( const address addr, const size_t size);
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

address
MS_CreateArrayFromSizeAndLocal( FreeNode *freenode, const size_t num_mem, const size_t size, const void *local){
  address addr = 0;
  FreeNode *ff = NULL;
  size_t alo_size;
#ifdef DEBUG
  u64 tutime = getnanosec();
#endif
  alo_size = ( num_mem * size + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  alo_size = alo_size < MIN_ALO_SIZE ? MIN_ALO_SIZE : alo_size;
  assert( alo_size);
  assert( freenode != NULL);
  if( freenode -> begining + MIN_ALO_SIZE <= freenode -> end){
    FreeNode *nf = freenode;
    
    do{
      if( ( ( nf -> end == nf -> begining + alo_size) ||
	    ( nf -> end >= nf -> begining + alo_size + MIN_ALO_SIZE)) &&
	  ( nf -> begining + alo_size >= ( ( nf -> begining + alo_size) & ~( SLAB_SIZE - 1)) + MIN_ALO_SIZE ||
	    nf -> begining + alo_size == ( ( nf -> begining + alo_size) & ~( SLAB_SIZE - 1)))){
	ff = nf;
	break;
      }
      nf = ( FreeNode *)nf -> next;
    }while( nf -> next != ( address)freenode);
  }
  
  if( ff == NULL){
    size_t slab_alo_size = ( alo_size + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
    address new_slab = MS_CreateSlabFromSize( slab_alo_size);
    if( freenode -> begining + MIN_ALO_SIZE > freenode -> end){
      assert( freenode -> begining == freenode -> end);
      ff = freenode;
    }else{
      ff = ( FreeNode *)( new_slab);
      ff -> next = freenode -> next;
      ff -> prev = ( address)freenode;
      ( ( FreeNode *) freenode -> next) -> prev = ( address)ff;
      freenode -> next = ( address)ff;
    }
    ff -> begining = new_slab;
    ff -> end      = new_slab + slab_alo_size;
  }
  
  assert( ff != NULL);
  
  if( ( address)ff >= ff -> begining &&
      ( address)ff <  ff -> begining + alo_size){
    assert( ( address)ff == ff -> begining);
    if( ff -> end > ff -> begining + alo_size){
      assert( ff -> end >= ff -> begining + alo_size + MIN_ALO_SIZE);
      *( FreeNode *)( ff -> begining + alo_size) = *ff;
      ff = ( FreeNode *)( ff -> begining + alo_size);
      ( ( FreeNode *)ff -> next) -> prev = ( address)ff;
      ( ( FreeNode *)ff -> prev) -> next = ( address)ff;
    }else{
      assert( ff -> end == ff -> begining + alo_size);
      ( ( FreeNode *)ff -> next) -> prev = ff -> prev;
      ( ( FreeNode *)ff -> prev) -> next = ff -> next;
    }
  }
  
  addr           = ff -> begining;
  ff -> begining += alo_size;
    
  {
    u32 i = num_mem;
    while( i--){
      memcpy( ( void *)( addr + i * size), local, size);
    }
  }
  
  {
#ifdef DEBUG
    u64 mytime = getnanosec();
    mytime = mytime < tutime ? 0 : mytime - tutime;
    
    if( ff != freenode &&
	ff -> end < ff -> begining + MIN_ALO_SIZE){
      ff -> begining = ff -> end;
    }
    
    DEBUG_PRINT( debug_out, "\rslab: %u  \t left %u   \t alo_size:  %u   \t %llu.%09llu \n",  SLAB_SIZE, ff -> end - ff -> begining, alo_size, mytime / U64C( 1000000000), mytime % U64C( 1000000000));
#endif
  }
  
  return addr;
}


address
MS_FreeFromSize( FreeNode *freenode, const address addr, const size_t size){
  FreeNode *ff = freenode;
  size_t alo_size;
#ifdef DEBUG
  u64 tutime = getnanosec();
#endif
  alo_size = ( size + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  alo_size = alo_size < MIN_ALO_SIZE ? MIN_ALO_SIZE : alo_size;
  assert( freenode != NULL);
  assert( addr != 0);
  
  assert( addr >= ( addr & ~( SLAB_SIZE - 1)) + MIN_ALO_SIZE ||
	  addr == ( addr & ~( SLAB_SIZE - 1)));
  
  if( ff -> begining == addr + alo_size){
    ff -> begining = addr;
  }else if( ff -> end == addr){
    ff -> end = addr + alo_size;
  }else{
    ff = ( FreeNode *)addr;
    ff -> begining = addr;
    ff -> end      = addr + alo_size;
    
    assert( ( ff -> end + MIN_ALO_SIZE <= ( ( ff -> end + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1))));
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
	assert( ff -> end       >= ( address)nf + sizeof( FreeNode));
	assert( ff -> begining  <= ( address)nf);
	assert( ff -> end       >= nf -> end);
	assert( ff -> begining  <= nf -> begining);
	( ( FreeNode *)nf -> next) -> prev = nf -> prev;
	( ( FreeNode *)nf -> prev) -> next = nf -> next;
      }
      nf = ( FreeNode *)nf -> next;
    }
  }
  
  if( ( ( ff -> end) & ~( SLAB_SIZE - 1)) >
      ( ( ff -> begining + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1))){
    size_t slab_size = ( ( ff -> end) & ~( SLAB_SIZE - 1)) -
      ( ( ff -> begining + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1));
    FreeNode *nf = MS_CreateLocal( FreeNode, 0);
    
    nf -> end = ff -> end;
    nf -> begining = ( ff -> end) & ~( SLAB_SIZE - 1);
    
    ff -> end = ( ff -> begining + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
    ff -> begining = ff -> begining;

    assert( slab_size == ( slab_size & ~( SLAB_SIZE - 1)));
    assert( ff -> end + slab_size == nf -> begining);
    
    if( ff != freenode){
      ff = MS_CreateLocalFromLocal( FreeNode, ff);
    }
    
    if( nf -> end != nf -> begining){
      assert( nf -> end >= nf -> begining + MIN_ALO_SIZE);
      *( FreeNode *)( nf -> begining) = *nf;
      nf = ( FreeNode *)( nf -> begining);
      nf -> next = freenode -> next;
      nf -> prev = ( address)freenode;
      ( ( FreeNode *)freenode -> next) -> prev = ( address)nf;
      freenode -> next = ( address)nf;
    }
    
    MS_FreeSlabFromSize( ff -> end, slab_size);
  }
  
  if( ff != freenode &&
      ff -> begining != ff ->  end){
    assert( ff -> end >= ff -> begining + MIN_ALO_SIZE);
    *( FreeNode *)( ff -> begining) = *ff;
    ff = ( FreeNode *)( ff -> begining);
    ff -> next = freenode -> next;
    ff -> prev = ( address)freenode;
    ( ( FreeNode *)freenode -> next) -> prev = ( address)ff;
    freenode -> next = ( address)ff;
  }
  
  {
#ifdef DEBUG
    u64 mytime = getnanosec();
    mytime = mytime < tutime ? 0 : mytime - tutime;
    DEBUG_PRINT( debug_out, "\rslab: %u  \t left %u   \t free_size: %u   \t %llu.%09llu \n",  SLAB_SIZE, ff -> end - ff -> begining, alo_size, mytime / U64C( 1000000000), mytime % U64C( 1000000000));
#endif
  }
    
  return 0;
}


static inline address
MS_CreateSlabFromSize( const size_t size){
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
  assert( ( addr & ~( SLAB_SIZE - 1)) == addr);
  DEBUG_PRINT( debug_out, "\raloc_slab!!    \n");
  return addr;
}


static inline address
MS_FreeSlabFromSize( const address addr, const size_t size){
  size_t alo_size = ( size + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
  assert( alo_size == size);
  assert( ( addr & ~( SLAB_SIZE - 1)) == addr);
  if( addr != 0){
    munmap( ( void *)addr, alo_size);
    DEBUG_PRINT( debug_out, "\rFree_slab!!    \n");
  }
  return 0;
}

