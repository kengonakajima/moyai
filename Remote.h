#ifndef _REMOTE_H_
#define _REMOTE_H_

#include "Pool.h"

// basic buffering

extern inline unsigned int get_u32(const char *buf){ return *((unsigned int*)(buf)); }
extern inline unsigned short get_u16(const char *buf){ return *((unsigned short*)(buf)); }
extern inline unsigned char get_u8(const char *buf){ return *((unsigned char*)(buf)); }
extern inline void set_u32(char *buf, unsigned int v){ (*((unsigned int*)(buf))) = (unsigned int)(v) ; }
extern inline void set_u16(char *buf, unsigned short v){ (*((unsigned short*)(buf))) = (unsigned short)(v); }
extern inline void set_u8( char *buf, unsigned char v){ (*((unsigned char*)(buf))) = (unsigned char)(v); }
extern inline float get_f32(const char *buf) { return *((float*)(buf)); }


// packet structures
typedef struct {
    float x,y;
} PacketVec2;
typedef struct {
    float x,y,z;
} PacketVec3;
typedef struct {
    uint8_t r,g,b,a;
} PacketColor;
typedef struct  {
    uint32_t prop_id; // non-zero
    uint32_t layer_id; // non-zero for layer, zero for child props
    uint32_t parent_prop_id; // non-zero for child props, zero for layer props
    PacketVec2 loc;
    PacketVec2 scl;
    int32_t index;
    uint32_t tiledeck_id; // non-zero
    int32_t debug;
    float rot;
    PacketColor color;
    uint32_t shader_id;
    uint32_t optbits;
    int32_t priority;
    uint8_t fliprotbits;
} PacketProp2DSnapshot;
class Prop2D;
void makePacketProp2DSnapshot( PacketProp2DSnapshot *out, Prop2D *tgt, Prop2D *parent );
inline uint8_t toFlipRotBits(bool xflip, bool yflip, bool uvrot) {
    uint8_t out=0;
    if(xflip) out|=0x1;
    if(yflip) out|=0x2;
    if(uvrot) out|=0x4;
    return out;
}
inline void fromFlipRotBits(uint8_t bits, bool *xfl, bool *yfl, bool *uvr ) {
    if(bits&0x01) *xfl=true; else *xfl=false;
    if(bits&0x02) *yfl=true; else *yfl=false;
    if(bits&0x04) *uvr=true; else *uvr=false;
}
inline bool getXFlipFromFlipRotBits(uint8_t bits) { return bits & 0x01; }
inline bool getYFlipFromFlipRotBits(uint8_t bits) { return bits & 0x02; }
inline bool getUVRotFromFlipRotBits(uint8_t bits) { return bits & 0x04; }

#define PROP2D_OPTBIT_ADDITIVE_BLEND 0x00000001

typedef struct {
    uint32_t shader_id;
    float epsilon;
    PacketColor from_color;
    PacketColor to_color;
} PacketColorReplacerShaderSnapshot;

inline void copyColorToPacketColor( PacketColor *dest, Color *src ) {
    src->toRGBA( &dest->r, &dest->g, &dest->b, &dest->a );
}
inline void copyPacketColorToColor( Color *dest, PacketColor *src ) {
    dest->fromRGBA( src->r, src->g, src->b, src->a );
}

typedef struct {
    uint32_t prim_id;
    uint32_t prim_type; // from PRIMTYPE
    PacketVec2 a;
    PacketVec2 b;
    PacketColor color;
    float line_width;
} PacketPrim;
class Prim;
void copyPrimToPacketPrim( PacketPrim*out, Prim *src );



///////

class Prop2D;
class MoyaiClient;
class Grid;
class ColorReplacerShader;
class PrimDrawer;
class SoundSystem;
class Keyboard;
class Mouse;
class Client;
class JPEGCoder;
class Sound;
class BufferArray;

typedef std::unordered_map<unsigned int,Client*>::iterator ClientIteratorType;

typedef enum {
    LOCSYNCMODE_DEFAULT = 0, // non-linear motion with location sync scoring , (changes velocity every poll at most)
    LOCSYNCMODE_LINEAR = 1, // velocity never changes after init
} LOCSYNCMODE;

typedef enum {
    // generic
    PACKETTYPE_PING = 1,
    PACKETTYPE_TIMESTAMP = 2,
    PACKETTYPE_ZIPPED_RECORDS = 8, // not used in server-reprecator connection.
    
    // client to server 
    PACKETTYPE_C2S_KEYBOARD = 100,
    PACKETTYPE_C2S_MOUSE_BUTTON = 102,
    PACKETTYPE_C2S_CURSOR_POS = 103,
    PACKETTYPE_C2S_TOUCH_BEGIN = 104,
    PACKETTYPE_C2S_TOUCH_MOVE = 105,
    PACKETTYPE_C2S_TOUCH_END = 106,
    PACKETTYPE_C2S_TOUCH_CANCEL = 107,

    // reprecator to server
    PACKETTYPE_R2S_CLIENT_LOGIN = 150, // accepting new client, getting new id number of this client
    PACKETTYPE_R2S_CLIENT_LOGOUT = 151,
    PACKETTYPE_R2S_KEYBOARD = 155,
    PACKETTYPE_R2S_MOUSE_BUTTON = 156,
    PACKETTYPE_R2S_CURSOR_POS = 157,
    
    // server to reprecator
    PACKETTYPE_S2R_NEW_CLIENT_ID = 170,
    PACKETTYPE_S2R_CAMERA_CREATE = 175,
    PACKETTYPE_S2R_CAMERA_DYNAMIC_LAYER = 176,
    PACKETTYPE_S2R_CAMERA_LOC = 177,
    PACKETTYPE_S2R_VIEWPORT_CREATE = 180,
    PACKETTYPE_S2R_VIEWPORT_DYNAMIC_LAYER = 181,
    PACKETTYPE_S2R_VIEWPORT_SCALE = 182,
    PACKETTYPE_S2R_PROP2D_TARGET_CLIENT = 183,
    PACKETTYPE_S2R_PROP2D_LOC = 184,     // send this movement to target client
    
    // server to client
    PACKETTYPE_S2C_PROP2D_SNAPSHOT = 200, 
    PACKETTYPE_S2C_PROP2D_LOC = 201,
    PACKETTYPE_S2C_PROP2D_INDEX = 203,
    PACKETTYPE_S2C_PROP2D_SCALE = 204,
    PACKETTYPE_S2C_PROP2D_ROT = 205,
    PACKETTYPE_S2C_PROP2D_FLIPROTBITS = 206,
    PACKETTYPE_S2C_PROP2D_COLOR = 208,
    PACKETTYPE_S2C_PROP2D_OPTBITS = 209,
    PACKETTYPE_S2C_PROP2D_PRIORITY = 210,
    PACKETTYPE_S2C_PROP2D_DELETE = 230,
    PACKETTYPE_S2C_PROP2D_CLEAR_CHILD = 240,
    PACKETTYPE_S2C_PROP2D_LOC_VEL = 250,
    PACKETTYPE_S2C_PROP2D_INDEX_LOC = 251,
    PACKETTYPE_S2C_PROP2D_LOC_SCL = 252,
    
    PACKETTYPE_S2C_LAYER_CREATE = 300,
    PACKETTYPE_S2C_LAYER_VIEWPORT = 301,
    PACKETTYPE_S2C_LAYER_CAMERA = 302,
    PACKETTYPE_S2C_VIEWPORT_CREATE = 330,
    //    PACKETTYPE_S2C_VIEWPORT_SIZE = 331,  not used now
    PACKETTYPE_S2C_VIEWPORT_SCALE = 332,
    PACKETTYPE_S2C_VIEWPORT_DYNAMIC_LAYER = 333,    
    PACKETTYPE_S2C_CAMERA_CREATE = 340,
    PACKETTYPE_S2C_CAMERA_LOC = 341,
    PACKETTYPE_S2C_CAMERA_DYNAMIC_LAYER = 342, // cam id, layer id: camera belongs to the layer's dynamic_cameras
    
    PACKETTYPE_S2C_TEXTURE_CREATE = 400,
    PACKETTYPE_S2C_TEXTURE_IMAGE = 401,
    PACKETTYPE_S2C_IMAGE_CREATE = 420,
    PACKETTYPE_S2C_IMAGE_LOAD_PNG = 421,
    PACKETTYPE_S2C_IMAGE_ENSURE_SIZE = 424,
    PACKETTYPE_S2C_IMAGE_RAW = 425,
    
    PACKETTYPE_S2C_TILEDECK_CREATE = 440,
    PACKETTYPE_S2C_TILEDECK_TEXTURE = 441,
    PACKETTYPE_S2C_TILEDECK_SIZE = 442,
    PACKETTYPE_S2C_GRID_CREATE = 460, // with its size (id,w,h)
    PACKETTYPE_S2C_GRID_DECK = 461, // with gid,tdid
    PACKETTYPE_S2C_GRID_PROP2D = 462, // with gid,propid    
    PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT = 464, // index table, array of int32_t
    PACKETTYPE_S2C_GRID_TABLE_FLIP_SNAPSHOT = 465, // xfl|yfl|uvrot bitfield in array of uint8_t
    PACKETTYPE_S2C_GRID_TABLE_TEXOFS_SNAPSHOT = 466, //  array of Vec2
    PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT = 467, // color table, array of PacketColor: 4 * float32    
    PACKETTYPE_S2C_GRID_DELETE = 470,

    PACKETTYPE_S2C_TEXTBOX_CREATE = 500, // tb_id, uint32_t
    PACKETTYPE_S2C_TEXTBOX_FONT = 501,    // tb_id, font_id
    PACKETTYPE_S2C_TEXTBOX_STRING = 502,    // tb_id, utf8str
    PACKETTYPE_S2C_TEXTBOX_LOC = 503,    // tb_id, x,y
    PACKETTYPE_S2C_TEXTBOX_SCL = 504,    // tb_id, x,y    
    PACKETTYPE_S2C_TEXTBOX_COLOR = 505,    // tb_id, PacketColor
    PACKETTYPE_S2C_TEXTBOX_PRIORITY = 506, // tb_id, priority
    PACKETTYPE_S2C_TEXTBOX_LAYER = 510,     // tb_id, l_id
    PACKETTYPE_S2C_FONT_CREATE = 540, // fontid, utf8 string array
    PACKETTYPE_S2C_FONT_CHARCODES = 541, // fontid, utf8str
    PACKETTYPE_S2C_FONT_LOADTTF = 542, // fontid, filepath    

    PACKETTYPE_S2C_COLOR_REPLACER_SHADER_SNAPSHOT = 600, //
    PACKETTYPE_S2C_PRIM_BULK_SNAPSHOT = 610, // array of PacketPrim

    PACKETTYPE_S2C_SOUND_CREATE_FROM_FILE = 650,
    PACKETTYPE_S2C_SOUND_CREATE_FROM_SAMPLES = 651,
    PACKETTYPE_S2C_SOUND_DEFAULT_VOLUME = 653,
    PACKETTYPE_S2C_SOUND_PLAY = 660,
    PACKETTYPE_S2C_SOUND_STOP = 661,    
    PACKETTYPE_S2C_SOUND_POSITION = 662,

    PACKETTYPE_S2C_JPEG_DECODER_CREATE = 700,
    PACKETTYPE_S2C_CAPTURED_FRAME = 701,
    PACKETTYPE_S2C_CAPTURED_AUDIO = 710,
    
    PACKETTYPE_S2C_FILE = 800, // send file body and path

    PACKETTYPE_S2C_WINDOW_SIZE = 900, // u2



    PACKETTYPE_MAX = 1000,
    
    PACKETTYPE_ERROR = 2000, // error code
} PACKETTYPE;

#define MODKEY_BIT_SHIFT 0x01
#define MODKEY_BIT_CONTROL 0x02
#define MODKEY_BIT_ALT 0x04
int calcModkeyBits(bool shift, bool ctrl, bool alt );
void getModkeyBits(int val, bool *shift, bool *ctrl, bool *alt );

class ChangeEntry {
public:
    Prop2D *p;
    PacketProp2DSnapshot *pkt;
    ChangeEntry() : p(NULL),pkt(NULL) {}
    ChangeEntry(Prop2D*p, PacketProp2DSnapshot *pkt) : p(p), pkt(pkt) {}    
};

class Reprecator {
public:
    uv_tcp_t listener;
    RemoteHead *parent_rh;
    ObjectPool<Client> cl_pool;
    ObjectPool<Client> logical_cl_pool;
    
    Reprecator(RemoteHead *parent_rh, int portnum);
    void addRealClient(Client*cl); // real clients are reproxies
    void delRealClient(Client*cl);
    void addLogicalClient(Client*cl); // logical clients are viewers
    void delLogicalClient(Client*cl);
    Client *getLogicalClient(uint32_t logclid);
    void heartbeat();
};
class Stream;    
class RemoteHead {
public:
    uv_tcp_t listener;
    MoyaiClient *target_moyai;
    SoundSystem *target_soundsystem;
    ObjectPool<Client> cl_pool;
    int window_width, window_height;
    bool enable_spritestream;
    bool enable_videostream;
    bool enable_timestamp;
    JPEGCoder *jc;
    BufferArray *audio_buf_ary;

    Reprecator *reprecator;

    bool enable_compression;
    
    void enableSpriteStream() { enable_spritestream = true; };
    void enableVideoStream( int w, int h, int pixel_skip );
    void enableReprecation(int portnum);
    void disableTimestamp() { enable_timestamp = false; }
    
    void (*on_connect_cb)(RemoteHead*rh,Client *cl);
    void (*on_disconnect_cb)(RemoteHead*rh, Client *cl);
    void (*on_keyboard_cb)(Client *cl,int kc,int act,int modshift,int modctrl,int modalt);
    void (*on_mouse_button_cb)(Client *cl, int btn, int act, int modshift, int modctrl, int modalt );
    void (*on_mouse_cursor_cb)(Client *cl, int x, int y );
    static const int DEFAULT_PORT = 22222;
    RemoteHead() : target_moyai(0), target_soundsystem(0), window_width(0), window_height(0), enable_spritestream(0), enable_videostream(0), enable_timestamp(true), jc(NULL), audio_buf_ary(0), reprecator(NULL), enable_compression(true), on_connect_cb(0), on_disconnect_cb(0), on_keyboard_cb(0), on_mouse_button_cb(0), on_mouse_cursor_cb(0), changelist_used(0), sorted_changelist_max_send_num(20), sort_sync_thres(50), linear_sync_score_thres(50), nonlinear_sync_score_thres(50) {
    }
    void addClient(Client*cl);
    void delClient(Client*cl);
    Client *getFirstClient();
    int getClientCount();
    
    void track2D();
    void broadcastCapturedScreen();
    bool startServer( int portnum );
    void setWindowSize(int w, int h) { window_width = w; window_height = h; }
    void setOnConnectCallback( void (*f)(RemoteHead *rh, Client *cl) ) { on_connect_cb = f; }
    void setOnDisconnectCallback( void (*f)(RemoteHead *rh, Client *cl ) ) { on_disconnect_cb = f; }
    void setOnKeyboardCallback( void (*f)(Client*cl,int,int,int,int,int) ) { on_keyboard_cb = f; }
    void setOnMouseButtonCallback( void (*f)(Client*cl,int,int,int,int,int) ) { on_mouse_button_cb = f; }
    void setOnMouseCursorCallback( void (*f)(Client*cl,int,int) ) { on_mouse_cursor_cb = f; }
    void heartbeat();
    void flushBufferToNetwork();
    void scanSendAllPrerequisites( Stream *outstream );
    void scanSendAllProp2DSnapshots( Stream *outstream );
    void notifyProp2DDeleted( Prop2D *prop_deleted );
    void notifyGridDeleted( Grid *grid_deleted );
    void notifyChildCleared( Prop2D *owner_prop, Prop2D *child_prop );
    void setTargetSoundSystem(SoundSystem*ss) { target_soundsystem = ss; }
    void setTargetMoyaiClient(MoyaiClient*mc) { target_moyai = mc; }
    void notifySoundPlay( Sound *snd, float vol );
    void notifySoundStop( Sound *snd );
    void appendAudioSamples( uint32_t numChannels, float *interleavedSamples, uint32_t numSamples );
    
    void broadcastUS1Bytes( uint16_t usval, const char *data, size_t datalen );
    void broadcastUS1UI1Bytes( uint16_t usval, uint32_t uival, const char *data, size_t datalen );    
    void broadcastUS1UI1( uint16_t usval, uint32_t uival );
    void broadcastUS1UI2( uint16_t usval, uint32_t ui0, uint32_t ui1, bool reprecator_only = false );
    void broadcastUS1UI3( uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2 );
    void broadcastUS1UI4( uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2, uint32_t ui3 );    
    void broadcastUS1UI1Wstr( uint16_t usval, uint32_t uival, wchar_t *wstr, int wstr_num_letters );
    void broadcastUS1UI1F1( uint16_t usval, uint32_t uival, float f0 );
    void broadcastUS1UI1F2( uint16_t usval, uint32_t uival, float f0, float f1 );
    void broadcastUS1UI2F2( uint16_t usval, uint32_t ui0, uint32_t ui1, float f0, float f1 );    
    void broadcastUS1UI1F4( uint16_t usval, uint32_t uival, float f0, float f1, float f2, float f3 );
    void broadcastUS1UI1UC1( uint16_t usval, uint32_t uival, uint8_t ucval );    

    void nearcastUS1UI1F2( Prop2D *p, uint16_t usval, uint32_t uival, float f0, float f1 );
    void nearcastUS1UI3( Prop2D *p, uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2, bool reprecator_only = false );
    void nearcastUS1UI3F2( Prop2D *p, uint16_t usval, uint32_t uival, uint32_t u0, uint32_t u1, float f0, float f1 );

    void broadcastUS1UI2ToReprecator( uint16_t usval, uint32_t ui0, uint32_t ui1 );
    
    void broadcastTimestamp();

    static const char *funcidToString(PACKETTYPE pkt);


    ChangeEntry changelist[4096];
    int changelist_used;
    int sorted_changelist_max_send_num;
    int sort_sync_thres;    
    float linear_sync_score_thres;
    float nonlinear_sync_score_thres;
    
    void clearChangelist() { changelist_used=0; }
    bool appendNonlinearChangelist(Prop2D *p, PacketProp2DSnapshot *pkt);
    void broadcastSortedChangelist();
    void setSortSyncThres(int thres) { sort_sync_thres = thres; }
    void setLinearSyncScoreThres(float thres) { linear_sync_score_thres = thres; }
    void setNonLinearSyncScoreThres(float thres) { nonlinear_sync_score_thres = thres; }
    ObjectPool<Deck> prereq_deck_pool;
    void addPrerequisites(Deck *dk);
};



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
    void scanProp2D( Prop2D *parentprop );
    void flipCurrentBuffer();
    int checkDiff();
    void broadcastDiff( bool force );
};
typedef enum {
    GTT_INDEX = 1,
    GTT_FLIP = 2,
    GTT_TEXOFS = 3,
    GTT_COLOR = 4,
} GRIDTABLETYPE;

#define GTT_FLIP_BIT_X 0x01
#define GTT_FLIP_BIT_Y 0x02
#define GTT_FLIP_BIT_UVROT 0x04

class TrackerGrid {
public:
    Grid *target_grid;
    int32_t *index_table[2];
    uint8_t *flip_table[2]; // ORing GTT_FLIP_BIT_*
    PacketVec2 *texofs_table[2];
    PacketColor *color_table[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerGrid(RemoteHead *rh, Grid *target);
    ~TrackerGrid();
    void scanGrid();
    bool checkDiff(GRIDTABLETYPE gtt);
    void flipCurrentBuffer();
    void broadcastDiff( Prop2D *owner, bool force );
    void broadcastGridConfs( Prop2D *owner ); // util sendfunc
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
    void broadcastDiff( bool force );
};

class TrackerColorReplacerShader {
public:
    ColorReplacerShader *target_shader;
    PacketColorReplacerShaderSnapshot pktbuf[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerColorReplacerShader(RemoteHead *rh, ColorReplacerShader *target ) : target_shader(target), cur_buffer_index(0), parent_rh(rh) {};
    ~TrackerColorReplacerShader();
    void scanShader();
    void flipCurrentBuffer();
    bool checkDiff();
    void broadcastDiff( bool force );    
};

class TrackerPrimDrawer {
public:
    PrimDrawer *target_pd;
    PacketPrim *pktbuf[2]; // Each points to an array of PacketPrim
    int pktnum[2];
    int pktmax[2]; // malloced size
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerPrimDrawer( RemoteHead *rh, PrimDrawer *target )     : target_pd(target), cur_buffer_index(0), parent_rh(rh) {
        pktbuf[0] = pktbuf[1] = 0;
        pktnum[0] = pktnum[1] = 0;
        pktmax[0] = pktmax[1] = 0;
    }
    ~TrackerPrimDrawer();
    void scanPrimDrawer();
    void flipCurrentBuffer();
    bool checkDiff();
    void broadcastDiff( Prop2D *owner, bool force );
};

class Deck;
class TrackerImage {
public:
    Image *target_image;
    uint8_t *imgbuf[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerImage( RemoteHead *rh, Image *target );
    ~TrackerImage();
    void scanImage();
    void flipCurrentBuffer();
    bool checkDiff();
    void broadcastDiff( Deck *owner_dk, bool force );

};
class Camera;
class Layer;

class TrackerCamera {
public:
    Camera *target_camera;
    Vec2 locbuf[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerCamera( RemoteHead *rh, Camera *target );
    ~TrackerCamera();
    void scanCamera();
    void flipCurrentBuffer();
    bool checkDiff();
    void broadcastDiff( bool force );
    void unicastDiff( Client *dest, bool force );
    void unicastCreate( Client *dest );
};
class Viewport;
class TrackerViewport {
public:
    Viewport *target_viewport;
    Vec2 sclbuf[2];
    int cur_buffer_index;
    RemoteHead *parent_rh;
    TrackerViewport( RemoteHead *rh, Viewport *target );
    ~TrackerViewport();
    void scanViewport();
    void flipCurrentBuffer();
    bool checkDiff();
    void broadcastDiff( bool force );
    void unicastDiff( Client *dest, bool force );
    void unicastCreate( Client *dest );
};


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

class BufferArray {
public:
    Buffer **buffers;
    size_t buffer_num;
    size_t buffer_used;
    BufferArray( int maxnum );
    ~BufferArray();
    bool push(const char *data, size_t len);
    Buffer *getTop();
    void shift();
    size_t getUsedNum() { return buffer_used; }
};

class ReprecationProxy;

class Stream {
public:
    
    static int idgen;
    int id;    
    Buffer sendbuf;    
    Buffer recvbuf;
    Buffer unzipped_recvbuf; // receive zipped_records in recvbuf and then decompress to unzipped_recvbuf and then parseRecord again
    uv_tcp_t *tcp;
    bool use_compression;
    Stream( uv_tcp_t *sk, size_t sendbufsize, size_t recvbufsize, bool compress );
    void flushSendbuf(size_t unitsize);    
};

class Client : public Stream {
public:
    
    RemoteHead *parent_rh; 
    ReprecationProxy *parent_reproxy;
    Reprecator *parent_reprecator;
    
    double initialized_at;
    Camera *target_camera;
    Viewport *target_viewport;

    uint32_t global_client_id; // used by reproxy

    Stream *reprecator_stream; // used only when logical client
    
    Client( uv_tcp_t *sk, RemoteHead *rh, bool compress );
    Client( uv_tcp_t *sk, ReprecationProxy *reproxy, bool compress );
    Client( uv_tcp_t *sk, Reprecator *repr, bool compress );
    Client( RemoteHead *rh );
    static Client *createLogicalClient( Stream *reprecator_stream, RemoteHead *rh );
    void init();
    ~Client();
    void saveStream( const char *data, size_t datalen );
    void flushStreamToFile();
    bool canSee(Prop2D*p);
    Stream *getStream() {
        if(this->tcp) return this; else return this->reprecator_stream;
    }
    bool isLogical() { return reprecator_stream; }
    void flushSendbufToNetwork();
};


class ReprecationProxy {
public:
    uv_tcp_t listener;
    ObjectPool<Client> cl_pool;    
    void (*func_callback)( Stream *s, uint16_t funcid, char *data, uint32_t datalen );
    void (*accept_callback)(Stream *s);
    void (*close_callback)(Stream *s);
    ReprecationProxy(int port);
    void setFuncCallback( void (*cb)( Stream *s, uint16_t funcid, char *data, uint32_t datalen ) ) {func_callback = cb;}
    void setAcceptCallback( void (*cb)(Stream*s) ) { accept_callback = cb; }
    void setCloseCallback( void (*cb)(Stream*) ) { close_callback = cb; }
    void addClient( Client *cl);
    void delClient(Client*cl);
    Client *getClient(unsigned int id) { return cl_pool.get(id); }
    Client *getClientByGlobalId(unsigned int gclid);
    void broadcastUS1RawArgs(uint16_t funcid, const char*data, size_t datalen );
    void heartbeat();
};
  

            
// record parser
bool parseRecord( Stream *s, Buffer *recvbuf, const char *data, size_t datalen, void (*funcCallback)( Stream *s, uint16_t funcid, char *data, uint32_t datalen ) );


// send funcs
int sendUS1( Stream *out, uint16_t usval );
int sendUS1RawArgs( Stream *s, uint16_t usval, const char *data, uint32_t datalen );
int sendUS1Bytes( Stream *out, uint16_t usval, const char *buf, uint16_t datalen );
int sendUS1UI1Bytes( Stream *out, uint16_t usval, uint32_t uival, const char *buf, uint32_t datalen );
int sendUS1UI1( Stream *out, uint16_t usval, uint32_t ui0 );
int sendUS1UI2( Stream *out, uint16_t usval, uint32_t ui0, uint32_t ui1 );    
int sendUS1UI3( Stream *out, uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2 );
int sendUS1UI4( Stream *out, uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2, uint32_t ui3 );
int sendUS1UI5( Stream *out, uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2, uint32_t ui3, uint32_t ui4 );
int sendUS1UI1F1( Stream *out, uint16_t usval, uint32_t uival, float f0 );    
int sendUS1UI1F2( Stream *out, uint16_t usval, uint32_t uival, float f0, float f1 );
int sendUS1UI2F2( Stream *s, uint16_t usval, uint32_t uival0, uint32_t uival1, float f0, float f1 ) ;
int sendUS1UI3F2( Stream *s, uint16_t usval, uint32_t uival0, uint32_t uival1, uint32_t uival2, float f0, float f1 ) ;
int sendUS1UI1F4( Stream *out, uint16_t usval, uint32_t uival, float f0, float f1, float f2, float f3 );
int sendUS1UI1UC1( Stream *out, uint16_t usval, uint32_t uival, uint8_t ucval );
int sendUS1UI1Str( Stream *out, uint16_t usval, uint32_t uival, const char *cstr );
int sendUS1UI2Str( Stream *out, uint16_t usval, uint32_t ui0, uint32_t ui1, const char *cstr );
int sendUS1StrBytes( Stream *out, uint16_t usval, const char *cstr, const char *data, uint32_t datalen );
int sendUS1UI1Wstr( Stream *out, uint16_t usval, uint32_t uival, wchar_t *wstr, int wstr_num_letters );
int sendUS1F2( Stream *out, uint16_t usval, float f0, float f1 );
void sendFile( Stream *outstream, const char *filename );
void sendPing( Stream *s );
void sendWindowSize( Stream *outstream, int w, int h );
void sendViewportCreateScale( Stream *outstream, Viewport *vp );
void sendCameraCreateLoc( Stream *outstream, Camera *cam );
void sendLayerSetup( Stream *outstream, Layer *l );
void sendImageSetup( Stream *outstream, Image *img );
class Texture;
void sendTextureCreateWithImage( Stream *outstream, Texture *tex );
void sendDeckSetup( Stream *outstream, Deck *dk );
class Font;
void sendFontSetupWithFile( Stream *outstream, Font *f ) ;
void sendColorReplacerShaderSetup( Stream *outstream, ColorReplacerShader *crs ) ;
void sendSoundSetup( Stream *outstream, Sound *snd ) ;

// parse helpers
void parsePacketStrBytes( char *inptr, char *outcstr, char **outptr, size_t *outsize );



void moyai_libuv_alloc_buffer( uv_handle_t *handle, size_t suggested_size, uv_buf_t *outbuf );

void uv_run_times( int maxcount );


/*
  demo2d --headless
    + @HEADLESS_SERVER_PORT     <------------- viewer
    + @REPRECATOR_SERVER_PORT   <------------- viewer --reprecate
                                                  + @REPRECATOR_PROXY_PORT   <---------------- viewer
 */
#define HEADLESS_SERVER_PORT 22222  
#define REPRECATOR_SERVER_PORT 22223 
#define REPRECATOR_PROXY_PORT 22224

bool init_tcp_listener( uv_tcp_t *l, void *data, int portnum, void (*cb)(uv_stream_t*l,int status) ) ;

#endif
