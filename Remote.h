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


#define MAX_PACKET_SIZE (1024*8)

///////
// HMP: Headless Moyai Protocol
class RemoteHead;
class HMPListener : public Listener {
public:
    RemoteHead *remote_head;
    HMPListener(Network *nw, RemoteHead *rh) : Listener(nw), remote_head(rh) {};
    virtual ~HMPListener(){};
    virtual void onAccept( int newfd );    
};
class HMPConn : public Conn {
public:
    HMPConn( Network *nw, int newfd ) : Conn(nw,newfd) {};
    virtual ~HMPConn() {};
    virtual void onError( NET_ERROR e, int eno );
    virtual void onClose();
    virtual void onConnect();
    virtual void onPacket( uint16_t funcid, char *argdata, size_t argdatalen );

    // send
    void sendFile( const char *filename );
    
};
class Prop2D;
class RemoteHead {
public:
    int tcp_port;
    Network *nw;
    HMPListener *listener;
    MoyaiClient *target_moyai;
    static const int DEFAULT_PORT = 22222;
    RemoteHead( MoyaiClient *m ) : tcp_port(0), nw(0), listener(0), target_moyai(m) {
    }
    void track2D();
    bool startServer( int portnum, bool to_log_syscall = false );
    void heartbeat();
    void scanSendAllGraphicsPrerequisites( HMPConn *outco );
    void scanSendAllProp2DSnapshots( HMPConn *c );
    void notifyProp2DDeleted( Prop2D *prop_deleted );
    void notifyGridDeleted( Grid *grid_deleted );
};


typedef enum {
    // generic
    PACKETTYPE_PING = 1,    
    // client to server 
    PACKETTYPE_C2S_GET_ALL_PREREQUISITES = 100,    
    PACKETTYPE_C2S_KEYBOARD_DOWN = 200,
    PACKETTYPE_C2S_KEYBOARD_UP = 201,    
    PACKETTYPE_C2S_MOUSE_DOWN = 202,
    PACKETTYPE_C2S_MOUSE_UP = 203,    
    PACKETTYPE_C2S_TOUCH_BEGIN = 204,
    PACKETTYPE_C2S_TOUCH_MOVE = 205,
    PACKETTYPE_C2S_TOUCH_END = 206,
    PACKETTYPE_C2S_TOUCH_CANCEL = 207,
    
    // server to client
    PACKETTYPE_S2C_PROP2D_SNAPSHOT = 400, 
    PACKETTYPE_S2C_PROP2D_LOC = 401,
    PACKETTYPE_S2C_PROP2D_GRID = 402,
    PACKETTYPE_S2C_PROP2D_INDEX = 403,
    PACKETTYPE_S2C_PROP2D_SCALE = 404,
    PACKETTYPE_S2C_PROP2D_ROT = 405,
    PACKETTYPE_S2C_PROP2D_XFLIP = 406,
    PACKETTYPE_S2C_PROP2D_YFLIP = 407,
    PACKETTYPE_S2C_PROP2D_COLOR = 408,    
    PACKETTYPE_S2C_PROP2D_DELETE = 410,
    
    PACKETTYPE_S2C_LAYER_CREATE = 420,
    PACKETTYPE_S2C_LAYER_VIEWPORT = 421,
    PACKETTYPE_S2C_LAYER_CAMERA = 422,
    PACKETTYPE_S2C_VIEWPORT_CREATE = 430,
    PACKETTYPE_S2C_VIEWPORT_SIZE = 431,
    PACKETTYPE_S2C_VIEWPORT_SCALE = 432,    
    PACKETTYPE_S2C_CAMERA_CREATE = 440,
    PACKETTYPE_S2C_CAMERA_LOC = 441,
    
    PACKETTYPE_S2C_TEXTURE_CREATE = 450,
    PACKETTYPE_S2C_TEXTURE_IMAGE = 451,
    PACKETTYPE_S2C_IMAGE_CREATE = 460,
    PACKETTYPE_S2C_IMAGE_LOAD_PNG = 461,
    
    PACKETTYPE_S2C_TILEDECK_CREATE = 470,
    PACKETTYPE_S2C_TILEDECK_TEXTURE = 471,
    PACKETTYPE_S2C_TILEDECK_SIZE = 472,
    PACKETTYPE_S2C_GRID_CREATE = 480, // with its size (id,w,h)
    PACKETTYPE_S2C_GRID_DECK = 481, // with gid,tdid
    PACKETTYPE_S2C_GRID_PROP2D = 482, // with gid,propid    
    PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT = 484, // index table, array of int32_t
    PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT = 485, // color table, array of PacketColor: 4 * float32
    PACKETTYPE_S2C_GRID_DELETE = 490,

    PACKETTYPE_S2C_TEXTBOX_CREATE = 500, // tb_id, uint32_t
    PACKETTYPE_S2C_TEXTBOX_FONT = 501,    // tb_id, font_id
    PACKETTYPE_S2C_TEXTBOX_STRING = 502,    // tb_id, utf8str
    PACKETTYPE_S2C_TEXTBOX_LOC = 503,    // tb_id, x,y
    PACKETTYPE_S2C_TEXTBOX_COLOR = 505,    // tb_id, PacketColor
    PACKETTYPE_S2C_TEXTBOX_LAYER = 510,     // tb_id, l_id
    PACKETTYPE_S2C_FONT_CREATE = 540, // fontid, utf8 string array
    PACKETTYPE_S2C_FONT_CHARCODES = 541, // fontid, utf8str
    PACKETTYPE_S2C_FONT_LOADTTF = 542, // fontid, filepath    
    
    PACKETTYPE_S2C_FILE = 800, // send file body and path

    PACKETTYPE_ERROR = 2000, // error code
} PACKETTYPE;


class Prop2D;
class Tracker2D {
public:
    Prop2D *target_prop2d;
    PacketProp2DSnapshot pktbuf[2]; // flip-flop    
    int cur_buffer_index;
    RemoteHead *parent_rh;
    Tracker2D(RemoteHead *rh, Prop2D *target ) : target_prop2d(target), cur_buffer_index(0), parent_rh(rh) {
        memset( pktbuf, 0, sizeof(pktbuf) );
    }
    ~Tracker2D();
    void scanProp2D();
    void flipCurrentBuffer();
    bool checkDiff();
    void broadcastDiff( Listener *listener, bool force );
};
typedef enum {
    GTT_INDEX = 1,
    GTT_XFLIP = 2,
    GTT_YFLIP = 3,
    GTT_TEXOFS = 4,
    GTT_UVROT = 5,
    GTT_COLOR = 6,
} GRIDTABLETYPE;

class TrackerGrid {
public:
    Grid *target_grid;
    int32_t *index_table[2];
    PacketColor *color_table[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerGrid(RemoteHead *rh, Grid *target);
    ~TrackerGrid();
    void scanGrid();
    bool checkDiff(GRIDTABLETYPE gtt);
    void flipCurrentBuffer();
    void broadcastDiff( Prop2D *owner, Listener *listener, bool force );
    void broadcastGridConfs( Prop2D *owner, Listener *listener ); // util sendfunc
};

class TextBox;
class TrackerTextBox {
public:
    TextBox *target_tb;
    static const int MAX_STR_LEN = 1024;
    // flip flop diff checker
    uint8_t strbuf[2][MAX_STR_LEN];
    size_t str_bytes[2];
    PacketProp2DSnapshot pktbuf[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;    
    TrackerTextBox(RemoteHead *rh, TextBox *target);
    ~TrackerTextBox();
    void scanTextBox();
    void flipCurrentBuffer();
    bool checkDiff();    
    void broadcastDiff( Listener *listener, bool force );
};


#endif
