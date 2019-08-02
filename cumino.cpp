/*
  lumino, rumino, jumino, cumino
 */

#ifndef WIN32
#include <sys/time.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h> //mkdir
#endif

#ifdef WIN32
#include <direct.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef __APPLE__
#include <ftw.h>
#endif


#include "snappy/snappy-c.h"


#include "cumino.h"

#if (TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE)
#import <Foundation/Foundation.h>
#endif




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


bool validateDir( DIR4 d ) {
    switch(d) {
    case DIR4_UP:
    case DIR4_DOWN:
    case DIR4_RIGHT:
    case DIR4_LEFT:
        return true;
    default:
        return false;
    }
}
DIR4 randomDir(){
    switch( irange(0,4) ) {
    case 0: return DIR4_UP;
    case 1: return DIR4_RIGHT;
    case 2: return DIR4_LEFT;
    case 3: return DIR4_DOWN;
    }
    assert(false);/* not reached */
    return DIR4_NONE; 
}
DIR4 randomTurnDir( DIR4 d ){
    switch(d){
    case DIR4_UP:
    case DIR4_DOWN:
        return birandom() ? DIR4_LEFT : DIR4_RIGHT;
    case DIR4_RIGHT:
    case DIR4_LEFT:
        return birandom() ? DIR4_UP : DIR4_DOWN;
    default:
        return DIR4_NONE;
    }
}
DIR4 reverseDir( DIR4 d ){
    switch(d){
    case DIR4_UP: return DIR4_DOWN;
    case DIR4_DOWN: return DIR4_UP;
    case DIR4_RIGHT: return DIR4_LEFT;
    case DIR4_LEFT: return DIR4_RIGHT;
    default:
        return DIR4_NONE;
    }
}

DIR4 rightDir( DIR4 d ) {
    switch(d){
    case DIR4_UP: return DIR4_RIGHT; 
    case DIR4_DOWN: return DIR4_LEFT;
    case DIR4_RIGHT: return DIR4_DOWN;
    case DIR4_LEFT: return DIR4_UP;
    default:
        return DIR4_NONE;
    }
}
DIR4 leftDir( DIR4 d ) {
    switch(d){
    case DIR4_UP: return DIR4_LEFT;
    case DIR4_DOWN: return DIR4_RIGHT;
    case DIR4_RIGHT: return DIR4_UP;
    case DIR4_LEFT: return DIR4_DOWN;
    default:
        return DIR4_NONE;
    }    
}

// 4方向のみ
DIR4 dxdyToDir(int dx, int dy ){
    if(dx>0){
        assert(dy==0);
        return DIR4_RIGHT;
    } else if(dx<0){
        assert(dy==0);        
        return DIR4_LEFT;
    } else if(dy>0){
        assert(dx==0);        
        return DIR4_UP;
    }else if(dy<0){
        assert(dx==0);                
        return DIR4_DOWN;
    } else {
        assert(dx==0&&dy==0);                
        return DIR4_NONE;
    }
}
DIR4 clockDir(DIR4 d) {
    switch(d) {
    case DIR4_NONE: return DIR4_NONE;
    case DIR4_UP: return DIR4_RIGHT;
    case DIR4_RIGHT: return DIR4_DOWN;
    case DIR4_DOWN: return DIR4_LEFT;
    case DIR4_LEFT: return DIR4_UP;
    }
    assert(false);
    return DIR4_NONE;
}
void dirToDXDY( DIR4 d, int *dx, int *dy ){
    *dx = *dy = 0;
    switch(d){
    case DIR4_NONE: return;
    case DIR4_RIGHT: *dx = 1; return;
    case DIR4_LEFT: *dx = -1; return;
    case DIR4_UP: *dy = 1; return;
    case DIR4_DOWN: *dy = -1; return;
    }
}

bool writeFileOffset( const char *path, const char *data, size_t sz, size_t offset, bool to_sync ){
#ifdef WIN32    
    FILE *fp = fopen( path, "wb");
    if(!fp){
        print("writeFileOffset: cannot open file:'%s err:'%s'" , path, strerror(errno) );
        return false;
    }
    if( fwrite( data, 1, sz, fp ) != sz ) return false;
    if( to_sync ) {
        print("writeFileOffset: fsync() is not available on win32");
    }
    fclose(fp);
#else
    int fd = open( path, O_CREAT | O_RDWR, 0644 );
    if(fd<0) return false;
    int rc;
    rc = lseek( fd, offset, SEEK_SET );
    if(rc<0) {
        print("writeFileOffset: lseek failed for file '%s' err:'%s'", path, strerror(errno) );
        close(fd);
        return false;
    }
    rc = write( fd, data, sz );
    if(rc != (int)sz ) {
        print("writeFileOffset: write failed for file '%s' err:'%s'", path, strerror(errno) );
        close(fd);
        return false;
    }
    if( to_sync ) {
        fsync(fd);
    }
    close(fd);
#endif    
    
    
    return true;
}

bool writeFile( const char *path, const char *data, size_t sz, bool to_sync ){
    return writeFileOffset( path, data, sz, 0, to_sync );
}
bool appendFile( const char *path, const char *data, size_t sz ) {
    FILE *fp = fopen(path, "a+b");
    if(!fp) {
        print("appendFile: can't open file '%s' err:'%s'", path, strerror(errno) );
        return false;
    }
    if( fwrite( data, 1, sz,fp ) != sz ) return false;
    fclose(fp);
    return true;
}
int getFileSize( const char *path ) {
#ifdef WIN32
	struct stat s;
	int r = stat(path, &s);
	if (r < 0)return -1;
	return s.st_size;
#else
    struct stat s;
    int r=stat(path,&s);
    if(r<0)return -1;
    return s.st_size;
#endif        
}
bool readFileOffset( const char *path, char *data, size_t *sz, size_t offset ){
    size_t toread = *sz;
#ifdef WIN32    
    FILE *fp = fopen( path, "rb");
    if( !fp) return false;
    size_t rl = fread( (void*)data, 1, toread, fp );
    fclose(fp);
    *sz = rl;
#else
    int fd = open( path, O_RDONLY );
    if(fd<0) {
        //        print("readFileOffset: can't open file:'%s' err:'%s'", path, strerror(errno) );
        return false;
    }
    int rc;
    rc = lseek( fd, offset, SEEK_SET );
    if(rc<0) {
        //        print("readFileOffset: lseek failed for file '%s' err:'%s'", path, strerror(errno) );
        close(fd);
        return false;
    }
    ssize_t readret = read( fd, data, toread );
    if(readret<0) {
        //        print("readFileOffset: read failed for file '%s' err:'%s'", path, strerror(errno) );
        close(fd);
        return false;
    }
    *sz = readret;
    close(fd);
#endif    
    return true;

}
bool readFile( const char *path, char *data, size_t *sz ){
    return readFileOffset( path, data, sz, 0 );
}

bool deleteFile( const char *path ) {
    int err = ::remove(path);
    return (err==0);    
}
bool makeDirectory(const char *path) {
#ifdef WIN32    
    if(_mkdir(path)==0) return true;
#endif
#if defined(__APPLE__)
    if(mkdir(path, S_IRWXU)==0) return true;
#endif    
#ifdef __linux    
    if(mkdir(path)==0) return true;
#endif
    return false;
}

int bytesum(const char *s, size_t l ) {
    int total=0;
    for(size_t i=0;i<l;i++){
        total+=s[i];
    }
    return total;
}
void dump(const char*s, size_t l) {
    for(size_t i=0;i<l;i++){
        prt( "%02x ", s[i] & 0xff );
        if((i%8)==7) prt("  ");
        if((i%16)==15) prt("\n");
    }
    prt("\n");
}
void gsubString(char *s, char from, char to) {
    while(*s) {
        if(*s==from) *s=to;
        s++;
    }
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
int findCharIndexOf( const char *s, char ch ) {
    for(int i=0;;i++){
        if( s[i] == ch ) return i;
    }
    return -1;
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


///////////

int memCompressSnappy( char *out, int outlen, char *in, int inlen ) {
    size_t maxsz = snappy_max_compressed_length(inlen);
    assertmsg( (size_t)outlen >= maxsz, "snappy requires buffer size:%d given:%d", maxsz, outlen );
    size_t osz = outlen;
    snappy_status ret = snappy_compress( in, inlen, out, &osz);
    if(ret == SNAPPY_OK ) return (int)osz; else assertmsg(false,"snappy_compress failed. outlen:%d inlen:%d ret:%d", outlen, inlen,ret );
    return 0;
}
int memDecompressSnappy( char *out, int outlen, char *in, int inlen ) {
    size_t osz = outlen;
    snappy_status ret = snappy_uncompress( in, inlen, out, &osz );
    if(ret == SNAPPY_OK ) return osz; else assertmsg(false,"snappy_uncompress failed: %d", ret );
    return 0;
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
    if( copylen >= (int)sizeof(buf) ) copylen = sizeof(buf)-1;
    strncpy( buf, s, copylen );
    buf[copylen] = '\0';
    return strtoul( buf, NULL, 10 );
}
unsigned long long strtoulllen( const char *s, int l ) {
    char buf[64];
    int copylen = l;
    if( copylen >= (int)sizeof(buf) ) copylen = sizeof(buf)-1;
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
int countChar(const char *s, int ch){    
    int cnt=0;
    for(int i=0;;i++){
        if(s[i]==0)break;
        if(s[i]==ch)cnt++;
    }
    return cnt;
}

void sleepMilliSec( int ms ) {
#if defined(__APPLE__) || defined(__linux__)
        // glfwSleep(to_sleep); not present in 3.0
    if(ms>0) usleep( ms * 1000 );
#endif
#if defined(WIN32)        
    if( ms > 0 ) Sleep( ms );
#endif
}

const char *platformCStringPath( const char *path ) { 
#if (TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE)
    NSBundle *b = [NSBundle mainBundle];
    NSString *dir = [b resourcePath];
    NSString *nspath = [ [ NSString alloc] initWithUTF8String:path ];
    NSArray *parts = [NSArray arrayWithObjects:
                      dir, nspath, (void *)nil]; // including dummy strings for example
    NSString *nsfullpath = [NSString pathWithComponents:parts];
    const char *cpath = [nsfullpath fileSystemRepresentation];
    //    print("iOS: CPATH:%s",cpath);
#else
    const char *cpath = path;
#endif
    return cpath;
}

#include "mt19937.h"

unsigned long long g_cumino_random_count=0;
unsigned long cumino_random() {
    g_cumino_random_count++;
    return genrand_int32();
}
void cumino_srandom(unsigned long seed) {
    init_genrand(seed);
}


static const unsigned int crc32tab[256] = { 
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};


unsigned int crc32(char *p, int len) {
	unsigned int crcinit = 0;
	unsigned int crc = 0;

	crc = crcinit ^ 0xFFFFFFFF;
	for (; len--; p++) {
		crc = ((crc >> 8) & 0x00FFFFFF) ^ crc32tab[(crc ^ (*p)) & 0xFF];
	}
	return crc ^ 0xFFFFFFFF;
}


#ifdef __APPLE__
int moyai_unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    int rv = remove(fpath);
    if (rv) perror(fpath);
    return rv;
}
int moyai_rm_rf(const char *path) {
    return nftw(path, moyai_unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}
#else
int silently_remove_directory(LPCTSTR dir) // Fully qualified name of the directory being deleted, without trailing backslash
{
	SHFILEOPSTRUCT file_op = {
		NULL,
		FO_DELETE,
		dir,
		L"",
		FOF_NOCONFIRMATION |
		FOF_NOERRORUI |
		FOF_SILENT,
		false,
		0,
		L"" };
	int re=SHFileOperation(&file_op);
	print("shfileop: ret:%d %d", re, file_op.fAnyOperationsAborted);
	OutputDebugString(dir);
    return re;
}

#endif

bool removeDirectory(const char *path) {
	print("removedirectory:%s", path);
#ifdef __APPLE__
    int ret=moyai_rm_rf(path);
    return ret==0;
#else
	wchar_t hoge[1024];
	memset(hoge, 0, sizeof(hoge)); // double-null needed
	mbstowcs(hoge, path, strlen(path));
	int ret= silently_remove_directory(hoge);
    return ret==0;
#endif        
}
