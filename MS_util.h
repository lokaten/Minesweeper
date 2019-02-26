

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

#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 199901
#define INLINE static inline
#endif
#endif

#ifdef UNSAFE
#undef assert
#define assert( exp) (void)0
#endif

#ifdef INLINE 
#else
#define INLINE 
#endif

#ifdef NO_TERM
#define TERM( string) ""
#else
#define TERM( string) string
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

typedef struct{
  s16 x;
  s16 y;
}MS_pos;

    
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
  
/*ENUT     0b00101111*/
#define ENUT     0x2f

/*ESET     0b10000000*/
#define ESET     0x80
#define SSET     7
/*EFLAG    0b01000000*/
#define EFLAG    0x40
#define SFLAG    6
/*ECOVER   0b00100000*/
#define ECOVER   0x20
#define SCOVER   5
/*EMINE    0b00010000*/
#define EMINE    0x10
#define SMINE    4
/*ECOUNT   0b00001111*/
#define ECOUNT   0x0f

#define MS_RAND_MAX   4294967295u
  /*
#undef LOCALE_
#define LOCALE_( name) __FILE__##name
  */
#ifdef LOCALE_

static inline void *LOCALE_( MS_Create)( size_t, u16, ...);
INLINE int LOCALE_( MS_Free)( void *);
static inline u32 LOCALE_( gen_divobj)( u32);
static inline u32 LOCALE_( mol_)( u32, u32, u32);
static inline u32 LOCALE_( div_)( u32, u32, u32);
/* protype of named parmenter function daclaration, migth be useful in other cases*/
typedef struct{ unsigned long seed;}MS_rand_args;
INLINE __uint32_t LOCALE_( MS_rand)( MS_rand_args);
#define MS_rand( exp) LOCALE_( MS_rand)( ( MS_rand_args){ exp})

INLINE unsigned long LOCALE_( rand_seed)( void);
INLINE int LOCALE_( print)( FILE *, const char *, ...);
INLINE __uint64_t LOCALE_( getmicrosec)( void);
INLINE __uint64_t LOCALE_( getnanosec)( void);

static inline void *
LOCALE_( MS_Create)( size_t alo_size, u16 num_elements, ...){
  void *ret = NULL;
  void *ptr = ( void *)malloc( alo_size * ( num_elements ? num_elements: 1));
  va_list data;
  assert( alo_size);
  va_start( data, num_elements); /* no goto before this line */
  if( ptr == NULL) goto end;
  ret = ptr;
  while( num_elements--){
    memcpy( ptr, va_arg( data, void *), alo_size);
    ptr = ( char *)ptr + alo_size;
  }
 end:
  va_end( data);
  return ret;
}
#define MS_Create( type, ...) ( type *)LOCALE_( MS_Create)( sizeof( type), 1, ( void *)&( ( type){ __VA_ARGS__}))
#define MS_CreateEmpty( type) ( type *)LOCALE_( MS_Create)( sizeof( type), 0)
#define MS_CreateCopy( type, src) ( type *)LOCALE_( MS_Create)( sizeof( type), 1, ( void *)src)

INLINE int
LOCALE_( MS_Free)( void *ptr){
  if( ptr != NULL) free( ptr);
  return 0;
}
#define MS_Free LOCALE_( MS_Free)

/* divsion is slow, make sure we don't do it more then we have to*/

/* genrate a divobj from the divaider */
static inline u32
LOCALE_( gen_divobj)( u32 a){
  return (u32)( ( UINT64_C( 4294967295) + (u64)a) / (u64)a);
}
#define gen_divobj LOCALE_( gen_divobj)

/* divobj = gen_divobj( a)
 * ( b % a) => 
 */
static inline u32
LOCALE_( mol_)( u32 b, u32 a, u32 divobj){
  u32 ret = ( ( ( (u64)b * (u64)divobj) & UINT64_C( 4294967295)) * (u64)a) >> 32;
  assert( LOCALE_( div_)( b, a, divobj) * a + ret == b);
  return ret;
}
#define mol_ LOCALE_( mol_)

/* divobj = gen_divobj( a)
 * ( b / a) =>
 */
static inline u32
LOCALE_( div_)( u32 b, u32 a, u32 divobj){
  u32 ret = ( (u64)b * (u64)divobj) >> 32;
  assert( ret * a <= b && ret * a + a > b);
  return ret;
}
#define div_ LOCALE_( div_)

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
INLINE __uint32_t
LOCALE_( MS_rand)( MS_rand_args args){
  return ( ( ( ( __uint64_t)args.seed + 2654435405lu) * 2654435909lu) & 4294967295lu);
}


/*
 * return a seed, for use with MS_rand( __uint32_t seed)
 */
INLINE unsigned long
LOCALE_( rand_seed)( void){
  unsigned long seed;
  
#ifdef CLOCK_REALTIME
  struct timespec tv;
  
  clock_gettime( CLOCK_REALTIME, &tv);
  
  seed = ( __uint32_t)tv.tv_nsec;
#else
  seed = ( __uint32_t)time( NULL);
#endif
  
  return seed;
}
#define MS_rand_seed LOCALE_( rand_seed)

_Pragma("GCC diagnostic ignored \"-Wformat-nonliteral\"")

INLINE int
LOCALE_( print)( FILE *stream, const char * format, ...){
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
#define MS_print LOCALE_( print)
#ifdef DEBUG
#define DEBUG_PRINT LOCALE_( print)
#else
#define DEBUG_PRINT( ...) 0
#endif

/*
 * get system time in microsecond...
 */
INLINE __uint64_t
LOCALE_( getmicrosec)( void){
#ifdef CLOCK_REALTIME
  struct timespec tv;

  clock_gettime( CLOCK_REALTIME, &tv);

  return ( ( __uint64_t)tv.tv_sec * 1000000 + ( __uint64_t)tv.tv_nsec / 1000);
#else
  return ( __uint64_t)time( NULL) * 1000000;
#endif
}
#define getmicrosec LOCALE_( getmicrosec)

  
/*
 * get the amount of nanoseconds from a point in the past
 */
INLINE __uint64_t
LOCALE_( getnanosec)( void){
#ifdef CLOCK_MONOTONIC
  struct timespec tv;
  
  clock_gettime( CLOCK_MONOTONIC, &tv);
    
  return ( ( __uint64_t)tv.tv_sec * 1000000000 + ( __uint64_t)tv.tv_nsec);
#else
  return ( __uint64_t)time( NULL) * 1000000000;
#endif
}
#define getnanosec LOCALE_( getnanosec)

#endif
  
#ifdef __cplusplus
}
#endif
#endif
