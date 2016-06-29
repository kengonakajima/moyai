#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <locale.h>

#ifndef WIN32
#include <strings.h>
#endif

#include "client.h"


Keyboard *g_keyboard;

// config

static const int SCRW=768, SCRH=512;



void winclose_callback( GLFWwindow *w ){
    exit(0);
}

void glfw_error_cb( int code, const char *desc ) {
    print("glfw_error_cb. code:%d desc:'%s'", code, desc );
}

//

class Particle : public Prop2D {
public:
    Vec2 v;
    Particle(TileDeck *dk) : Prop2D() {
        setDeck(dk);
        setScl(32);
        setIndex(0);        
        v = Vec2( range(-100,100), range(-100,100) );        
    }
    bool prop2DPoll(double dt) {
        loc += v*dt;
        if(loc.x<-SCRW/2||loc.x>SCRW/2) v.x*=-1;
        if(loc.y<-SCRH/2||loc.y>SCRH/2) v.y*=-1;
        return true;
    }
};

void kbdCallback( GLFWwindow *window, int keycode, int scancode, int action, int mods ) {
    g_keyboard->update( keycode, action, 0, 0, 0 ); // dont read mod keys
}
void onRemoteKeyboardCallback( Client *cl, int kc, int act, int modshift, int modctrl, int modalt ) {
    g_keyboard->update(kc,act,modshift,modctrl,modalt);
}
//

int main(int argc, char **argv )
{
    bool headless_mode=false;
    for(int i=0;;i++) {
        if(!argv[i])break;
        if(strcmp(argv[i], "--headless") == 0 ) headless_mode = true;
    }

    print("program start");

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
    window =  glfwCreateWindow( SCRW, SCRH, "min2d", NULL, NULL );
    if(window == NULL ) {
        print("can't open glfw window");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);    
    glfwSetWindowCloseCallback( window, winclose_callback );
    glfwSetInputMode( window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSwapInterval(0); // set 1 to use vsync. Use 0 for fast screen capturing and headless
    glfwSetKeyCallback( window, kbdCallback );
#ifdef WIN32
	glewInit();
#endif
    glClearColor(0.2,0.2,0.2,1);

    SoundSystem *ss = new SoundSystem();
    Sound *bgm = ss->newSound( "assets/gymno1short.wav" );
    bgm->play();

    g_keyboard = new Keyboard();
    
    MoyaiClient *moyai_client = new MoyaiClient(window,SCRW,SCRH);
    
    if( headless_mode ) {
        Moyai::globalInitNetwork();
        RemoteHead *rh = new RemoteHead();
        if( rh->startServer(22222) == false ) {
            print("headless server: can't start server. port:%d", 22222 );
            exit(1);
        }
        rh->enableSpriteStream();
        moyai_client->setRemoteHead(rh);
        rh->setTargetMoyaiClient(moyai_client);
        ss->setRemoteHead(rh);
        rh->setTargetSoundSystem(ss);
        rh->setOnKeyboardCallback(onRemoteKeyboardCallback);
    }

    
    Viewport *viewport = new Viewport();
    int retina = 1;
#if defined(__APPLE__)
    retina = 2;
#endif    
    viewport->setSize(SCRW*retina,SCRH*retina); // set actual framebuffer size to output
    viewport->setScale2D(SCRW,SCRH); // set scale used by props that will be rendered

    float zoom_rate = 1.0f;
    Vec2 center(0,0);
    Camera *camera = new Camera();
    camera->setLoc(0,0);

    Layer *l = new Layer();
    moyai_client->insertLayer(l);
    l->setViewport(viewport);
    l->setCamera(camera);

    Texture *t = new Texture();
    t->load( "./assets/base.png" );
    
    TileDeck *deck = new TileDeck();
    deck->setTexture(t);
    deck->setSize(32,32,8,8);

    Prop2D *p=NULL, *pp=NULL;
    Grid *g=NULL;
    CharGrid *cg=NULL;
    
#if 1
    // normal single
    p = new Prop2D();
    p->setDeck(deck);
    p->setIndex(1);
    p->setScl(64,64);
    p->setLoc(0,0);
    l->insertProp(p);

    // with prim
    pp = new Prop2D();
    pp->setScl(1.0f);
    pp->setLoc(100,0);
    pp->addRect( Vec2(0,0), Vec2(-100,-100), Color(0,0,1,0.5) );
    pp->addLine( Vec2(0,0), Vec2(100,100), Color(1,0,0,1) );
    pp->addLine( Vec2(0,0), Vec2(100,-100), Color(0,1,0,1), 5 );
    l->insertProp(pp);
   

    // grid
    g = new Grid(4,4);
    for(int x=0;x<4;x++) {
        for(int y=0;y<4;y++) {
            //        g->set(x,y,80+((x+y)%10));
            g->set(x,y,((x+y)%3));
        }
    }
    g->setXFlip(0,0,true); 
    g->setYFlip(0,1,true);
    g->setUVRot(0,2,true);

    Prop2D *gp = new Prop2D();
    gp->setDeck(deck);
    gp->addGrid(g);
    gp->setScl(32)    ;
    gp->setLoc(50,0);
    gp->setRot(20);
    gp->setIndex(0);
    l->insertProp(gp);

    // uvrot 
    Prop2D *rotp = new Prop2D();
    rotp->setDeck(deck);
    rotp->setScl(32);
    rotp->setLoc(-300,-100);
    rotp->setUVRot(true);
    rotp->setIndex(0);
    l->insertProp(rotp);

    // chargrid
    Texture *ft = new Texture();
    ft->load("./assets/font_only.png");
    TileDeck *fdeck =new TileDeck();
    fdeck->setTexture(ft);
    fdeck->setSize(32,32,8,8);
    cg = new CharGrid(8,8);
    cg->ascii_offset = -32;
    cg->setDeck(fdeck);
    cg->printf(0,0,Color(1,1,1,1), "WHITE" );
    cg->printf(1,1,Color(1,0,0,1), "RED" );
    cg->printf(2,2,Color(0,1,0,1), "GREEN" );
    cg->printf(3,3,Color(0,0,1,1), "BLUE" );
    Prop2D *cgp = new Prop2D();
    cgp->addGrid(cg);
    cgp->setScl(16);
    cgp->setLoc(50,-100);
    l->insertProp(cgp);

    // children
    Prop2D *chp = new Prop2D();
    chp->setLoc(-200,-200);
    chp->setDeck(deck);
    chp->setScl(48);
    chp->setIndex(0);
    for(int i=0;i<8;i++) {
        Prop2D *p = new Prop2D();
        p->setDeck(deck);
        p->setLoc( chp->loc + Vec2( (i+1)*30,0 ) );
        p->setIndex(0);
        p->setScl( 36-i*3 );
        chp->addChild(p);
    }
    
    Prop2D *dynchp = new Prop2D();
    dynchp->setLoc( chp->loc + Vec2(0,-30) );
    dynchp->setIndex(0);
    dynchp->setScl(32);
    dynchp->setDeck(deck);
    chp->addChild(dynchp);
    l->insertProp(chp);
#endif    
    
    // text
    wchar_t charcodes[] = L" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~あいうえお";
    
    Font *font = new Font();
    font->loadFromTTF("./assets/cinecaption227.ttf", charcodes, 24 );
    TextBox *tbs[20];
    for(int i=0;i<20;i++) {
        tbs[i] = new TextBox();
        tbs[i]->setFont(font);
        tbs[i]->setString("A");
        tbs[i]->setScl(1+(float)i/10.0f);
        tbs[i]->setLoc(i*10-250,0);
        l->insertProp(tbs[i]);
    }
    TextBox *movtb = new TextBox();
    movtb->setFont(font);
    movtb->setString("ABCabc\n01234あいうえお");
    movtb->setScl(3);
    movtb->setLoc(0,-150);
    l->insertProp(movtb);

    // multiple viewport and layer
    Viewport *vp2 = new Viewport(); // testing multiple viewport scaling
    vp2->setSize(SCRW*retina,SCRH*retina); 
    vp2->setScale2D(SCRW*2,SCRH*2); 
    Camera *cam2 = new Camera();
    Layer *l2 = new Layer();
    l2->setViewport(vp2);
    l2->setCamera(cam2);
    Prop2D *p2 = new Prop2D();
    p2->setDeck(deck);
    p2->setScl(48,48);
    p2->setIndex(0);
    p2->setLoc(200,-200);
    l2->insertProp(p2);
    moyai_client->insertLayer(l2);

    // main loop

    while( !glfwWindowShouldClose(window) ){
        static int frame_counter = 0;
        static int loop_counter = 0;

        static double last_t = now();
        
        double t = now();
        double dt = t -last_t;
        last_t = t;

        double loop_start_at = t;
                
        frame_counter ++;
        loop_counter++;
        
        Vec2 at(::sin(t)*100,0);
        if(p){
            p->setLoc(at);
            if( loop_counter%21==0 )  p->setIndex( irange(0,3));
            static float rot=0;
            rot+=0.05;
            p->setRot(rot);
            p->setScl( 40 + ::sin(t/2) * 30 );
            if(pp) {
                pp->setRot(rot/2.0f);
            }

            if( loop_counter % 50 == 0 ) {
                float alpha = range(0.2, 1.0f);
                Color col(range(0,1),range(0,1),range(0,1),alpha);
                p->setColor(col);
            }
            if( loop_counter % 120 == 0 ) {
                switch(irange(0,3)) {
                case 0: p->setXFlip( irange(0,2)); break;
                case 1: p->setYFlip( irange(0,2)); break;
                case 2: p->setUVRot( irange(0,2)); break;
                }
            }            
        }

        int cnt = moyai_client->poll(dt);        

        if(g) {
            g->set( irange(0,4), irange(0,4), irange(0,3) );
            g->setColor( irange(0,4), irange(0,4), Color( range(0,1), range(0,1), range(0,1), range(0,1) ) );
        }

        float tbr = 4 + ::sin(t)*3;
        movtb->setScl(tbr);

        Format fmt("%d", loop_counter);
        tbs[19]->setString(fmt.buf);

        
        // fps disp
        static double last_print_at = 0;
        if(last_print_at == 0){
            last_print_at = t;
        } else if( last_print_at < t-1 ){
        fprintf(stderr,"FPS:%d prop:%d drawcall:%d\n", frame_counter, cnt, moyai_client->last_draw_call_count  );
            frame_counter = 0;
            last_print_at = t;
        }

        moyai_client->render();
        //        print("drawcnt:%d", moyai_client->last_draw_call_count );
        if( g_keyboard->getKey( 'Q') ) {
            print("Q pressed");
            exit(0);
            break;
        }
        if( g_keyboard->getKey( 'L' ) ) {
            zoom_rate += 0.2;
            if( zoom_rate > 8 ) zoom_rate = 8;
        }
        if( g_keyboard->getKey( 'K' ) ) {
            zoom_rate -= 0.1;
            if( zoom_rate < 0.1 ) zoom_rate = 0.1;
        }
        viewport->setScale2D(SCRW * zoom_rate,SCRH * zoom_rate); 

        float scrollspeed = 10;
        if( g_keyboard->getKey( 'W' ) ) {
            center.y -= scrollspeed;
        }
        if( g_keyboard->getKey( 'S' ) ) {
            center.y += scrollspeed;
        }
        if( g_keyboard->getKey( 'A' ) ) {
            center.x += scrollspeed;
        }
        if( g_keyboard->getKey( 'D' ) ) {
            center.x -= scrollspeed;
        }
        camera->setLoc(center);

        
        if( g_keyboard->getKey( '1' ) ) {
            for(int i=0;i<50;i++) {
                Prop *p = new Particle(deck);
                l->insertProp(p);
            }
        }
        if( g_keyboard->getKey( '2' ) ) {
            if(cg) cg->printf(0,4, Color(1,1,1,1), Format( "CNT:%d", loop_counter).buf);
        }
        if( loop_counter % 25 == 0 ) {
            if( dynchp ) {
                bool res = chp->clearChild(dynchp);
                assert(res);
                delete dynchp;
                dynchp = NULL;
            } else {
                dynchp = new Prop2D();
                dynchp->setLoc( chp->loc + Vec2(0,-30) );
                dynchp->setIndex(0);
                dynchp->setScl(32);
                dynchp->setDeck(deck);
                chp->addChild(dynchp);
            }            
        }
        
        glfwPollEvents();

        double loop_end_at = now();
        double loop_time = loop_end_at - loop_start_at;
        double ideal_frame_time = 1.0f / 60.0f;
        if(loop_time < ideal_frame_time ) {
            double to_sleep_sec = ideal_frame_time - loop_time;
            int to_sleep_msec = (int) (to_sleep_sec*1000);
            if( to_sleep_msec > 0 ) sleepMilliSec(to_sleep_msec);
        }
    }
    glfwTerminate();

    print("program finished");
    

    return 0;
}
