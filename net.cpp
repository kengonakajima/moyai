// Moyai network : moynet

#include "cumino.h"
#include "common.h"
#include "net.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>

long long g_moynet_total_sent_bytes;
long long g_moynet_total_received_bytes;



void Moyai::globalInitNetwork() {
    static bool g_global_init_done = false;
    
    if( g_global_init_done ) return;
    
#ifdef WIN32
    WSADATA data;
    WSAStartup(MAKEWORD(2,0), &data);
#endif
#ifndef WIN32
    signal( SIGPIPE, SIG_IGN );
#endif        

}

void *NET_MALLOC( size_t sz ) {
    fprintf(stderr, "NET_MALLOC size: %lu\n", sz );
    void *ptr = malloc(sz);
    return ptr;
}

/////////////

Buffer::Buffer() : buf(0), size(0), used(0) {
}
void Buffer::ensureMemory( size_t sz ) {
    buf = (char*) NET_MALLOC(sz);
    assert(buf);
    size = sz;
    used = 0;    
}

Buffer::~Buffer() {
    assert(buf);
    free(buf);
    size = used = 0;
}


static void write_callback( struct ev_loop *loop, struct ev_io *watcher, int revents );
static void read_callback( struct ev_loop *loop, struct ev_io *watcher, int revents );


Conn::Conn( Network *nw, int id, int fd ) : id(id), fd(fd), connecting(false), userptr(0), read_watcher(0), write_watcher(0), parent_listener(0), parent_nw(nw) {
    fprintf(stderr, "Conn::Conn id:%d fd:%d",id,fd);

    sendbuf.ensureMemory( SENDBUF_SIZE );
    recvbuf.ensureMemory(  RECVBUF_SIZE );
    
    write_watcher = (struct ev_io*) NET_MALLOC( sizeof(struct ev_io) );
    ev_io_init( write_watcher, write_callback, fd, EV_WRITE );
    ev_io_start( nw->evloop, write_watcher );
    write_watcher->data  = this;
    
    read_watcher = (struct ev_io*) NET_MALLOC( sizeof(struct ev_io) );    
    ev_io_init( read_watcher, read_callback, fd, EV_READ );
    ev_io_start( nw->evloop, read_watcher );
    read_watcher->data = this;

}

Conn::~Conn() {
    ev_io_stop( parent_nw->evloop, read_watcher );
    ev_io_stop( parent_nw->evloop, write_watcher );
    free( read_watcher );
    free( write_watcher );
    ::close( fd );
}
void Conn::notifyError( NET_ERROR he, int eno ) {
    onError( he, eno);
    onClose();
}


// ALL or NOTHING. never push part of the given data.
// return true if all data is pushed.
bool Buffer::push( const char *data, size_t datasz ) {
    size_t left = size - used;
    if( left < datasz ) return false;
    memcpy( buf + used, data, datasz );
    used += datasz;
    //    fprintf(stderr, "buffer_push: pushed %d bytes, used: %d\n", (int)datasz, (int)b->used );
    return true;
}
bool Buffer::pushWithNum32( const char *data, size_t datasz ) {
    size_t left = size - used;
    if( left < 4 + datasz ) return false;
    set_u32( buf + used, datasz );
    used += 4;
    push( data, datasz );
    return true;
}
bool Buffer::pushU32( unsigned int val ) {
    size_t left = size - used;
    if( left < 4 ) return false;
    set_u32( buf + used, val );
    used += 4;
    //    fprintf(stderr, "buffer_push_u32: pushed 4 bytes. val:%u\n",val );
    return true;
}
bool Buffer::pushU16( unsigned short val ) {
    size_t left = size - used;
    if( left < 2 ) return false;
    set_u16( buf + used, val );
    used += 2;
    return true;
}
bool Buffer::pushU8( unsigned char val ) {
    size_t left = size - used;
    if( left < 1 ) return false;
    set_u8( buf + used, val );
    used += 1;
    return true;
}

// ALL or NOTHING. true when success
bool Buffer::shift( size_t toshift ) {
    if( used < toshift ) return false;
    if( toshift == used ) { // most cases
        used = 0;
        return true;
    }
    // 0000000000 size=10
    // uuuuu      used=5
    // ss         shift=2
    //   mmm      move=3
    memmove( buf, buf + toshift, used - toshift );
    used -= toshift;
    return true;
}

bool Conn::push( const char *data, size_t datasz ) {
    return sendbuf.push( data, datasz );
}
size_t Conn::getSendbufRoom() {
    return sendbuf.getRoom();
}


///////////////

// returns negative if error
static void write_callback( struct ev_loop *loop, struct ev_io *watcher, int revents ) {
    Conn *c = (Conn*) watcher->data;
    assert(c);
    //    fprintf(stderr, "write_callback. fd:%d connecting:%d\n", c->fd, c->connecting );
    if( c->connecting ) {
        c->onConnect();
        c->connecting = false;        
    }

    if( c->sendbuf.used == 0 ) {
        ev_io_stop( c->parent_nw->evloop, c->write_watcher );
        return;
    }
    
    ssize_t ss = send( watcher->fd, c->sendbuf.buf, c->sendbuf.used, MSG_DONTWAIT );
    if( c->parent_nw->syscall_log ) fprintf(stderr, "send( %d, %p, %d, MSG_DONTWAIT ) => %d\n", watcher->fd, c->sendbuf.buf, (int)c->sendbuf.used, (int)ss );
    if(ss==-1) {
        c->onError( NET_ERROR_WRITE, errno );
        c->onClose();
        delete c;
    } else {
        c->parent_nw->total_sent_bytes += ss;
        c->sendbuf.shift( ss );
        if( c->sendbuf.used == 0 ) {
            ev_io_stop( c->parent_nw->evloop, c->write_watcher );
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
    Conn *c = (Conn*) watcher->data;
    assert(c);
    //    fprintf(stderr, "read_callback. fd:%d\n", c->fd );
    size_t space = c->recvbuf.getRoom();
    if( space == 0 ) return;

    ssize_t rsz = recv( watcher->fd, c->recvbuf.buf + c->recvbuf.used, space, 0 );
    if( c->parent_nw->syscall_log ) fprintf(stderr, "recv( %d, %p, %d ) => %d\n", watcher->fd, c->recvbuf.buf+ c->recvbuf.used, (int)space, (int)rsz );
    if( rsz < 0 ) {
        if( is_would_block_error() ) {
            // again later!
        } else if( is_any_error() ) {
            c->onError( NET_ERROR_READ, get_error_number() );
            delete c;
        }
    } else if( rsz == 0 ) {
        // EOF! closed by client.
        if( c->parent_nw->syscall_log ) fprintf(stderr, "recv(fd:%d) returned 0.\n", watcher->fd );
        c->onClose();
        delete c;
    } else {
        c->parent_nw->total_recv_bytes += rsz;
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
                c->onError( NET_ERROR_FORMAT, 0 );
                delete c;
                return;
            }
            //            fprintf(stderr, "dispatching func_id:%d record_len:%lu\n", func_id, record_len );
            //            dump( c->recvbuf.buf + 4+4, record_len-4);
            c->onFunction( func_id, (char*) c->recvbuf.buf +2+2, record_len - 2 );
            c->recvbuf.shift( 2 + record_len );
            //            fprintf(stderr, "after dispatch recv func: buffer used: %zu\n", c->recvbuf.used );
            //            if( c->recvbuf.used > 0 ) dump( c->recvbuf.buf, c->recvbuf.used );
        }
    }
    
}

static void accept_callback( struct ev_loop *loop, struct ev_io *watcher, int revents ) {
    fprintf(stderr,"accept callback\n");
    Listener *l = (Listener*) watcher->data;
    assert(l);
    Network *nw = l->parent_nw;
    assert(nw);
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    int new_fd = accept( l->fd, (struct sockaddr*) &addr, &addrlen );
    if( nw->syscall_log ) fprintf( stderr, "accept( %d, %p, %d ) => %d\n", l->fd, &addr, addrlen, new_fd );

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
        Conn *c = new Conn(nw,nw->getNewConnId(),new_fd);
        c->parent_listener = l;
        l->onAccept(c);
    } else {
        // error
    }
}

Listener *Network::createListener( const char *addr, int portnum, void (*acb)(Listener*,Conn*), void (*funccb)( Conn *co, int funcid, char *argdata, size_t argdatalen ) ) {

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

    Listener *l = new Listener(this,fd);
    l->accept_watcher = (struct ev_io*) NET_MALLOC( sizeof(struct ev_io));
    memset( (void*) l->accept_watcher, 0, sizeof(struct ev_io));
    l->accept_watcher->data = l;    
    ev_io_init( l->accept_watcher, accept_callback, l->fd, EV_READ );
    ev_io_start( this->evloop, l->accept_watcher );
    return l;
}

Conn *Network::connectToServer( const char *host, int portnum ) {
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
    

    if( ::connect( new_fd, res->ai_addr, res->ai_addrlen ) == -1 ) {
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

    Conn *c = new Conn(this, getNewConnId(), new_fd );
    c->connecting = true;
    return c;
}



void Network::heartbeat() {
    ev_loop( this->evloop, EVLOOP_NONBLOCK );
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


void Network::heartbeatWithTimeoutMicroseconds( int timeout_us ) {
    if( timeout_us == 0 ) {
        heartbeat();
    } else {
        double st = now();
        heartbeat();
        double et = now();
        double dt = et - st;
        double timeout = (double)(timeout_us) / 1000000.0f;
        if( dt < timeout ) {
            usleep( (timeout-dt) * 1000000 );
        }
    }
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
