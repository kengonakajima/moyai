#ifndef _NET_H_
#define _NET_H_

#include <ev.h>

#include "Pool.h"


typedef enum {
    NET_ERROR_WRITE = -10,
    NET_ERROR_READ = -20,    
    NET_ERROR_CONNECT = -30,
    NET_ERROR_FORMAT = -40,    
} NET_ERROR;


class Buffer {
public:
    char *buf;
    size_t size;
    size_t used;
    Buffer();
    ~Buffer();
    void ensureMemory( size_t sz );
    size_t getRoom() { return size - used; }
    bool shift( size_t toshift );    
    bool pushWithNum32( const char *data, size_t datasz );
    bool push( const char *data, size_t datasz );
    bool pushU32( unsigned int val );
    bool pushU16( unsigned short val );
    bool pushU8( unsigned char val );
            
};

extern inline unsigned int get_u32(const char *buf){ return *((unsigned int*)(buf)); }
extern inline unsigned short get_u16(const char *buf){ return *((unsigned short*)(buf)); }
extern inline unsigned char get_u8(const char *buf){ return *((unsigned char*)(buf)); }
extern inline void set_u32(char *buf, unsigned int v){ (*((unsigned int*)(buf))) = (unsigned int)(v) ; }
extern inline void set_u16(char *buf, unsigned short v){ (*((unsigned short*)(buf))) = (unsigned short)(v); }
extern inline void set_u8( char *buf, unsigned char v){ (*((unsigned char*)(buf))) = (unsigned char)(v); }
extern inline float get_f32(const char *buf) { return *((float*)(buf)); }

class Listener;
class Network;

class Conn {
public:
    int id;
    int fd;
    Buffer sendbuf, recvbuf;
    bool connecting;
    void *userptr;
    struct ev_io *read_watcher, *write_watcher;
    Listener *parent_listener;
    Network *parent_nw;
    static int idgen;

    virtual void onError( NET_ERROR e, int eno ) {};
    virtual void onClose() {}
    virtual void onConnect() {}
    virtual void onFunction( int funcid, char *argdata, size_t argdatalen ) {};
    
    static const int SENDBUF_SIZE = 1024*1024;
    static const int RECVBUF_SIZE = 1024*1024;
    
    Conn( Network *nw ); // for connect
    Conn( Network *nw, int fd ); // for listen
    virtual ~Conn();

    bool push( const char *data, size_t datasz );
    size_t getSendbufRoom();

    void notifyError( NET_ERROR e, int eno );
    bool connectToServer( const char *host, int portnum );
};


class Listener {
public:
    int fd;
    struct ev_io *accept_watcher;    
    Network *parent_nw;
    static const int MAXCONN = 100;
    ObjectPool<Conn> conn_pool;
    Listener(Network *nw) : fd(-1), accept_watcher(0), parent_nw(nw) {
    }
    bool startListen( const char *addr, int tcpport );
    virtual ~Listener() {};
    virtual void onAccept( int newfd ) {};
    void addConn(Conn*c);
};


class Network {
public:
    bool syscall_log;
    long long total_sent_bytes;
    long long total_recv_bytes;
    struct ev_loop *evloop;    
    Network() : syscall_log(false), total_sent_bytes(0), total_recv_bytes(0) {
        evloop = ev_default_loop(0);        
    }
    ~Network() {}
    static Network *create();


    void heartbeat();
    void heartbeatWithTimeoutMicroseconds( int timeout_us );
};




#endif
