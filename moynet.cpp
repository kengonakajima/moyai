// Moyai network : moynet

#include "cumino.h"
#include "moynet.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>

struct ev_loop *g_moynet_evloop;


void moynet_global_init() {

#ifdef WIN32
    WSADATA data;
    WSAStartup(MAKEWORD(2,0), &data);
#endif
#ifndef WIN32
    signal( SIGPIPE, SIG_IGN );
#endif        

    g_moynet_evloop = ev_default_loop(0);        

}

void *MOYNET_MALLOC( size_t sz ) {
    fprintf(stderr, "MOYNET_MALLOC size: %lu\n", sz );
    void *ptr = malloc(sz);
    return ptr;
}

/////////////

void buffer_init( buffer_t *b, size_t sz ) {
    b->buf = (char*) MOYNET_MALLOC( sz);
    assert(b->buf);
    b->size = sz;
    b->used = 0;
}
void buffer_finalize( buffer_t *b ) {
    assert(b->buf);
    free(b->buf);
    b->size = b->used = 0;
}

static void write_callback( struct ev_loop *loop, struct ev_io *watcher, int revents );
static void read_callback( struct ev_loop *loop, struct ev_io *watcher, int revents );

static int g_conn_id_gen = 1;

void conn_init( conn_t *c, moynet_t *h, int fd ) {
    fprintf(stderr, "conn_init: pointer: %p\n", c );
    c->parent_moynet = h;
    c->parent_listener = NULL;
    c->fd = fd;
    c->id = g_conn_id_gen ++;
    c->connecting = false;
    buffer_init( & c->sendbuf, CONN_SENDBUF_SIZE );
    buffer_init( & c->recvbuf, CONN_RECVBUF_SIZE );
    
    c->write_watcher = (struct ev_io*) MOYNET_MALLOC( sizeof(struct ev_io) );
    ev_io_init( c->write_watcher, write_callback, fd, EV_WRITE );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    c->write_watcher->data  = c;
    
    c->read_watcher = (struct ev_io*) MOYNET_MALLOC( sizeof(struct ev_io) );    
    ev_io_init( c->read_watcher, read_callback, fd, EV_READ );
    ev_io_start( g_moynet_evloop, c->read_watcher );
    c->read_watcher->data = c;
    c->on_error = NULL;
    c->userptr = NULL;
}

void conn_finalize( conn_t *c ) {
    ev_io_stop( g_moynet_evloop, c->read_watcher );
    ev_io_stop( g_moynet_evloop, c->write_watcher );
    free( c->read_watcher );
    free( c->write_watcher );
    buffer_finalize( &c->sendbuf );
    buffer_finalize( &c->recvbuf );
    close( c->fd );
}
void conn_error_finalize( conn_t *c, MOYNET_ERROR he, int eno ) {
    if(c->on_error) c->on_error(c, he, eno);
    if(c->on_close) c->on_close(c);
    conn_finalize(c);
}


// ALL or NOTHING. never push part of the given data.
// return true if all data is pushed.
bool buffer_push( buffer_t *b, const char *data, size_t datasz ) {
    size_t left = b->size - b->used;
    if( left < datasz ) return false;
    memcpy( b->buf + b->used, data, datasz );
    b->used += datasz;
    //    fprintf(stderr, "buffer_push: pushed %d bytes, used: %d\n", (int)datasz, (int)b->used );
    return true;
}
bool buffer_push_with_num32( buffer_t *b, const char *data, size_t datasz ) {
    size_t left = b->size - b->used;
    if( left < 4 + datasz ) return false;
    set_u32( b->buf + b->used, datasz );
    b->used += 4;
    buffer_push( b, data, datasz );
    return true;
}
bool buffer_push_u32( buffer_t *b, unsigned int val ) {
    size_t left = b->size - b->used;
    if( left < 4 ) return false;
    set_u32( b->buf + b->used, val );
    b->used += 4;
    //    fprintf(stderr, "buffer_push_u32: pushed 4 bytes. val:%u\n",val );
    return true;
}
bool buffer_push_u16( buffer_t *b, unsigned short val ) {
    size_t left = b->size - b->used;
    if( left < 2 ) return false;
    set_u16( b->buf + b->used, val );
    b->used += 2;
    return true;
}
bool buffer_push_u8( buffer_t *b, unsigned char val ) {
    size_t left = b->size - b->used;
    if( left < 1 ) return false;
    set_u8( b->buf + b->used, val );
    b->used += 1;
    return true;
}

// ALL or NOTHING. true when success
bool buffer_shift( buffer_t *b, size_t toshift ) {
    if( b->used < toshift ) return false;
    if( toshift == b->used ) { // most cases
        b->used = 0;
        return true;
    }
    // 0000000000 size=10
    // uuuuu      used=5
    // ss         shift=2
    //   mmm      move=3
    memmove( b->buf, b->buf + toshift, b->used - toshift );
    b->used -= toshift;
    return true;
}

bool conn_push( conn_t *c, const char *data, size_t datasz ) {
    return buffer_push( &c->sendbuf, data, datasz );
}
size_t conn_get_sendbuf_room( conn_t *c ) {
    return ( c->sendbuf.size - c->sendbuf.used );
}

///////////////

// returns negative if error
static void write_callback( struct ev_loop *loop, struct ev_io *watcher, int revents ) {
    conn_t *c = (conn_t*) watcher->data;
    assert(c);
    //    fprintf(stderr, "write_callback. fd:%d connecting:%d\n", c->fd, c->connecting );
    if( c->connecting ) {
        if( c->on_connect ) c->on_connect(c);
        c->connecting = false;        
    }

    if( c->sendbuf.used == 0 ) {
        ev_io_stop( g_moynet_evloop, c->write_watcher );
        return;
    }
    
    ssize_t ss = send( watcher->fd, c->sendbuf.buf, c->sendbuf.used, MSG_DONTWAIT );
    if( c->parent_moynet->syscall_log ) fprintf(stderr, "send( %d, %p, %d, MSG_DONTWAIT ) => %d\n", watcher->fd, c->sendbuf.buf, (int)c->sendbuf.used, (int)ss );
    if(ss==-1) {
        if( c->on_error ) c->on_error( c, MOYNET_ERROR_WRITE, errno );
        if( c->on_close ) c->on_close( c );
        conn_finalize(c);
        free(c);
    } else {
        g_moynet_total_sent_bytes += ss;
        buffer_shift( & c->sendbuf, ss );
        if( c->sendbuf.used == 0 ) {
            ev_io_stop( g_moynet_evloop, c->write_watcher );
            //            fprintf(stderr, "stop write watcher on fd:%d\n", c->fd );
        }
    }
}
static bool is_would_block_error() {
#if WIN32
    return( GetLastError() == WSAEWOULDBLOCK );
#else
    return( errno == EWOULDBLOCK || errno == EINPROGRESS );
#endif
}
static bool is_any_error() {
#if WIN32    
    return( GetLastError() != 0 );
#else
    return( errno != 0 );
#endif    
}
static int get_error_number() {
#if WIN32    
    return GetLastError();
#else
    return errno;
#endif    

}

void moynet_dump( const char *s, size_t l ) {
    for(int i=0;i<l;i++) {
        fprintf(stderr, "%02x ", s[i] & 0xff );
        if( (i%16) == 15 ) fprintf(stderr,"\n");
    }
    fprintf(stderr,"\n");
}

static void read_callback( struct ev_loop *loop, struct ev_io *watcher, int revents ) {
    conn_t *c = (conn_t*) watcher->data;
    assert(c);
    //    fprintf(stderr, "read_callback. fd:%d\n", c->fd );
    size_t space = c->recvbuf.size - c->recvbuf.used;
    if( space == 0 ) return;

    ssize_t rsz = recv( watcher->fd, c->recvbuf.buf + c->recvbuf.used, space, 0 );
    if( c->parent_moynet->syscall_log ) fprintf(stderr, "recv( %d, %p, %d ) => %d\n", watcher->fd, c->recvbuf.buf+ c->recvbuf.used, (int)space, (int)rsz );
    if( rsz < 0 ) {
        if( is_would_block_error() ) {
            // again later!
        } else if( is_any_error() ) {
            conn_error_finalize( c, MOYNET_ERROR_READ, get_error_number() );
            free(c);
        }
    } else if( rsz == 0 ) {
        // EOF! closed by client.
        if( c->parent_moynet->syscall_log ) fprintf(stderr, "recv(fd:%d) returned 0.\n", watcher->fd );
        if(c->on_close) c->on_close(c);
        conn_finalize(c);
        free(c);
    } else {
        g_moynet_total_received_bytes += rsz;
        c->recvbuf.used += rsz;
        // Parse RPC
        //        fprintf(stderr, "recvbuf used:%zu\n", c->recvbuf.used );
        //        moynet_t *h = c->parent_moynet;
        while(true) { // process everything in one poll
            //            print("recvbuf:%d", c->recvbuf.used );
            if( c->recvbuf.used < (2+2) ) return; // need more data from network
            //              <---RECORDLEN------>
            // [RECORDLEN32][FUNCID32][..DATA..]            
            size_t record_len = get_u16( c->recvbuf.buf );
            unsigned int func_id = get_u16( c->recvbuf.buf + 2 );

            if( c->recvbuf.used < (2+record_len) ) {
                print("need. used:%d reclen:%d", c->recvbuf.used, record_len );
                return; // need more data from network
            }
            if( record_len < 2 ) {
                fprintf(stderr, "invalid packet format" );
                conn_error_finalize( c, MOYNET_ERROR_FORMAT, 0 );
                free(c);
                return;
            }
            //            fprintf(stderr, "dispatching func_id:%d record_len:%lu\n", func_id, record_len );
            //            dump( c->recvbuf.buf + 4+4, record_len-4);
            if( c->on_function ) {
                c->on_function( c, func_id, (char*) c->recvbuf.buf +2+2, record_len - 2 );
            }
            buffer_shift( & c->recvbuf, 2 + record_len );
            //            fprintf(stderr, "after dispatch recv func: buffer used: %zu\n", c->recvbuf.used );
            //            if( c->recvbuf.used > 0 ) dump( c->recvbuf.buf, c->recvbuf.used );
        }
    }
    
}

moynet_t *moynet_create() {
    moynet_t *h = (moynet_t*) MOYNET_MALLOC( sizeof(moynet_t) );
    moynet_init(h);
    return h;
}
void moynet_init( moynet_t *h ) {
    h->syscall_log = false;
}
static void accept_callback( struct ev_loop *loop, struct ev_io *watcher, int revents ) {
    fprintf(stderr,"accept callback\n");
    listener_t *l = (listener_t*) watcher->data;
    assert(l);
    moynet_t *h = l->parent_moynet;
    assert(h);
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    int new_fd = accept( l->fd, (struct sockaddr*) &addr, &addrlen );
    if( h->syscall_log ) fprintf( stderr, "accept( %d, %p, %d ) => %d\n", l->fd, &addr, addrlen, new_fd );

    if( new_fd != -1 ) {
#ifndef WIN32
        int flag = fcntl( new_fd, F_GETFL );
        if( flag < 0 ){
            fprintf(stderr, "socket getfl error\n" );
            close(new_fd);
            return;
        }
        if( fcntl( new_fd, F_SETFL, flag|O_NONBLOCK)<0){
            fprintf(stderr, "socket nonblock setfl error\n" ); 
            close(new_fd);
            return;
        }
#endif
        //
        conn_t *c = (conn_t*) MOYNET_MALLOC( sizeof(conn_t) );
        conn_init( c, h, new_fd );
        c->parent_listener = l;
        c->on_function = l->on_function;
    
        if(l->on_accept) l->on_accept( l, c );
    } else {
        // error
    }
}

listener_t *moynet_create_listener( moynet_t *h, const char *addr, int portnum, void (*acb)(listener_t*,conn_t*), void (*funccb)( conn_t *co, int funcid, char *argdata, size_t argdatalen ) ) {

    struct addrinfo hints, *res;
    memset(&hints,0,sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = PF_INET;
    hints.ai_flags = AI_PASSIVE;
    char pstr[32];
    snprintf(pstr,sizeof(pstr),"%d",portnum);
    getaddrinfo(addr, pstr, &hints, &res );
    int fd = socket( res->ai_family, res->ai_socktype, res->ai_protocol );

    int opt = 1;
    if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == -1 ){
        fprintf( stderr, "setsockopt(reuseaddr) error:%s", strerror(errno)  );
        close(fd); // setsockopt error
        return NULL;
    }

    struct linger lingeropt;
    lingeropt.l_onoff =  1;
    lingeropt.l_linger = 2;

    if( setsockopt( fd, SOL_SOCKET, SO_LINGER,  (const char*)&lingeropt, sizeof(lingeropt)) == -1 ){
        fprintf( stderr, "setsockopt(reuseaddr) error:%s", strerror(errno) );
        close(fd);
        return NULL;
    }
    
    if( bind( fd, res->ai_addr, res->ai_addrlen ) == -1 ){
        fprintf( stderr, "bind error");
        close(fd); 
        return NULL;
    }
    if( listen( fd, SOMAXCONN ) == -1 ){
        fprintf( stderr, "listen error");
        close(fd); 
        return NULL;
    }

#ifdef WIN32
    DWORD  dwNonBlocking = 1;
    ioctlsocket(fd, FIONBIO, &dwNonBlocking);
#endif

    freeaddrinfo(res);

    listener_t *l = (listener_t*)MOYNET_MALLOC( sizeof(listener_t));
    l->fd = fd;
    l->parent_moynet = h;
    l->on_accept = acb;
    l->on_function = funccb;
    l->accept_watcher = (struct ev_io*) MOYNET_MALLOC( sizeof(struct ev_io));
    memset( (void*) l->accept_watcher, 0, sizeof(struct ev_io));
    l->accept_watcher->data = l;    
    ev_io_init( l->accept_watcher, accept_callback, l->fd, EV_READ );
    ev_io_start( g_moynet_evloop, l->accept_watcher );
    return l;
}

conn_t *moynet_connect( moynet_t *h, const char *host, int portnum, void (*ccb)(conn_t*), void (*funccb)(conn_t *, int fid, char *dt, size_t dtlen ) ) {
    struct addrinfo hints, *res;
    memset( &hints, 0, sizeof(hints) );
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = PF_INET;
    char pstr[32];
    snprintf( pstr,sizeof(pstr),"%d", portnum );
    getaddrinfo( host, pstr, &hints, &res );
    if(!res){
        fprintf(stderr, "invalid host address or port? '%s':%d", host, portnum );
        return NULL;
    }
    int new_fd = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
    if( new_fd == -1 ) {
        fprintf(stderr, "socket() error. errno:%d\n",errno );
        return NULL;
    }

#ifndef WIN32
    int flag = fcntl( new_fd, F_GETFL );
    if( flag < 0 ){
        fprintf(stderr, "socket getfl error. errno:%s\n", strerror(errno) );
        close(new_fd);
        return NULL;
    }
    if( fcntl( new_fd, F_SETFL, flag|O_NONBLOCK)<0){
        fprintf(stderr, "socket nonblock setfl error. errno:%s\n", strerror(errno) );
        close(new_fd);
        return NULL;
    }
#endif
    

    if( connect( new_fd, res->ai_addr, res->ai_addrlen ) == -1 ) {
        if( is_would_block_error() == false ) {
            fprintf(stderr, "moynet_connect: connect() failed: errno:%d\n", errno );
            close( new_fd );
            return NULL;
        }
    }
#ifdef WIN32
    DWORD  dwNonBlocking = 1;
    ioctlsocket(new_fd, FIONBIO, &dwNonBlocking);
#endif        

    freeaddrinfo(res);

    conn_t *c = (conn_t*) MOYNET_MALLOC( sizeof(conn_t) );
    conn_init( c, h, new_fd );
    c->connecting = true;
    c->on_connect = ccb;
    c->on_function = funccb;
    return c;
}

void moynet_finalize( moynet_t *h ) {
    free(h);
}

void moynet_heartbeat( moynet_t *h ) {
    ev_loop( g_moynet_evloop, EVLOOP_NONBLOCK );
}



#ifdef WIN32
#define EPOCHFILETIME (116444736000000000i64)                                                     
                                                                                                  
static int gettimeofday(struct timeval *tv, struct timezone *tz) {                                       
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
        // OK in Visual C++ 6.0
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
#endif


void moynet_heartbeat_with_timeout_us( moynet_t *h, int timeout_us ) {
    if( timeout_us == 0 ) {
        moynet_heartbeat(h);        
    } else {
        double st = now();
        moynet_heartbeat(h);
        double et = now();
        double dt = et - st;
        double timeout = (double)(timeout_us) / 1000000.0f;
        if( dt < timeout ) {
            usleep( (timeout-dt) * 1000000 );
        }
    }
}

int send_packet_noarg( conn_t *c, unsigned short pkttype ) {
    if(!c)return 0;
    size_t totalsize = 2 + 2;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
}
int send_packet_i1( conn_t *c, unsigned short pkttype, int iarg0 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    assert( sizeof(int) == 4 );
    buffer_push( & c->sendbuf, (char*)&iarg0, 4 );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
}
int send_packet_i2( conn_t *c, unsigned short pkttype, int iarg0, int iarg1 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    assert( sizeof(int) == 4 );
    buffer_push( & c->sendbuf, (char*)&iarg0, 4 );
    buffer_push( & c->sendbuf, (char*)&iarg1, 4 );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
}
int send_packet_i3( conn_t *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    assert( sizeof(int) == 4 );
    buffer_push( & c->sendbuf, (char*)&iarg0, 4 );
    buffer_push( & c->sendbuf, (char*)&iarg1, 4 );
    buffer_push( & c->sendbuf, (char*)&iarg2, 4 );    
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
}
int send_packet_i4( conn_t *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2, int iarg3 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    assert( sizeof(int) == 4 );
    buffer_push( & c->sendbuf, (char*)&iarg0, 4 );
    buffer_push( & c->sendbuf, (char*)&iarg1, 4 );
    buffer_push( & c->sendbuf, (char*)&iarg2, 4 );
    buffer_push( & c->sendbuf, (char*)&iarg3, 4 );        
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
}
int send_packet_i5( conn_t *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2, int iarg3, int iarg4 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4 + 4 + 4;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    assert( sizeof(int) == 4 );
    buffer_push( & c->sendbuf, (char*)&iarg0, 4 );
    buffer_push( & c->sendbuf, (char*)&iarg1, 4 );
    buffer_push( & c->sendbuf, (char*)&iarg2, 4 );
    buffer_push( & c->sendbuf, (char*)&iarg3, 4 );
    buffer_push( & c->sendbuf, (char*)&iarg4, 4 );            
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
}
int send_packet_ints( conn_t *c, unsigned short pkttype, int *iargs, int argn ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4*argn;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    assert( sizeof(int) == 4 );
    buffer_push( &c->sendbuf, (char*)iargs, argn * sizeof(int) );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
}

int send_packet_f2( conn_t *c, unsigned short pkttype, float f0, float f1 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    assert( sizeof(int) == 4 );
    buffer_push( & c->sendbuf, (char*)&f0, 4 );
    buffer_push( & c->sendbuf, (char*)&f1, 4 );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
}
int send_packet_i1_f1( conn_t *c, unsigned short pkttype, unsigned int i0, float f0 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    assert( sizeof(unsigned int) == 4 );
    buffer_push( & c->sendbuf, (char*)&i0, 4 );    
    buffer_push( & c->sendbuf, (char*)&f0, 4 );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
    
}
int send_packet_i1_f2( conn_t *c, unsigned short pkttype, unsigned int i0, float f0, float f1 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    assert( sizeof(unsigned int) == 4 );
    buffer_push( & c->sendbuf, (char*)&i0, 4 );    
    buffer_push( & c->sendbuf, (char*)&f0, 4 );
    buffer_push( & c->sendbuf, (char*)&f1, 4 );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
}
int send_packet_i2_f2( conn_t *c, unsigned short pkttype, unsigned int i0, unsigned int i1, float f0, float f1 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4 + 4;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    assert( sizeof(unsigned int) == 4 );
    buffer_push( & c->sendbuf, (char*)&i0, 4 );
    buffer_push( & c->sendbuf, (char*)&i1, 4 );        
    buffer_push( & c->sendbuf, (char*)&f0, 4 );
    buffer_push( & c->sendbuf, (char*)&f1, 4 );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;    
}
int send_packet_i1_floats( conn_t *c, unsigned short pkttype, unsigned int i0, float *fargs, int argn ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 * argn ;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype ); // packet type
    buffer_push( & c->sendbuf, (char*)&i0, 4 );
    assert( sizeof(float) == 4 );
    buffer_push( & c->sendbuf, (char*)fargs, argn * sizeof(float) );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;    
}

int send_packet_bytes( conn_t *c, unsigned short pkttype, char *buf, size_t buflen ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + buflen;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype );
    buffer_push( & c->sendbuf, buf, buflen );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
}
// [record-len:16][pkttype:16][cstr-len:8][cstr-body][data-len:16][data-body]
int send_packet_str_bytes( conn_t *c, unsigned short pkttype, const char *cstr, const char *data, unsigned short datalen ) {
    if(!c)return 0;    
    int cstrlen = strlen(cstr);
    assert( cstrlen <= 255 );
    size_t totalsize = 2 + 2 + 1 + cstrlen + 2 + datalen;
    assertmsg( totalsize <= 65535, "datalen too big? : %d", datalen );
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype );
    buffer_push_u8( &c->sendbuf, (unsigned char) cstrlen );
    buffer_push( & c->sendbuf, cstr, cstrlen );
    buffer_push_u16( & c->sendbuf, datalen );
    buffer_push( & c->sendbuf, data, datalen );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    //    print("send_packet_str_bytes: cstrlen:%d datalen:%d totallen:%d", cstrlen, datalen, totalsize );
    return totalsize;
}
void parse_packet_str_bytes( char *inptr, char *outcstr, char **outptr, size_t *outsize ) {
    unsigned char slen = get_u8(inptr);
    char *s = inptr + 1;
    unsigned short datalen = get_u16(inptr+1+slen);
    *outptr = inptr + 1 + slen + 2;
    memcpy( outcstr, s, slen );
    outcstr[slen]='\0';
    *outsize = (size_t) datalen;
}
// [record-len:16][pkttype:16][i0:32][cstr-len:8][cstr-body]
int send_packet_i1_str( conn_t *c, unsigned short pkttype, int i0, const char *cstr ) {
    if(!c)return 0;    
    int cstrlen = strlen(cstr);
    assert( cstrlen <= 255 );
    size_t totalsize = 2 + 2 + 4 + 1 + cstrlen;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype );
    buffer_push_u32( & c->sendbuf, i0 );
    buffer_push_u8( &c->sendbuf, (unsigned char) cstrlen );
    buffer_push( & c->sendbuf, cstr, cstrlen );
    ev_io_start( g_moynet_evloop, c->write_watcher );
    return totalsize;
}
int send_packet_i1_bytes( conn_t *c, unsigned short pkttype, int iarg0, const char *buf, unsigned short datalen ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + datalen;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype );
    buffer_push_u32( & c->sendbuf, iarg0 );
    buffer_push( & c->sendbuf, buf, datalen );
    return totalsize;
}
int send_packet_i2_bytes( conn_t *c, unsigned short pkttype, int iarg0, int iarg1, const char *buf, unsigned short datalen ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + datalen;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype );
    buffer_push_u32( & c->sendbuf, iarg0 );
    buffer_push_u32( & c->sendbuf, iarg1 );
    buffer_push( & c->sendbuf, buf, datalen );
    return totalsize;
}
int send_packet_i3_bytes( conn_t *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2, const char *buf, unsigned short datalen ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4 + datalen;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype );
    buffer_push_u32( & c->sendbuf, iarg0 );
    buffer_push_u32( & c->sendbuf, iarg1 );
    buffer_push_u32( & c->sendbuf, iarg2 );    
    buffer_push( & c->sendbuf, buf, datalen );
    return totalsize;
}
int send_packet_i4_bytes( conn_t *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2, int iarg3, const char *buf, unsigned short datalen ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4 + 4 + datalen;
    if( conn_get_sendbuf_room(c) < totalsize ) return 0;
    buffer_push_u16( & c->sendbuf, totalsize - 2 ); // record-len
    buffer_push_u16( & c->sendbuf, pkttype );
    buffer_push_u32( & c->sendbuf, iarg0 );
    buffer_push_u32( & c->sendbuf, iarg1 );
    buffer_push_u32( & c->sendbuf, iarg2 );
    buffer_push_u32( & c->sendbuf, iarg3 );    
    buffer_push( & c->sendbuf, buf, datalen );
    return totalsize;
}


///////////////////////
// OSXでは、gettimeofdayは速くて正確だが、usleepが極めて不正確で支障があるので
static void osxHighResSleep( int us ) {
    double sec = ((double)us) / 1000000.0f;
    double start_at = now();
    double end_at = start_at + sec;
    while(1) {
        double t = now();
        if( t >= end_at ) break;
        usleep(100);
    }
}
void highResSleep( double second ) {
#ifdef __APPLE__
    osxHighResSleep( second * 1000000 );
#endif
#ifdef __linux__
    usleep(second * 1000000 );
#endif
}
