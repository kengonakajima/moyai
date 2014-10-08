/*
  lumino, rumino, jumino, cumino
 */

#ifndef WIN32
#include <sys/time.h>
#include <strings.h>
#endif


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "zlib.h"
#include "lz4/lz4.h"
#include "lz4/lz4hc.h"

#include "cumino.h"

#ifdef WIN32

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME        tagFileTime;
    LARGE_INTEGER   largeInt;
    __int64         val64;
    static int      tzflag;

    if (tv)
    {
        GetSystemTimeAsFileTime(&tagFileTime);

        largeInt.LowPart  = tagFileTime.dwLowDateTime;
        largeInt.HighPart = tagFileTime.dwHighDateTime;
        val64 = largeInt.QuadPart;
        val64 = val64 - EPOCHFILETIME;
        val64 = val64 / 10;
        tv->tv_sec  = (long)(val64 / 1000000);
        tv->tv_usec = (long)(val64 % 1000000);
    }

    if (tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }

#if _MSC_VER == 1310
        //Visual C++ 6.0でＯＫだった・・ 
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
#else
        long _Timezone = 0;
         _get_timezone(&_Timezone);
         tz->tz_minuteswest = _Timezone / 60;

		 int _Daylight = 0;
         _get_daylight(&_Daylight);
         tz->tz_dsttime = _Daylight;
#endif         
    }

    return 0;
}

int read( int fdesc, char *buf, size_t nbytes ){
	return recv( fdesc, buf, nbytes, 0 );
}
void close( SOCKET s ){
	closesocket(s);
}

long random() { 
	int top = rand();
	int bottom = rand();
	long out = ( top << 16 ) | bottom;
	return out;
}
#endif

bool g_enablePrint = true;
void enablePrint(bool enable){
    g_enablePrint = enable;
}

void prt(const char *fmt, ... ){
    if(!g_enablePrint)return;
    char dest[1024*16];
    va_list argptr;
    va_start( argptr, fmt );
    vsprintf( dest, fmt, argptr );
    va_end( argptr );
    fprintf( stderr, "%s", dest );   
#ifdef WIN32
	OutputDebugStringA(dest);
#endif
}

void print( const char *fmt, ... ){
    if(!g_enablePrint)return;    
    char dest[1024*16];
    va_list argptr;
    va_start( argptr, fmt );
    vsnprintf( dest, sizeof(dest), fmt, argptr );
    va_end( argptr );
    fprintf( stderr, "%s\n", dest );
#ifdef WIN32
	OutputDebugStringA(dest);
	OutputDebugStringA("\n");
#endif
}



void assertmsg( bool cond, const char *fmt, ... ) 
{
#if (!defined(WIN32)) || defined(DEBUG) || defined(_DEBUG)
    if(!cond){
        print("XXXXXXXXXXXXXXx fmt:%s",fmt);
        char dest[1024*16];
        va_list argptr;
        va_start( argptr, fmt );
        vsprintf( dest, fmt, argptr );
        va_end( argptr );
        print( "%s\n", dest );
        assert(cond);
    }
#endif


}



void quickSortSwap(SorterEntry *a, SorterEntry *b ){
  SorterEntry temp;
  temp = *a;
  *a=*b;
  *b=temp;
}

void quickSortF(SorterEntry array[],  int left ,int right){
    float center;
    int l,r;
    if(left <= right){
        center = array[ ( left + right ) / 2 ].val;
        l = left;
        r = right;
        while(l <= r){
            while(array[l].val < center)l++;
            while(array[r].val > center)r--;
            if(l <= r){
                quickSortSwap(&array[l],&array[r]);
                l++;
                r--;
            }
        }
        quickSortF(array, left, r);
        quickSortF(array, l, right);
    }
}


bool validateDir( DIR d ) {
    switch(d) {
    case DIR_UP:
    case DIR_DOWN:
    case DIR_RIGHT:
    case DIR_LEFT:
        return true;
    default:
        return false;
    }
}
DIR randomDir(){
    switch( irange(0,4) ) {
    case 0: return DIR_UP;
    case 1: return DIR_RIGHT;
    case 2: return DIR_LEFT;
    case 3: return DIR_DOWN;
    }
    assert(false);/* not reached */
    return DIR_NONE; 
}
DIR randomTurnDir( DIR d ){
    switch(d){
    case DIR_UP:
    case DIR_DOWN:
        return birandom() ? DIR_LEFT : DIR_RIGHT;
    case DIR_RIGHT:
    case DIR_LEFT:
        return birandom() ? DIR_UP : DIR_DOWN;
    default:
        return DIR_NONE;
    }
}
DIR reverseDir( DIR d ){
    switch(d){
    case DIR_UP: return DIR_DOWN;
    case DIR_DOWN: return DIR_UP;
    case DIR_RIGHT: return DIR_LEFT;
    case DIR_LEFT: return DIR_RIGHT;
    default:
        return DIR_NONE;
    }
}

DIR rightDir( DIR d ) {
    switch(d){
    case DIR_UP: return DIR_RIGHT; 
    case DIR_DOWN: return DIR_LEFT;
    case DIR_RIGHT: return DIR_DOWN;
    case DIR_LEFT: return DIR_UP;
    default:
        return DIR_NONE;
    }
}
DIR leftDir( DIR d ) {
    switch(d){
    case DIR_UP: return DIR_LEFT;
    case DIR_DOWN: return DIR_RIGHT;
    case DIR_RIGHT: return DIR_UP;
    case DIR_LEFT: return DIR_DOWN;
    default:
        return DIR_NONE;
    }    
}

// 4方向のみ
DIR dxdyToDir(int dx, int dy ){
    if(dx>0){
        assert(dy==0);
        return DIR_RIGHT;
    } else if(dx<0){
        assert(dy==0);        
        return DIR_LEFT;
    } else if(dy>0){
        assert(dx==0);        
        return DIR_UP;
    }else if(dy<0){
        assert(dx==0);                
        return DIR_DOWN;
    } else {
        assert(dx==0&&dy==0);                
        return DIR_NONE;
    }
}

void dirToDXDY( DIR d, int *dx, int *dy ){
    *dx = *dy = 0;
    switch(d){
    case DIR_NONE: return;
    case DIR_RIGHT: *dx = 1; return;
    case DIR_LEFT: *dx = -1; return;
    case DIR_UP: *dy = 1; return;
    case DIR_DOWN: *dy = -1; return;
    }
}

bool writeFile( const char *path, const char *data, size_t sz ){
    FILE *fp = fopen( path, "wb");
    if(!fp){
        print("cannot open file: '%s" , path );
        return false;
    }
    if( fwrite( data, 1, sz, fp ) != sz ) return false;
    fclose(fp);
    return true;
}
bool readFile( const char *path, char *data, size_t *sz ){
    size_t toread = *sz;
    FILE *fp = fopen( path, "rb");
    if( !fp) return false;
    size_t rl = fread( (void*)data, 1, toread, fp );
    fclose(fp);
    *sz = rl;
    return true;

}

void dump(const char*s, size_t l) {
    for(size_t i=0;i<l;i++){
        prt( "%02x ", s[i] & 0xff );
        if((i%8)==7) prt("  ");
        if((i%16)==15) prt("\n");
    }
    prt("\n");
}


int getModifiedTime( const char *path, time_t *out ) {
    struct stat st;
    int r = stat(path,&st);
    if( r < 0 ) return -1;
    *out = st.st_mtime;
    return 0;
}


bool g_cumino_mem_debug = false;
unsigned long g_cumino_total_malloc_count=0;
unsigned long g_cumino_total_malloc_size=0;

class MemEntry {
public:
    size_t sz;
    int tag;
    MemEntry *next;
};


MemEntry *g_cumino_memtops[1024];

#define ENVSIZE 32
void *MALLOC( size_t sz ) {
    void *out;
    if( g_cumino_mem_debug ) {
        assert( ENVSIZE >= sizeof(MemEntry) );
        void *envaddr = malloc( sz + ENVSIZE );
        out = (void*)( ((char*)envaddr) + ENVSIZE ); 
        MemEntry *e = (MemEntry*)envaddr;
        memset( e, 0, sizeof(MemEntry) );
        
        unsigned long long addr = (unsigned long long) envaddr;
        int mod = (addr/16) % elementof(g_cumino_memtops);
        if( g_cumino_memtops[mod] ){
            e->next = g_cumino_memtops[mod];
            //                        print("ins:%p out:%p",e,out);
        } else {
            //                        print("newtop:%p out:%p",e,out);
        }
        g_cumino_memtops[mod] = e;
        e->sz = sz;
    } else {
        out = malloc(sz);
    }
	g_cumino_total_malloc_count++;
	g_cumino_total_malloc_size += sz;
    return out;
}
void FREE( void *ptr ) {
    if( g_cumino_mem_debug ) {
        char *envaddr = ((char*)ptr) - ENVSIZE;
        unsigned long long addr = (unsigned long long)envaddr;
        int mod = (addr/16) % elementof(g_cumino_memtops);
        MemEntry *tgt = (MemEntry*) envaddr;
        MemEntry *cur = g_cumino_memtops[mod];
        assert(cur);
        if(cur==tgt){
            //                        print("found on top! %p given:%p", tgt, ptr );
            g_cumino_memtops[mod] = cur->next;
        } else {
            MemEntry *prev = NULL;
            while(cur) {
                if(cur == tgt ) {
                    //                                        print("found in list! %p given:%p", cur, ptr );
                    assert(prev);
                    prev->next = cur->next;
                }
                prev = cur;
                cur = cur->next;
            }

        }
        free(envaddr);
    } else {
        free(ptr);
    }
	g_cumino_total_malloc_count --;
}

//void *operator new(size_t sz) {
//    return MALLOC(sz);
//}
//void operator delete(void*ptr) {
//    FREE(ptr);
//}

class MemStatEnt {
public:
    size_t sz;
    int count;
};

int cuminoPrintMemStat( int thres_count ) {
    MemStatEnt ents[2048];
    memset( ents, 0, sizeof(ents) );
    for(int i=0;i<elementof(g_cumino_memtops);i++){
        MemEntry *cur = g_cumino_memtops[i];
        while(cur) {
            for(int j=0;j<elementof(ents);j++){
                if(ents[j].sz==cur->sz) {
                    ents[j].count++;
                    break;
                } else if( ents[j].sz == 0 ) {
                    ents[j].count = 1;
                    ents[j].sz = cur->sz;
                    break;
                }
                assert(j!=elementof(ents)-1); // exhausted
            }
            cur = cur->next;
        }
    }
    
    //

    SorterEntry sorter[ elementof(ents) ];
    int si=0;
    for(int i=0;i<elementof(ents);i++) {
        if( ents[i].sz > 0 ) {
            sorter[si].ptr = & ents[i];
            sorter[si].val = (float) ents[i].count * ents[i].sz;
            si++;
        }
    }
    quickSortF( sorter, 0, si-1 );
    
    //
    for(int i=0;i<si;i++){
        MemStatEnt *e = (MemStatEnt*) sorter[i].ptr;
        if( e->count >= thres_count ) {
            print("[%d] size:%d count:%d  total:%d", i, e->sz, e->count, e->sz * e->count );
        }
    }

    //
    long long total_size=0;
    int obj_count=0;
    for(int i=0;i<elementof(ents);i++){
        if(ents[i].sz==0)break;
        total_size += ents[i].sz * ents[i].count;
        obj_count += ents[i].count;
    }
    print( "Total: %lld bytes %lld Mibytes", total_size, total_size/1024/1024);
    return obj_count;
}

bool findChar( const char *s, char ch ) {
    const char *p = s;
    while(*p) {
        if(*p == ch ) return true;
        p++;
    }
    return false;
}

////////////


double g_measure_start_time;
char *g_measure_name;
void startMeasure(const char *name) {
    g_measure_name = (char*)name;
    g_measure_start_time = now();
}

void endMeasure() {
    double et = now();
    if(!g_measure_name) g_measure_name = (char*) "no name";
    print("endMeasure at %s : %f(ms)", g_measure_name, (et-g_measure_start_time)*1000 );
}

void highResolutionSleep( float second ) {
#if defined(__APPLE__) || defined(__linux__)
    usleep( second * 1000000 );
#elif defined(WIN32)
    
#else
    assertmsg(false, "not implemented" );
#endif    
}

///////////

int memDecompress( char *out, int outlen, const char *in, int inlen ) {
    char buf[4096];
    int out_so_far =0;
    z_stream z;
    
    memset(&z,0,sizeof(z));
    z.zalloc = NULL;
    z.zfree = NULL;
    z.opaque = NULL;
    if(inflateInit( &z ) != Z_OK ){
        return -123;
    }
    
    z.next_in = (Bytef*)in;
    z.avail_in = inlen;
    z.next_out = (Bytef*)buf;
    z.avail_out = sizeof(buf);

    while(1){
        int r,outsz;
//        printf( "AAAAAA %d\n",out_so_far );
        r = inflate( &z, Z_NO_FLUSH );
        if( r == Z_STREAM_END || r == Z_BUF_ERROR ){
            break;
        }
        if( r != Z_OK) {
            return -1234;
        }

        /* inflation OK! */
        outsz = sizeof(buf)- z.avail_out;
        if( (out_so_far +outsz)> outlen){
            return -12345;
        }
        memcpy( out + out_so_far , buf, outsz );
        out_so_far += outsz;
        z.next_out = (Bytef*)buf;
        z.avail_out = sizeof(buf);
    }

    return out_so_far;
}

int memCompress( char *out, int outlen, const char *in, int inlen ) {
    char buf[4096];
    int out_so_far = 0;
    z_stream z;
    
    memset(&z,0,sizeof(z));
    z.zalloc = NULL;
    z.zfree = NULL;
    z.opaque = NULL;
    if(deflateInit( &z, Z_DEFAULT_COMPRESSION ) != Z_OK ){
        return -123;
    }
    
    z.avail_in = inlen;
    z.next_in = (Bytef*)in;

    /* got hint from ssh3 code */
    do {

        int r,outsz;
        
        z.next_out = (Bytef*)buf;
        z.avail_out = sizeof(buf);
        if( z.avail_in != 0 ){
            r = deflate( &z, Z_PARTIAL_FLUSH);
        } else {
            r = deflate( &z, Z_SYNC_FLUSH );
        }

        if( r == Z_OK ){
            outsz = sizeof(buf) - z.avail_out;
            if( (out_so_far + outsz) > outlen ){
                deflateEnd(&z);
                return -1234;
            }
            memcpy( out + out_so_far, buf, outsz );
            out_so_far += outsz;
        } else {
            deflateEnd(&z);
            return -12345;
        }
    } while( z.avail_out == 0 );

    deflateEnd(&z);
    return out_so_far;
}


int memCompressLZ4( char *out, int outlen, const char *in, int inlen ) {
    return LZ4_compressHC ( in, out, inlen );
}
int memDecompressLZ4( char *out, int outlen, const char *in, int inlen ) {
    return LZ4_decompress_safe ( in, out, inlen, outlen );
}


unsigned int hash_pjw( const char* s ) {
    char *p;
    unsigned int h = 0 ,g;
    for( p = (char*) s ; *p ; p ++ ){
        h = ( h<< 4 ) + (*p);
        if( (g = h & 0xf0000000) != 0){
            h = h ^ (g>>24);
            h = h ^ g;
        }
    }
    return h;
}   

int atoilen( const char *s, int l ) {
    char buf[64];
    strncpy( buf, s, l );
    return atoi(buf);
}
unsigned int strtoullen( const char *s, int l ) {
    char buf[64];
    int copylen = l;
    if( copylen >= sizeof(buf) ) copylen = sizeof(buf)-1;
    strncpy( buf, s, copylen );
    buf[copylen] = '\0';
    return strtoul( buf, NULL, 10 );
}
unsigned long long strtoulllen( const char *s, int l ) {
    char buf[64];
    int copylen = l;
    if( copylen >= sizeof(buf) ) copylen = sizeof(buf)-1;
    strncpy( buf, s, copylen );
    buf[copylen] = '\0';
#if defined(WIN32)
    return _strtoui64( buf, NULL, 10 );
#else    
    return strtoull( buf, NULL, 10 );
#endif    
}
void truncateString( char *out, const char *in, int outlen ) {
    strncpy( out, in, outlen );
    int in_l = strlen(in);
    if(in_l>outlen) {
        out[outlen] = '\0';
    } else {
        out[in_l] = '\0';
    }
}
