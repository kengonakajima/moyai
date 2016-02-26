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
// HMP: Headless Moyai Protocol
class HMPListener : public Listener {
public:
    HMPListener(Network *nw) : Listener(nw) {};
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
    virtual void onFunction( int funcid, char *argdata, size_t argdatalen );    
};

class RemoteHead {
public:
    int tcp_port;
    Network *nw;
    HMPListener *listener;
    static const int DEFAULT_PORT = 22222;
    RemoteHead() : tcp_port(0), nw(0), listener(0) {
    }
    void track2D( Moyai *m );
    bool startServer( int portnum );
    void heartbeat() { nw->heartbeat(); }
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


typedef enum {
    // 汎用のping
    PACKETTYPE_PING = 1,
    // クライアイントからレプリケータ(Cli to Repr)へのコマンド. Player IDが添付されていない。
    PACKETTYPE_C2R_KEYBOARD_DOWN = 100,
    PACKETTYPE_C2R_KEYBOARD_UP = 101,    
    PACKETTYPE_C2R_MOUSE_DOWN = 102,
    PACKETTYPE_C2R_MOUSE_UP = 103,    
    PACKETTYPE_C2R_TOUCH_BEGIN = 104,
    PACKETTYPE_C2R_TOUCH_MOVE = 105,
    PACKETTYPE_C2R_TOUCH_END = 106,
    PACKETTYPE_C2R_TOUCH_CANCEL = 107,
    // レプリケータからゲームサーバ(Repr to Server)へのコマンド. Player IDが添付されている。
    PACKETTYPE_R2S_KEYBOARD_DOWN = 200, 
    PACKETTYPE_R2S_KEYBOARD_UP = 201,
    PACKETTYPE_R2S_NEW_CONNECTION = 260, // クライアントからの新しい接続があった。
    PACKETTYPE_R2S_LOST_CONNETION = 261, // クライアントからの接続が切れた。
    PACKETTYPE_R2S_QUERY_PRELOADS = 270, // ゲーム起動時に準備が必要なもの(画像、音、Deckなど)すべてを要求する
    
    // ゲームサーバーからレプリケータへのコマンド
    PACKETTYPE_S2R_NEW_PLAYER = 300, // 新しいプレイヤーを作成してIDを発行した。
    
    // ゲームサーバーからレプリケータ、レプリケータからクライアントへのコマンド
    PACKETTYPE_S2R_PROP2D_CREATE_SNAPSHOT = 400, // Prop2Dを生成するために必要な情報を一度に送る
    PACKETTYPE_S2R_PROP2D_LOC = 401,
    PACKETTYPE_S2R_PROP2D_GRID = 402,
    PACKETTYPE_S2R_PROP2D_INDEX = 403,
    PACKETTYPE_S2R_PROP2D_SCALE = 404,
    PACKETTYPE_S2R_PROP2D_ROT = 405,
    PACKETTYPE_S2R_PROP2D_XFLIP = 406,
    PACKETTYPE_S2R_PROP2D_YFLIP = 407,
    PACKETTYPE_S2R_PROP2D_COLOR = 408,
    PACKETTYPE_S2R_PROP2D_DELETE = 410,
    
    PACKETTYPE_S2R_LAYER_CREATE = 420,
    PACKETTYPE_S2R_LAYER_VIEWPORT = 421,
    PACKETTYPE_S2R_LAYER_CAMERA = 422,
    PACKETTYPE_S2R_VIEWPORT_CREATE = 430,
    PACKETTYPE_S2R_VIEWPORT_SIZE = 431,
    PACKETTYPE_S2R_VIEWPORT_SCALE = 432,    
    PACKETTYPE_S2R_CAMERA_CREATE = 440,
    PACKETTYPE_S2R_CAMERA_LOC = 441,
    
    PACKETTYPE_S2R_TEXTURE_CREATE = 450,
    PACKETTYPE_S2R_TEXTURE_IMAGE = 451,
    PACKETTYPE_S2R_IMAGE_CREATE = 460,
    PACKETTYPE_S2R_IMAGE_LOAD_PNG = 461,
    
    PACKETTYPE_S2R_TILEDECK_CREATE = 470,
    PACKETTYPE_S2R_TILEDECK_TEXTURE = 471,
    PACKETTYPE_S2R_TILEDECK_SIZE = 472,
    PACKETTYPE_S2R_GRID_CREATE_SNAPSHOT = 480, // Gridの情報を一度に1種類送る
    PACKETTYPE_S2R_GRID_TABLE_SNAPSHOT = 481, // Gridの水平移動各種テーブル
    PACKETTYPE_S2R_GRID_INDEX = 482, // indexが変化した。
    PACKETTYPE_S2R_FILE = 490, // ファイルを直接送信する step 1: ファイルを作成してIDを割りつける。

    // レプリケータからクライアントへのコマンド. 200足してみる
    PACKETTYPE_R2C_PROP2D_CREATE_SNAPSHOT = 600,
    PACKETTYPE_R2C_PROP2D_LOC = 601,
    PACKETTYPE_R2C_PROP2D_GRID = 602,
    PACKETTYPE_R2C_PROP2D_INDEX = 603,
    PACKETTYPE_R2C_PROP2D_SCALE = 604,
    PACKETTYPE_R2C_PROP2D_ROT = 605,
    PACKETTYPE_R2C_PROP2D_XFLIP = 606,
    PACKETTYPE_R2C_PROP2D_YFLIP = 607,
    PACKETTYPE_R2C_PROP2D_COLOR = 608,
    PACKETTYPE_R2C_PROP2D_DELETE = 610,

    PACKETTYPE_R2C_LAYER_CREATE = 620,
    PACKETTYPE_R2C_LAYER_VIEWPORT = 621,
    PACKETTYPE_R2C_LAYER_CAMERA = 622,
    PACKETTYPE_R2C_VIEWPORT_CREATE = 630,
    PACKETTYPE_R2C_VIEWPORT_SIZE = 631,
    PACKETTYPE_R2C_VIEWPORT_SCALE = 632,    
    PACKETTYPE_R2C_CAMERA_CREATE = 640,
    PACKETTYPE_R2C_CAMERA_LOC = 641,

    PACKETTYPE_R2C_TEXTURE_CREATE = 650,
    PACKETTYPE_R2C_TEXTURE_IMAGE = 651,
    PACKETTYPE_R2C_IMAGE_CREATE = 660,
    PACKETTYPE_R2C_IMAGE_LOAD_PNG = 661,
    
    PACKETTYPE_R2C_TILEDECK_CREATE = 670,
    PACKETTYPE_R2C_TILEDECK_TEXTURE = 671,
    PACKETTYPE_R2C_TILEDECK_SIZE = 672,
    PACKETTYPE_R2C_GRID_CREATE_SNAPSHOT = 680, // Gridの情報を一度に1種類送る
    PACKETTYPE_R2C_GRID_TABLE_SNAPSHOT = 681, // Gridの水平移動各種テーブル
    PACKETTYPE_R2C_GRID_INDEX = 682, // indexが変化した。
    PACKETTYPE_R2C_FILE = 690, // ファイルを直接送信する step 1: ファイルを作成してIDを割りつける。
    
    
    PACKETTYPE_ERROR = 2000, // 何らかのエラー。エラー番号を返す
} PACKETTYPE;



// send funcs
int sendPacketNoarg( unsigned short pkttype );
int sendPacketI1( unsigned short pkttype, int iarg0 );
int sendPacketI2( unsigned short pkttype, int iarg0, int arg1 );
int sendPacketI3( unsigned short pkttype, int iarg0, int arg1, int arg2 );
int sendPacketI4( unsigned short pkttype, int iarg0, int arg1, int arg2, int arg3 );
int sendPacketI5( unsigned short pkttype, int i0, int i1, int i2, int i3, int i4 );
int sendPacketInts( unsigned short pkttype, int *iargs, int argn );
int sendPacketF2( unsigned short pkttype, float f0, float f1 );
int sendPacketI1F1( unsigned short pkttype, unsigned int i0, float f0 );
int sendPacketI1F2( unsigned short pkttype, unsigned int i0, float f0, float f1 );
int sendPacketI2F2( unsigned short pkttype, unsigned int i0, unsigned int i1, float f0, float f1 );
int sendPacketI1Floats(unsigned short pkttype, unsigned int i0, float *fargs, int argn );
int sendPacketBytes( unsigned short pkttype, char *buf, size_t buflen );
int sendPacketStrBytes(unsigned short pkttype, const char *cstr, const char *buf, unsigned short datalen );
int sendPacketI1Str( unsigned short pkttype, int iarg0, const char *cstr );
int sendPacketI1Bytes(unsigned short pkttype, int iarg0, const char *buf, unsigned short datalen );
int sendPacketI2Bytes(unsigned short pkttype, int iarg0, int iarg1, const char *buf, unsigned short datalen );
int sendPacketI3Bytes(unsigned short pkttype, int iarg0, int iarg1, int iarg2, const char *buf, unsigned short datalen );
int sendPacketI4Bytes(unsigned short pkttype, int iarg0, int iarg1, int iarg2, int iarg3, const char *buf, unsigned short datalen );


#endif
