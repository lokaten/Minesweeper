
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
#define CASH_LINE U32C( 64) //may need tuning per arch

static inline FreeNode * InsertFreeNode( FreeNode *freenode, const FreeNode *pf);
static inline FreeNode * MoveFreeNode( FreeNode *ff);
static inline address MS_FreeSlabFromSize( const address, const size_t size);


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
    
    DEBUG_PRINT( debug_out, "\rfreenode -> begining: %lu \t freenode -> end: %lu \n", ff -> begining, ff -> end);
  }
  
  assert( ff -> begining + alo_size <= ff -> end);
  
  addr           = ff -> begining;
  ff -> begining += alo_size;
  
  assert( ff -> begining < ff -> end);
  
  *( FreeNode *)addr = ( FreeNode){ .prev = ff -> begining, .next = ff -> begining, .begining = addr, .end = ff -> end};
  
  ff = MoveFreeNode( ff);
  
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
      void *ptr = mmap( ( void *)freenode -> end, slab_alo_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_NORESERVE | MAP_PRIVATE, -1, 0);
      assert( ptr != MAP_FAILED);
      addr = ( address)ptr;
    }
    ff = MS_CreateLocal( FreeNode, .begining = addr, .end = addr + slab_alo_size);
    freenode -> end = ff -> end > freenode -> end? ff -> end: freenode -> end;
    freenode -> begining = ff -> begining < freenode -> begining? ff -> begining: freenode -> begining;
    ff = InsertFreeNode( freenode, ff);
    addr = ( ff -> begining + alo_size > ( ( ff -> begining + CASH_LINE - 1) & ~( CASH_LINE - 1)) ?
	     ( unlikely( ff -> begining + alo_size > ( ( ff -> begining + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1))) ?
	       ( ff -> begining + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1):
	       ( ff -> begining + CASH_LINE - 1) & ~( CASH_LINE - 1)):
	     ff -> begining);
    if unlikely( alo_size > SLAB_SIZE){
      addr = ( addr + slab_alo_size - alo_size) & ~( MIN_ALO_SIZE - 1);
    }
    DEBUG_PRINT( debug_out, "\raloc_slab!!    \n\rfreenode -> begining: %lu \t freenode -> end: %lu \n", freenode -> begining, freenode -> end);
  }
  
  if( addr != ff -> begining){
    FreeNode *tf = MS_CreateLocalFromLocal( FreeNode, ff);
    assert( ff -> begining + MIN_ALO_SIZE <= addr);
#ifdef DEBUG
    frag = addr - ff -> begining;
#endif
    ff -> end = addr;
    MoveFreeNode( ff);
    tf -> prev = ff -> begining;
    ff = tf;
  }
  
  ff -> begining = addr + alo_size;
  
  if( ff -> begining + MIN_ALO_SIZE > ( ( ff -> begining + CASH_LINE - 1) & ~( CASH_LINE - 1))){
    ff -> begining = ( ff -> begining + CASH_LINE - 1) & ~( CASH_LINE - 1);
  }
  
  ff = MoveFreeNode( ff);
  
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
    FreeNode *nf = MS_CreateEmptyLocal( FreeNode);
    
    nf -> end = ff -> end;
    nf -> begining = ( ff -> end) & ~( SLAB_SIZE - 1);
    
    ff -> end = ( ff -> begining + SLAB_SIZE - 1) & ~( SLAB_SIZE - 1);
    ff -> begining = ff -> begining;
    
    assert( slab_size == ( slab_size & ~( SLAB_SIZE - 1)));
    assert( ff -> end + slab_size == nf -> begining);
    
    nf -> prev = ff -> begining;
    nf -> next = ff -> next;
    MoveFreeNode( nf);
    
    ff = MS_CreateLocalFromLocal( FreeNode, ff);
    MoveFreeNode( ff);
    
    freenode -> end = freenode -> end == nf -> begining? ff -> end: freenode -> end;
    freenode -> begining = freenode -> begining == ff -> end? ff -> end + slab_size: freenode -> begining;
    
    MS_FreeSlabFromSize( ff -> end, slab_size);
    
    DEBUG_PRINT( debug_out, "\rfreenode -> begining: %lu \t freenode -> end: %lu \n", freenode -> begining, freenode -> end);
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
  
  ff = MoveFreeNode( ff);
  
  return ff;
}


static inline FreeNode *
MoveFreeNode( FreeNode *ff){
  if unlikely( ff -> end == ff -> begining){
    ( ( FreeNode *)ff -> prev) -> next = ff -> next;
  }else{
    assert( ff -> end >= ff -> begining + MIN_ALO_SIZE);
    *( FreeNode *)( ff -> begining) = *ff;
    ff = ( FreeNode *)( ff -> begining);
    ( ( FreeNode *)ff -> prev) -> next = ff -> begining;
  }
  
  return ff;
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

