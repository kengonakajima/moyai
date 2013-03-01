#include <GL/glfw.h>

#include <stdio.h>
#include <assert.h>
#include <strings.h>
#include <math.h>
#include <unistd.h>

#include "moyai.h"

static const int SCRW=966, SCRH=544;

Moyai *g_moyai;
Viewport *g_viewport3d;
Viewport *g_viewport2d;
TileDeck *g_deck;
Layer *g_hud_layer;

Camera *g_hud_camera;

void updateGame() {
    static double last_print_at = 0;
    static int frame_counter = 0;
    static double last_poll_at = now();

    frame_counter ++;
    
    double t = now();
    double dt = t - last_poll_at;
    
    g_moyai->pollAll(dt);

    if( t > last_print_at + 1 ) {
        print("FPS:%d", frame_counter );
        frame_counter = 0;
        last_print_at = t;
    }

    g_moyai->renderAll();
    
    last_poll_at = t;


}
    
int main() {
    g_moyai = new Moyai();

    glfwInit();
    glfwOpenWindow( SCRW,SCRH, 0,0,0,0, 0,0, GLFW_WINDOW );
    glfwSetWindowTitle( "demo3d");
    glfwEnable( GLFW_STICKY_KEYS );
    glfwSwapInterval(1); // vsync

    glClearColor(0,0,0,1);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(true );

    g_viewport2d = new Viewport();
    g_viewport2d->setSize(SCRW,SCRH);
    g_viewport2d->setScale(SCRW,SCRH);

    g_hud_layer = new Layer();
    g_hud_layer->setViewport(g_viewport2d);

    g_hud_camera = new Camera();
    g_hud_camera->setLoc(0,0);

    g_hud_layer->setCamera( g_hud_camera );

    g_moyai->insertLayer( g_hud_layer );
    
    Texture *t = new Texture();
    t->load( "./assets/base.png" );

    g_deck = new TileDeck();
    g_deck->setTexture(t);
    g_deck->setSize(16,16,16,16,256,256);

    Prop *p = new Prop();
    p->setDeck( g_deck );
    p->setIndex(0);
    p->setLoc(0,0);
    p->setScl(32,32);
    g_hud_layer->insertProp(p);

    while(1) {
        int scrw, scrh;
        glfwGetWindowSize( &scrw, &scrh );

        updateGame();
        
        if( glfwGetKey('Q') ) {
            print("Q pressed");
            break;
        }

        
    }
    print("program finished");
    return 0;
}
