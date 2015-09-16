
#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include <sys/time.h>

#ifdef _MS_UTIL_H__
#else
#define _MS_UTIL_H__
#ifdef __cplusplus
extern "C" {
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

typedef struct{
  __uint16_t x;
  __uint16_t y;
}MS_pos;


typedef struct{
  __uint8_t *data;
  unsigned long realwidth;
  unsigned long realheight;
  unsigned long width;
  unsigned long height;
  unsigned long subwidth;
  unsigned long subheight;
  unsigned long global;
}MS_field;

    
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

/*EC_ZERO*/
#define EC_ZERO 0
/*EC_ONE*/
#define EC_ONE 1
/*EC_TWO*/
#define EC_TWO 2
/*EC_THERE*/
#define EC_THERE 3
/*EC_FOUR*/
#define EC_FOUR 4
/*EC_FIVE*/
#define EC_FIVE 5
/*EC_SIX*/
#define EC_SIX 6
/*EC_SEVEN*/
#define EC_SEVEN 7
/*EC_EIGHT*/
#define EC_EIGHT 8


#define MS_RAND_MAX   4294967290u
#define MS_RAND16_MAX 65535
#define MS_RAND8_MAX  255

#ifdef LOCALE_

/*
 * return a "random" number...
 *
 * usagae:
 * 
 * exmpel 0: seed = MS_rand_seed();
 * 
 * "exmpel 0" is the recomended way to get a good seed, it works around known
 * bad seed's, and will also be updated for futuer stupidity that the devoloper
 * behinde "MS_util.h" migth have...
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
  return ( ( ( ( __uint64_t)seed + 2654435671u) * 2654435803u) % 4294967291u);
}
#define MS_rand LOCALE_( MS_rand)

  
/*
 * same as "MS_rand()"
 */
INLINE __uint16_t
LOCALE_( MS_rand16)( __uint16_t seed){
  return ( ( ( ( __uint32_t)seed + 1) * 40897) % 65537) - 1;
}
#define MS_rand16 LOCALE_( MS_rand16)

/*
 * same as "MS_rand()"
 */
INLINE __uint8_t
LOCALE_( MS_rand8)( __uint8_t seed){
  return ( ( ( ( __uint16_t)seed + 1) * 167) % 257) - 1;
}
#define MS_rand8 LOCALE_( MS_rand8)



/*
 * return a seed, for use with MS_rand( void)
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

  /* break loop-back...*/
  while( seed == MS_rand( seed)){
    seed += 42;
  }
      
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
