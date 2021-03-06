
#ifdef MS_UTIL_H__
#else
#define MS_UTIL_H__
#ifdef __cplusplus
extern "C" {
#endif


#include <sys/time.h> // clock_gettime

#include <assert.h>

#include <stdint.h>


#include <stdio.h> // FILE

#ifndef NO_TERM
#include <stdarg.h> // va_list
#endif

#ifndef CLOCK_REALTIME
#include <time.h>
#endif

#define TRUE  1
#define FALSE 0

#ifdef __builtin_expect
#define likely(   x)     ( __builtin_expect( !!( x), TRUE))
#define unlikely( x)     ( __builtin_expect( (   x), FALSE))
#define expect(   x, y)  ( __builtin_expect( (   x), ( y)))
#else
#define likely(   x)     ( x)
#define unlikely( x)     ( x)
#define expect(   x, y)  ( x)
#endif

#ifdef DEBUG
#define dassert( exp) assert( exp)
#else
#define dassert( exp) (void)( exp)
#endif

#ifdef NO_TERM
#define TERM( string) ""
#else
#define TERM( string) string
#endif

#ifdef __cplusplus
#else
typedef _Bool bool;
#endif

#ifdef SMALL
typedef uint_least64_t u64;
typedef uint_least32_t u32;
typedef uint_least16_t u16;
typedef uint_least8_t  u8;

typedef int_least64_t  s64;
typedef int_least32_t  s32;
typedef int_least16_t  s16;
typedef int_least8_t   s8;
#else
typedef uint_fast64_t u64;
typedef uint_fast32_t u32;
typedef uint_fast16_t u16;
typedef uint_fast8_t  u8;

typedef int_fast64_t  s64;
typedef int_fast32_t  s32;
typedef int_fast16_t  s16;
typedef int_fast8_t   s8;
#endif

typedef uintptr_t address;

#define U64C   ( u64)UINT64_C
#define U32C   ( u32)UINT32_C
#define U16C   ( u16)UINT16_C
#define U8C    ( u8 )UINT8_C

#define S64C   ( s64)INT64_C
#define S32C   ( s32)INT32_C
#define S16C   ( s16)INT16_C
#define S8C    ( s8 )INT8_C


typedef struct{
  s32 xdiff;
  s32 ydiff;
  s32 realxdiff;
  s32 realydiff;
  u32 width;
  u32 height;
  u32 element_width;
  u32 element_height;
  u32 realwidth;
  u32 realheight;
}MS_video;

typedef struct{
  FILE *out;
  FILE *err;
  FILE *deb;
  FILE *hlp;
}MS_stream;

typedef struct{
  address prev;
  address next;
  address begining;
  address end;
}FreeNode;

#define MS_RAND_MAX U32C( 0xffffffff)

#define FUNC_CALL( name, ...) name( &( const struct parm##name){__VA_ARGS__})
#define FUNC_DEC( pre, name, arg) struct parm##name{ arg};pre name( const struct parm##name *)
#define FUNC_DEF( pre, name) pre name( const struct parm##name *parm)

static inline u32 gen_divobj( u32);
static inline u32 mol_( u32, u32, u32);
static inline u32 div_( u32, u32, u32);
static inline u32 MS_rand( u32);

static inline u32 MS_rand_seed( void);
static inline int MS_print( FILE *, const char *, ...);
static inline __uint64_t getmicrosec( void);
static inline __uint64_t getnanosec( void);

#ifdef DEBUG
#define DEBUG_PRINT( file, ...) MS_print( file, __VA_ARGS__)
#else
#define DEBUG_PRINT( ...) (void)0
#endif

FreeNode *MS_CreateFreeList( void);
void *MS_FreeFreeList( FreeNode *);

address MS_CreateArrayFromSizeAndLocal( FreeNode *, const size_t, const size_t, const void *);
#define MS_Create( freenode, type, ...) ( type *)MS_CreateArrayFromSizeAndLocal( freenode, 1, sizeof( type), &( const type){ __VA_ARGS__})
#define MS_CreateFromSize( freenode, size) MS_CreateArrayFromSizeAndLocal( freenode, size, sizeof( char), &( const char){0})
#define MS_CreateFromLocal( freenode, type, local) ( type *)MS_CreateArrayFromSizeAndLocal( freenode, 1, sizeof( type), local)
#define MS_CreateEmpty( freenode, type) ( type *)MS_CreateArrayFromSizeAndLocal( freenode, 1, sizeof( type), &( const type){0})
#define MS_CreateEmptyArray( freenode, num_mem, type) ( type *)MS_CreateArrayFromSizeAndLocal( freenode, num_mem, sizeof( type), &( const type){0})
#define MS_CreateArray( freenode, num_mem, type, ...) ( type *)MS_CreateArrayFromSizeAndLocal( freenode, num_mem, sizeof( type), &( const type){ __VA_ARGS__})
#define MS_CreateArrayFromLocal( freenode, num_mem, type, local) ( type *)MS_CreateArrayFromSizeAndLocal( freenode, num_mem, sizeof( type), local)

address MS_FreeFromSize( FreeNode *, const address, const size_t);
#define MS_Free( freenode, addr) MS_FreeFromSize( freenode, ( const address)addr, sizeof( *addr))
#define MS_FreeArray( freenode, addr, num_mem) MS_FreeFromSize( freenode, ( const address)addr, num_mem * sizeof( *addr))

#define MS_CreateLocal( type, ...) &( type){ __VA_ARGS__}
#define MS_CreateLocalFromSize( size) ( void *)char[ size]
#define MS_CreateLocalFromLocal( type, local) ( type *)&( ( union{ type T;}){ .T = *local})
#define MS_CreateEmptyLocal( type) ( type *)&( ( union{ type T;}){ .T = ( const type){0}})


// divsion is slow, make sure we don't do it more then we have to

// genrate a divobj from the divaider
static inline u32
gen_divobj( u32 a){
  dassert( a > 0);
  dassert( a < U32C( 0xffff));
  return (u32)( ( U64C( 0xffffffff) + (u64)a) / (u64)a);
}

// divobj = gen_divobj( a)
// ( b % a) =>
//
static inline u32
mol_( u32 b, u32 a, u32 divobj){
  u32 ret = ( (u64)( ( b * divobj) & U32C( 0xffffffff)) * (u64)a) >> 32;
  dassert( div_( b, a, divobj) * a + ret == b);
  return ret;
}

// divobj = gen_divobj( a)
// ( b / a) =>
//
static inline u32
div_( u32 b, u32 a, u32 divobj){
  u32 ret = ( (u64)b * (u64)divobj) >> 32;
  dassert( ret * a <= b && ret * a + a > b);
  return ret;
}

//
// ( seed = MS_rand( seed))
//
static inline u32
MS_rand( const u32 seed){
  u32 ret = (u32)( (u32)seed * U32C( 2654435769)) & U32C( 0xffffffff);
  return ret;
}


static inline u64
MS_rand64( const u64 seed64){
  u64 ret = (u64)( (u64)seed64 * U64C( 11400714819323198485)) & U64C( 0xffffffffffffffff);
  return ret;
}


//
// return a seed, for use with MS_rand( __uint32_t seed)
//
static inline u32
MS_rand_seed( void){
  u32 seed;
  
#ifdef CLOCK_REALTIME
  struct timespec tv;
  
  clock_gettime( CLOCK_REALTIME, &tv);
  
  seed = ( u32)tv.tv_nsec;
#else
  seed = ( u32)time( NULL);
#endif
  
  return seed;
}


static inline int
MS_print( FILE *stream, const char * format, ...){
  int ret = 0;
#ifdef NO_TERM
  ( void) stream;
  ( void) format;
#else
_Pragma("GCC diagnostic ignored \"-Wformat-nonliteral\"")
  
  if( stream != NULL){
    va_list args;
    va_start( args, format);
    ret = vfprintf( ( FILE *)stream, format, args);
    fflush( stream);
    va_end( args);
  }
 
// pragma to turn back on -wformat-nonliteral?
#endif
  return ret;
}


//
// get system time in microsecond...
//
static inline  __uint64_t
getmicrosec( void){
#ifdef CLOCK_REALTIME
  struct timespec tv;
  
  clock_gettime( CLOCK_REALTIME, &tv);
  
  return ( ( __uint64_t)tv.tv_sec * 1000000 + ( __uint64_t)tv.tv_nsec / 1000);
#else
  return ( __uint64_t)time( NULL) * 1000000;
#endif
}


//
// get the amount of nanoseconds from a point in the past
//
static inline  __uint64_t
getnanosec( void){
#ifdef CLOCK_REALTIME
  struct timespec tv;
  
  clock_gettime( CLOCK_MONOTONIC, &tv);
  
  return ( ( __uint64_t)tv.tv_sec * 1000000000 + ( __uint64_t)tv.tv_nsec);
#else
  return ( __uint64_t)time( NULL) * 1000000000;
#endif
}

  
#ifdef __cplusplus
}
#endif
#endif
