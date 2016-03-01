// HMP (Headless Moyai Protocol viewer client)
#include "client.h"
#include "vw.h"

static const int SCRW=966, SCRH=544;

ObjectPool<Layer> g_layer_pool;
ObjectPool<Viewport> g_viewport_pool;
ObjectPool<Camera> g_camera_pool;
ObjectPool<Texture> g_texture_pool;
ObjectPool<Image> g_image_pool;
ObjectPool<TileDeck> g_tiledeck_pool;
ObjectPool<Prop2D> g_prop2d_pool;

MoyaiClient *g_moyai_client;        
Network *g_nw;
FileDepo *g_filedepo;
HMPClientConn *g_conn;
GLFWwindow *g_window;

Viewport *g_debug_viewport;
Layer *g_debug_layer;
Font *g_debug_font;
TextBox *g_debug_tb;


File::File( const char *inpath, const char *indata, size_t indata_len ) {
    strncpy( path, inpath, sizeof(path) );
    data = (char*) MALLOC( indata_len );
    memcpy( data, indata, indata_len );
    data_len = indata_len;
    print("File: init. path:'%s' size:%d data:%x %x %x %x", path, indata_len, indata[0], indata[1], indata[2], indata[3] );
}


File *FileDepo::get( char *path ) {
    for(int i=0;i<elementof(files);i++) {
        if( files[i] && files[i]->comparePath(path) ) {
            return files[i];
        }
    }
    return NULL;
}

File *FileDepo::ensure( char *path, char *data, size_t datalen ) {
    File *f = get(path);
    if(f) return f;    
    for(int i=0;i<elementof(files);i++) {
        if( files[i] == NULL ) {
            print("ensure: alloc ind:%d", i );
            files[i] = new File( path, data, datalen );
            return files[i];
        }
    }
    assertmsg( false, "file full for: '%s'", path );
    return NULL;
}

File *FileDepo::getByIndex(int ind) {
    assert(ind>=0 && ind< MAX_FILES);
    return files[ind];
}



///////////////////

void winclose_callback( GLFWwindow *w ){
    exit(0);
}

void glfw_error_cb( int code, const char *desc ) {
    print("glfw_error_cb. code:%d desc:'%s'", code, desc );
}


///////////////////
void HMPClientConn::onError( NET_ERROR e, int eno ) {
    print("HMPClientConn::onError. e:%d eno:%d",e,eno);
}
void HMPClientConn::onClose() {
    print("HMPClientConn::onClose.");    
}
void HMPClientConn::onConnect() {
    print("HMPClientConn::onConnect");
    sendUS1( PACKETTYPE_C2S_GET_ALL_PREREQUISITES );
}
void HMPClientConn::onPacket( uint16_t funcid, char *argdata, size_t argdatalen ) {
    //    print("HMPClientConn::onPacket");
    switch(funcid) {
    case PACKETTYPE_S2C_PROP2D_SNAPSHOT:
        {
            //        print("PACKETTYPE_S2C_PROP2D_SNAPSHOT len:%d", argdatalen );            
            PacketProp2DSnapshot pkt;
            assert( argdatalen == sizeof(pkt) );
            memcpy(&pkt,argdata,sizeof(pkt));
            //            prt("s%d ", pkt.prop_id );


            if( pkt.debug) print("packettype_prop2d_create! id:%d layer_id:%d loc:%f,%f scl:%f,%f index:%d tdid:%d", pkt.prop_id, pkt.layer_id, pkt.loc.x, pkt.loc.y, pkt.scl.x, pkt.scl.y, pkt.index, pkt.tiledeck_id );
            Layer *layer = g_layer_pool.get( pkt.layer_id );
            if(!layer) {
                prt("_l");
                break;
            }

            TileDeck *dk = g_tiledeck_pool.get( pkt.tiledeck_id );
            if(!dk) {
                break;
            }
            Prop2D *prop = g_prop2d_pool.get(pkt.prop_id);
            if(!prop) {
                prop = g_prop2d_pool.ensure(pkt.prop_id);
                layer->insertProp(prop);
            }
            prop->setDeck(dk);
            prop->setIndex(pkt.index);
            prop->setScl( Vec2(pkt.scl.x,pkt.scl.y) );
            prop->setLoc( Vec2(pkt.loc.x, pkt.loc.y) );
            prop->setRot( pkt.rot );
            prop->setXFlip( pkt.xflip );
            prop->setYFlip( pkt.yflip );
            Color col( pkt.color.r, pkt.color.g, pkt.color.b, pkt.color.a );
            prop->setColor(col);
        }

        break;
    case PACKETTYPE_S2C_LAYER_CREATE:
        {
            uint32_t id = get_u32( argdata+0 );
            print("PACKETTYPE_S2C_LAYER_CREATE layer_id:%d", id );

            Layer *l = g_layer_pool.get(id);
            if(!l) {
                l = g_layer_pool.ensure(id);
                g_moyai_client->insertLayer(l);
                print("created a layer" );
            } else {
                print("layer found");
            }
        }
        break;

    case PACKETTYPE_S2C_VIEWPORT_CREATE:
        {
            unsigned int viewport_id = get_u32(argdata);
            print("received viewport_create. id:%d", viewport_id );
            Viewport *vp = g_viewport_pool.ensure(viewport_id);
            assert(vp);            
        }
        break;

    case PACKETTYPE_S2C_VIEWPORT_SIZE:
        {
            unsigned int viewport_id = get_u32(argdata);
            unsigned int w = get_u32(argdata+4);
            unsigned int h = get_u32(argdata+8);
            
            print("received viewport_size id:%d w:%d h:%d", viewport_id, w,h );
            Viewport *vp = g_viewport_pool.ensure(viewport_id);
            assert(vp);
            vp->setSize(w,h);
        }
        break;
    case PACKETTYPE_S2C_VIEWPORT_SCALE:
        {
            unsigned int viewport_id = get_u32(argdata);
            float sclx = get_f32(argdata+4);
            float scly = get_f32(argdata+8);
            print("received viewport_scale id:%d x:%f y:%f", viewport_id, sclx, scly );            
                        
            Viewport *vp = g_viewport_pool.ensure(viewport_id);
            assert(vp);
            vp->setScale2D(sclx,scly);
        }
        break;

    case PACKETTYPE_S2C_CAMERA_CREATE:
        {
            unsigned int camera_id = get_u32(argdata);
            print("received camera_create. id:%d", camera_id );
            Camera *cam = g_camera_pool.ensure(camera_id);
            assert(cam);
        }
        break;
    case PACKETTYPE_S2C_CAMERA_LOC:
        {
            unsigned int camera_id = get_u32(argdata);
            float x = get_f32(argdata+4);
            float y = get_f32(argdata+4+4);
            print("received camera_loc. id:%d (%f,%f)", camera_id, x,y );            
            Camera *cam = g_camera_pool.get(camera_id);
            assert(cam);
            cam->setLoc(x,y);
        }
        break;

    case PACKETTYPE_S2C_LAYER_CAMERA:
        {
            unsigned int layer_id = get_u32(argdata);
            unsigned int camera_id = get_u32(argdata+4);
            print("received layer_camera. l:%d cam:%d", layer_id, camera_id );                        
            Layer *l = g_layer_pool.get(layer_id);
            assert(l);
            Camera *cam = g_camera_pool.ensure(camera_id);
            assert(cam);
            l->setCamera(cam);
        }
        break;
    case PACKETTYPE_S2C_LAYER_VIEWPORT:
        {
            unsigned int layer_id = get_u32(argdata);
            unsigned int viewport_id = get_u32(argdata+4);
            print("received layer_viewport. l:%d vp:%d", layer_id, viewport_id );
            Layer *l = g_layer_pool.get(layer_id);
            assert(l);
            Viewport *vp = g_viewport_pool.ensure(viewport_id);
            assert(vp);
            l->setViewport(vp);
        }
        break;


    case PACKETTYPE_S2C_TEXTURE_CREATE:
        {
            unsigned int tex_id = get_u32(argdata);
            print("received texture_create. id:%d", tex_id );
            Texture *tex = g_texture_pool.ensure(tex_id);
            assert(tex);
        }
        break;
    case PACKETTYPE_S2C_TEXTURE_IMAGE:
        {
            unsigned int tex_id = get_u32(argdata);
            unsigned int img_id = get_u32(argdata+4);
            print("received texture_image. tex:%d img:%d", tex_id, img_id );
            Texture *tex = g_texture_pool.get(tex_id);
            assert(tex);
            Image *img = g_image_pool.get(img_id);
            assert(img);
            tex->setImage(img);
        }
        break;
    case PACKETTYPE_S2C_IMAGE_CREATE:
        {
            unsigned int img_id = get_u32(argdata);
            Image *img = g_image_pool.ensure(img_id);
            assert(img);
            print("received image_create. id:%d", img_id );
        }
        break;
    case PACKETTYPE_S2C_IMAGE_LOAD_PNG:        
        {
            unsigned int img_id = get_u32(argdata);
            unsigned char pathlen = get_u8(argdata+4);
            char *path = argdata+4+1;
            char cstrpath[256];
            memcpy( cstrpath, path, pathlen );
            cstrpath[pathlen]='\0';
            print("received image loadpng. id:%d path:'%s' ", img_id, cstrpath );
            Image *img = g_image_pool.ensure(img_id);
            File *fe = g_filedepo->get(cstrpath);
            assert(fe);
            bool ret = img->loadPNGMem( (unsigned char*) fe->data, fe->data_len );
            assert(ret);
            img->setOptionalLoadPath( cstrpath );
        }
        break;
    case PACKETTYPE_S2C_TILEDECK_CREATE:
        {
            unsigned int dk_id = get_u32(argdata);
            print("received tiledeck_create. id:%d", dk_id);
            TileDeck *dk = g_tiledeck_pool.ensure(dk_id);
            assert(dk);
        }
        break;        
    case PACKETTYPE_S2C_TILEDECK_TEXTURE:
        {
            unsigned int dk_id = get_u32(argdata);
            unsigned int tex_id = get_u32(argdata+4);
            print("received tiledeck_texture. dk:%d tex:%d", dk_id, tex_id );
            
            TileDeck *dk = g_tiledeck_pool.get(dk_id);
            assert(dk);
            Texture *tex = g_texture_pool.get(tex_id);
            assert(tex);
            dk->setTexture(tex);
        }
        break;
    case PACKETTYPE_S2C_TILEDECK_SIZE: 
        {
            unsigned int dk_id = get_u32(argdata);
            int sprw = get_u32(argdata+4);
            int sprh = get_u32(argdata+8);
            int cellw = get_u32(argdata+12);
            int cellh = get_u32(argdata+16);
            print("received tiledeck_size. dk:%d %d,%d,%d,%d", dk_id, sprw, sprh, cellw, cellh );            
            TileDeck *dk = g_tiledeck_pool.get(dk_id);
            assert(dk);
            dk->setSize( sprw, sprh, cellw, cellh );

        }
        break;
    case PACKETTYPE_S2C_FILE:
        {
            // pathと内容を同時に受け取る。
            char *dataptr;
            size_t datasize;
            char cstrpath[256];
            parsePacketStrBytes( argdata, cstrpath, &dataptr, &datasize );
            
            print("received file. path:'%s' datalen:%d data:%x %x %x %x", cstrpath, datasize, dataptr[0], dataptr[1], dataptr[2], dataptr[3] );
            g_filedepo->ensure( cstrpath, dataptr, datasize );
        }
        break;
    case PACKETTYPE_S2C_PROP2D_DELETE:
        {
            unsigned int prop_id = get_u32(argdata);
            Prop2D *prop = g_prop2d_pool.get(prop_id);
            if(prop) {
                prt("D[%d]", prop_id);
                prop->to_clean = true;
                g_prop2d_pool.del(prop_id);
            }
        }
        break;
        
    default:
        print("unhandled packet type:%d", funcid );
        break;
    }
    
}


void setupDebugStat() {
    int retina = 1;
#if defined(__APPLE__)
    retina = 2;
#endif    
    g_debug_viewport = new Viewport();
    g_debug_viewport->setSize(SCRW*retina,SCRH*retina);
    g_debug_viewport->setScale2D(SCRW,SCRH);
    g_debug_layer = new Layer();
    g_debug_layer->setViewport(g_debug_viewport);
    g_moyai_client->insertLayer(g_debug_layer);
    g_debug_font = new Font();
    wchar_t charcodes[] = L" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    g_debug_font->loadFromTTF("./assets/cinecaption227.ttf", charcodes, 12 );
    g_debug_tb = new TextBox();
    g_debug_tb->setFont(g_debug_font);
    g_debug_tb->setScl(1);
    g_debug_tb->setLoc(-SCRW/2+10,SCRH/2-15);
    g_debug_tb->setString("not init");
    g_debug_layer->insertProp(g_debug_tb);    
}
void updateDebugStat( const char *s ) {
    g_debug_tb->setString(s);
}

int main( int argc, char **argv ) {

    const char *host = "localhost";
    if( argc > 1 && argv[1] ) host = argv[1];
    int port = 22222;
    if( argc > 2 && argv[2] ) port = atoi(argv[2]);
    print("viewer config: host:'%s' port:%d", host, port );
    
    Moyai::globalInitNetwork();
    g_nw = new Network();    
    int fd = g_nw->connectToServer(host,port);
    if(fd<0) {
        print("can't connect to server");
        return 1;
    }
    g_conn = new HMPClientConn(g_nw,fd);

    //

    if( !glfwInit() ) {
        print("can't init glfw");
        exit(1);        
    }

    glfwSetErrorCallback( glfw_error_cb );
    g_window =  glfwCreateWindow( SCRW, SCRH, "headless moyai viewer", NULL, NULL );
    if(g_window == NULL ) {
        print("can't open glfw window");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(g_window);    
    glfwSetWindowCloseCallback( g_window, winclose_callback );
    glfwSetInputMode( g_window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSwapInterval(1); // vsync
#ifdef WIN32
	glewInit();
#endif
    glClearColor(0.2,0.2,0.2,1);
    
    g_moyai_client = new MoyaiClient( g_window );

    g_filedepo = new FileDepo();

    print("start viewer loop");

    // Client side debug status
    setupDebugStat();

    while( !glfwWindowShouldClose(g_window) ){
        static double last_poll_at = now();

        double t = now();
        double dt = t - last_poll_at;

        glfwPollEvents();
        g_nw->heartbeat();
        int polled = g_moyai_client->poll(dt);
        int rendered = g_moyai_client->render();
        Format fmt( "polled:%d rendered:%d", polled, rendered );
        updateDebugStat( fmt.buf );

        last_poll_at = t;
    }
    delete g_nw;
    return 0;
}
