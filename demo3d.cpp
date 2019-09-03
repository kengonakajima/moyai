#include <stdio.h>
#include <assert.h>
#include <strings.h>
#include <math.h>
#include <unistd.h>

#include "client.h"

static const int SCRW=966, SCRH=544;

MoyaiClient *g_moyai_client;
Viewport *g_viewport3d;
Viewport *g_viewport2d;
TileDeck *g_deck;
Layer *g_hud_layer;
Layer *g_main_layer;

Camera *g_hud_camera;
Camera *g_main_camera;

Texture *g_wood_tex;
TileDeck *g_wood_deck;
Texture *g_sol_tex;

Mesh *g_colmesh;
Mesh *g_texmesh;
Mesh *g_texcolmesh;
Mesh *g_texnormmesh;


Light *g_light;
Material *g_material;

Prop3D *g_prop_col;
Prop3D *g_prop_texnorm;
Prop3D *g_prop_texcol;
Prop3D *g_prop_with_children;
Prop3D *g_children[3];

GLFWwindow *g_window;

void updateGame() {
    static double last_print_at = 0;
    static int frame_counter = 0;

    frame_counter ++;

    static double last_t=now();
    double t = now();
    double dt = t - last_t;
    last_t = t;
    
    g_moyai_client->poll(dt);

    if( t > last_print_at + 1 ) {
        print("FPS:%d", frame_counter );
        frame_counter = 0;
        last_print_at = t;
    }

    // move camera
    //    Vec3 campos = g_main_camera->loc;
    //    g_main_camera->setLoc( campos + Vec3(dt,0,0));    
    
    // props
    if( g_prop_col ){
        g_prop_col->loc.x += dt/10;
        g_prop_col->rot.z += dt*100;
        g_prop_col->rot.y += dt*100;        
    }
    if( g_prop_texnorm ){
        g_prop_texnorm->loc.x -= dt/10;
        g_prop_texnorm->rot.x -= dt*50;
        g_prop_texnorm->rot.z -= dt*30;
    }
    if( g_prop_texcol ){
        g_prop_texcol->loc.y += dt/10;
        g_prop_texcol->rot.x -= dt*100;
        g_prop_texcol->rot.z -= dt*100;        
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
    
    g_moyai_client->render();
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

    // for cube without normals (counter clockwise)
    IndexBufferType cube_indexes_wo_normals[36] = {
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

    IndexBuffer *cube_ib_wo_normals = new IndexBuffer();
    cube_ib_wo_normals->set(cube_indexes_wo_normals,36);
    

    // for cube with normals (counter clockwise)
    IndexBufferType cube_indexes_with_normals[36] = {
        // bottom ABCD
        0,3,1, // ADB 
        3,2,1, // DCB
        
        // top EFGH 
        7,5,6, // HFG
        4,5,7, // EFH

        // left ADHE : 8,9,10,11
        11,9,8, // EDA
        11,10,9, // EHD

        // right BCGH : 12,13,14,15
        15,12,13, // FBC
        15,13,14, // FCG

        // front ABFE : 16,17,18,19
        19,16,17, // EAB
        19,17,18, // EBF

        // rear DCGH : 20,21,22,23
        23,21,20, // HCD
        23,22,21, // HGC
    };

    IndexBuffer *cube_ib_with_normals = new IndexBuffer();
    cube_ib_with_normals->set(cube_indexes_with_normals,36);
    
    // cube with tex and normal
    VertexFormat *texnormvf = new VertexFormat();
    texnormvf->declareCoordVec3();
    texnormvf->declareUV();
    texnormvf->declareNormal();    


    VertexBuffer *texnormvb = new VertexBuffer();
    texnormvb->setFormat(texnormvf);
    texnormvb->reserve(6*4); // need normals for A~H for flat shading

    Vec3 _a(-d,-d,d);  // A red
    Vec3 _b(d,-d,d); // B blue
    Vec3 _c(d,-d,-d); // C yellow
    Vec3 _d(-d,-d,-d); // D green
    Vec3 _e(-d,d,d); // E white
    Vec3 _f(d,d,d); // F purple
    Vec3 _g(d,d,-d); // G white
    Vec3 _h(-d,d,-d); // H white

    // ABCD
    texnormvb->setCoord( 0, _a); texnormvb->setCoord( 1, _b); texnormvb->setCoord( 2, _c); texnormvb->setCoord( 3, _d); 
    // EFGH
    texnormvb->setCoord( 4, _e); texnormvb->setCoord( 5, _f); texnormvb->setCoord( 6, _g); texnormvb->setCoord( 7, _h);
    // ADHE
    texnormvb->setCoord( 8, _a); texnormvb->setCoord( 9, _d); texnormvb->setCoord(10, _h); texnormvb->setCoord(11, _e);
    // BCGF
    texnormvb->setCoord(12, _b); texnormvb->setCoord(13, _c); texnormvb->setCoord(14, _g); texnormvb->setCoord(15, _f);
    // ABFE
    texnormvb->setCoord(16, _a); texnormvb->setCoord(17, _b); texnormvb->setCoord(18, _f); texnormvb->setCoord(19, _e);
    // DCGH
    texnormvb->setCoord(20, _d); texnormvb->setCoord(21, _c); texnormvb->setCoord(22, _g); texnormvb->setCoord(23, _h);        

    Vec2 uv_a(0,0);
    Vec2 uv_b(1,0);
    Vec2 uv_c(1,1);
    Vec2 uv_d(0,1);
    Vec2 uv_e(0,0);
    Vec2 uv_f(1,0);
    Vec2 uv_g(1,1);
    Vec2 uv_h(0,1);

    // ABCD 
    texnormvb->setUV( 0, uv_a ); texnormvb->setUV( 1, uv_b ); texnormvb->setUV( 2, uv_c ); texnormvb->setUV( 3, uv_d );
    // EFGH
    texnormvb->setUV( 4, uv_e ); texnormvb->setUV( 5, uv_f ); texnormvb->setUV( 6, uv_g ); texnormvb->setUV( 7, uv_h );
    // ADHE
    texnormvb->setUV( 8, uv_a ); texnormvb->setUV( 9, uv_d ); texnormvb->setUV(10, uv_h ); texnormvb->setUV(11, uv_e );
    // BCGF
    texnormvb->setUV(12, uv_b ); texnormvb->setUV(13, uv_c ); texnormvb->setUV(14, uv_g ); texnormvb->setUV(15, uv_f );
    // ABFE
    texnormvb->setUV(16, uv_a ); texnormvb->setUV(17, uv_b ); texnormvb->setUV(18, uv_f ); texnormvb->setUV(19, uv_e );
    // DCGH
    texnormvb->setUV(20, uv_d ); texnormvb->setUV(21, uv_c ); texnormvb->setUV(22, uv_g ); texnormvb->setUV(23, uv_h );

    Vec3 abcd_norm(0,-1,0), efgh_norm(0,1,0), adhe_norm(-1,0,0), bcgf_norm(1,0,0), abfe_norm(0,0,1), dcgh_norm(0,0,-1);

    // ABCD
    texnormvb->setNormal(0,abcd_norm); texnormvb->setNormal(1,abcd_norm); texnormvb->setNormal(2,abcd_norm); texnormvb->setNormal(3,abcd_norm);
    // EFGH
    texnormvb->setNormal(4,efgh_norm); texnormvb->setNormal(5,efgh_norm); texnormvb->setNormal(6,efgh_norm); texnormvb->setNormal(7,efgh_norm);
    // ADHE
    texnormvb->setNormal(8,adhe_norm); texnormvb->setNormal(9,adhe_norm); texnormvb->setNormal(10,adhe_norm); texnormvb->setNormal(11,adhe_norm);
    // BCGF
    texnormvb->setNormal(12,bcgf_norm); texnormvb->setNormal(13,bcgf_norm); texnormvb->setNormal(14,bcgf_norm); texnormvb->setNormal(15,bcgf_norm);
    // ABFE
    texnormvb->setNormal(16,abfe_norm); texnormvb->setNormal(17,abfe_norm); texnormvb->setNormal(18,abfe_norm); texnormvb->setNormal(19,abfe_norm);
    // DCGH
    texnormvb->setNormal(20,dcgh_norm); texnormvb->setNormal(21,dcgh_norm); texnormvb->setNormal(22,dcgh_norm); texnormvb->setNormal(23,dcgh_norm);
    



    g_colmesh = new Mesh();
    g_colmesh->setVertexBuffer(colvb);
    g_colmesh->setIndexBuffer(cube_ib_wo_normals);
    g_colmesh->setPrimType( GL_TRIANGLES );

    g_texmesh = new Mesh();
    g_texmesh->setVertexBuffer(texvb);
    g_texmesh->setIndexBuffer(cube_ib_wo_normals);
    g_texmesh->setPrimType( GL_TRIANGLES );

    g_texcolmesh = new Mesh();
    g_texcolmesh->setVertexBuffer(texcolvb);
    g_texcolmesh->setIndexBuffer(cube_ib_wo_normals);
    g_texcolmesh->setPrimType( GL_TRIANGLES );

    g_texnormmesh = new Mesh();
    g_texnormmesh->setVertexBuffer(texnormvb);
    g_texnormmesh->setIndexBuffer(cube_ib_with_normals);
    g_texnormmesh->setPrimType(GL_TRIANGLES);


    g_wood_tex = new Texture();
    g_wood_tex->load( "assets/wood256.png" );
    g_wood_deck = new TileDeck();
    g_wood_deck->setTexture(g_wood_tex);
    g_wood_deck->setSize(1,1,256,256);
        
    g_sol_tex = new Texture();
    g_sol_tex->load( "assets/dragon8.png" );
    
    

    g_prop_col = new Prop3D();
    g_prop_col->setMesh(g_colmesh);
    g_prop_col->setScl(Vec3(1,1,1));
    g_prop_col->setLoc(Vec3(0,0,0));
    g_main_layer->insertProp(g_prop_col);


    g_prop_texnorm = new Prop3D();
    g_prop_texnorm->setMesh(g_texnormmesh);
    g_prop_texnorm->setScl(Vec3(1,1,1));
    g_prop_texnorm->setLoc(Vec3(0,0,0));
    g_prop_texnorm->setDeck( g_wood_deck);
    g_prop_texnorm->setMaterial(g_material);
    g_main_layer->insertProp(g_prop_texnorm);

    g_prop_texcol = new Prop3D();
    g_prop_texcol->setMesh(g_texcolmesh );
    g_prop_texcol->setScl(Vec3(1,1,1));
    g_prop_texcol->setLoc(Vec3(0,0,0));
    g_prop_texcol->setDeck(g_wood_deck);
    g_main_layer->insertProp(g_prop_texcol);



    if(1){
        g_children[0] = new Prop3D();
        g_children[0]->setMesh( g_colmesh );
        g_children[0]->setScl( Vec3(0.2,0.2,0.2) );
        g_children[0]->setLoc( Vec3(1,0,0) );
        g_children[1] = new Prop3D();
        g_children[1]->setMesh( g_texmesh );
        g_children[1]->setScl( Vec3(0.2,0.2,0.2) );
        g_children[1]->setLoc( Vec3(0,0,1) );
        g_children[1]->setDeck( g_wood_deck );
        g_children[2] = new Prop3D();
        g_children[2]->setMesh( g_texcolmesh );
        g_children[2]->setScl( Vec3(0.2,0.2,0.2) );
        g_children[2]->setLoc( Vec3(1,0,0) );
        g_children[2]->setDeck( g_wood_deck );

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

void winclose_callback( GLFWwindow *w ){
    exit(0);
}


int main() {
    


    glfwInit();
    g_window = glfwCreateWindow( SCRW,SCRH, "demo3d", NULL, NULL );
    if(!g_window) {
        print("can't create window");
        return 1;
    }
    glfwMakeContextCurrent(g_window);
    glfwSetWindowCloseCallback( g_window, winclose_callback );
    glfwSetInputMode( g_window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSwapInterval(1); // 1 for use vsync

    glClearColor(0,0,0,1);
    
    glEnable(GL_DEPTH_TEST);    
    glEnable(GL_DEPTH_BUFFER_BIT);    

    g_moyai_client = new MoyaiClient(g_window, SCRW, SCRH);
    // 3d
    g_viewport3d = new Viewport();
    int retina = 1;
#if defined(__APPLE__)
    retina = 2;
#endif    
    g_viewport3d->setSize(SCRW*retina,SCRH*retina);
    g_viewport3d->setClip3D( 0.01, 100 );
    g_main_layer = new Layer();
    g_main_layer->setViewport(g_viewport3d);    
    g_main_camera = new Camera();
    g_main_camera->setLoc(0.001,0,3);
    g_main_camera->setLookAt(Vec3(0,0,0), Vec3(0,1,0));    
    g_main_layer->setCamera( g_main_camera );


    // 2d
    g_viewport2d = new Viewport();
    g_viewport2d->setSize(SCRW*retina,SCRH*retina);
    g_viewport2d->setScale2D(SCRW,SCRH);
    g_hud_layer = new Layer();
    g_hud_layer->setViewport(g_viewport2d);
    g_hud_camera = new Camera();
    g_hud_camera->setLoc(0,0);
    g_hud_layer->setCamera( g_hud_camera );


    // draw 2d after 3d
    g_moyai_client->insertLayer( g_main_layer );    
    g_moyai_client->insertLayer( g_hud_layer );

    
    Texture *t = new Texture();
    t->load( "./assets/base.png" );

    g_deck = new TileDeck();
    g_deck->setTexture(t);
    g_deck->setSize(32,32,8,8);

    Prop2D *p = new Prop2D();
    p->setDeck( g_deck );
    p->setIndex(0);
    p->setLoc(200,200);
    p->setScl(32,32);
    g_hud_layer->insertProp(p);


    // light and material
    g_light = new Light();
    g_light->pos = Vec3(-50,50,100);
    g_main_layer->setLight(g_light);    
    g_material = new Material();
    g_material->ambient = Color(0.3,0.3,0.3,1);

    setupCube();

    
    while(!glfwWindowShouldClose(g_window)) {
        int scrw, scrh;
        glfwGetFramebufferSize( g_window, &scrw, &scrh );

        updateGame();
        
        if( glfwGetKey( g_window, 'Q') ) {
            print("Q pressed");
            break;
        }
        glfwPollEvents();
    }
    print("program finished");
    return 0;
}
