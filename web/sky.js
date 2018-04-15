var g_moyai_client;

var g_stop_render=false;
function stopRender() {
    g_stop_render = true;
}

var SCRW=800, SCRH=600;

g_moyai_client = new MoyaiClient(SCRW,SCRH,window.devicePixelRatio);
var screen = document.getElementById("screen");
screen.appendChild( g_moyai_client.renderer.domElement );


var g_keyboard = new Keyboard();
g_keyboard.setupBrowser(window);
var g_mouse = new Mouse();
g_mouse.setupBrowser(window,screen);


var g_viewport3d = new Viewport();
g_viewport3d.setSize(SCRW,SCRH);
g_viewport3d.setClip3D( 0.01, 100 );

var g_viewport2d = new Viewport();
g_viewport2d.setSize(SCRW,SCRH);
g_viewport2d.setScale2D(SCRW,SCRH);

var g_main_layer = new Layer();
g_moyai_client.insertLayer(g_main_layer);
g_main_layer.setViewport(g_viewport3d);

var g_main_camera = new Camera();
g_main_camera.setLoc(-4,4,20);
g_main_camera.setLookAt(new Vec3(0,0,0), new Vec3(0,1,0));
g_main_layer.setCamera(g_main_camera);

var g_hud_layer = new Layer();
g_hud_layer.setViewport(g_viewport2d);
g_moyai_client.insertLayer(g_hud_layer);
g_hud_camera = new Camera();
g_hud_camera.setLoc(0,0);
g_hud_layer.setCamera( g_hud_camera );

var g_skyblue_tex = new Texture();
g_skyblue_tex.loadPNGMem( skyblue_png );
var g_skyblue_deck = new TileDeck();
g_skyblue_deck.setTexture(g_skyblue_tex);
g_skyblue_deck.setSize(4,4,16,16);

var g_skydawn_tex = new Texture();
g_skydawn_tex.loadPNGMem( skydawn_png );
var g_skydawn_deck = new TileDeck();
g_skydawn_deck.setTexture(g_skydawn_tex);
g_skydawn_deck.setSize(4,4,16,16);

var g_skynight_tex = new Texture();
g_skynight_tex.loadPNGMem( skynight_png );
var g_skynight_deck = new TileDeck();
g_skynight_deck.setTexture(g_skynight_tex);
g_skynight_deck.setSize(4,4,16,16);

var g_sun_tex = new Texture();
g_sun_tex.loadPNGMem(sun_png);

var g_light = new Light();
g_light.pos = new Vec3(-50,50,100);
g_main_layer.setLight(g_light);

var g_blue_prop = new Prop2D();
g_blue_prop.setLoc(-300,32);
g_blue_prop.setScl(128,128);
g_blue_prop.setDeck(g_skyblue_deck);
g_blue_prop.setIndex(4);
g_hud_layer.insertProp(g_blue_prop);

var g_dawn_prop = new Prop2D();
g_dawn_prop.setLoc(-300,200);
g_dawn_prop.setScl(128,128);
g_dawn_prop.setDeck(g_skydawn_deck);
g_dawn_prop.setIndex(4);
g_hud_layer.insertProp(g_dawn_prop);


var g_night_prop = new Prop2D();
g_night_prop.setLoc(-300,-200+64);
g_night_prop.setScl(128,128);
g_night_prop.setDeck(g_skynight_deck);
g_night_prop.setIndex(4);
g_hud_layer.insertProp(g_night_prop);


var g_outp0 = new Prop2D();
g_outp0.setLoc(100,0);
g_outp0.setScl(512,256);
g_outp0.setDeck(g_skynight_deck);
g_outp0.setIndex(4);
g_hud_layer.insertProp(g_outp0);

var g_outp1 = new Prop2D();
g_outp1.setLoc(108,0);
g_outp1.setScl(512,256);
g_outp1.setDeck(g_skydawn_deck);
g_outp1.setIndex(4);
g_hud_layer.insertProp(g_outp1);

var g_outp2 = new Prop2D();
g_outp2.setLoc(116,0);
g_outp2.setScl(512,256);
g_outp2.setDeck(g_skyblue_deck);
g_outp2.setIndex(4);
g_hud_layer.insertProp(g_outp2);

var g_sun_sample = new Prop2D();
g_sun_sample.setLoc(-300,-240);
g_sun_sample.setScl(128,128);
g_sun_sample.setTexture(g_sun_tex);
g_sun_sample.use_additive_blend=true;
g_hud_layer.insertProp(g_sun_sample);

var g_sunp = new Prop2D();
g_sunp.setLoc(100,0);
g_sunp.setScl(128,128);
g_sunp.setTexture(g_sun_tex);
g_sunp.use_additive_blend=true;
g_hud_layer.insertProp(g_sunp);

// hour: 0~24
function updateSkyColor() {
    var base_sec=(g_outp0.accum_time/2*60*60) % (24*60*60);
    var h = base_sec / 60.0/60.0

    var blue_alpha=0.5;
    var dawn_alpha=0;
    var night_alpha=1; // 夜はいつも1

    var blue_start_h=4; //昼のはじまり
    var blue_max_h=7; // 昼が最大になる時間
    var blue_dim_start_h=15; // 昼の減衰開始
    var blue_min_h=20; // 昼が最小(0)になる時間

    if(h>blue_start_h && h<blue_max_h) {
        blue_alpha=(h-blue_start_h)/(blue_max_h-blue_start_h);
    } else if(h>=blue_max_h&&h<blue_dim_start_h) {
        blue_alpha=1;
    } else if(h>=blue_dim_start_h&&h<blue_min_h)  {
        blue_alpha=1-(h-blue_dim_start_h)/(blue_min_h-blue_dim_start_h);
    } else {
        blue_alpha=0;
    }

    var sunrise_start_h=3;
    var sunrise_max_h=5;
    var sunrise_min_h=7;
    var dawn_start_h=17;
    var dawn_max_h=19;
    var dawn_min_h=21;
    
    if(h>sunrise_start_h&&h<=sunrise_max_h) {
        dawn_alpha=(h-sunrise_start_h)/(sunrise_max_h-sunrise_start_h);
    } else if(h>sunrise_max_h&&h<=sunrise_min_h) {
        dawn_alpha=1-(h-sunrise_max_h)/(sunrise_min_h-sunrise_max_h);
    } else if(h>dawn_start_h&&h<=dawn_max_h) {
        dawn_alpha=(h-dawn_start_h)/(dawn_max_h-dawn_start_h);
    } else if(h>dawn_max_h&&h<=dawn_min_h) {
        dawn_alpha=1-(h-dawn_max_h)/(dawn_min_h-dawn_max_h);
    }else {
        dawn_alpha=0;
    }

    var rad=(h/24.0)*2.0*Math.PI - Math.PI/2.0;
    g_sunp.setLoc(100,Math.sin(rad)*100);
    var sun_r=1, sun_g=1, sun_b=1;
    var sun_start_sunrise=4;
    var sun_max=7;
    var sun_dim_start=17;
    var sun_min=20;
    
    // green and blue of sun
    if(h>sun_start_sunrise&&h<=sun_max) {
        sun_g=sun_b=(h-sun_start_sunrise)/(sun_max-sun_start_sunrise);
    } else if(h>sun_max&&h<=sun_dim_start) {
        sun_g=sun_b=1;
    } else if(h>sun_dim_start&&h<=sun_min) {
        sun_g=sun_b=1-(h-sun_dim_start)/(sun_min-sun_dim_start);
    } else {
        sun_g=sun_b=0;
    }
    // red of sun
    var sun_morning_min=3;
    var sun_red_dim_start=19
    var sun_night_min=22;

    if(h<sun_morning_min) {
        sun_r=0;
    } else if(h>=sun_morning_min&&h<sun_start_sunrise) {
        sun_r=(h-sun_morning_min)/(sun_start_sunrise-sun_morning_min);
    } else if(h>=sun_start_sunrise&&h<=sun_red_dim_start) {
        sun_r=1;
    } else if(h>=sun_red_dim_start&&h<=sun_night_min){
        sun_r=1-(h-sun_red_dim_start)/(sun_night_min-sun_red_dim_start);
    } else {
        sun_r=0;
    }

    console.log("hour:",h, "blue:",blue_alpha, "dawn:",dawn_alpha, "sun_rgb:",sun_r,sun_g,sun_b);        

    g_sunp.setColor(new Color(sun_r,sun_g,sun_b,1));
    g_sun_sample.setColor(new Color(sun_r,sun_g,sun_b,1));
    
    g_outp0.setColor(new Color(1,1,1,night_alpha));
    g_outp1.setColor(new Color(1,1,1,dawn_alpha));
    g_outp2.setColor(new Color(1,1,1,blue_alpha));
}

var last_anim_at = new Date().getTime();

var fps=0;
function animate() {
    if(!g_stop_render) requestAnimationFrame(animate);
    if(!g_moyai_client)return;

    fps++;


    var now_time = new Date().getTime();
    var dt = (now_time - last_anim_at) / 1000.0;

    
    updateSkyColor();

    
    last_anim_at = now_time;    
    g_moyai_client.poll(dt);
    g_moyai_client.render();


    //    g_main_camera.setLoc( g_main_camera.loc.x+0.1,0,3);
    
    
}

animate();

