#ifndef _MOYNET_H_
#define _MOYNET_H_

#include <ev.h>

typedef struct _moynet_t {
    bool syscall_log;
} moynet_t;

typedef struct _buffer_t {
    char *buf;
    size_t size;
    size_t used;
} buffer_t;

#define CONN_SENDBUF_SIZE (1024*1024)
#define CONN_RECVBUF_SIZE (1024*1024)

typedef enum {
    MOYNET_ERROR_WRITE = -10,
    MOYNET_ERROR_READ = -20,    
    MOYNET_ERROR_CONNECT = -30,
    MOYNET_ERROR_FORMAT = -40,    
} MOYNET_ERROR;

typedef struct _conn_t {
    int id;
    int fd;
    buffer_t sendbuf, recvbuf;
    bool connecting;
    void *userptr;
    
    struct ev_io *read_watcher, *write_watcher;
    struct _listener_t *parent_listener;
    struct _moynet_t *parent_moynet;

    void (*on_error)( struct _conn_t *c, MOYNET_ERROR e, int eno );
    void (*on_close)( struct _conn_t *c );
    void (*on_connect)( struct _conn_t *c );
    void (*on_function)( _conn_t *conn, int funcid, char *argdata, size_t argdatalen );
} conn_t;

#define MOYNET_LISTENER_MAX_CONN 100
typedef struct _listener_t {
    int fd;
    struct ev_io *accept_watcher;    
    moynet_t *parent_moynet;
    void (*on_accept)( struct _listener_t *l, conn_t *newcon );
    void (*on_function)( conn_t *conn, int funcid, char *argdata, size_t argdatalen );
} listener_t;

void moynet_global_init();
moynet_t *moynet_create();
void moynet_finalize( moynet_t *h );

listener_t *moynet_create_listener( moynet_t *h, const char *addr, int portnum, void (*acb)(listener_t*,conn_t*), void (*funccb)(conn_t *,int funcid, char *data, size_t datalen ) );
conn_t *moynet_connect( moynet_t *h, const char *host, int portnum, void (*ccb)(conn_t*), void (*funccb)(conn_t *, int funcid, char *data, size_t datalen) );


void moynet_init( moynet_t *h );
void moynet_heartbeat( moynet_t *h );
void moynet_heartbeat_with_timeout_us( moynet_t *h, int timeout_us );

bool conn_push( conn_t *c, const char *data, size_t datasz );
size_t conn_get_sendbuf_room( conn_t *c );

void conn_finalize( conn_t *c );
bool buffer_push_with_num32( buffer_t *b, const char *data, size_t datasz );
bool buffer_push( buffer_t *b, const char *data, size_t datasz );
bool buffer_push_u32( buffer_t *b, unsigned int val );
bool buffer_push_u16( buffer_t *b, unsigned short val );
bool buffer_push_u8( buffer_t *b, unsigned char val );
        
extern inline unsigned int get_u32(const char *buf){ return *((unsigned int*)(buf)); }
extern inline unsigned short get_u16(const char *buf){ return *((unsigned short*)(buf)); }
extern inline unsigned char get_u8(const char *buf){ return *((unsigned char*)(buf)); }
extern inline void set_u32(char *buf, unsigned int v){ (*((unsigned int*)(buf))) = (unsigned int)(v) ; }
extern inline void set_u16(char *buf, unsigned short v){ (*((unsigned short*)(buf))) = (unsigned short)(v); }
extern inline void set_u8( char *buf, unsigned char v){ (*((unsigned char*)(buf))) = (unsigned char)(v); }
extern inline float get_f32(const char *buf) { return *((float*)(buf)); }

extern long long g_moynet_total_sent_bytes;
extern long long g_moynet_total_received_bytes;


extern struct ev_loop *g_moynet_evloop;

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

typedef enum {
    ET_NO_BACKEND = 5000, // バックエンドがない。 REPR>CLI 
} ERRORTYPE;

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
    unsigned int prop_id; // non-zero
    unsigned int layer_id; // non-zero
    PacketVec2 loc;
    PacketVec2 scl;
    int index;
    unsigned int tiledeck_id; // non-zero
    unsigned int grid_id; // 0 for nothing
    int debug;
    float rot;
    unsigned int xflip; // TODO:smaller size
    unsigned int yflip;
    PacketColor color;
} PacketProp2DCreateSnapshot;

typedef enum {
    GRIDTABLETYPE_INDEX = 0,
    GRIDTABLETYPE_XFLIP = 1,
    GRIDTABLETYPE_YFLIP = 2,
    GRIDTABLETYPE_TEXOFS = 3, // UV offset of each texture. (2floats per 1 element)
    GRIDTABLETYPE_ROT = 4, // 0/1
    GRIDTABLETYPE_COLOR = 5, // 4floats per 1 element    
} GRIDTABLETYPE;

typedef struct {
    unsigned int id;
    unsigned short width;
    unsigned short height;
    unsigned int tiledeck_id;
    // unsigned int fragmentshader_id;
    float enfat_epsilon;
} PacketGridCreateSnapshot;


// send funcs

int send_packet_noarg( conn_t *c, unsigned short pkttype );
int send_packet_i1( conn_t *c, unsigned short pkttype, int iarg0 );
int send_packet_i2( conn_t *c, unsigned short pkttype, int iarg0, int arg1 );
int send_packet_i3( conn_t *c, unsigned short pkttype, int iarg0, int arg1, int arg2 );
int send_packet_i4( conn_t *c, unsigned short pkttype, int iarg0, int arg1, int arg2, int arg3 );
int send_packet_i5( conn_t *c, unsigned short pkttype, int i0, int i1, int i2, int i3, int i4 );
int send_packet_ints( conn_t *c, unsigned short pkttype, int *iargs, int argn );
int send_packet_f2( conn_t *c, unsigned short pkttype, float f0, float f1 );
int send_packet_i1_f1( conn_t *c, unsigned short pkttype, unsigned int i0, float f0 );
int send_packet_i1_f2( conn_t *c, unsigned short pkttype, unsigned int i0, float f0, float f1 );
int send_packet_i2_f2( conn_t *c, unsigned short pkttype, unsigned int i0, unsigned int i1, float f0, float f1 );
int send_packet_i1_floats( conn_t *c, unsigned short pkttype, unsigned int i0, float *fargs, int argn );
int send_packet_bytes( conn_t *c, unsigned short pkttype, char *buf, size_t buflen );
int send_packet_str_bytes( conn_t *c, unsigned short pkttype, const char *cstr, const char *buf, unsigned short datalen );
int send_packet_i1_str( conn_t *c, unsigned short pkttype, int iarg0, const char *cstr );
int send_packet_i1_bytes( conn_t *c, unsigned short pkttype, int iarg0, const char *buf, unsigned short datalen );
int send_packet_i2_bytes( conn_t *c, unsigned short pkttype, int iarg0, int iarg1, const char *buf, unsigned short datalen );
int send_packet_i3_bytes( conn_t *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2, const char *buf, unsigned short datalen );
int send_packet_i4_bytes( conn_t *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2, int iarg3, const char *buf, unsigned short datalen );

void parse_packet_str_bytes( char *inptr, char *outcstr, char **outptr, size_t *outsize );


// callback func pointers
extern void (*moynet_broadcaster_intargs)( PACKETTYPE pkttype, int *iargs, int argn );



#endif
