
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
#define CASH_LINE 64 //my need tuning per arch

static inline void ExcludeFreeNode( FreeNode *ff);
static inline FreeNode * InsertFreeNode( FreeNode *freenode, const FreeNode *pf);
static inline void MoveFreeNode( const address addr, FreeNode *ff);
static inline address MS_CreateSlabFromSize( const size_t size);
#define MS_CreateSlab() MS_CreateSlabFromSize( SLAB_SIZE)
static inline address MS_FreeSlabFromSize( const address addr, const size_t size);
#define MS_FreeSlab( addr) MS_FreeSlabFromSize( addr, SLAB_SIZE)


FreeNode *
MS_CreateFreeList( void){
  address addr = 0;
  FreeNode *ff = NULL;
  size_t alo_size;
#ifdef DEBUG
  u64 tutime = getnanosec();
#endif
  alo_size = MIN_ALO_SIZE;
  
  {
    size_t slab_alo_size = SLAB_SIZE;
    address new_slab;
    {
      void *ptr = mmap( ( void *)( 1 << 30), slab_alo_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_NORESERVE | MAP_PRIVATE, -1, 0);
      assert( ptr != MAP_FAILED);
      new_slab = ( address)ptr;
      DEBUG_PRINT( debug_out, "\raloc_slab!!    \n");
    }
    ff = MS_CreateLocal( FreeNode, .begining = new_slab, .end = new_slab + slab_alo_size, .prev = new_slab, .next = new_slab);
  }
  
  assert( ff -> begining + alo_size <= ff -> end);
  
  addr           = ff -> begining;
  ff -> begining += alo_size;
  
  assert( ff -> begining < ff -> end);
  
  *( FreeNode *)addr = ( FreeNode){ .prev = ff -> begining, .next = ff -> begining, .begining = 0, .end = 0};
  
  MoveFreeNode( ff -> begining, ff);
  ff = ( FreeNode *)ff -> begining;
  
  {
#ifdef DEBUG
    u64 mytime = getnanosec();
    mytime = mytime < tutime ? 0 : mytime - tutime;
    
    DEBUG_PRINT( debug_out, "\rslab: %u  \t left %u   \t alo_size:  %u   \t % 6llu.%03llu \n",  SLAB_SIZE, ff -> end - ff -> begining, alo_size, mytime / U64C( 1000), mytime % U64C( 1000));
#endif
  }
  
  return ( FreeNode *)addr;
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
  FreeNode *ff = freenode;
  size_t alo_size;
#ifdef DEBUG
  size_t frag = 0;
  u64 tutime = getnanosec();
#endif
  alo_size = ( num_mem * size + ALIGNMENT - 1) & ~( ALIGNMENT - 1);
  alo_size = alo_size < MIN_ALO_SIZE ? MIN_ALO_SIZE : alo_size;
  assert( freenode != NULL);
  
  if likely( alo_size + MIN_ALO_SIZE < SLAB_SIZE){
    do{
      ( ( FreeNode *)ff -> next) -> prev = ( address)ff;
      ff = ( FreeNode *)ff -> next;
      addr = ( ff -> begining + alo_size > ( ( ff -> begining + CASH_LINE - 1) & ~( CASH_LINE - 1)) ?
	       ( unlikely( ff -> begining + alo_size > ( ( ff -> begining + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1))) ?
		 ( ff -> begining + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1):
		 ( ff -> begining + CASH_LINE - 1) & ~( CASH_LINE - 1)):
	       ff -> begining);
    }while unlikely( ff -> end <= addr + alo_size &&
		     ff != freenode);
  }
  
  if unlikely( ff == freenode){
    size_t slab_alo_size = ( alo_size + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
    {
      void *ptr = mmap( ( void *)( ( ( FreeNode *)freenode -> prev) -> end), slab_alo_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_NORESERVE | MAP_PRIVATE, -1, 0);
      assert( ptr != MAP_FAILED);
      addr = ( address)ptr;
      DEBUG_PRINT( debug_out, "\raloc_slab!!    \n");
    }
    ff = MS_CreateLocal( FreeNode, .begining = addr, .end = addr + slab_alo_size);
    ff = InsertFreeNode( freenode, ff);
    addr = ( ff -> begining + alo_size > ( ( ff -> begining + CASH_LINE - 1) & ~( CASH_LINE - 1)) ?
	     ( unlikely( ff -> begining + alo_size > ( ( ff -> begining + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1))) ?
	       ( ff -> begining + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1):
	       ( ff -> begining + CASH_LINE - 1) & ~( CASH_LINE - 1)):
	     ff -> begining);
  }
  
  if( addr != ff -> begining){
    FreeNode *tf = MS_CreateLocalFromLocal( FreeNode, ff);
    assert( ff -> begining + MIN_ALO_SIZE <= addr);
#ifdef DEBUG
    frag = addr - ff -> begining;
#endif
    tf -> next = ff -> begining;
    tf -> end = addr;
    ff -> begining = addr;
    MoveFreeNode( ff -> begining, ff);
    ff = ( FreeNode *)ff -> begining;
    MoveFreeNode( tf -> begining, tf);
    ff -> prev = tf -> begining;
  }
  
  assert( ff -> begining + alo_size <= ff -> end);
  
  addr           = ff -> begining;
  ff -> begining += alo_size;
  
  if( ff -> begining + MIN_ALO_SIZE > ( ( ff -> begining + CASH_LINE - 1) & ~( CASH_LINE - 1))){
    ff -> begining = ( ff -> begining + CASH_LINE - 1) & ~( CASH_LINE - 1);
  }
  
  if likely( ff -> end != ff -> begining){
    if unlikely( ff -> next == ( address)freenode){
      freenode -> prev = ff -> begining;
    }
    MoveFreeNode( ff -> begining, ff);
    ff = ( FreeNode *)ff -> begining;
  }else{
    if unlikely( ff -> next == ( address)freenode){
      freenode -> prev = ff -> prev;
    }
    ExcludeFreeNode( ff);
  }
  
  ff = MS_CreateLocalFromLocal( FreeNode, ff);
  
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
    
    DEBUG_PRINT( debug_out, "\rslab: %u  \t left %u   \t alo_size:  %u   \t % 6llu.%03llu \t frag: %lu \n",  SLAB_SIZE, ff -> end - ff -> begining, alo_size, mytime / U64C( 1000), mytime % U64C( 1000), frag);
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
  assert( size != 0);
  
  ff = MS_CreateLocal( FreeNode, .begining = addr, .end = addr + alo_size);
  
  if( ( ff -> end + MIN_ALO_SIZE > ( ( ff -> end + CASH_LINE - 1) & ~( CASH_LINE - 1)))){
    ff -> end = ( ff -> end + CASH_LINE - 1) & ~( CASH_LINE - 1);
  }
  
  ff = InsertFreeNode( freenode, ff);
  
  if unlikely( ( ( ff -> end) & ~( SLAB_SIZE - 1)) >
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
    
    if( nf -> end >= nf -> begining + MIN_ALO_SIZE){
      nf -> prev = ff -> begining;
      nf -> next = ff -> next;
      if( freenode -> prev == ( address)ff){
	freenode -> prev = nf -> begining;
      }
      MoveFreeNode( nf -> begining, nf);
    }
    
    if( ff -> begining + MIN_ALO_SIZE > ff -> end){
      ff = MS_CreateLocalFromLocal( FreeNode, ff);
      if( freenode -> prev == ( address)ff){
	freenode -> prev = ff -> prev;
      }
      ExcludeFreeNode( ff);
    }
    
    MS_FreeSlabFromSize( ff -> end, slab_size);
  }
  
  {
#ifdef DEBUG
    u64 mytime = getnanosec();
    mytime = mytime < tutime ? 0 : mytime - tutime;
    DEBUG_PRINT( debug_out, "\rslab: %u  \t left %u   \t free_size: %u   \t % 6llu.%03llu \n",  SLAB_SIZE, ff -> end - ff -> begining, alo_size, mytime / U64C( 1000), mytime % U64C( 1000));
#endif
  }
  
  return 0;
}


static inline void
ExcludeFreeNode( FreeNode *ff){
  assert( ff -> begining != 0);
  ( ( FreeNode *)ff -> prev) -> next = ff -> next;
}


static inline FreeNode *
InsertFreeNode( FreeNode *freenode, const FreeNode *pf){
  FreeNode *nf = freenode;
  FreeNode *ff = MS_CreateLocalFromLocal( FreeNode, pf);
  assert( pf -> end >= pf -> begining + MIN_ALO_SIZE);
  
  assert( pf -> begining >= ( pf -> begining & ~( CASH_LINE - 1)) + MIN_ALO_SIZE ||
	  pf -> begining == ( pf -> begining & ~( CASH_LINE - 1)));
  
  assert( pf -> end >= ( pf -> end & ~( CASH_LINE - 1)) + MIN_ALO_SIZE ||
	  pf -> end == ( pf -> end & ~( CASH_LINE - 1)));
  
  while likely( nf -> next < pf -> begining &&
		( FreeNode *)nf -> next != freenode){
    assert( ( ( FreeNode *)nf -> next) -> begining == nf -> next);
    assert( nf -> begining <= ( ( FreeNode *)nf -> next) -> begining);
    ( ( FreeNode *)nf -> next) -> prev = ( address)nf;
    nf = ( FreeNode *)nf -> next;
  }
  
  assert( nf -> begining <= pf -> begining);
  
  ff -> prev = ( address)nf;
  ff -> next = nf -> next;
  
  if unlikely( ff -> begining == nf -> end){
    ff -> begining = nf -> begining;
    ff -> prev = nf -> prev;
  }
  
  if unlikely( ff -> end == nf -> next){
    assert( ( ( FreeNode *)nf -> next) -> begining == nf -> next);
    ( ( FreeNode *)nf -> next) -> prev = ( address)nf;
    nf = ( FreeNode *)nf -> next;
    ff -> end = nf -> end;
    ff -> next = nf -> next;
  }
  
  assert( ff -> begining <= pf -> begining);
  assert( ff -> end >= pf -> end);
  
  if unlikely( nf -> next == ( address)freenode){
    freenode -> prev = ff -> begining;
  }
  MoveFreeNode( ff -> begining, ff);
  ff = ( FreeNode *)ff -> begining;
  
  return ff;
}


static inline void
MoveFreeNode( const address addr, FreeNode *ff){
  assert( addr == ff -> begining);
  *( FreeNode *)( addr) = *ff;
  ff = ( FreeNode *)( addr);
  ( ( FreeNode *)ff -> prev) -> next = addr;
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

