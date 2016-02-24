#ifndef _REMOTE_H_
#define _REMOTE_H_

// packet structures
typedef struct {
    float x,y;
} PacketVec2;
typedef struct {
    float x,y,z;
} PacketVec3;
typedef struct {
    float r,g,b,a;
} PacketColor;
typedef struct  {
    uint32_t prop_id; // non-zero
    uint32_t layer_id; // non-zero
    PacketVec2 loc;
    PacketVec2 scl;
    int32_t index;
    uint32_t tiledeck_id; // non-zero
    uint32_t grid_id; // 0 for nothing
    int32_t debug;
    float rot;
    uint32_t xflip; // TODO:smaller size
    unsigned int yflip;
    PacketColor color;
} PacketProp2DSnapshot;


///////

class RemoteHead {
public:
    int tcp_port;    
    RemoteHead() : tcp_port(0) {
    }
    void track2D( Moyai *m );
    bool startServer( int portnum );
};


class Prop2D;
class Tracker2D {
public:
    PacketProp2DSnapshot pktbuf[2]; // flip-flop    
    int cur_buffer_index;
    Tracker2D() : cur_buffer_index(0) {
        memset( pktbuf, 0, sizeof(pktbuf) );
    }
    void scanProp2D( Prop2D *);
    void flipCurrentBuffer();
    size_t getDiffPacket( char *outpktbuf, size_t maxoutsize );
    size_t getCurrentPacket( char *outpktbuf, size_t maxoutsize );
};


#endif
