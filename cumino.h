#ifndef _CUMINO_H_
#define _CUMINO_H_

#include <stdio.h>
#include <stdarg.h>

#include <sys/time.h>
#include <math.h>
#include <stdlib.h>



inline double now() {
    struct timeval tmv;
    gettimeofday( &tmv, NULL );
    return tmv.tv_sec  + (double)(tmv.tv_usec) / 1000000.0f;
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
inline double max( double a, double b ){
    return  (a>b) ? a:b;
}
inline double min( double a, double b ){
    return (a<b) ? a:b;
}

inline double range( double a, double b ) {
    long r = random();
    double rate = (double)r / (double)(0x7fffffff);
    double _a = min(a,b);
    double _b = max(a,b);
    return _a + (_b-_a)*rate;
}
inline int irange( int a, int b ) {
    double r = range(a,b);
    return (int)r;
}
inline bool birandom(){
    if( range(0,1) < 0.5 ) return true; else return false;
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

void enablePrint(bool enable);
void print( const char *fmt, ... );
void prt(const char *fmt, ... );

void assertmsg( bool cond, const char *fmt, ... );

#define elementof(x) ( (int)(sizeof(x) / sizeof(x[0])))
#define choose(ary) ( ary[ irange(0, elementof(ary)) ] )

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
    DIR_NONE=-1,    
    DIR_UP=0,
    DIR_RIGHT=1,
    DIR_DOWN=2,
    DIR_LEFT=3,
} DIR;

DIR randomDir();
DIR reverseDir(DIR d);
DIR randomTurnDir( DIR d );
DIR dxdyToDir(int dx, int dy );
void dirToDXDY( DIR d, int *dx, int *dy );
bool birandom();
DIR rightDir( DIR d );
DIR leftDir( DIR d );

inline int sign(float f){
    if(f>0) return 1; else if(f<0) return -1; else return 0;
}

bool writeFile( const char *path, const char *data, size_t sz );
bool readFile( const char *path, char *data, size_t *sz );
int getModifiedTime( const char *path, time_t *out );

void dump(const char*s, size_t l);

extern bool g_cumino_mem_debug;

void *MALLOC( size_t sz );
void FREE( void *ptr );

void *operator new(size_t sz);
void operator delete(void*ptr);

int cuminoPrintMemStat(int thres_count);

bool findChar( const char *s, char ch );


#endif
