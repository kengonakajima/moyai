#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <locale.h>

#ifndef WIN32
#include <strings.h>
#endif

#include "client.h"

TileDeck *g_deck;
Layer *g_static_bg_layer;
Layer *g_field_layer;
Layer *g_char_layer;
Camera *g_static_camera;

Keyboard *g_local_keyboard;

// config

static const int SCRW=966, SCRH=544;




void winclose_callback( GLFWwindow *w ){
    exit(0);
}

void glfw_error_cb( int code, const char *desc ) {
    print("glfw_error_cb. code:%d desc:'%s'", code, desc );
}

//

class PC : public Prop2D {
public:   
    Client *cl;
    Keyboard *keyboard;
    Camera *camera;
    PC(Client *cl) : Prop2D(), cl(cl) {
        setDeck(g_deck);
        setScl(32);
        setIndex(0);
        keyboard = new Keyboard();
        camera = new Camera(cl);
        g_field_layer->addDynamicCamera(camera);
        g_char_layer->addDynamicCamera(camera);
    }
    virtual bool prop2DPoll(double dt) {
        float speed = 3;
        if( keyboard->getKey( 'W' ) ) loc.y += speed;
        if( keyboard->getKey( 'S' ) ) loc.y -= speed;
        if( keyboard->getKey( 'A' ) ) loc.x -= speed;
        if( keyboard->getKey( 'D' ) ) loc.x += speed;
        camera->setLoc(loc);
        return true;
    }
};

ObjectPool<PC> g_pc_cl_pool; // client idから検索

PC *addPC( Client *cl ) {
    PC *pc = g_pc_cl_pool.get(cl->id);
    if(pc) {
        assertmsg(false, "can't add a pc twice. id:%d",cl->id);
    }
    pc = new PC(cl);
    g_pc_cl_pool.set(cl->id,pc);
    print("added a client[%d] to pc pool", cl->id );
    g_char_layer->insertProp(pc);
    return pc;
}
// use client id is 0
void addLocalPC( PC *to_add ) {
    g_pc_cl_pool.set(0,to_add);
}
void delPC( PC *pc ) {
    PC *found = g_pc_cl_pool.get( pc->cl->id );
    if(!found) {
        assertmsg( false, "can't find a clid:%d in pc pool", pc->cl->id );
    }
    g_pc_cl_pool.del(pc->cl->id);
    delete pc;
}
PC *getPC(Client *cl) {
    return g_pc_cl_pool.get(cl->id);
}
PC *getLocalPC() {
    for( std::unordered_map<unsigned int,PC*>::iterator it = g_pc_cl_pool.idmap.begin(); it != g_pc_cl_pool.idmap.end(); ++it ) {
        PC *pc = it->second;
        if(pc->cl == 0) {
            return pc;            
        }
    }
    return NULL;
}



class Enemy : public Prop2D {
public:
    Enemy() : Prop2D() {
    }
    virtual bool prop2DPoll(double dt) {
        return true;
    }
};

void localKeyboardCallback( GLFWwindow *window, int keycode, int scancode, int action, int mods ) {
    g_local_keyboard->update( keycode, action, 0, 0, 0 ); // dont read mod keys
}
void onRemoteKeyboardCallback( Client *cl, int kc, int act, int modshift, int modctrl, int modalt ) {
    PC *pc = getPC(cl);
    print("onRemoteKeyboardCallback kc:%d act:%d cl:%d", kc, act, cl->id );
    if(!pc)return;

    pc->keyboard->update(kc,act,modshift,modctrl,modalt);
}
//

void setupBG() {
    int w = 10, h = 6;
    for(int y=-h;y<h;y++) {
        for(int x=-w;x<w;x++) {
            Prop2D *bgprop = new Prop2D();
            bgprop->setDeck(g_deck);
            bgprop->setIndex(3);
            bgprop->setScl(32);
            Vec2 at(x*32,y*32);
            bgprop->setLoc(at);
            g_field_layer->insertProp(bgprop);
            if( range(0,100) < 5 ) { 
                Prop2D *treeprop = new Prop2D();
                treeprop->setDeck(g_deck);
                treeprop->setIndex(2);
                treeprop->setScl(32);
                treeprop->setLoc(at);
                g_field_layer->insertProp(treeprop);
            }
        }
    }
}

void onConnectCallback( RemoteHead *rh, Client *cl ) {
    print("onConnectCallback: clid:%d",cl->id);
    addPC(cl);
}

int main(int argc, char **argv )
{

#ifdef __APPLE__    
    setlocale( LC_ALL, "ja_JP");
#endif
#ifdef WIN32    
    setlocale( LC_ALL, "jpn");
#endif    
    
    // glfw
    if( !glfwInit() ) {
        print("can't init glfw");
        return 1;
    }

    GLFWwindow *window;
    glfwSetErrorCallback( glfw_error_cb );
    window =  glfwCreateWindow( SCRW, SCRH, "demo2d", NULL, NULL );
    if(window == NULL ) {
        print("can't open glfw window");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);    
    glfwSetWindowCloseCallback( window, winclose_callback );
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSwapInterval(1); // vsync
    glfwSetKeyCallback( window, localKeyboardCallback );
#ifdef WIN32
	glewInit();
#endif
    glClearColor(0.2,0.2,0.2,1);

    g_local_keyboard = new Keyboard();
    
    MoyaiClient *moyai_client = new MoyaiClient(window,SCRW,SCRH);
    
    Moyai::globalInitNetwork();
    RemoteHead *rh = new RemoteHead();
    if( rh->startServer(22222) == false ) {
        print("headless server: can't start server. port:%d", 22222 );
        exit(1);
    }
    moyai_client->setRemoteHead(rh);
    rh->setTargetMoyaiClient(moyai_client);
    rh->setOnKeyboardCallback(onRemoteKeyboardCallback);
    rh->setOnConnectCallback(onConnectCallback);
    
    Viewport *viewport = new Viewport();
    int retina = 1;
#if defined(__APPLE__)
    retina = 2;
#endif    
    viewport->setSize(SCRW*retina,SCRH*retina); // set actual framebuffer size to output
    viewport->setScale2D(SCRW,SCRH); // set scale used by props that will be rendered

    g_static_camera = new Camera();
    g_static_camera->setLoc(0,0);

    g_static_bg_layer = new Layer();
    moyai_client->insertLayer(g_static_bg_layer);
    g_static_bg_layer->setViewport(viewport);
    g_static_bg_layer->setCamera(g_static_camera);

    g_field_layer = new Layer(); // dont call setCamera here, later use addDynamicCamera for each players
    moyai_client->insertLayer(g_field_layer);
    g_field_layer->setViewport(viewport);
    g_char_layer = new Layer(); // dont call setCamera here too
    moyai_client->insertLayer(g_char_layer);
    g_char_layer->setViewport(viewport);


    Texture *t = new Texture();
    t->load( "./assets/base.png" );
    
    g_deck = new TileDeck();
    g_deck->setTexture(t);
    g_deck->setSize(32,32,8,8);


    setupBG();
    
    // main loop
    while( !glfwWindowShouldClose(window) ){
        static double last_print_at = 0;
        static int frame_counter = 0;
        static double last_poll_at = now();
        static int loop_counter = 0;
        
        double t = now();
        double dt = t - last_poll_at;
        double ideal_frame_time = 1.0f / 60.0f;
        if(dt < ideal_frame_time ) {
            double to_sleep_sec = ideal_frame_time - dt;
            int to_sleep_msec = (int) (to_sleep_sec*1000);
            if( to_sleep_msec > 0 ) sleepMilliSec(to_sleep_msec);
        }
        last_poll_at = t;
        
        frame_counter ++;
        loop_counter++;

        glfwPollEvents();
        
        int cnt = moyai_client->poll(dt);        
        
        // fps disp
        if(last_print_at == 0){
            last_print_at = t;
        } else if( last_print_at < t-1 ){
        fprintf(stderr,"FPS:%d prop:%d drawcall:%d\n", frame_counter, cnt, moyai_client->last_draw_call_count  );
            frame_counter = 0;
            last_print_at = t;
        }

        moyai_client->render();
        //        print("drawcnt:%d", moyai_client->last_draw_call_count );
        if( g_local_keyboard->getKey( 'Q') ) {
            print("Q pressed");
            exit(0);
            break;
        }
        

    }
    glfwTerminate();

    print("program finished");
    

    return 0;
}
