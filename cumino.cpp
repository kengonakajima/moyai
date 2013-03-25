/*
  lumino, rumino, jumino, cumino
 */

#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>

#include "cumino.h"


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
}

void print( const char *fmt, ... ){
    if(!g_enablePrint)return;    
    char dest[1024*16];
    va_list argptr;
    va_start( argptr, fmt );
    vsprintf( dest, fmt, argptr );
    va_end( argptr );
    fprintf( stderr, "%s\n", dest );
}

void assertmsg( bool cond, const char *fmt, ... ) {
    if(!cond){
        char dest[1024*16];
        va_list argptr;
        va_start( argptr, fmt );
        vsprintf( dest, fmt, argptr );
        va_end( argptr );
        fprintf( stderr, "%s\n", dest );
        assert(cond);
    }
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
            //            print("ins:%p out:%p",e,out);
        } else {
            //            print("newtop:%p out:%p",e,out);
        }
        g_cumino_memtops[mod] = e;
        e->sz = sz;
    } else {
        out = malloc(sz);
    }

    return out;
}
void FREE( void *ptr ) {
    char *envaddr = ((char*)ptr) - ENVSIZE;
    if( g_cumino_mem_debug ) {
        unsigned long long addr = (unsigned long long)envaddr;
        int mod = (addr/16) % elementof(g_cumino_memtops);
        MemEntry *tgt = (MemEntry*) envaddr;
        MemEntry *cur = g_cumino_memtops[mod];
        assert(cur);
        if(cur==tgt){
            //            print("found on top! %p given:%p", tgt, ptr );
            g_cumino_memtops[mod] = cur->next;
        } else {
            MemEntry *prev = NULL;
            while(cur) {
                if(cur == tgt ) {
                    //                    print("found in list! %p given:%p", cur, ptr );
                    assert(prev);
                    prev->next = cur->next;
                }
                prev = cur;
                cur = cur->next;
            }

        }
    }
    
    free(envaddr);
}
void *operator new(size_t sz) {
    return MALLOC(sz);
}
void operator delete(void*ptr) {
    FREE(ptr);
}

class MemStatEnt {
public:
    size_t sz;
    int count;
};

int cuminoPrintMemStat() {
    MemStatEnt ents[2048];
    memset( ents, 0, sizeof(ents) );
    for(int i=0;i<elementof(g_cumino_memtops);i++){
        MemEntry *cur = g_cumino_memtops[i];
        while(cur) {
            bool done=false;
            for(int i=0;i<elementof(ents);i++){
                if(ents[i].sz==cur->sz) {
                    ents[i].count++;
                    done=true;
                    break;
                } else if( ents[i].sz == 0 ) {
                    ents[i].count = 1;
                    ents[i].sz = cur->sz;
                    done=true;
                    break;
                }
            }
            if(done)break;
            cur = cur->next;
        }
    }
    int obj_count=0;
    for(int i=0;i<elementof(ents);i++){
        if(ents[i].sz==0)break;
        print("[%d] size:%d count:%d  total:%d", i, ents[i].sz, ents[i].count, ents[i].sz * ents[i].count );
        obj_count += ents[i].count;
    }
    return obj_count;
}
