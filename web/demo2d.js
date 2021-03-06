
// button funcs

var g_stop_render=false;
function stopRender() {
    g_stop_render = true;
}


/////////// 

var SCRW=1024, SCRH=512;
Moyai.init(SCRW,SCRH);
var screen = document.getElementById("screen");
var canvas=Moyai.getDomElement();
screen.appendChild(canvas);


var g_keyboard = new Keyboard();
g_keyboard.setupBrowser(window);
var g_mouse = new Mouse();
g_mouse.setupBrowser(window,screen);
var g_touch = new Touch();
g_touch.setupBrowser(window,screen);

var g_viewport = new Viewport();
g_viewport.setSize(SCRW,SCRH);
g_viewport.setScale2D(SCRW,SCRH);
var g_hud_viewport = new Viewport();
g_hud_viewport.setSize(SCRW,SCRH);
g_hud_viewport.setScale2D(SCRW,SCRH);

var g_camera = new OrthographicCamera(-SCRW/2,SCRW/2,SCRH/2,-SCRH/2);
g_camera.setLoc(0,0);
var g_hud_camera = new OrthographicCamera(-SCRW/2,SCRW/2,SCRH/2,-SCRH/2);
g_hud_camera.setLoc(0,0);

var g_main_layer = new Layer();
Moyai.insertLayer(g_main_layer);
g_main_layer.setCamera(g_camera);
g_main_layer.setViewport(g_viewport);

var g_hud_layer = new Layer();
Moyai.insertLayer(g_hud_layer);
g_hud_layer.setCamera(g_hud_camera);
g_hud_layer.setViewport(g_hud_viewport);

var g_base_atlas = new Texture();
g_base_atlas.loadPNG( "./assets/base.png", 256,256 );
g_base_deck = new TileDeck();
g_base_deck.setTexture(g_base_atlas);
g_base_deck.setSize(32,32,8,8 );

var g_bmpfont_atlas = new Texture();
g_bmpfont_atlas.loadPNG("./assets/font_only.png", 256,256);
g_bmpfont_deck = new TileDeck();
g_bmpfont_deck.setTexture( g_bmpfont_atlas );
g_bmpfont_deck.setSize(32,32, 8,8 );

var tmplayer = new Layer();
Moyai.insertLayer(tmplayer);
tmplayer.setViewport(g_viewport);
tmplayer.setCamera(g_camera);

var t = new Texture();
t.loadPNG( "./assets/base.png", 256,256 );
var t2 = new Texture();
t2.loadPNG( "./assets/base.png", 256,256 );

var deck = new TileDeck();
deck.setTexture(t);
deck.setSize(32,32,8,8);
var d2 = new TileDeck();
d2.setTexture(t2);
d2.setSize(32,32,8,8 );

if(1) {
    for(var i=0;i<16;i++) {
        var sclp = new Prop2D();
        sclp.setDeck(deck);
        sclp.setIndex(1);
        sclp.setScl(16,16);
        sclp.setLoc(-200+20*i,0);
        sclp.setRot( Math.PI/8.0 );
        sclp.prop2DPoll = function(dt) {
            if(this.poll_count%3==0) {
                if(this.poll_count%2==0) {
                    this.setDeck(g_bmpfont_deck);
                } else {
                    this.setDeck(g_base_deck);
                }
            }
            if(this.id%3==0) this.setXFlip(this.poll_count%2);
            if(this.id%7==0) this.setYFlip(this.poll_count%2);
            this.setIndex(this.poll_count%4);
            return true;
        }
        g_main_layer.insertProp(sclp);
    }
}

if(1) {
    var sclprot = new Prop2D();
    sclprot.setDeck(g_base_deck);
    sclprot.setIndex(0);
    sclprot.setScl(32,32);
    sclprot.setLoc(-300,0);
    sclprot.setRot( Math.PI/8 );
    sclprot.setUVRot(true);
    g_main_layer.insertProp(sclprot);    
}
if(1) {
    var colp = new Prop2D();
    colp.setColor(0.5,1,1,1);
    colp.setDeck(d2);
    colp.setIndex(1);
    colp.setScl(24,24);
    colp.setLoc(50,-20);
    g_main_layer.insertProp(colp);
}
if(1) {
    var statprimp = new Prop2D(); // a prop that has a prim with no changes
    statprimp.setDeck(deck);
    statprimp.setIndex(1);
    statprimp.setColor(0,0,1,1);
    statprimp.addLine(vec2.fromValues(-1,1),vec2.fromValues(1,-1),Color.fromValues(1,1,1,1), 3);
    statprimp.setLoc(100,-100);
    g_main_layer.insertProp(statprimp);
}


if(1) {
    // static grids
    var gridp = new Prop2D();
    gridp.setDeck(d2);
    gridp.setScl(24,24);
    gridp.setLoc(-100,-300);
    var g = new Grid(8,8);
    g.setDeck(d2);
    var iii=1;
    for(var x=0;x<8;x++){
        for(var y=0;y<8;y++){
            if(y==6||x==6) continue;
            g.set(x,y,iii % 3);
            g.setColor( x,y, Color.fromValues(range(0.5,1), range(0.5,1), range(0.5,1), range(0.2,1) ));
            if(x==0) g.setXFlip(x,y,true);
            if(x==1) g.setYFlip(x,y,true);
            if(x==2) g.setUVRot(x,y,true);
            if(x==3) {
                var ofs = vec2.fromValues(0.5,0.5);
                g.setTexOffset(x,y,ofs);
            }
            iii++;
        }
    }
    g.setUVRot(7,7,true);
    gridp.addGrid(g);
    gridp.prop2DPoll = function(dt) {
        this.grids[0].set(4,4, this.poll_count%4);
        if(this.poll_count%21==0) {
            if(this.poll_count%2==0) {
                this.grids[0].setDeck(g_base_deck);
            } else {
                this.grids[0].setDeck(g_bmpfont_deck);
            }
        }
        return true;
    }
    g_main_layer.insertProp(gridp);
}


if(1) {
    var p2 = new Prop2D(); // index is not set for this prop2d
    p2.setColor(1,1,0,0.5);
    p2.setDeck(d2);
    p2.setScl(12,12);
    p2.setLoc(-100,100);
    p2.addGrid(g);
    tmplayer.insertProp(p2);
}

if(1) {
    // alpha
    var dragontex = new Texture();
    dragontex.loadPNG( "./assets/dragon8.png", 8,8 );

    for(var j=0;j<2;j++) {
        for(var i=0;i<6;i++) {
            var p = new Prop2D();
            p.setTexture(dragontex);
            //p.setDeck(deck);
            p.setScl(32,32);
            p.setColor(1,1,1,0.7);
            p.setLoc(-SCRW/2+50 + i * 10,-SCRH/2+70 + (i%2)*10 + j*80);
            if(j==0) p.use_additive_blend = true;
            g_main_layer.insertProp(p);
        }
    }
}




var charcodes = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~あいうえおぁぃぅぇぉかきくけこがぎぐげごさしすせそざじずぜぞたちつてとだぢづでどなにぬねのはひふへほばびぶべぼぱぴぷぺぽまみむめもやゆよゃゅょらりるれろわをん、。アイウエオァィゥェォカキクケコガギグゲゴサシスセソザジズゼゾタチツテトダヂヅデドナニヌネノハヒフヘホバビブベボパピプペポマミムメモヤユヨャュョラリルレロワヲンーッっ　「」";

var charcodes_dummy_to_save_emacs = " !\"#$%&\'";

var g_font = new Font();
g_font.loadFromTTFFile( "./assets/cinecaption227.ttf", charcodes, 12 );


if(1)  {
    var g_tb = new TextBox();
    g_tb.setFont(g_font);
    g_tb.setString("!\"$%_-#dummyほげ");
    g_tb.setScl(1,1);
    g_tb.setLoc(22,22);
g_tb.prop2DPoll = function(dt) {
    this.setString("hoge:"+this.poll_count);
    return true;
}
    g_main_layer.insertProp(g_tb);

}

if(1) {
    var t4 = new TextBox();
    t4.setFont(g_font);
    t4.setString( "ABC012あいうえお\nあいうえお(utf8)。" );
    t4.setLoc(-100,-90);
    t4.setScl(1.5,1.5);
    g_main_layer.insertProp(t4);
}


if(1) {
    // Check bottom line
    var t5 = new TextBox();
    t5.setFont(g_font);
    t5.setString( "THIS SHOULDN'T SINK UNDER BOTTOM LINE : このもじはしたにしずまない1ぎょうめ\n"+
                  "THIS SHOULDN'T SINK UNDER BOTTOM LINE : このもじはしたにしずまない2ぎょうめ"
                );
    t5.setLoc(-SCRW/2,-SCRH/2);
    t5.setScl(1,1);
    g_main_layer.insertProp(t5);

}

if(1)  {
    // Image manipulation
    var dragonimg = new MoyaiImage();
    dragonimg.loadPNG( "./assets/dragon8.png", 8,8 );
    dragonimg.onload = function() {
        console.log("dragonimg onload");
        for(var y=0;y<8;y++){
            for(var x=0;x<8;x++) {
                var pc = this.getPixelRaw(x,y);
                if( pc.r == 255 && pc.g == 255 && pc.b == 255 && pc.a == 255 ) {
                    //            console.log("setPixelRaw: replacing at: %d,%d",x,y);
                    this.setPixelRaw( x,y, 255,0,0,255 );
                }
            }
        }
        var dragontex0 = new Texture();
        dragontex0.setMoyaiImage( dragonimg );
        // red nose by replacing color
        var dragonp0 = new Prop2D();
        dragonp0.setLoc( SCRW/2-40, 0);
        dragonp0.setTexture( dragontex0 );
        dragonp0.setScl(32,32);
        g_main_layer.insertProp(dragonp0);
        
    }
    var dragontex1 = new Texture();
    dragontex1.loadPNG( "./assets/dragon8.png", 8,8 );        
    dragontex1.onload = function() {
        console.log("dragontex1 onload");
        // white nose
        var dragonp1 = new Prop2D();
        dragonp1.setLoc( SCRW/2-80, 0);
        dragonp1.setTexture(dragontex1);
        dragonp1.setScl(32,32);
        g_main_layer.insertProp(dragonp1);
    }

}


if(1) {
    // bitmap font
    var scorep = new Prop2D();
    scorep.setLoc( -SCRW/2+32,SCRH/2-100 );
    var scoregrid = new CharGrid(8,8);
    scoregrid.setDeck(g_bmpfont_deck );
    scoregrid.setAsciiOffset(-32);
    scoregrid.print( 0,0, Color.fromValues(1,1,1,1), "SCORE: 1234" );
    scoregrid.print( 0,1, Color.fromValues(1,1,0,1), "$#!?()[hoge]" );
    scoregrid.setColor( 3,0, Color.fromValues(0,1,1,1));
    scorep.addGrid(scoregrid);
    g_hud_layer.insertProp(scorep);
}


if(1) {
    // line prop
    var g_linep = new Prop2D();
    g_linep.addRect( vec2.fromValues(-10,-10), vec2.fromValues(0,0), Color.fromValues(1,1,1,1));
    g_linep.addRect( vec2.fromValues(-10,10), vec2.fromValues(0,0), Color.fromValues(1,0,1,1));
    g_linep.addRect( vec2.fromValues(10,-10), vec2.fromValues(0,0), Color.fromValues(0,1,1,1));    
    g_linep.addRect( vec2.fromValues(10,10), vec2.fromValues(0,0), Color.fromValues(1,1,0,1));
    g_linep.addLine( vec2.fromValues(0,0), vec2.fromValues(100,-50), Color.fromValues(0,1,0,1), 5 );
    g_linep.line_prim = g_linep.addLine( vec2.fromValues(0,0), vec2.fromValues(100,100), Color.fromValues(1,0,0,1) );


    g_linep.setLoc(0,200);
    g_linep.setScl(1.0,1.0);
    g_main_layer.insertProp(g_linep);

    g_linep.prop2DPoll = function(dt) {
        this.loc[1] = Math.sin( this.poll_count/50 ) * 200;        
        this.line_prim.a[0] = 0;
        this.line_prim.a[1] = 0;    
        this.line_prim.b[0] = 100;
        this.line_prim.b[1] = range(100,150);
        if( this.poll_count/10%3 == 0 ){
            this.yl = g_linep.addLine(vec2.fromValues(0,0), vec2.fromValues( -30, -60), Color.fromValues(1,1,0,1), 3 );
        }else if( this.poll_count/10%3 == 1 ) {
            if( this.yl ) {
                var yl = g_linep.getPrimById(this.yl.id);
                if(yl) {
                    g_linep.deletePrim(yl.id);
                }
            }
        }
        return true;
    }

    if(1) {
        // add child to line prop
        for(var i=0;i<35;i++) {
            var childp = new Prop2D();
            childp.setDeck(g_base_deck);
            childp.setScl(16,44);
            childp.setRot( Math.PI * 0.25 );
            childp.setIndex(0);
            childp.setLoc(-322+10*i,-222);
            g_linep.addChild(childp);
        }

    }
}

if(1) {
    // dynamic images
    var g_img = new MoyaiImage();
    g_img.setSize(256,256);
    for(var i=0;i<256;i++){
        var c = Color.fromValues( Math.random(), Math.random(), Math.random(),1 );
        g_img.setPixel( i,i, c );
    }
    for(var i=0;i<1000;i++){
        g_img.setPixel( irange(0,256), irange(0,256), Color.fromValues( range(0,1), range(0,1), range(0,1),1 ));
    }
    //g_img.writePNG( "dynamic_out.png");
    var g_dyn_texture =  new Texture();
    g_dyn_texture.setMoyaiImage(g_img);


    var p = new Prop2D();
    var d = new TileDeck();
    d.setTexture(g_dyn_texture);
    d.setSize( 16,16,16,16);
    p.setDeck(d);
    p.setLoc(200,200);
    p.setScl(128,128);
    p.setIndex(0);
    g_main_layer.insertProp(p);

    p.prop2DPoll = function(dt) {
        for(var i=0;i<1000;i++){
            g_img.setPixel( irange(0,256), irange(0,256), Color.fromValues( range(0,1), range(0,1), range(0,1),1 ));
        }
        g_dyn_texture.setMoyaiImage(g_img);
        return true;
    }
}


if(1) {
    var g_bullet = new Prop2D();
    g_bullet.setDeck(g_base_deck);
    g_bullet.setIndex(32);
    g_bullet.setScl(32,32);
    g_bullet.setLoc(0,0);
    g_main_layer.insertProp(g_bullet);

    g_bullet.prop2DPoll = function(dt) {
        this.setLoc(0, 100*Math.sin(this.poll_count/10));
        this.setIndex(32+(Math.floor(this.poll_count/2)%4));
    }
}

if(1) {
    var mat = new ColorReplacerShaderMaterial();
    var g_shader_prop = new Prop2D();
    g_shader_prop.material=mat;
    g_shader_prop.setDeck(g_base_deck);
    g_shader_prop.setIndex(0);
    g_shader_prop.setLoc(0,0);
    g_shader_prop.setUVRot(true);
    g_main_layer.insertProp(g_shader_prop);

    g_shader_prop.prop2DPoll = function(dt) {
        this.material.setColor( Color.fromCode(0xF7E26B), Color.fromValues( range(0,1),range(0,1),range(0,1),1), 0.02);
        return true;
    }
}

if(1) {
    function isMousePressed(prop) {
        if(g_mouse.getButton(0)) {
            var x=g_mouse.cursor_pos[0]-SCRW/2;
            var y=SCRH/2-g_mouse.cursor_pos[1];
            var at=vec2.fromValues(x,y);
            if(prop.hit(at,0)) return true;
        }
        if(g_touch.touching) {
            var x=g_touch.last_touch_pos[0]-SCRW/2;
            var y=SCRH/2-g_touch.last_touch_pos[1];
            var at=vec2.fromValues(x,y);
            if(prop.hit(at,0)) return true;
        }
        return false;
    }
    // mobile controller pad
    var scl=64;
    var ymgn=-100;
    var up=new Prop2D();
    up.setDeck(g_base_deck);
    up.setIndex(192);
    up.setScl(scl,scl);
    up.setColor(1,1,1,0.4);
    up.setLoc(-SCRW/2+scl*2,scl*3 + ymgn);    
    g_hud_layer.insertProp(up);
    var down=new Prop2D();
    down.setDeck(g_base_deck);
    down.setIndex(192);
    down.setScl(scl,scl);
    down.setYFlip(true);
    down.setColor(1,1,1,0.4);    
    down.setLoc(-SCRW/2+scl*2,scl + ymgn);
    g_hud_layer.insertProp(down);
    var left=new Prop2D();
    left.setDeck(g_base_deck);
    left.setIndex(192);
    left.setScl(scl,scl);
    left.setUVRot(true);
    left.setYFlip(true);    
    left.setColor(1,1,1,0.4);    
    left.setLoc(-SCRW/2+scl,scl*2 + ymgn);
    g_hud_layer.insertProp(left);
    var right=new Prop2D();
    right.setDeck(g_base_deck);
    right.setIndex(192);
    right.setScl(scl,scl);
    right.setUVRot(true);
    right.setColor(1,1,1,0.4);    
    right.setLoc(-SCRW/2+scl*3,scl*2+ymgn);
    g_hud_layer.insertProp(right);

    left.prop2DPoll=function(dt) {
        if(isMousePressed(this)) g_camera.setLoc( g_camera.loc[0]-5, g_camera.loc[1] );
        return true;
    }
    right.prop2DPoll=function(dt) {
        if(isMousePressed(this)) g_camera.setLoc( g_camera.loc[0]+5, g_camera.loc[1] );
        return true;
    }
    up.prop2DPoll=function(dt) {
        if(isMousePressed(this)) g_camera.setLoc( g_camera.loc[0], g_camera.loc[1]-5 );
        return true;
    }
    down.prop2DPoll=function(dt) {
        if(isMousePressed(this)) g_camera.setLoc( g_camera.loc[0], g_camera.loc[1]+5 );
        return true;
    }    

}



///////////////


var g_sound_system = new SoundSystem();
var g_explosion_sound = g_sound_system.newSoundFromFile("./assets/blobloblll.wav","wav");
var g_bgm_sound = g_sound_system.newBGMFromFile("./assets/gymno1short.wav","wav");
var samples = new Array(44100/4);
for(var i=0;i<samples.length;i++) samples[i] = Math.cos(i/20.0);
var g_mem_sound = g_sound_system.newSoundFromMemory(samples,"float");


////////////////////

var anim_cnt=0;
var last_anim_at = new Date().getTime();
var g_yellow_line_prim_id;

function animate() {
    if(anim_cnt<50000) {
        anim_cnt++;
	    if(!g_stop_render) requestAnimationFrame( animate );
    }


    if( g_keyboard.getKey('z') ) {
        g_viewport.setScale2D( g_viewport.scl[0] / 1.05, g_viewport.scl[1] / 1.05 );
    }
    if( g_keyboard.getKey('x') ) {
        g_viewport.setScale2D( g_viewport.scl[0] * 1.05, g_viewport.scl[1] * 1.05 );
    }
    if( g_keyboard.getKey('a') ) {
        g_camera.setLoc( g_camera.loc[0]-5, g_camera.loc[1] );
    }
    if( g_keyboard.getKey('d') ) {
        g_camera.setLoc( g_camera.loc[0]+5, g_camera.loc[1] );
    }

    if( g_keyboard.getToggled( 'u' ) ) {
        g_keyboard.clearToggled('u');
        g_mem_sound.play();
    }
    if( g_keyboard.getToggled( 'r' ) ) {
        g_keyboard.clearToggled('r');
        g_mem_sound.play(0.2);
    }    
    if( g_keyboard.getToggled( 't' ) ) {
        g_keyboard.clearToggled('t');        
        g_bgm_sound.play();
    }
    if( g_keyboard.getToggled('o')) {
        g_bgm_sound.setTimePositionSec(9);        
    }    
    
    if( g_keyboard.getToggled( 'y' ) ) {
        g_keyboard.clearToggled('y');
        g_explosion_sound.play();
    }

    var now_time = new Date().getTime();
    var dt = now_time - last_anim_at;
    last_anim_at = now_time;
    Moyai.poll(dt/1000.0);
    Moyai.render();
    
}




animate();
