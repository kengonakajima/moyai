﻿#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <locale.h>

#ifndef WIN32
#include <strings.h>
#endif

#include "client.h"





// config

static const int SCRW=966, SCRH=544;



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

//

int main(int argc, char **argv )
{
    

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
#ifdef WIN32
	glewInit();
#endif
    glClearColor(0.2,0.2,0.2,1);

    MoyaiClient *moyai_client = new MoyaiClient(window);

    Viewport *viewport = new Viewport();
    int retina = 1;
#if defined(__APPLE__)
    retina = 2;
#endif    
    viewport->setSize(SCRW*retina,SCRH*retina); // set actual framebuffer size to output
    viewport->setScale2D(SCRW,SCRH); // set scale used by props that will be rendered

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

    // normal single
    Prop2D *p = new Prop2D();
    p->setDeck(deck);
    p->setIndex(1);
    p->setScl(64,64);
    p->setLoc(0,0);
    l->insertProp(p);

    // with prim
    Prop2D *pp = new Prop2D();
    pp->setScl(1.0f);
    pp->setLoc(100,0);
    pp->addRect( Vec2(0,0), Vec2(-100,-100), Color(0,0,1,0.5) );
    pp->addLine( Vec2(0,0), Vec2(100,100), Color(1,0,0,1) );
    pp->addLine( Vec2(0,0), Vec2(100,-100), Color(0,1,0,1), 5 );
    l->insertProp(pp);
   

    // grid
    Grid *g = new Grid(4,4);
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


    // chargrid
    Texture *ft = new Texture();
    ft->load("./assets/font_only.png");
    TileDeck *fdeck =new TileDeck();
    fdeck->setTexture(ft);
    fdeck->setSize(32,32,8,8);
    CharGrid *cg = new CharGrid(8,8);
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
    
    // text
    wchar_t charcodes[] = L" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    
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
    movtb->setString("ABCabc\n01234");
    movtb->setScl(3);
    movtb->setLoc(0,-150);
    l->insertProp(movtb);

    //
    

    while( !glfwWindowShouldClose(window) ){
        static double last_print_at = 0;
        static int frame_counter = 0;
        static double last_poll_at = now();
        static int loop_counter = 0;
        
        double t = now();
        double dt = t - last_poll_at;
        last_poll_at = t;
        
        frame_counter ++;
        loop_counter++;
        
        Vec2 at(::sin(t)*100,0);
        p->setLoc(at);
        if( loop_counter%21==0 )  p->setIndex( irange(0,3));
        static float rot=0;
        rot+=0.05;
        p->setRot(rot);
        pp->setRot(rot/2.0f);
        p->setScl( 40 + ::sin(t/2) * 30 );
        int cnt = moyai_client->poll(dt);
        

        if( loop_counter % 50 == 0 ) {
            float alpha = range(0.2, 1.0f);
            Color col(range(0,1),range(0,1),range(0,1),alpha);
            p->setColor(col);
        }
        if( loop_counter % 120 == 0 ) {
            switch(irange(0,3)) {
            case 0: p->setXFlip( irange(0,2)); print("XFL"); break;
            case 1: p->setYFlip( irange(0,2)); print("YFL"); break;
            case 2: p->setUVRot( irange(0,2)); print("UVROT"); break;
            }
        }

        g->set( irange(0,4), irange(0,4), irange(0,3) );
        g->setColor( irange(0,4), irange(0,4), Color( range(0,1), range(0,1), range(0,1), range(0,1) ) );

        float tbr = 4 + ::sin(t)*3;
        movtb->setScl(tbr);

        Format fmt("%d", loop_counter);
        tbs[19]->setString(fmt.buf);

        
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
        if( glfwGetKey( window, 'Q') ) {
            print("Q pressed");
            exit(0);
            break;
        }
        if( glfwGetKey( window, 'L' ) ) {
            camera->setLoc(100,100);
        } else {
            camera->setLoc(0,0);
        }
        if( glfwGetKey( window, 'S' ) ) {
            viewport->setScale2D(SCRW/2,SCRH/2); 
        } else {
            viewport->setScale2D(SCRW,SCRH); 
        }

        
        if( glfwGetKey( window, '1' ) ) {
            for(int i=0;i<50;i++) {
                Prop *p = new Particle(deck);
                l->insertProp(p);
            }
        }
        if( glfwGetKey( window, '2' ) ) {
            cg->printf(0,4, Color(1,1,1,1), Format( "CNT:%d", loop_counter).buf);
        }
        
        glfwPollEvents();
    }
    glfwTerminate();

    print("program finished");
    

    return 0;
}