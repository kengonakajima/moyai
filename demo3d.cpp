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
Layer *g_main_layer;

Camera *g_hud_camera;
Camera *g_main_camera;

Texture *g_wood_tex;
Texture *g_sol_tex;

Mesh *g_colmesh;
Mesh *g_texmesh;
Mesh *g_texcolmesh;
Mesh *g_billboardmesh;



Prop3D *g_prop_col;
Prop3D *g_prop_tex;
Prop3D *g_prop_texcol;
Prop3D *g_prop_billboard;
Prop3D *g_prop_with_children;
Prop3D *g_children[3];

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

    // move camera
    Vec3 campos = g_main_camera->loc;
    g_main_camera->setLoc( campos + Vec3(dt,0,0));    

    if( g_prop_col ){
        g_prop_col->loc.x += dt/10;
        g_prop_col->rot.z += dt*100;
        g_prop_col->rot.y += dt*100;        
    }

    if( g_prop_tex ){
        g_prop_tex->loc.x -= dt/10;
        g_prop_tex->rot.x -= dt*50;
        g_prop_tex->rot.z -= dt*30;
    }

    if( g_prop_texcol ){
        g_prop_texcol->loc.y += dt/10;
        g_prop_texcol->rot.x -= dt*100;
        g_prop_texcol->rot.z -= dt*100;        
    }
    if( g_prop_billboard ) {
        g_prop_billboard->loc.x = cos(g_prop_billboard->accum_time);
        g_prop_billboard->loc.y = sin(g_prop_billboard->accum_time);
        g_prop_billboard->loc.z = 0;//cos(g_prop_billboard->accum_time*2);
        //        g_prop_billboard->rot.x = g_prop_billboard->accum_time * 100;
    }


    if( g_prop_with_children ) {
        g_prop_with_children->loc.y = sin(g_prop_with_children->accum_time*0.5)*1.2;
        g_prop_with_children->loc.x = cos(g_prop_with_children->accum_time*0.5)*1.2;
        for(int i=0;i<elementof(g_children);i++) {
            Prop3D *child = g_children[i];
            //            child->loc.y = sin(g_prop_with_children->accum_time*(2+i) )/7.0;
            //            child->loc.x = cos(g_prop_with_children->accum_time*(2+i) )/7.0;
            child->rot.x = child->accum_time * 500;
            child->rot.z = child->accum_time * 500;            
        }
    }
    //    print("propx:%f r:%f", g_prop_0->loc.x, g_prop_0->rot3d.z );
    
    g_moyai->renderAll();
    
    last_poll_at = t;
}

void setupCube() {
    // colored cube
    VertexFormat *colvf = new VertexFormat();
    colvf->declareCoordVec3();
    colvf->declareColor();


    VertexBuffer *colvb = new VertexBuffer();
    colvb->setFormat(colvf);
    colvb->reserve(8);


    // 
    //
    //   +y
    //    ^
    //                     d,d,-d
    //     H ------------- G
    //    /|              /|
    //   / |             / |
    //  E ------------- F  |
    //  |  |            |  |      -z               7   6
    //  |  |            |  |      /               4   5
    //  |  D -----------|- C
    //  | /             | /
    //  |/              |/                         3   2
    //  A ------------- B     >   +x              0   1
    //  -d,-d,d


    float d = 0.2; 
    colvb->setCoord( 0, Vec3(-d,-d,d) );  // A red
    colvb->setCoord( 1, Vec3(d,-d,d) ); // B blue
    colvb->setCoord( 2, Vec3(d,-d,-d) ); // C yellow
    colvb->setCoord( 3, Vec3(-d,-d,-d) ); // D green
    colvb->setCoord( 4, Vec3(-d,d,d) ); // E white
    colvb->setCoord( 5, Vec3(d,d,d) ); // F purple
    colvb->setCoord( 6, Vec3(d,d,-d) ); // G white
    colvb->setCoord( 7, Vec3(-d,d,-d) ); // H white    
    
    colvb->setColor( 0, Color(1,0,0,1) );
    colvb->setColor( 1, Color(0,0,1,1) );
    colvb->setColor( 2, Color(1,1,0,1) );
    colvb->setColor( 3, Color(0,1,0,1) );
    colvb->setColor( 4, Color(1,1,1,1) );
    colvb->setColor( 5, Color(1,0,1,1) );    
    colvb->setColor( 6, Color(1,1,1,1) );    
    colvb->setColor( 7, Color(1,1,1,1) );        
    
    // cube with tex, no color

    VertexFormat *texvf = new VertexFormat();
    texvf->declareCoordVec3();
    texvf->declareUV();

    VertexBuffer *texvb = new VertexBuffer();
    texvb->setFormat(texvf);
    texvb->reserve(8);

    texvb->setCoord( 0, Vec3(-d,-d,d) );  // A
    texvb->setCoord( 1, Vec3(d,-d,d) ); // B 
    texvb->setCoord( 2, Vec3(d,-d,-d) ); // C 
    texvb->setCoord( 3, Vec3(-d,-d,-d) ); // D 
    texvb->setCoord( 4, Vec3(-d,d,d) ); // E     
    texvb->setCoord( 5, Vec3(d,d,d) ); // F 
    texvb->setCoord( 6, Vec3(d,d,-d) ); // G 
    texvb->setCoord( 7, Vec3(-d,d,-d) ); // H     
    
    texvb->setUV( 0, Vec2(0,0) );
    texvb->setUV( 1, Vec2(1,0) );
    texvb->setUV( 2, Vec2(1,1) );
    texvb->setUV( 3, Vec2(0,1) );
    texvb->setUV( 4, Vec2(0,0) );
    texvb->setUV( 5, Vec2(1,0) );
    texvb->setUV( 6, Vec2(1,1) );
    texvb->setUV( 7, Vec2(0,1) );



    // cube with tex and color 
    VertexFormat *texcolvf = new VertexFormat();
    texcolvf->declareCoordVec3();
    texcolvf->declareColor();
    texcolvf->declareUV();


    VertexBuffer *texcolvb = new VertexBuffer();
    texcolvb->setFormat(texcolvf);
    texcolvb->reserve(8);

    texcolvb->setCoord( 0, Vec3(-d,-d,d) );  // A red
    texcolvb->setCoord( 1, Vec3(d,-d,d) ); // B blue
    texcolvb->setCoord( 2, Vec3(d,-d,-d) ); // C yellow
    texcolvb->setCoord( 3, Vec3(-d,-d,-d) ); // D green
    texcolvb->setCoord( 4, Vec3(-d,d,d) ); // E white
    texcolvb->setCoord( 5, Vec3(d,d,d) ); // F purple
    texcolvb->setCoord( 6, Vec3(d,d,-d) ); // G white
    texcolvb->setCoord( 7, Vec3(-d,d,-d) ); // H white

    texcolvb->setUV( 0, Vec2(0,0) );
    texcolvb->setUV( 1, Vec2(1,0) );
    texcolvb->setUV( 2, Vec2(1,1) );
    texcolvb->setUV( 3, Vec2(0,1) );
    texcolvb->setUV( 4, Vec2(0,0) );
    texcolvb->setUV( 5, Vec2(1,0) );
    texcolvb->setUV( 6, Vec2(1,1) );
    texcolvb->setUV( 7, Vec2(0,1) );    

    texcolvb->setColor( 0, Color(1,0,0,0.5) );
    texcolvb->setColor( 1, Color(0,0,1,0.5) );
    texcolvb->setColor( 2, Color(1,1,0,0.5) );
    texcolvb->setColor( 3, Color(0,1,0,0.5) );
    texcolvb->setColor( 4, Color(1,1,1,0.5) );
    texcolvb->setColor( 5, Color(1,0,1,0.5) );    
    texcolvb->setColor( 6, Color(1,1,1,0.5) );    
    texcolvb->setColor( 7, Color(1,1,1,0.5) );        
    

    
    // for cube (counter clockwise)
    int cube_indexes[36] = {
        // bottom
        0,3,1, // ADB 
        3,2,1, // DCB
        // top
        7,5,6, // HFG
        4,5,7, // EFH


        // left
        4,3,0, // EDA
        4,7,3, // EHD

        // right
        5,1,2, // FBC
        5,2,6, // FCG

        // front
        4,0,1, // EAB
        4,1,5, // EBF

        // rear
        7,2,3, // HCD
        7,6,2, // HGC
    };

    IndexBuffer *cube_ib = new IndexBuffer();
    cube_ib->set(cube_indexes,36);

    // for billboards
    int board_indexes[6] = {
        0,1,4, // ABE
        1,5,4, // BFE
    };

    IndexBuffer *board_ib = new IndexBuffer();
    board_ib->set(board_indexes,6);

    g_colmesh = new Mesh();
    g_colmesh->setVertexBuffer(colvb);
    g_colmesh->setIndexBuffer(cube_ib);
    g_colmesh->setPrimType( GL_TRIANGLES );

    g_texmesh = new Mesh();
    g_texmesh->setVertexBuffer(texvb);
    g_texmesh->setIndexBuffer(cube_ib);
    g_texmesh->setPrimType( GL_TRIANGLES );

    g_texcolmesh = new Mesh();
    g_texcolmesh->setVertexBuffer(texcolvb);
    g_texcolmesh->setIndexBuffer(cube_ib);
    g_texcolmesh->setPrimType( GL_TRIANGLES );

    g_billboardmesh = new Mesh();
    g_billboardmesh->setVertexBuffer(texvb);
    g_billboardmesh->setIndexBuffer( board_ib );
    g_billboardmesh->setPrimType( GL_TRIANGLES );

    g_wood_tex = new Texture();
    g_wood_tex->load( "assets/wood256.png" );
    g_sol_tex = new Texture();
    g_sol_tex->load( "assets/sol.png" );    
    

    g_prop_col = new Prop3D();
    g_prop_col->setMesh(g_colmesh);
    g_prop_col->setScl(Vec3(1,1,1));
    g_prop_col->setLoc(Vec3(0,0,0));
    g_main_layer->insertProp(g_prop_col);


    g_prop_tex = new Prop3D();
    g_prop_tex->setMesh(g_texmesh);
    g_prop_tex->setScl(Vec3(1,1,1));
    g_prop_tex->setLoc(Vec3(0,0,0));
    g_prop_tex->setTexture( g_wood_tex);
    g_main_layer->insertProp(g_prop_tex);

    g_prop_texcol = new Prop3D();
    g_prop_texcol->setMesh(g_texcolmesh );
    g_prop_texcol->setScl(Vec3(1,1,1));
    g_prop_texcol->setLoc(Vec3(0,0,0));
    g_prop_texcol->setTexture(g_wood_tex);
    g_main_layer->insertProp(g_prop_texcol);


    g_prop_billboard = new Prop3D();
    g_prop_billboard->setMesh( g_billboardmesh);
    g_prop_billboard->setScl(Vec3(0.5,0.5,0.5));
    g_prop_billboard->setLoc(Vec3(0,0,0));
    g_prop_billboard->setTexture(g_sol_tex);
    g_prop_billboard->setBillboardIndex(0);
    g_main_layer->insertProp(g_prop_billboard);

    if(1){
        g_children[0] = new Prop3D();
        g_children[0]->setMesh( g_colmesh );
        g_children[0]->setScl( Vec3(0.2,0.2,0.2) );
        g_children[0]->setLoc( Vec3(1,0,0) );
        g_children[1] = new Prop3D();
        g_children[1]->setMesh( g_texmesh );
        g_children[1]->setScl( Vec3(0.2,0.2,0.2) );
        g_children[1]->setLoc( Vec3(0,0,1) );
        g_children[1]->setTexture( g_wood_tex );
        g_children[2] = new Prop3D();
        g_children[2]->setMesh( g_texcolmesh );
        g_children[2]->setScl( Vec3(0.2,0.2,0.2) );
        g_children[2]->setLoc( Vec3(1,0,0) );
        g_children[2]->setTexture( g_wood_tex );

        g_prop_with_children = new Prop3D();
        g_prop_with_children->reserveChildren(3);
                g_prop_with_children->addChild(g_children[0]);
        //        g_prop_with_children->addChild(g_children[1]);
        //        g_prop_with_children->addChild(g_children[2]);
        g_prop_with_children->setLoc( Vec3(0.1,0,0 ) );
        g_prop_with_children->setScl( Vec3(1,1,1) );
        g_main_layer->insertProp( g_prop_with_children );
    }
        
    
    
}

void memTestDebug() {
    int objcnt;
    
    char *pointers[100000];
    for(int i=0;i<elementof(pointers);i++){
        size_t sz = irange(1,10)*103;
        pointers[i] = (char*) MALLOC(sz); 
        memset(pointers[i],0xff,sz);
    }
    objcnt = cuminoPrintMemStat(1); 
    assertmsg( objcnt == 100000, "real:%d", objcnt );    
    
    for(int i=0;i<elementof(pointers)-1;i++){
        FREE(pointers[i]);
    }

    objcnt = cuminoPrintMemStat(1); 
    assertmsg( objcnt == 1, "real:%d", objcnt );
    
    Vec3 *vecs[100000];
    for(int i=0;i<elementof(vecs);i++){
        vecs[i] = new Vec3(0,0,0);
    }

    objcnt = cuminoPrintMemStat(1); 
    assertmsg( objcnt == 100001, "real:%d", objcnt );
    
    for(int i=0;i<elementof(vecs)-1;i++){
        delete vecs[i];
    }
    objcnt = cuminoPrintMemStat(1); 
    assert( objcnt == 2 );

    FREE( pointers[elementof(pointers)-1] );
    delete vecs[elementof(vecs)-1];

    objcnt = cuminoPrintMemStat(1); 
    assert( objcnt == 0 );
}

int main() {
        g_cumino_mem_debug = true;
        memTestDebug();
    
    g_moyai = new Moyai();

    glfwInit();
    glfwOpenWindow( SCRW,SCRH, 8,8,8,8, 16,0, GLFW_WINDOW );
    glfwSetWindowTitle( "demo3d");
    glfwEnable( GLFW_STICKY_KEYS );
    glfwSwapInterval(1); // vsync

    glClearColor(0,0,0,1);

    glEnable(GL_DEPTH_TEST);    
    glEnable(GL_DEPTH_BUFFER_BIT);    
    glDepthMask(true );

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // 3d
    g_viewport3d = new Viewport();
    g_viewport3d->setSize(SCRW,SCRH);
    g_viewport3d->setClip3D( 0.01, 100 );
    g_main_layer = new Layer();
    g_main_layer->setViewport(g_viewport3d);    
    g_main_camera = new Camera();
    g_main_camera->setLoc(0.001,0,3);
    g_main_camera->setLookAt(Vec3(0,0,0), Vec3(0,1,0));    
    g_main_layer->setCamera( g_main_camera );


    // 2d
    g_viewport2d = new Viewport();
    g_viewport2d->setSize(SCRW,SCRH);
    g_viewport2d->setScale2D(SCRW,SCRH);
    g_hud_layer = new Layer();
    g_hud_layer->setViewport(g_viewport2d);
    g_hud_camera = new Camera();
    g_hud_camera->setLoc(0,0);
    g_hud_layer->setCamera( g_hud_camera );


    // draw 2d after 3d
    g_moyai->insertLayer( g_main_layer );    
    g_moyai->insertLayer( g_hud_layer );

    
    Texture *t = new Texture();
    t->load( "./assets/base.png" );

    g_deck = new TileDeck();
    g_deck->setTexture(t);
    g_deck->setSize(16,16,16,16,256,256);

    Prop2D *p = new Prop2D();
    p->setDeck( g_deck );
    p->setIndex(0);
    p->setLoc(200,200);
    p->setScl(32,32);
    g_hud_layer->insertProp(p);


    setupCube();

    
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
