

#ifdef _MS_UTIL_H__
#else
#define _MS_UTIL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

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

#ifdef INLINE 
#else
#define INLINE 
#endif

#ifdef NDEBUG
#undef assert
#define assert( exp) ( void)( exp);
#endif

typedef struct{
  __uint16_t x;
  __uint16_t y;
}MS_pos;

    
typedef struct{
  unsigned long realheight;
  unsigned long realwidth;
  unsigned long height;
  unsigned long width;
  unsigned long realxdiff;
  unsigned long realydiff;
  unsigned long xdiff;
  unsigned long ydiff;
}MS_video;


typedef struct{
  FILE *err;
  FILE *out;
  FILE *deb;
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
#define MS_RAND16_MAX 65535
#define MS_RAND8_MAX  255

#ifdef LOCALE_

INLINE unsigned long
LOCALE_( gen_divobj)( unsigned long a){
  return ( 8589934591lu + a) / a;
}
#define gen_divobj LOCALE_( gen_divobj)
  
INLINE unsigned long
LOCALE_( mol_)( unsigned long b, unsigned long a, unsigned long divobj){
  return ( ( ( ( b * divobj) & 8589934591lu) * a) >> 33);
}
#define mol_ LOCALE_( mol_)
  
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
LOCALE_( MS_rand)( __uint32_t seed){
  return ( ( ( ( __uint64_t)seed + 2654435405lu) * 2654435909lu) & 4294967295lu);
}
#define MS_rand LOCALE_( MS_rand)


/*
 * return a seed, for use with MS_rand( __uint32_t seed)
 */
INLINE __uint32_t
LOCALE_( rand_seed)( void){
  __uint32_t seed;
  
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
