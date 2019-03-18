

#ifdef MS_UTIL_H__
#else
#define MS_UTIL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdlib.h> /* malloc */
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h> /* memcpy */

#include <sys/time.h>

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
#undef NULL
#define NULL nullptr
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

#define U64C( exp)   ( u64)UINT64_C( exp)
#define U32C( exp)   ( u32)UINT32_C( exp)
#define U16C( exp)   ( u16)UINT16_C( exp)
#define U8C(  exp)   ( u8 )UINT8_C(  exp)

#define S64C( exp)   ( s64)INT64_C( exp)
#define S32C( exp)   ( s32)INT32_C( exp)
#define S16C( exp)   ( s16)INT16_C( exp)
#define S8C(  exp)   ( s8 )INT8_C(  exp)


    
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

#define MS_RAND_MAX   U32C( 0xffffffff)

static inline void *MS_CreateFromRef( const size_t, const void *);
static inline int MS_Free( void *);
static inline u32 gen_divobj( u32);
static inline u32 mol_( u32, u32, u32);
static inline u32 div_( u32, u32, u32);
/* protype of named parmenter function daclaration, migth be useful in other cases*/
static inline u32 MS_rand( u32);

static inline u32 MS_rand_seed( void);
static inline int MS_print( FILE *, const char *, ...);
static inline __uint64_t getmicrosec( void);
static inline __uint64_t getnanosec( void);

static inline void *
MS_CreateFromRef( const size_t alo_size, const void *vptr){
  void *ptr = ( void *)malloc( alo_size);
  assert( alo_size);
  assert( ptr != NULL);
  memcpy( ptr, vptr, alo_size);
  return ptr;
}
#define MS_Create( type, ...) ( type *)MS_CreateFromRef( sizeof( type), &( const type){ __VA_ARGS__})
#define MS_CreateEmpty( type) ( type *)MS_CreateFromRef( sizeof( type), &( const type){0})


static inline  int
MS_Free( void *ptr){
  if( ptr != NULL) free( ptr);
  return 0;
}

/* divsion is slow, make sure we don't do it more then we have to*/

/* genrate a divobj from the divaider */
static inline u32
gen_divobj( u32 a){
  return (u32)( ( U64C( 0xffffffff) + (u64)a) / (u64)a);
}

/* divobj = gen_divobj( a)
 * ( b % a) => 
 */
static inline u32
mol_( u32 b, u32 a, u32 divobj){
  u32 ret = ( (u64)( ( b * divobj) & U32C( 0xffffffff)) * (u64)a) >> 32;
  dassert( div_( b, a, divobj) * a + ret == b);
  return ret;
}

/* divobj = gen_divobj( a)
 * ( b / a) =>
 */
static inline u32
div_( u32 b, u32 a, u32 divobj){
  u32 ret = ( (u64)b * (u64)divobj) >> 32;
  dassert( ret * a <= b && ret * a + a > b);
  return ret;
}

/*
 * return a "random" number...
 *
 * usagae:
 * 
 * exmpel 0: seed = MS_rand_seed();
 * 
 * "exmpel 0" is the recomended way to get a good seed.
 * 
 * exmpel 1: MS_rand( ++seed);
 *
 * "exmpel 1" might sem like a good idea, but it makes it farly simple to
 * predict seed and furthur return value's, ther for is "exmpel 2" recumanded...
 *
 * exmpel 2: seed = MS_rand( seed);
 * 
 * if you have two calls to MS_rand() with litel diferance betwen seed,
 * it's farliy easy to caluculate the "secret" numbers.
 */
static inline u32
MS_rand( u32 seed){
  return ( u32)( ( (u64)seed + U64C( 2654435405)) * U64C( 2654435909)) & U32C( 0xffffffff);
}


/*
 * return a seed, for use with MS_rand( __uint32_t seed)
 */
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


_Pragma("GCC diagnostic ignored \"-Wformat-nonliteral\"")

static inline int
MS_print( FILE *stream, const char * format, ...){
  int ret = 0;
#ifdef NO_TERM
  ( void) stream;
  ( void) format;
#else
  if( stream != NULL){
    va_list args;
    va_start( args, format);
    ret = vfprintf( stream, format, args);
    fflush( stream);
    va_end( args);
  }
#endif
  return ret;
}
#ifdef DEBUG
#define DEBUG_PRINT MS_print
#else
#define DEBUG_PRINT( ...) (void)0
#endif

/*
 * get system time in microsecond...
 */
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


/*
 * get the amount of nanoseconds from a point in the past
 */
static inline  __uint64_t
getnanosec( void){
#ifdef CLOCK_MONOTONIC
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
