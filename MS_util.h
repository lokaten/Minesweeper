

#ifdef _MS_UTIL_H__
#else
#define _MS_UTIL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h> //memcpy

#include <sys/time.h>
#ifdef CLOCK_MONOTONIC
#else
#include <SDL2/SDL_timer.h>
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

#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 199901
#define INLINE static inline
#endif
#endif

#ifdef INLINE 
#else
#define INLINE 
#endif

#ifdef NDEBUG
#undef assert
#define assert( exp) ( void)( exp);
#endif


typedef uint_fast64_t u64;
typedef uint_fast32_t u32;
typedef uint_fast16_t u16;
typedef uint_fast8_t  u8;

typedef int_fast64_t  s64;
typedef int_fast32_t  s32;
typedef int_fast16_t  s16;
typedef int_fast8_t   s8;

typedef struct{
  u16 x;
  u16 y;
}MS_pos;

    
typedef struct{
  unsigned long realheight;
  unsigned long realwidth;
  unsigned long height;
  unsigned long width;
  unsigned long element_width;
  unsigned long element_height;
  signed long realxdiff;
  signed long realydiff;
  signed long xdiff;
  signed long ydiff;
}MS_video;


typedef struct{
  FILE *err;
  FILE *out;
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

#ifdef LOCALE_

INLINE void *LOCALE_( MS_Create)( size_t, void *);
INLINE void *LOCALE_( MS_Free)( void *);
INLINE unsigned long LOCALE_( gen_divobj)( unsigned long);
INLINE unsigned long LOCALE_( mol_)( unsigned long, unsigned long, unsigned long);
INLINE unsigned long LOCALE_( div_)( unsigned long, unsigned long, unsigned long);
/* protype of named parmenter function daclaration, migth be useful in other cases*/
typedef struct{ unsigned long seed;}MS_rand_args;
INLINE __uint32_t LOCALE_( MS_rand)( MS_rand_args);
#define MS_rand( exp) LOCALE_( MS_rand)( ( MS_rand_args){ exp})

INLINE unsigned long LOCALE_( rand_seed)( void);
INLINE int LOCALE_( print)( FILE *, const char *, ...);
INLINE __uint64_t LOCALE_( getmicrosec)( void);
INLINE __uint64_t LOCALE_( getnanosec)( void);

INLINE void *
LOCALE_( MS_Create)( size_t alo_size, void *data){
  void *ret = ( void *)malloc( alo_size);
  memcpy( ret, data, alo_size);
  return ret;
}
#define MS_Create( type, exp) ( type *)LOCALE_( MS_Create)( sizeof( type), ( void *)&( exp))
#define MS_CreateEmpty( type) ( type *)LOCALE_( MS_Create)( sizeof( type), ( void *)&( ( type){0}))

INLINE void *
LOCALE_( MS_Free)( void *ptr){
  if( ptr != NULL) free( ptr);
  return NULL;
}
#define MS_Free( exp) LOCALE_( MS_Free)( ( void *)(exp))

/* divsion is slow, make sure we don't do it more then we have to*/

/* genrate a divobj from the divaider */
INLINE unsigned long
LOCALE_( gen_divobj)( unsigned long a){
  assert( a < ( 1lu << 32));
  return ( 8589934591lu + a) / a;
}
#define gen_divobj LOCALE_( gen_divobj)

/* divobj = gen_divobj( a)
 * ( b % a) => 
 */
INLINE unsigned long
LOCALE_( mol_)( unsigned long b, unsigned long a, unsigned long divobj){
  unsigned long ret = ( ( ( ( b * divobj) & 8589934591lu) * a) >> 33);
  //signficantly slower, but more corect version
  //unsigned long ret = b >= a? a > 2? ( ( ( b * divobj) & 8589934591lu) * a) >> 33: ( b & ( a - 1)): b;
  assert( ret == b % a);
  return ret;
}
#define mol_ LOCALE_( mol_)

/* divobj = gen_divobj( a)
 * ( b / a) =>
 */
INLINE unsigned long
LOCALE_( div_)( unsigned long b, unsigned long a, unsigned long divobj){
  unsigned long ret = ( b * ( divobj >> 1)) >> 32;
  ( void)a; /* we only take in a for the assert */
  assert( ret == b / a);
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
  if( stream != NULL){
    va_list args;
    va_start( args, format);
    ret = vfprintf( stream, format, args);
    fflush( stream);
    va_end( args);
  }
  return ret;
}
#define MS_print LOCALE_( print)

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
  return ( __uint64_t)SDL_GetTicks() * 1000000;
#endif
}
#define getnanosec LOCALE_( getnanosec)

#endif
  
#ifdef __cplusplus
}
#endif
#endif
