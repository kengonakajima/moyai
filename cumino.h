#ifndef _CUMINO_H_
#define _CUMINO_H_

#include <stdio.h>
#include <stdarg.h>

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


#if defined(__APPLE__)
#include "TargetConditionals.h" // for TARGET_IPHONE_SIMULATOR,TARGET_OS_IPHONE,TARGET_OS_MAC
#endif




#ifndef WIN32
#include <sys/time.h>

typedef unsigned long long ULARGE_INTEGER;
typedef long long LARGE_INTEGER;

#endif


#ifdef WIN32
//#include "stdafx.h"

#include    <windows.h>

#include    <time.h>

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;    
};

#define EPOCHFILETIME (116444736000000000i64)

int gettimeofday(struct timeval *tv, struct timezone *tz);
#ifndef _WIN32_FDAPI_H
int read( int fdesc, char *buf, size_t nbytes );
void close( SOCKET s );
#endif
long random();

#define snprintf sprintf_s

void srandom( unsigned int seed );

#endif//WIN32

inline double now() {
    struct timeval tmv;
    gettimeofday( &tmv, NULL );
    return tmv.tv_sec  + (double)(tmv.tv_usec) / 1000000.0f;
}
inline long long now_msec() {
    struct timeval tmv;
    gettimeofday( &tmv, NULL );
    return tmv.tv_sec*1000  + tmv.tv_usec/1000;    
}
inline long long now_usec() {
    struct timeval tmv;
    gettimeofday( &tmv, NULL );
    return tmv.tv_sec*1000000  + tmv.tv_usec;
}


inline float len(float x0, float y0, float x1, float y1 ){
    return sqrt( (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
}
inline float len(float x0, float y0, float z0, float x1, float y1, float z1 ){
    return sqrt( (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) + (z1-z0)*(z1-z0) );    
}

inline void normalize( float *x, float *y, float *z, float l ) {
    float ll = len(0,0,0,*x,*y,*z);
    if(ll==0){
        *x=0; *y=0; *z=0;
    } else {
        *x = *x / ll * l;
        *y = *y / ll * l;
        *z = *z / ll * l;
    }
}

inline void normalize( float *x, float *y, float l ) {
    float ll = len(0,0,*x,*y);
    if(ll==0){
        *x=0; *y=0;
    } else {
        *x = *x / ll * l;
        *y = *y / ll * l;    
    }
}

inline float maxf( float a, float b ){
    return (a>b) ? a:b;
}
inline float maxf( float a, float b, float c ) {
    return maxf( maxf(a,b), c );
}
inline float maxf( float a, float b, float c, float d ) {
    return maxf( maxf(a,b), maxf(c,d) );
}
inline double maxd( double a, double b ){
    return  (a>b) ? a:b;
}
inline double mind( double a, double b ){
    return (a<b) ? a:b;
}
inline int maxi( int a, int b ){
    return  (a>b) ? a:b;
}
inline int mini( int a, int b ){
    return (a<b) ? a:b;
}



inline double range( double a, double b ) {
    long r = random();
    double rate = (double)r / (double)(0x7fffffff);
    double _a = mind(a,b);
    double _b = maxd(a,b);
    return _a + (_b-_a)*rate;
}
inline int irange( int a, int b ) {
    double r = range(a,b);
    return (int)r;
}
inline bool birandom(){
    if( range(0,1) < 0.5 ) return true; else return false;
}
inline int plusMinusOne() {
    if(range(0,1)<0.5) return -1; else return 1;
}

inline float avg( float a, float b ) {
    return (a+b)/2;
}
inline float absolute( float a ) {
    if(a<0)return a*-1; else return a;
}
inline float interpolate( float left, float right, float zero_to_one ) {
    return left * ( 1 - zero_to_one ) + right * zero_to_one;
}
inline bool isPowerOf2(unsigned int x) {
    return !( x & (x-1) );
}
inline void swapf( float *a, float *b ) {
    float tmp = *b;
    *b = *a;
    *a = tmp;
}


void enablePrint(bool enable);
void print( const char *fmt, ... );
void prt(const char *fmt, ... );

void assertmsg( bool cond, const char *fmt, ... );

#ifndef elementof
#define elementof(x) ( (int)(sizeof(x) / sizeof(x[0])))
#endif

#define choose(ary) ( ary[ irange(0, elementof(ary)) ] )
#define nchoose(ary,n) ( ary[ irange(0, n) ] )


inline int chooseInt2( int a, int b ) {
    if( (random()%2) == 0 ) return a; else return b;
}

class SorterEntry {
public:
    float val;
    void *ptr;
    SorterEntry(float v, void *p) : val(v), ptr(p){}
    SorterEntry() : val(0), ptr(0) {}
};
void quickSortF(SorterEntry array[], int left ,int right);


typedef enum {
    DIR4_NONE=-1,    
    DIR4_UP=0,
    DIR4_RIGHT=1,
    DIR4_DOWN=2,
    DIR4_LEFT=3,
} DIR4;

DIR4 randomDir();
DIR4 reverseDir(DIR4 d);
DIR4 randomTurnDir( DIR4 d );
DIR4 dxdyToDir(int dx, int dy );
void dirToDXDY( DIR4 d, int *dx, int *dy );
bool birandom();
DIR4 rightDir( DIR4 d );
DIR4 leftDir( DIR4 d );
bool validateDir( DIR4 d);

inline int sign(float f){
    if(f>0) return 1; else if(f<0) return -1; else return 0;
}

bool writeFile( const char *path, const char *data, size_t sz, bool to_sync = false );
bool writeFileOffset( const char *path, const char *data, size_t sz, size_t offset, bool to_sync );
bool appendFile( const char *path, const char *data, size_t sz );
bool readFile( const char *path, char *data, size_t *sz );
bool readFileOffset( const char *path, char *data, size_t *sz, size_t offset );
int getModifiedTime( const char *path, time_t *out );
bool deleteFile( const char *path );

void dump(const char*s, size_t l);

extern bool g_cumino_mem_debug;
extern unsigned long g_cumino_total_malloc_count;
extern unsigned long g_cumino_total_malloc_size;

void *MALLOC( size_t sz );
void FREE( void *ptr );

//void *operator new(size_t sz);
//void operator delete(void*ptr);

int cuminoPrintMemStat(int thres_count);

bool findChar( const char *s, char ch );
int findCharIndexOf( const char *s, char ch );

void startMeasure(const char *name);
void endMeasure();

int memCompressSnappy( char *out, int outlen, char *in, int inlen );
int memDecompressSnappy( char *out, int outlen, char *in, int inlen );


#ifndef INVALID_SOCKET // winsock
#define INVALID_SOCKET (-1)
#endif

unsigned int hash_pjw( const char* s );
int atoilen( const char *s, int l );
unsigned int strtoullen( const char *s, int l );
unsigned long long strtoulllen( const char *s, int l );

class Format {
public:
    char buf[1024];
    Format( const char *fmt, ... ) {
        va_list argptr;
        va_start( argptr, fmt );
        vsnprintf( buf, sizeof(buf), fmt, argptr );
        va_end( argptr );
    }
    void trimWith( int len, char ch = '.' ) {
        int l = strlen(buf);
        if( l < len )return;
        for(int i=len-3;i<len;i++) {
            buf[i] = ch;
        }
        buf[len] = '\0';
    }
};

void truncateString( char *out, const char *in, int outlen );

int countChar(const char *s, int ch);

void sleepMilliSec( int ms );
const char *platformCStringPath( const char *path );

void gsubString(char *s, char from, char to );

#endif
