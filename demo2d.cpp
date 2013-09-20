
#include <stdio.h>
#include <assert.h>
#include <math.h>

#ifndef WIN32
#include <strings.h>
#endif

//#include <GL/glfw.h>

#include "client.h"


MoyaiClient *g_moyai_client;
Viewport *g_viewport;
Layer *g_main_layer;
Texture *g_base_atlas;
TileDeck *g_base_deck;
Camera *g_camera;

Texture *g_bmpfont_atlas;
TileDeck *g_bmpfont_deck;

ColorReplacerShader *g_replacer_shader;


TextBox *g_tb;

AnimCurve *g_bullet_anim_curve;

SoundSystem *g_sound_system;
Sound *g_explosion_sound;

Image *g_img;
Texture *g_dyn_texture;

Prop2D *g_linep;

#define SCALE  2

int g_last_render_cnt ;

// data

enum {
    ATLAS_TERAZI = 0,
    ATLAS_TARKEN = 1,
    ATLAS_POWSTAR = 2,
    ATLAS_BULLET0 = 16,  
    ATLAS_ZOSHI0 = 32,
    ATLAS_EXPLOSION = 48,
    ATLAS_MYSHIP = 64,
    ATLAS_BEAM = 65,
    ATLAS_DEBRI0 = 66,
    ATLAS_STAR0 = 70,   
    ATLAS_DIGIT0 = 80,   /* 1:82,2:83,3:84,4:85,5:86,6:87,7:88,8:89,9:90 */
};

// config

static const int SCRW=966, SCRH=544;

class Char : public Prop2D {
public:
    Vec2 v;
    unsigned int cnt;

    Char( float x, float y, int ind ) {
        loc.x=x;
        loc.y=y;
        index=ind;
        v.x = v.y = 0;
        cnt = 0;
    }


    void aim(float aimx, float aimy, float vel ) {
        float dx = (aimx - loc.x), dy = (aimy - loc.y);
        normalize(&dx,&dy, vel);
        v.x = dx;
        v.y = dy;        
    }
    float distanceTo( Char *ch) {
        return len(loc.x,loc.y,ch->loc.x,ch->loc.y);
    }
    
    bool hit( Char*ch, float dia){
        return (loc.x > ch->loc.x - dia && loc.x < ch->loc.x + dia && loc.y > ch->loc.y - dia && loc.y < ch->loc.y + dia );
    }
    virtual void onHit( Char *ch){
        print("mover:onhit %p\n",ch);
    }

    virtual bool prop2DPoll(double dt){
        loc.x += v.x * dt * SCALE;
        loc.y += v.y * dt * SCALE;

        cnt ++;
        if( charPoll(dt) == false ){
            return false;
        }
        return true;
    }
    virtual bool charPoll(double dt){
        print("charpoll is not defined?");
        return true;
    }
    
};

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////



Pad *g_pad;


class MyShip;    
MyShip *g_pc;



class Enemy : public Char {
public:
    int hp;
    int level;
    Enemy(float x, float y, int index, int level ) : Char(x,y,index), level(level) {
        
    }
    virtual bool charPoll(double dt){
        if(enemyPoll(dt) == false ){
            return false;
        }
        return true;
    }
    virtual bool enemyPoll(double dt){
        return true;
    }
};



class Explosion : public Char {
public:
    float startScl;
    Explosion( float x, float y, float stscl ) : Char( x,y,ATLAS_EXPLOSION ), startScl(stscl) {
        assert(g_base_deck);
        setDeck(g_base_deck);
    }
    virtual bool charPoll(double dt){
        //        print("expl: scl:%f %d %f", scl.x, id , accumTime );
        float dur = 0.3;
        scl.y = scl.x = ( dur - accum_time )*startScl * 64;
        if(scl.x<0){
            return false;
        }
        return true;
    }
};
Explosion *createExplosion(float x, float y, float startScl ){
    assert(g_main_layer);
    Explosion *e = new Explosion(x,y,startScl);
    g_main_layer->insertProp(e);
    g_explosion_sound->play( range(0.1,1) );
    return e;
}



///////////////

class Bullet : public Enemy {
public:
    double last_explode_at;
    
    Bullet( float x, float y, float aimx, float aimy, int level, bool isBig ) : Enemy( x,y, ATLAS_BULLET0, level ) {
        float vel = 30 + (10*level);
        if(vel>250)vel=250;
        aim(aimx, aimy, vel );

        setDeck( g_base_deck );
        setIndex(ATLAS_BULLET0);
        setAnim( g_bullet_anim_curve );
        last_explode_at = 0;
    }

    virtual bool enemyPoll(double dt){
        if( Vec2(0,0).len( loc) > 300 ){
            // 一定距離飛んだら消える
            createExplosion(loc.x,loc.y,3);
            return false;
        }
        if( last_explode_at < accum_time-1 ){
            createExplosion(loc.x,loc.y,6);
            last_explode_at = accum_time;
        }        
        return true;
    }
};
Bullet *createBullet(float x, float y, float aimx, float aimy, int level, bool isBig ) {
    Bullet *b = new Bullet(x,y,aimx,aimy,level,isBig);
    g_main_layer->insertProp(b);
    return b;
}


////////////
    
class MyShip : public Char {
public:
    int hp,maxhp;
    float vel;
    float shootTime;
    float shootWidth;
    float shootVel ;
    float shootInterval;
    MyShip(float x, float y) : Char(x,y, ATLAS_MYSHIP ) {
        hp = 30;
        maxhp = 30;
        vel = 200;
        shootTime = 0;
        shootWidth = 8;
        shootVel = 300;
        shootInterval = 0.3;
        setDeck( g_base_deck );
        setFragmentShader( g_replacer_shader );
    }
    
    virtual bool charPoll(double dt){
        Vec2 force;
        g_pad->getVec(&force);
        v.x = force.x * vel;
        v.y = force.y * vel;

        float c = absolute(::sin( accum_time * 10 ) );
        setColor( Color(c,c,c,1));
        if(cnt%1000==0){
            Bullet * b = createBullet(loc.x, loc.y, loc.x + range(-100,100), loc.y + range(-100,100), 1, false );
            //
            Vec2 aimv = b->v.rot(M_PI/8).normalize(100);
            createBullet( loc.x, loc.y, loc.x + aimv.x, loc.y + aimv.y, 4, false );
        }
        if(g_camera){
            g_camera->setLoc(loc.x, loc.y);
        }
        return true;
    }
};



MyShip *createMyShip(float x, float y){
    MyShip *ms = new MyShip(x,y);
    g_main_layer->insertProp(ms);
    return ms;
}








void updateGame(void) {
    static double last_print_at = 0;
    static int frame_counter = 0;
    static double last_poll_at = now();

    double t = now();
    double dt = t - last_poll_at;
    
    frame_counter ++;
    
    int cnt;
    cnt = g_moyai_client->pollAll(dt);

    if(last_print_at == 0){
        last_print_at = t;
    } else if( last_print_at < t-1 ){
        fprintf(stderr,"FPS:%d prop:%d render:%d\n", frame_counter, cnt, g_last_render_cnt  );
        frame_counter = 0;
        last_print_at = t;
    }


    char hoge[100];
    snprintf(hoge,sizeof(hoge),"hoge:%d", frame_counter );
    wchar_t whoge[100];
    mbstowcs(whoge,hoge,strlen(hoge));
    g_tb->setString(whoge);

    g_linep->loc.y = sin( now() ) * 200;

    // update dynamic image
    for(int i=0;i<1000;i++){
        g_img->setPixel( irange(0,256), irange(0,256), Color( range(0,1), range(0,1), range(0,1),1 ) );
    }
    g_dyn_texture->setImage(g_img);


    if( frame_counter % 1000 == 0 ) {
        Image *img = new Image();
        img->setSize( SCRW, SCRH );
        g_moyai_client->capture(img);
        bool ret = img->writePNG("_captured.png");
        assert(ret);
        print("captured in _captured.png");
        delete img;
    }
    
    last_poll_at = t;

}


void qstest(){
    SorterEntry tosort[5];
    tosort[0].val = 9;
    tosort[0].ptr = (void*)"aho";
    tosort[1].val = 5;
    tosort[1].ptr = (void*)"hoo";
    tosort[2].val = 1;
    tosort[2].ptr = (void*)"mog";
    tosort[3].val = 8;
    tosort[3].ptr = (void*)"tek";
    tosort[4].val = 10;
    tosort[4].ptr = (void*)"pak";
    
    quickSortF(tosort,0,5-1);
    for(int i=0;i<5;i++){
        print("val:%f %s",tosort[i].val, (char*)tosort[i].ptr );
    }
    assert( tosort[0].val == 1 );
    assert( strcmp( (char*)tosort[0].ptr, "mog" ) == 0 );
    assert( tosort[4].val == 10 );
    assert( strcmp( (char*)tosort[4].ptr, "pak" ) == 0 );    
    
}

void optest(){
    Vec2 a(1,2);
    Vec2 b(2,3);
    Vec2 c = a + b;
    assert( c.x == 3 );
    assert( c.y == 5 );
    assert( c == Vec2(3,5) );
    assert( c != Vec2(3,4) );
    assert( c != Vec2(3.1,5) );
    assert( !c.isZero() );
    c = Vec2(0,0);
    assert( c.isZero() );
    
    Vec2 d = a - b;
    assert( d.x == -1 );
    assert( d.y == -1 );

    Vec2 e = a * 2;
    assert( e.x == 2 );
    assert( e.y == 4 );

    Vec2 f(2,3);
    f *= 2;
    assert( f.x == 4);
    assert( f.y == 6);

    Vec2 g(2,4);
    g /= 2;
    assert( g.x == 1);
    assert( g.y == 2);

    Vec2 h(2,3);
    h += Vec2(1,2);
    assert( h.x == 3);
    assert( h.y == 5);    

    Vec2 k(2,3);
    k -= Vec2(5,5);
    assert( k.x == -3);
    assert( k.y == -2);    

    print("optest done");    
}





int main(int argc, char **argv )
{
    qstest();
    optest();
    
    print("program start");

    g_sound_system = new SoundSystem();
    g_explosion_sound = g_sound_system->newSound("./assets/blobloblll.wav" );
    g_explosion_sound->play();
    

    // glfw
    if( !glfwInit() ) {
        print("can't init glfw");
        return 1;
    }

    if( !glfwOpenWindow( SCRW, SCRH, 0,0,0,0, 0,0, GLFW_WINDOW ) ) {
        print("can't open glfw window");
        glfwTerminate();
        return 1;
    }
    glfwSetWindowTitle( "demo2d" );
    glfwEnable( GLFW_STICKY_KEYS );
    glfwSwapInterval(1); // vsync

	glewInit();

    glClearColor(0,0,0,1);

    // controls
    g_pad = new Pad();

    // shader
    
    g_replacer_shader = new ColorReplacerShader();
    if( !g_replacer_shader->init() ){
        print("can't initialize shader");
        return 0;
    }

    g_moyai_client = new MoyaiClient();

    g_viewport = new Viewport();
    g_viewport->setSize(SCRW,SCRH);
    g_viewport->setScale2D(SCRW,SCRH);

    Layer *l = new Layer();
    g_moyai_client->insertLayer(l);
    l->setViewport(g_viewport);

    Texture *t = new Texture();
    t->load( "./assets/base.png" );

    Texture *t2 = new Texture();
    t2->load( "./assets/base.png" );
    
    TileDeck *deck = new TileDeck();
    deck->setTexture(t);
    deck->setSize(16,16,16,16,256,256);
    TileDeck *d2 = new TileDeck();
    d2->setTexture(t2);
    d2->setSize(16,16, 16,16, 256,256 );

    for(int i=0;i<10;i++ ){
        Prop2D *p = new Prop2D();
        p->setDeck(deck);
        int cands[] = { 0,1,2, 16,17,18, 24,25,26,27, 32,40,41 };
        p->setIndex( cands[ irange(0, elementof(cands)) ]);
        float s = range(16,32);
        p->setScl(s,s);
        p->setLoc(range(-100,100), range(-100,100));
        l->insertProp(p);
    }
    Prop2D *sclp = new Prop2D();
    sclp->setDeck(deck);
    sclp->setIndex(1);
    sclp->setScl(16,16);
    sclp->setLoc(-200,0);
    sclp->seekScl( 128,128, 8);
    sclp->setRot( M_PI/8 );
    l->insertProp(sclp);

    Prop2D *sclprot = new Prop2D();
    sclprot->setDeck(deck);
    sclprot->setIndex(0);
    sclprot->setScl(16,16);
    sclprot->setLoc(-300,0);
    sclprot->seekScl( 128,128, 8);
    sclprot->setRot( M_PI/8 );
    sclprot->seekRot( M_PI*2, 4 );
    sclprot->setUVRot(true);
    l->insertProp(sclprot);    


    for(int i=0;i<10;i++){
        Prop2D *p = new Prop2D();
        p->setColor(range(0,1),range(0,1),range(0,1),range(0,1));
        p->setDeck(d2);
        p->setIndex( 1 + (i%2) ); //irange(0,16) );
        p->setScl(24,24);
        p->setLoc( range(-100,100), range(-100,100));
        l->insertProp(p);
    }

    {
        Prop2D *p = new Prop2D();
        p->setDeck(d2);
        p->setScl(24,24);
        p->setLoc(-100,-300);
        Grid *g = new Grid(8,8);
        g->setDeck(d2);
        int iii=1;
        for(int x=0;x<8;x++){
            for(int y=0;y<8;y++){
                g->set(x,y,iii % 3);
                g->setColor( x,y, Color(range(0.5,1), range(0.5,1), range(0.5,1), range(0.5,1) ));
                iii++;
            }
        }
        p->addGrid(g);
        l->insertProp(p);

        Prop2D *p2 = new Prop2D();
        p2->setColor(1,1,0,0.5);
        p2->setDeck(d2);
        p2->setScl(12,12);
        p2->setLoc(-100,100);
        p2->addGrid(g);
        l->insertProp(p2);
    }
    
    g_main_layer = new Layer();
    g_moyai_client->insertLayer(g_main_layer);
    g_main_layer->setViewport(g_viewport);

    g_base_atlas = new Texture();
    g_base_atlas->load("./assets/base.png");
    g_base_deck = new TileDeck();
    g_base_deck->setTexture(g_base_atlas);
    g_base_deck->setSize(16,16, 16,16, 256,256 );

    g_bmpfont_atlas = new Texture();
    g_bmpfont_atlas->load("./assets/font_only.png");
    g_bmpfont_deck = new TileDeck();
    g_bmpfont_deck->setTexture( g_bmpfont_atlas );
    g_bmpfont_deck->setSize(32,32, 8,8, 256,256 );
    
    g_camera = new Camera();
    g_camera->setLoc(0,0);

    g_main_layer->setCamera(g_camera);

    int bulletinds[] = { ATLAS_BULLET0, ATLAS_BULLET0+1, ATLAS_BULLET0+2,ATLAS_BULLET0+3};
    g_bullet_anim_curve = new AnimCurve( 0.2, true, bulletinds, elementof(bulletinds));
    


        
    ////////////////////
    wchar_t charcodes[] = L" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~あいうえおぁぃぅぇぉかきくけこがぎぐげごさしすせそざじずぜぞたちつてとだぢづでどなにぬねのはひふへほばびぶべぼぱぴぷぺぽまみむめもやゆよゃゅょらりるれろわをん、。アイウエオァィゥェォカキクケコガギグゲゴサシスセソザジズゼゾタチツテトダヂヅデドナニヌネノハヒフヘホバビブベボパピプペポマミムメモヤユヨャュョラリルレロワヲンーッっ　「」";    

    Font *font = new Font();
    font->loadFromTTF("./assets/cinecaption227.ttf", charcodes, 12 );

    g_tb = new TextBox();
    g_tb->setFont(font);
    g_tb->setString("dummy");

    g_main_layer->insertProp(g_tb);

    TextBox *t3 = new TextBox();
    t3->setFont(font);
    t3->setString( L"ABC012ほげ。\nふがふがふがの。" );
    t3->setLoc(-100,-50);
    g_main_layer->insertProp(t3);

    // Image manipulation
    Image *solimg = new Image();
    solimg->loadPNG( "assets/sol.png" );
    assert( solimg->width == 16 );
    assert( solimg->height == 16 );

    for(int y=0;y<16;y++){
        for(int x=0;x<16;x++) {
            unsigned char r,g,b,a;
            solimg->getPixelRaw( x,y, &r, &g, &b, &a );
            if( r == 255 && g == 255 && b == 255 && a == 255 ) {
                solimg->setPixelRaw( x,y, 255,0,0,255 );
            }
        }
    }

    Texture *soltex0 = new Texture();
    soltex0->setImage( solimg );
    
    Texture *soltex1 = new Texture();
    soltex1->load( "assets/sol.png" );

#if 1
    Prop2D *solp1 = new Prop2D();
    solp1->setLoc( SCRW/2-80, 0);
    solp1->setTexture(soltex1);
    solp1->setScl(32);
    g_main_layer->insertProp(solp1);    
#endif
    
    Prop2D *solp0 = new Prop2D();
    solp0->setLoc( SCRW/2-40, 0);
    solp0->setTexture( soltex0 );
    solp0->setScl(32);
    g_main_layer->insertProp(solp0);

    TileDeck *soldk = new TileDeck();
    soldk->setTexture(soltex0);
    soldk->setSize( 2,2,8,8, 16,16 );
    Prop2D *solp2 = new Prop2D();
    solp2->setLoc( SCRW/2-120, 0);
    solp2->setDeck( soldk );
    solp2->setScl(32);
    solp2->setIndex(1);
    g_main_layer->insertProp(solp2);

    

    

    // bitmap font
    Prop2D *scorep = new Prop2D();
    scorep->setLoc( -SCRW/2+32,SCRH/2-100 );
    CharGrid *scoregrid = new CharGrid(8,8);
    scoregrid->setDeck(g_bmpfont_deck );
    scoregrid->setAsciiOffset(-32);
    scoregrid->printf( 0,0, Color(1,1,1,1), "SCORE: %d",1234 );
    scoregrid->printf( 0,1, Color(1,1,0,1), "$#!?()[%s]", "hoge" );
    scoregrid->setColor( 3,0, Color(0,1,1,1));
    scorep->addGrid(scoregrid);
    g_main_layer->insertProp(scorep);

    // line prop
    g_linep = new Prop2D();
    g_linep->addLine( Vec2(0,0), Vec2(100,100), Color(1,0,0,1) );
    g_linep->addLine( Vec2(0,0), Vec2(100,-50), Color(0,1,0,1) );
    g_linep->addRect( Vec2(0,0), Vec2(-150,230), Color(0,0,1,0.5) );
    g_linep->setLoc(0,200);
    g_main_layer->insertProp(g_linep);
    // add child to line prop
    Prop2D *childp = new Prop2D();
    childp->setDeck(g_base_deck);
    childp->setScl(16,16);
    childp->seekRot( M_PI * 1000, 30 );
    childp->setIndex(0);
    childp->setLoc(-222,-222);
    //g_main_layer->insertProp(childp);
    g_linep->addChild(childp);


    // dynamic images
    {
        g_img = new Image();
        g_img->setSize(256,256);
        for(int i=0;i<256;i++){
            g_img->setPixel( i,i, Color(range(0,1), range(0,1),range(0,1),1));
        }
        g_dyn_texture =  new Texture();
        g_dyn_texture->load("assets/base.png");
        g_dyn_texture->setImage(g_img);

        g_img->writePNG( "assets/dynamic_out.png");
    
        Prop2D *p = new Prop2D();
        TileDeck *d = new TileDeck();
        d->setTexture(g_dyn_texture);
        d->setSize( 16,16,16,16,256,256);
        p->setDeck(d);
        p->setLoc(200,200);
        p->setScl(128,128);
        p->setIndex(0);
        g_main_layer->insertProp(p);
    }

    
    g_pc = createMyShip(0,0);
    assert(g_pc);


    while(1){
        int scrw, scrh;
        glfwGetWindowSize( &scrw, &scrh );

        updateGame();

        // replace white to random color
        g_replacer_shader->setColor( Color(1,1,1,1), Color( range(0,1),range(0,1),range(0,1),1), 0.02 );
        g_last_render_cnt = g_moyai_client->renderAll();

        if( glfwGetKey('Q') ) {
            print("Q pressed");
            break;
        }

        g_pad->readGLFW();
        
    }


    print("program finished");
    

    return 0;
}

