// HMP (Headless Moyai Protocol viewer client)
#include "client.h"
#include "vw.h"

static const int SCRW=966, SCRH=544;

ObjectPool<Layer> g_layer_pool;
ObjectPool<Viewport> g_viewport_pool;
ObjectPool<Camera> g_camera_pool;

MoyaiClient *g_moyai_client;        
Network *g_nw;
HMPClientConn *g_conn;
GLFWwindow *g_window;

void winclose_callback( GLFWwindow *w ){
    exit(0);
}

void glfw_error_cb( int code, const char *desc ) {
    print("glfw_error_cb. code:%d desc:'%s'", code, desc );
}


///
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
    print("HMPClientConn::onPacket");
    switch(funcid) {
    case PACKETTYPE_S2C_PROP2D_SNAPSHOT:
        print("PACKETTYPE_S2C_PROP2D_SNAPSHOT len:%d", argdatalen );
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
        
    default:
        print("unhandled packet type:%d", funcid );
        break;
    }
    
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
    
    bool done = false;
    while(!done) {
        g_nw->heartbeat();
    }
    delete g_nw;
    return 0;
}
