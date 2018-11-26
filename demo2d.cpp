#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <locale.h>

#ifndef WIN32
#include <strings.h>
#endif

#if defined(__APPLE__)
#define RETINA 2
#else
#define RETINA 1
#endif

#include "client.h"

#ifdef USE_GENVID
#include "genvid.h"
#include <chrono>

static const std::string sStream_Audio = "Audio";
static const std::string sStream_Video = "Video";
static const std::string sStream_GameData = "GameData";
static const std::string sStream_Popularity = "Popularity";
static const std::string sStream_ColorChanged = "ColorChanged";
static const std::string sStream_GameCopyright = "GameCopyright";

static const std::string sEvent_changeColor = "changeColor";
static const std::string sEvent_reset = "reset";
static const std::string sEvent_cheer = "cheer";

static const std::string sCommand_speed = "speed";
static const std::string sCommand_direction = "direction";
static const std::string sCommand_reset = "reset";

static std::string oldPopularityJSON;

static void GenvidSubscriptionCallback(const GenvidEventSummary * summary, void * userData);

static void GenvidSubscriptionCommandCallback(const GenvidCommandResult * summary, void * userData);

GenvidTimecode           gCurrentTc = -1;
GenvidTimecode           gPrevTc = -1;
GenvidTimecode           gFirstFrameTc = -1;
std::chrono::system_clock::time_point gStartTime = std::chrono::system_clock::now();
double                   gWorldTime = 0.0;
float                    gGenvidFramerate = 30.0f; // uses default value
float                    gGameFramerate = 0.f;   // uses default value
bool                     gEnableVSync = true;

bool gSilent = false; // Global flag to kill all sounds.



void GenvidSubscriptionCommandCallback(const GenvidCommandResult * result, void * /*userData*/)
{
	std::string id(result->id);
	std::string value(result->value);

	print("cmd:id:%s", id.c_str());
	print("cmd:value:%s", value.c_str());

}
void GenvidSubscriptionCallback(const GenvidEventSummary * summary, void * /*userData*/)
{
	print("subscriptioncallback");
	
}


HRESULT initGenvid() {
	GenvidStatus gs;
	gs = Genvid_Initialize();
	if (GENVID_FAILED(gs))
		return E_FAIL;

	gs = Genvid_CreateStream(sStream_Audio.c_str());
	if (GENVID_FAILED(gs))
		return E_FAIL;

	// Specify auto-capture video source.
	gs = Genvid_SetParameterInt(sStream_Audio.c_str(), "Audio.Source.WASAPI", 1);
	if (GENVID_FAILED(gs))
		return E_FAIL;

	// Create the video stream.
	gs = Genvid_CreateStream(sStream_Video.c_str());
	if (GENVID_FAILED(gs))
		return E_FAIL;

	if (gGenvidFramerate > 0)
	{
		gs = Genvid_SetParameterFloat(sStream_Video.c_str(), "framerate", gGenvidFramerate);
		if (GENVID_FAILED(gs))
			return E_FAIL;
	}

#if GENVID_USE_DXGISWAPCHAIN
	// Specify auto-capture video source.
	gs = Genvid_SetParameterPointer(sStream_Video.c_str(), "Video.Source.IDXGISwapChain", g_pSwapChain);
	if (GENVID_FAILED(gs))
		return E_FAIL;
#endif

	// Create stream for game data.
	gs = Genvid_CreateStream(sStream_GameData.c_str());
	if (GENVID_FAILED(gs))
		return E_FAIL;

	gs = Genvid_CreateStream(sStream_GameCopyright.c_str());
	if (GENVID_FAILED(gs))
		return E_FAIL;

	// this frame is receive one time only by each viewer when genvid client is connected
	const char copyright[] = "Copyright Genvid Technologies 2018";
	Genvid_SubmitGameData(-1, sStream_GameCopyright.c_str(), copyright, (int)(sizeof copyright));

	gs = Genvid_CreateStream(sStream_Popularity.c_str());
	if (GENVID_FAILED(gs))
		return E_FAIL;

	gs = Genvid_CreateStream(sStream_ColorChanged.c_str());
	if (GENVID_FAILED(gs))
		return E_FAIL;

	// Subscribe to events.
	gs = Genvid_Subscribe(sEvent_changeColor.c_str(), &GenvidSubscriptionCallback, nullptr);
	if (GENVID_FAILED(gs))
		return E_FAIL;
	gs = Genvid_Subscribe(sEvent_reset.c_str(), &GenvidSubscriptionCallback, nullptr);
	if (GENVID_FAILED(gs))
		return E_FAIL;
	gs = Genvid_Subscribe(sEvent_cheer.c_str(), &GenvidSubscriptionCallback, nullptr);
	if (GENVID_FAILED(gs))
		return E_FAIL;

	// Subscribe to commands.
	gs = Genvid_SubscribeCommand(sCommand_speed.c_str(), &GenvidSubscriptionCommandCallback, nullptr);
	if (GENVID_FAILED(gs))
		return E_FAIL;
	gs = Genvid_SubscribeCommand(sCommand_direction.c_str(), &GenvidSubscriptionCommandCallback, nullptr);
	if (GENVID_FAILED(gs))
		return E_FAIL;
	gs = Genvid_SubscribeCommand(sCommand_reset.c_str(), &GenvidSubscriptionCommandCallback, nullptr);
	if (GENVID_FAILED(gs))
		return E_FAIL;

	return S_OK;

}

void termGenvid()
{
	// Cancel command subscriptions.
	Genvid_UnsubscribeCommand(sCommand_reset.c_str(), &GenvidSubscriptionCommandCallback, nullptr);
	Genvid_UnsubscribeCommand(sCommand_direction.c_str(), &GenvidSubscriptionCommandCallback, nullptr);
	Genvid_UnsubscribeCommand(sCommand_speed.c_str(), &GenvidSubscriptionCommandCallback, nullptr);

	// Cancel event subscriptions.
	Genvid_Unsubscribe(sEvent_cheer.c_str(), &GenvidSubscriptionCallback, nullptr);
	Genvid_Unsubscribe(sEvent_reset.c_str(), &GenvidSubscriptionCallback, nullptr);
	Genvid_Unsubscribe(sEvent_changeColor.c_str(), &GenvidSubscriptionCallback, nullptr);

	// Destroy the streams.
	Genvid_DestroyStream(sStream_ColorChanged.c_str());
	Genvid_DestroyStream(sStream_Popularity.c_str());
	Genvid_DestroyStream(sStream_GameData.c_str());
	Genvid_DestroyStream(sStream_GameCopyright.c_str());
	Genvid_DestroyStream(sStream_Video.c_str());
	Genvid_DestroyStream(sStream_Audio.c_str());

	// Terminate the Genvid Native SDK.
	Genvid_Terminate();
}

#endif


bool g_headless_mode=false;
bool g_enable_videostream=false;
bool g_enable_spritestream=true;
bool g_enable_reprecation=false;
bool g_disable_timestamp=false;
bool g_disable_compress=false;

MoyaiClient *g_moyai_client;
Viewport *g_viewport;
Layer *g_main_layer;
Layer *g_hud_layer;
Texture *g_base_atlas;
TileDeck *g_base_deck;
Camera *g_camera;
Font *g_font;

Texture *g_bmpfont_atlas;
TileDeck *g_bmpfont_deck;

ColorReplacerShader *g_replacer_shader;


TextBox *g_tb;

AnimCurve *g_bullet_anim_curve;
AnimCurve *g_digit_anim_curve;

SoundSystem *g_sound_system;
Sound *g_explosion_sound;
Sound *g_bgm_sound;
Sound *g_mem_sound;

Image *g_img;
Texture *g_dyn_texture;

Prop2D *g_linep;
Prim *g_narrow_line_prim;

#define SCALE  2

int g_last_render_cnt ;
int g_send_wait_ms=0;

RemoteHead *g_rh;

GLFWwindow *g_window;

bool g_game_done = false;

Keyboard *g_keyboard;
Mouse *g_mouse;
Pad *g_pad;



// game data

enum {
    ATLAS_MYSHIP = 0,
    ATLAS_BULLET0 = 32,  
    ATLAS_EXPLOSION = 96,
    ATLAS_DIGIT0 = 160,  
};

// config

static const int SCRW=960, SCRH=544;

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
    if(range(0,100)>50) {
        g_explosion_sound->play( range(0.1,1) );
    } else {
        g_mem_sound->play();
    }
    
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
        if( Vec2(0,0).len( loc) > 300 ) {
            // 一定距離飛んだら消える
            createExplosion(loc.x,loc.y,3);
            return false;
        }
        if( last_explode_at < accum_time-1 && range(0,100)>99 ){
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

class Blocks : public Prop2D {
public:
    Grid *g;
    Blocks() : Prop2D() {
        g = new Grid(4,4);
        for(int y=0;y<4;y++) {
            for(int x=0;x<4;x++) {
                g->set(x,y,2);
            }
        }
        g->setDeck(g_base_deck);
        addGrid(g);

        setScl(16,16);
        setLoc( range(-300,300), range(-200,200) );        
    }
    virtual bool prop2DPoll(double dt) {
        if( accum_time > 3 ) {
            return false;
        }
        // changing index
        int ind = ( (int)(accum_time*2)%2)+1;
        g->set(1,0,ind);
        // changing color
        float fcol = ( (int)(accum_time*3)%2 ) * 0.5 + 0.5;
        g->setColor(2,0, Color(fcol,fcol,fcol,1.0f) );
        g->setXFlip(3,0,(int)(accum_time*3.1)%2);
        g->setYFlip(3,1,(int)(accum_time*2.8)%2);
        g->setUVRot(3,2,(int)(accum_time*2.4)%2);
        Vec2 texofs(0,0);
        if( (int)(accum_time*2.2)%2 == 0 ) texofs = Vec2(0.5,0.5);
        g->setTexOffset(3,3, &texofs);
        return true;
    }
    static Blocks *create() {
        Blocks *b = new Blocks();
        g_main_layer->insertProp(b);
        return b;
    }
};

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
        if(v.x>0) setXFlip(false); else if( v.x<0) setXFlip(true);

        float c = absolute(::sin( accum_time * 2 ) );
        setColor( Color(c,c,c,1));
        if(cnt%200==0){
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

//////
class Digit : public Prop2D {
public:
    Digit( Vec2 at ) : Prop2D() {
        setLoc(at);
        setAnim( g_digit_anim_curve );
        setDeck( g_base_deck );
        setScl( range(10,40), range(10,40) );
        seekColor( Color(1,0,0,0.2),0.5);
    }
    virtual bool prop2DPoll(double dt) {
        return true;
    }
    virtual void onAnimFinished() {
        to_clean = true;
    }
    static Digit*create(Vec2 at) {
        Digit *d = new Digit(at);
        g_main_layer->insertProp(d);
        return d;
    }
};

void createRandomDigit() {
    Vec2 at(range(-200,200), range(-200,200) );    
    Digit::create(at);
}

void gameUpdate(void) {
    glfwPollEvents();            
    g_pad->readKeyboard(g_keyboard);
    
    static double last_print_at = 0;
    static int frame_counter = 0;
    static int total_frame = 0;

    static double last_t=now();
    double t = now();
    double dt = t - last_t;
    last_t = t;
    double loop_start_at = t;
    
    frame_counter ++;
    total_frame ++;    

    // update texts    
    char hoge[100];
    snprintf(hoge,sizeof(hoge),"hoge:%d", frame_counter );
    wchar_t whoge[100];
    memset(whoge,0,sizeof(whoge));
    mbstowcs(whoge,hoge,strlen(hoge));
    if(g_tb) g_tb->setString(whoge);

    g_linep->loc.y = sin( now() ) * 200;


    
    // update dynamic image
    if( (total_frame % 500 ) == 0 ) {
        print("setpixel");
        for(int i=0;i<1000;i++){
            g_img->setPixel( irange(0,256), irange(0,256), Color( range(0,1), range(0,1), range(0,1),1 ) );
        }
    }
    g_dyn_texture->setImage(g_img);

    if( ( total_frame % 30 ) == 0 ) {
        createRandomDigit();
    }
    if( ( total_frame % 70 ) == 0 ) {
        Blocks::create();
    }

    if( g_keyboard->getKey('C') ) {
        Image *img = new Image();
        img->setSize( SCRW*RETINA, SCRH*RETINA );
        double st = now();
        g_moyai_client->capture(img);
        double et = now();
        bool ret = img->writePNG("_captured.png");
        double et2 = now();
        print("screen capture time:%f,%f", et-st,et2-et);
        
#if !(TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE)
        assert(ret);
#endif        
        print("captured in _captured.png");
        delete img;
    }

    // replace white to random color
    g_replacer_shader->setColor( Color(0xF7E26B), Color( range(0,1),range(0,1),range(0,1),1), 0.02 );

    if( g_keyboard->getKey( 'Q') ) {
        print("Q pressed");
        g_game_done = true;
        return;
    }
    if( g_keyboard->getKey('M' )) {
        g_bgm_sound->play(1);
        g_bgm_sound->setVolume(1);
    }
    if( g_keyboard->getKey('V') ) {
        g_sound_system->setVolume(0);
    }
    if( g_keyboard->getKey( 'K' ) ) {
        float bgmpos = g_bgm_sound->getTimePositionSec();
        print("bgm position: %f", bgmpos );
        g_bgm_sound->setTimePositionSec( bgmpos + 2.0f );
    }
    if( g_keyboard->getKey( 'L' ) ) {
        g_bgm_sound->stop();
    }
    if( g_keyboard->getKey( 'Z' ) ) {
        g_viewport->setScale2D( g_viewport->scl.x / 1.1f, g_viewport->scl.y / 1.1f );
    }
    if( g_keyboard->getKey( 'X' ) ) {
        g_viewport->setScale2D( g_viewport->scl.x * 1.1f, g_viewport->scl.y * 1.1f );        
    }
    if( g_keyboard->getKey( 'P' ) ) {
        float t = g_bgm_sound->getTimePositionSec();
        print("getPosition: %f", t);
        g_bgm_sound->setTimePositionSec(2.0f);
    }
    if( g_keyboard->getKey( 'Y' ) ) {
        g_moyai_client->batch_list.dump();
    }

    if( g_mouse->getButton(0) ) {
        Vec2 cp = g_mouse->getCursorPos();
        print("mouse button 0 %f,%f", cp.x, cp.y );
    }
    
    // moving lines
    g_narrow_line_prim->a = Vec2(0,0);
    g_narrow_line_prim->b = Vec2(100, range(100,150));

#if 1    
    // add/del prims
    {
        static int ylcnt=0;
        ylcnt++;
        static int yellow_line_prim_id=0;
        if( ylcnt%100 == 50 ){
            Prim *yl = g_linep->addLine( Vec2(0,0), Vec2( -30, range(-30,-100)), Color(1,1,0,1), 3 );
            if(yl) {
                yellow_line_prim_id = yl->id;
            }
        }
        if( ylcnt%100 == 99 ) {
            if( yellow_line_prim_id > 0 ) {
                Prim *yl = g_linep->getPrimById(yellow_line_prim_id);
                if(yl) {
                    g_linep->deletePrim(yl->id);
                }
            }
        }
    }
#endif

    if( g_bgm_sound->isPlaying() == false ) {
        g_bgm_sound->play();
    }

    int cnt;
    cnt = g_moyai_client->poll(dt);

    if(last_print_at == 0){
        last_print_at = t;
    } else if( last_print_at < t-1 ){
        fprintf(stderr,"FPS:%d prop:%d render:%d drawbatch:%d\n", frame_counter, cnt, g_last_render_cnt, g_moyai_client->batch_list.used  );
        frame_counter = 0;
        last_print_at = t;
    }
    
    double loop_end_at = now();
    double loop_time = loop_end_at - loop_start_at;
    double ideal_frame_time = 1.0f / 60.0f;
    if(loop_time < ideal_frame_time ) {
        double to_sleep_sec = ideal_frame_time - loop_time;
        int to_sleep_msec = (int) (to_sleep_sec*1000);
        if( to_sleep_msec > 0 ) sleepMilliSec(to_sleep_msec);
    }
}

// direct rendering using callback function
void drawCallback(Layer *l, DrawBatchList *bl) {
    float u0,v0,u1,v1;
    g_base_deck->getUVFromIndex(irange(0,3), &u0,&v0,&u1,&v1,0,0,0);
    bl->appendSprite1(g_viewport,NULL,BLENDTYPE_SRC_ALPHA,g_base_atlas->tex, Color(1,1,1,1), Vec2(200,-200), Vec2(100,100), 0,
                      Vec2(u0,v1), Vec2(u0,v0), Vec2(u1,v0), Vec2(u1,v1) );
    Prop2D::drawToDBL(l,bl,NULL,false,g_base_deck,irange(0,3),Color(1,1,1,1), Vec2(350,-200), Vec2(100,100), 0 );
    TextBox::drawToDBL(l,bl,false,g_font, "hogeほげDBL", Color(1,1,1,1), Vec2(200,-200), 1, 0 );
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

void comptestbig() {
    size_t sz=1024*1024;
    char *buf = (char*)MALLOC(sz);
    for(int i=0;i<sz;i++) buf[i] = irange(0,8);
    char *zipped = (char*)MALLOC(sz*2);
    char *inflated = (char*)MALLOC(sz);
    double t0 = now();
    int zipped_len = memCompressSnappy( zipped, sz*2, buf, sz);
    double t1 = now();
    int inflated_len = memDecompressSnappy( inflated, sz, zipped, zipped_len );
    double t2 = now();
    print("snappy big: %d bytes to %d byte. comptime:%f decomptime:%f", inflated_len, zipped_len, t1-t0, t2-t1 );
    FREE(buf);
    FREE(zipped);
    FREE(inflated);
}

void comptest() {
    char buf[] = "hogehogefugafugahogefugapiyopiyo";
    char zipped[1024];
    char inflated[1024];
    int zipped_len = memCompressSnappy( zipped, sizeof(zipped), buf, (int)strlen(buf) );
    int inflated_len = memDecompressSnappy( inflated, sizeof(inflated), zipped, zipped_len );
    inflated[inflated_len] = '\0';
    print("snappy: %d bytes to %d byte", inflated_len, zipped_len );
}


void winclose_callback( GLFWwindow *w ){
	termGenvid();
    exit(0);
}

void glfw_error_cb( int code, const char *desc ) {
    print("glfw_error_cb. code:%d desc:'%s'", code, desc );
}
void fbsizeCallback( GLFWwindow *window, int w, int h ) {
    print("fbsizeCallback: %d,%d",w,h);
#ifndef __linux__
	glViewport(0, 0, w, h);
#endif    
}

void keyboardCallback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
    g_keyboard->update( key, action, mods & GLFW_MOD_SHIFT, mods & GLFW_MOD_CONTROL, mods & GLFW_MOD_ALT );
}
void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods ) {
    g_mouse->updateButton( button, action, mods & GLFW_MOD_SHIFT, mods & GLFW_MOD_CONTROL, mods & GLFW_MOD_ALT );
}
void cursorPosCallback( GLFWwindow *window, double x, double y ) {
    g_mouse->updateCursorPosition( x,y);
}
void onConnectCallback( RemoteHead *rh, Client *cl) {
    print("onConnectCallback clid:%d",cl->id);
}
void onRemoteKeyboardCallback( Client *cl, int kc, int act, int modshift, int modctrl, int modalt ) {
    g_keyboard->update(kc,act,modshift,modctrl,modalt);
}
void onRemoteMouseButtonCallback( Client *cl, int btn, int act, int modshift, int modctrl, int modalt ) {
    g_mouse->updateButton( btn, act, modshift, modctrl, modalt );
}
void onRemoteMouseCursorCallback( Client *cl, int x, int y ) {
    g_mouse->updateCursorPosition(x,y);
}
void gameInit() {
    print("PacketProp2DSnapshot size:%d",sizeof(PacketProp2DSnapshot));
    qstest();
    optest();
    comptest();
    comptestbig();

    print("gameInit: headless_mode:%d spritestream:%d videostream:%d disable_compression:%d",
          g_headless_mode, g_enable_spritestream, g_enable_videostream, g_disable_compress );

#ifdef __APPLE__    
    setlocale( LC_ALL, "ja_JP");
#endif
#ifdef WIN32    
    setlocale( LC_ALL, "jpn");
#endif    

#ifdef USE_GENVID
	HRESULT gvres=initGenvid();
	if (gvres != S_OK) {
		print("initgenvid failed");
		return;
	}
#endif

    g_sound_system = new SoundSystem();
    g_explosion_sound = g_sound_system->newSound("./assets/blobloblll.wav" );
    g_explosion_sound->play();
    g_bgm_sound = g_sound_system->newBGM( "./assets/gymno1short.wav" );
    g_bgm_sound->play();
    {
        float samples[44100/4];
        for(int i=0;i<elementof(samples);i++) samples[i] = cos( (float)(i) / 20.0f );
        g_mem_sound = g_sound_system->newSoundFromMemory( samples, elementof(samples) );
    }
    
    // glfw
    if( !glfwInit() ) {
        print("can't init glfw");
        exit(1);
    }

    glfwSetErrorCallback( glfw_error_cb );
    g_window =  glfwCreateWindow( SCRW, SCRH, "demo2d", NULL, NULL );
    if(g_window == NULL ) {
        print("can't open glfw window");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(g_window);    
    glfwSetWindowCloseCallback( g_window, winclose_callback );
    //    glfwSetInputMode( g_window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSwapInterval(0); // set 1 to use vsync. Use 0 for fast screen capturing and headless
#ifdef WIN32
	glewInit();
#endif
    glClearColor(0.2,0.2,0.2,1);

    // controls
    g_keyboard = new Keyboard();
    glfwSetKeyCallback( g_window, keyboardCallback );
    g_mouse = new Mouse();
    glfwSetMouseButtonCallback( g_window, mouseButtonCallback );
    glfwSetCursorPosCallback( g_window, cursorPosCallback );

    glfwSetFramebufferSizeCallback( g_window, fbsizeCallback );
    g_pad = new Pad();

    // shader    
    g_replacer_shader = new ColorReplacerShader();
    if( !g_replacer_shader->init() ){
        print("can't initialize shader");
        exit(1);
    }

    g_moyai_client = new MoyaiClient(g_window, SCRW, SCRH );
    
    if( g_headless_mode ) {
        Moyai::globalInitNetwork();
        g_rh = new RemoteHead();
        if( g_rh->startServer(HEADLESS_SERVER_PORT) == false ) {
            print("headless server: can't start server. port:%d", HEADLESS_SERVER_PORT );
            exit(1);
        } 
            
        print("headless server listening on:%d",HEADLESS_SERVER_PORT);
        
        if( g_enable_spritestream ) g_rh->enableSpriteStream();
        if( g_enable_videostream ) g_rh->enableVideoStream(SCRW*RETINA,SCRH*RETINA,3);
        if( g_enable_reprecation ) g_rh->enableReprecation(REPRECATOR_SERVER_PORT);
        if( g_disable_timestamp ) g_rh->disableTimestamp();
        if( g_disable_compress ) g_rh->enable_compression = false;

        g_rh->setSendWait((double)(g_send_wait_ms)/1000.0);
        g_moyai_client->setRemoteHead(g_rh);
        g_rh->setTargetMoyaiClient(g_moyai_client);
        g_sound_system->setRemoteHead(g_rh);
        g_rh->setTargetSoundSystem(g_sound_system);
        g_rh->setOnKeyboardCallback(onRemoteKeyboardCallback);
        g_rh->setOnMouseButtonCallback(onRemoteMouseButtonCallback);
        g_rh->setOnMouseCursorCallback(onRemoteMouseCursorCallback);
        g_rh->setOnConnectCallback(onConnectCallback);
    }    

    g_viewport = new Viewport();
    g_viewport->setSize(SCRW*RETINA,SCRH*RETINA); // set actual framebuffer size to output
    g_viewport->setScale2D(SCRW,SCRH); // set scale used by props that will be rendered

#if 0
    {
        Texture *sss = new Texture();
        sss->load( "./assets/dragon8.png" );
        for(int y=0;y<sss->image->height;y++) {
            for(int x=0;x<sss->image->width;x++) {
                Color c = sss->image->getPixel(x,y);
                prt( "%d ", (int)(c.r*255) );
            }
            prt("\n");
        }
        return 0;
    }
#endif

    g_main_layer = new Layer();
    g_moyai_client->insertLayer(g_main_layer);
    g_main_layer->setViewport(g_viewport);

    g_hud_layer = new Layer();
    g_moyai_client->insertLayer(g_hud_layer);
    g_hud_layer->setViewport(g_viewport);
    g_hud_layer->setCallbackFunc(drawCallback);
    
    g_camera = new Camera();
    g_camera->setLoc(0,0);
    g_main_layer->setCamera(g_camera);

    // atlas
    g_base_atlas = new Texture();
    g_base_atlas->load("./assets/base.png");
    g_base_deck = new TileDeck();
    g_base_deck->setTexture(g_base_atlas);
    g_base_deck->setSize(32,32,8,8 );

    g_bmpfont_atlas = new Texture();
    g_bmpfont_atlas->load("./assets/font_only.png");
    g_bmpfont_deck = new TileDeck();
    g_bmpfont_deck->setTexture( g_bmpfont_atlas );
    g_bmpfont_deck->setSize(32,32, 8,8 );

    //    
    Layer *tmplayer = new Layer();
    g_moyai_client->insertLayer(tmplayer);
    tmplayer->setViewport(g_viewport);

    Texture *t = new Texture();
    t->load( "./assets/base.png" );

    Texture *t2 = new Texture();
    t2->load( "./assets/base.png" );
    
    TileDeck *deck = new TileDeck();
    deck->setTexture(t);
    deck->setSize(32,32,8,8);
    TileDeck *d2 = new TileDeck();
    d2->setTexture(t2);
    d2->setSize(32,32,8,8 );

    Prop2D *sclp = new Prop2D();
    sclp->setDeck(deck);
    sclp->setIndex(1);
    sclp->setScl(16,16);
    sclp->setLoc(-200,0);
    sclp->seekScl( 128,128, 8);
    sclp->setRot( M_PI/8 );
    g_main_layer->insertProp(sclp);

    Prop2D *sclprot = new Prop2D();
    sclprot->setDeck(deck);
    sclprot->setIndex(0);
    sclprot->setScl(16,16);
    sclprot->setLoc(-300,0);
    sclprot->seekScl( 128,128, 8);
    sclprot->setRot( M_PI/8 );
    sclprot->seekRot( M_PI*2, 4 );
    sclprot->setUVRot(true);
    g_main_layer->insertProp(sclprot);    

    Prop2D *colp = new Prop2D();
    colp->setColor(1,0,0,1);
    colp->setDeck(d2);
    colp->setIndex(1);
    colp->setScl(24,24);
    colp->setLoc(50,-20);
    g_main_layer->insertProp(colp);

    Prop2D *statprimp = new Prop2D(); // a prop that has a prim with no changes
    statprimp->setDeck(g_base_deck);
    statprimp->setIndex(1);
    statprimp->setColor(0,0,1,1);
    statprimp->addLine(Vec2(0,0),Vec2(1,1),Color(1,1,1,1), 3);
    statprimp->setLoc(100,-100);
    g_main_layer->insertProp(statprimp);

    // static grids
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
                if(x==0) g->setXFlip(x,y,true);
                if(x==1) g->setYFlip(x,y,true);
                if(x==2) g->setUVRot(x,y,true);
                if(x==3) {
                    Vec2 ofs(0.5,0.5);
                    g->setTexOffset(x,y,&ofs);
                }
                iii++;
            }
        }
        p->addGrid(g);
        g_main_layer->insertProp(p);
    }

    int bulletinds[] = { ATLAS_BULLET0, ATLAS_BULLET0+1, ATLAS_BULLET0+2,ATLAS_BULLET0+3};
    g_bullet_anim_curve = new AnimCurve( 0.2, true, bulletinds, elementof(bulletinds));
    int digitinds[] = { ATLAS_DIGIT0, ATLAS_DIGIT0+1, ATLAS_DIGIT0+2, ATLAS_DIGIT0+3 };
    g_digit_anim_curve = new AnimCurve( 0.2, false, digitinds, elementof(digitinds));
    

    Texture *dragontex = new Texture();
    dragontex->load( "./assets/dragon8.png");
    for(int j=0;j<2;j++) {
        for(int i=0;i<6;i++) {
            Prop2D *p = new Prop2D();
            p->setTexture(dragontex);
            p->setScl(32,32);
            p->setColor(1,1,1,0.3);
            p->setLoc(-SCRW/2+50 + i * 10,-SCRH/2+70 + (i%2)*10 + j*80);
            if(j==0) p->use_additive_blend = true;
            //p->setLoc(-200,-200);
            g_main_layer->insertProp(p);
        }
    }
        
    ////////////////////
    wchar_t charcodes[] = L" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~あいうえおぁぃぅぇぉかきくけこがぎぐげごさしすせそざじずぜぞたちつてとだぢづでどなにぬねのはひふへほばびぶべぼぱぴぷぺぽまみむめもやゆよゃゅょらりるれろわをん、。アイウエオァィゥェォカキクケコガギグゲゴサシスセソザジズゼゾタチツテトダヂヅデドナニヌネノハヒフヘホバビブベボパピプペポマミムメモヤユヨャュョラリルレロワヲンーッっ　「」";    

    g_font = new Font();
    g_font->loadFromTTF("./assets/cinecaption227.ttf", charcodes, 12 );


    g_tb = new TextBox();
    g_tb->setFont(g_font);
    g_tb->setString("dummy");
    g_tb->setScl(1);
    g_tb->setLoc(22,22);
    g_main_layer->insertProp(g_tb);

    TextBox *t3 = new TextBox();
    t3->setFont(g_font);
    t3->setString( L"ABC012あいうえお\nあいうえお(wchar_t)。" );
    t3->setLoc(-100,-50);
    t3->setScl(1);
    g_main_layer->insertProp(t3);


    TextBox *t4 = new TextBox();
    t4->setFont(g_font);
    t4->setString( "ABC012あいうえお\nあいうえお(mb-utf8)。" );
    t4->setLoc(-100,-90);
    t4->setScl(0.75f);
    g_main_layer->insertProp(t4);
    
    // Check bottom line
    TextBox *t5 = new TextBox();
    t5->setFont(g_font);
    t5->setString( L"THIS SHOULDN'T SINK UNDER BOTTOM LINE : このもじはしたにしずまない1ぎょうめ\n"
                   L"THIS SHOULDN'T SINK UNDER BOTTOM LINE : このもじはしたにしずまない2ぎょうめ"
                   );
    t5->setLoc(-SCRW/2,-SCRH/2);
    t5->setScl(1);
    g_main_layer->insertProp(t5);

    
    // Image manipulation
    Image *dragonimg = new Image();
    dragonimg->loadPNG( "assets/dragon8.png" );
    assert( dragonimg->width == 8 );
    assert( dragonimg->height == 8 );

    for(int y=0;y<8;y++){
        for(int x=0;x<8;x++) {
            unsigned char r,g,b,a;
            dragonimg->getPixelRaw( x,y, &r, &g, &b, &a );
            if( r == 255 && g == 255 && b == 255 && a == 255 ) {
                print("setPixelRaw: %d,%d",x,y);
                dragonimg->setPixelRaw( x,y, 255,0,0,255 );
            }
        }
    }

    Texture *dragontex0 = new Texture();
    dragontex0->setImage( dragonimg );
    
    Texture *dragontex1 = new Texture();
    dragontex1->load( "assets/dragon8.png" );

    Prop2D *dragonp1 = new Prop2D();
    dragonp1->setLoc( SCRW/2-80, 0);
    dragonp1->setTexture(dragontex1);
    dragonp1->setScl(32);
    g_main_layer->insertProp(dragonp1);    
    
    Prop2D *dragonp0 = new Prop2D();
    dragonp0->setLoc( SCRW/2-40, 0);
    dragonp0->setTexture( dragontex0 );
    dragonp0->setScl(32);
    g_main_layer->insertProp(dragonp0);
    

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
    g_narrow_line_prim = g_linep->addLine( Vec2(0,0), Vec2(100,100), Color(1,0,0,1) );
    g_linep->addLine( Vec2(0,0), Vec2(100,-50), Color(0,1,0,1), 5 );
    g_linep->addRect( Vec2(0,0), Vec2(-150,230), Color(0.2,0,1,0.5) );
    g_linep->setLoc(0,200);
    g_linep->setScl(1.0f);
    g_main_layer->insertProp(g_linep);
    // add child to line prop
    Prop2D *childp = new Prop2D();
    childp->setDeck(g_base_deck);
    childp->setScl(16,44);
    childp->seekRot( M_PI * 100, 30 );
    childp->setIndex(0);
    childp->setLoc(-222,-222);
    //g_main_layer->insertProp(childp);
    g_linep->addChild(childp);


    // dynamic images
    {
        g_img = new Image();
        g_img->setSize(256,256);
        for(int i=0;i<256;i++){
            Color c(range(0,1), range(0,1),range(0,1),1);
            g_img->setPixel( i,i, c );
        }
        g_img->writePNG( "dynamic_out.png");
        g_dyn_texture =  new Texture();
        g_dyn_texture->load("assets/base.png");
        g_dyn_texture->setImage(g_img);


    
        Prop2D *p = new Prop2D();
        TileDeck *d = new TileDeck();
        d->setTexture(g_dyn_texture);
        d->setSize( 16,16,16,16);
        p->setDeck(d);
        p->setLoc(200,200);
        p->setScl(128,128);
        p->setIndex(0);
        g_main_layer->insertProp(p);
    }
    
    g_pc = createMyShip(0,0);
    assert(g_pc);

}


void gameRender() {
    g_last_render_cnt = g_moyai_client->render();
}
void gameFinish() {
    glfwTerminate();
}



#if !(TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE)        
int main(int argc, char **argv )
{
    for(int i=0;;i++) {
        if(!argv[i])break;
        if(strcmp(argv[i], "--headless") == 0 ) g_headless_mode = true;
        if(strcmp(argv[i], "--videostream") == 0 ) g_enable_videostream = true;
        if(strcmp(argv[i], "--skip-spritestream") == 0 ) g_enable_spritestream = false;
        if(strcmp(argv[i], "--reprecation") == 0 ) g_enable_reprecation = true;
        if(strcmp(argv[i], "--disable-timestamp")==0) g_disable_timestamp = true;
        if(strcmp(argv[i], "--disable-compression")==0) g_disable_compress = true;
        if(strncmp( argv[i], "--send_wait_ms=", strlen("--send_wait_ms=") ) == 0 ){
            g_send_wait_ms = atoi( argv[i] + strlen("--send_wait_ms="));
        }
    }
        
    gameInit();
    while( !glfwWindowShouldClose(g_window) && (!g_game_done) ){
        gameUpdate();       
        gameRender();
    }
    gameFinish();
    print("program finished");
    return 0;
}
#endif
