// Replayer: Read saved stream file and send it to viewer via TCP.

#include "client.h"

bool g_serve_single_client = false;

static const size_t MAX_INPUT_FILE_SIZE = 128 * 1024 * 1024;

Buffer g_inbuf;

class ReplayClient {
public:
    static unsigned int idgen;
    unsigned int id;
    double accum_time;
    Buffer *stream;
    size_t ofs;
    uv_tcp_t *tcp;
    double first_stream_ts; // Timestamp of the first PACKETTYPE_TIMESTAMP
    double first_stream_accum_time;
    double latest_stream_ts; // Timestamp of next PACKETTYPE_TIMESTAMP
    bool done;
    ReplayClient( Buffer *s, uv_tcp_t *tcp ) : id(idgen++), accum_time(0), stream(s), ofs(0), tcp(tcp), first_stream_ts(0), first_stream_accum_time(0), latest_stream_ts(0), done(false) {
    }
    void poll(double dt) {
        accum_time += dt;
        if( ofs >= stream->used ) done=true;
        
        if(done)return;
        
        uint32_t record_len = get_u32(stream->buf+ofs);
        uint16_t funcid = get_u16(stream->buf+ofs+4);
        //        print("Record found at %d: len:%d funcid:%d", ofs, record_len, funcid );
        
        if(funcid == PACKETTYPE_TIMESTAMP) {
            uint32_t sec = get_u32(stream->buf+ofs+4+2);
            uint32_t usec = get_u32(stream->buf+ofs+4+2+4);
            double ts = sec + (double)(usec)/1000000.0f;
            if( first_stream_ts == 0 ) {
                //                print("first timestamp found: %f", ts );
                first_stream_ts = ts;
                first_stream_accum_time = accum_time;
            }
            latest_stream_ts = ts;
            ofs += 4 + 2+4+4; // recordlen + funcid + sec + usec
            sendUS1UI2( (uv_stream_t*)tcp, funcid, sec,usec);
        } else {
            if( first_stream_ts == 0 ) {
                //                print("stream data before the first ts. funcid:%d",funcid);
                sendUS1RawArgs( (uv_stream_t*)tcp, funcid, stream->buf + ofs + 4+ 2, record_len - 2 ); // don't include record_len and funcid
                ofs += 4 + record_len;
            } else {
                double elt = accum_time - first_stream_accum_time;
                double stream_elt = latest_stream_ts - first_stream_ts;
                if( elt < stream_elt ) {
                    //                    print("wait for %f second..", stream_elt - elt );
                } else {
                    //                    print("sending stream data after the first ts. funcid:%d elt:%f st_elt:%f argdatalen:%d",funcid, elt, stream_elt, record_len-2 );
                    sendUS1RawArgs( (uv_stream_t*)tcp, funcid, stream->buf + ofs + 4 + 2, record_len - 2 );
                    ofs += 4 + record_len;
                }
            }
        }
    }
};

unsigned int ReplayClient::idgen = 1;

ObjectPool<ReplayClient> g_clients;

void on_close_callback( uv_handle_t *s ) {
    ReplayClient *cl = (ReplayClient*)s->data;
    print("on_close_callback clid:%d",cl->id);
    g_clients.del(cl->id);
    delete cl;
    if( g_serve_single_client ) {
        print("g_serve_single_client is true, quitting.");
        exit(0);
    }
}

void on_read_callback( uv_stream_t *s, ssize_t nread, const uv_buf_t *inbuf ) {
    ReplayClient *cl = (ReplayClient*) s->data;
    if(nread>0){
        // nothing to do
    } else if( nread <= 0 ) {
        print("on_read_callback EOF. clid:%d", cl->id );
        uv_close( (uv_handle_t*)s, on_close_callback );
    }
}

    
void on_accept_callback( uv_stream_t *listener, int status ) {
    uv_tcp_t *newsock = (uv_tcp_t*) MALLOC( sizeof(uv_tcp_t));
    uv_tcp_init( uv_default_loop(), newsock );
    if( uv_accept( listener, (uv_stream_t*) newsock ) == 0 ) {
        ReplayClient *cl = new ReplayClient( &g_inbuf, newsock );
        newsock->data = cl;
        int r = uv_read_start( (uv_stream_t*) newsock, moyai_libuv_alloc_buffer, on_read_callback );
        if(r) {
            print("uv_read_start: fail. ret:%d", r );
        }
        g_clients.set(cl->id,cl);
        print("on_accept_callback client id:%d",cl->id);
    }
}

void pollReplClients(double dt) {
    for( std::unordered_map<unsigned int,ReplayClient*>::iterator it = g_clients.idmap.begin(); it != g_clients.idmap.end(); ++it ) {
        ReplayClient *repcl = it->second;
        repcl->poll(dt);
    }
}

bool validateStream( const char *buf, size_t l, bool dump ) {
    size_t ofs = 0;
    while(true) {
        uint32_t reclen = get_u32(buf+ofs);
        if(dump) {
            uint32_t funcid = get_u16(buf+ofs+4);
            print("@%08x Func:%d arglen:%d", ofs, funcid, reclen-2);
        }
        ofs += 4+reclen;        
        if(ofs==l) break;
    }
    return true;
}

int main( int argc, char **argv ) {
    if( argc < 2 ) {
        print("Usage: replayer FILENAME [--once]");
        return 1;    
    }

    if(argc > 2 && strcmp( argv[2], "--once" ) == 0 ) {
        g_serve_single_client = true;
    }
    
    char *path = argv[1];

    // setup stream
    g_inbuf.ensureMemory(MAX_INPUT_FILE_SIZE);
    size_t readsz = g_inbuf.size;
    bool readret = readFile( path, g_inbuf.buf, &readsz );
    if(!readret) {
        print("Can't read data from %s", path );
        return 1;
    }
    g_inbuf.used = readsz;    
    print("Read %d bytes from %s", readsz, path );
    if(!validateStream(g_inbuf.buf,g_inbuf.used, true) ) {
        print("invalid stream format!");
        return 1;
    }

    Moyai::globalInitNetwork();
    
    uv_tcp_t listener;    
    int r = uv_tcp_init( uv_default_loop(), &listener );
    if(r) {
        print("uv_tcp_init failed" );
        return 1;
    }
    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 22222, &addr );
    r = uv_tcp_bind( &listener, (const struct sockaddr*)&addr, SO_REUSEADDR );
    if(r) {
        print("uv_tcp_bind failed");
        return 1;
    }
    r = uv_listen( (uv_stream_t*)&listener, 10, on_accept_callback );
    if(r) {
        print("uv_tcp_listen failed");
        return 1;
    }

    print("start listening");

    bool done = false;
    while(!done) {
        static double last_poll_at = now();
        double t = now();
        double dt = t - last_poll_at;
        uv_run_times(100);
        pollReplClients(dt);
        last_poll_at = t;
    }
}    
 
