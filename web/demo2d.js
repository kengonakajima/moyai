
var g_ws;

var g_moyai_client;


function onPacket(ws,pkttype,argdata) {
    switch(pkttype) {
    case PACKETTYPE_ZIPPED_RECORDS:
        {
            //                console.log("zipped records:",argdata);
            var uncompressed = Snappy.uncompress(argdata.buffer);
            console.log(uncompressed);
            var dv = new DataView(uncompressed);
            ws.unzipped_rb.push(dv,uncompressed.byteLength);
            console.log("unzipped_rb:",ws.unzipped_rb);
        }
        break;
    case PACKETTYPE_S2C_WINDOW_SIZE:
        {
            var dv = new DataView(argdata.buffer);
            var w = dv.getUint32(0);
            var h = dv.getUint32(4);
            console.log("received window_size:",w,h);
        }
        break;
    }
}




// button funcs
function connectButton() {
    g_ws = createWSClient("ws://localhost:8888/");
    g_ws.onPacket = onPacket;
}
function disconnectButton() {
    g_ws.close();
}
var g_stop_render=false;
function stopRender() {
    g_stop_render = true;
}


/////////// testing

var SCRW=960, SCRH=544;
g_moyai_client = new MoyaiClient(SCRW,SCRH,window.devicePixelRatio);
var screen = document.getElementById("screen");
screen.appendChild( g_moyai_client.renderer.domElement );

var g_keyboard = new Keyboard();
g_keyboard.setupBrowser(window);
var g_mouse = new Mouse();
g_mouse.setupBrowser(window,screen);


var g_viewport = new Viewport();
g_viewport.setSize(SCRW,SCRH);
g_viewport.setScale2D(SCRW,SCRH);

var g_main_layer = new Layer();
g_moyai_client.insertLayer(g_main_layer);
g_main_layer.setViewport(g_viewport);

var g_hud_layer = new Layer();
g_moyai_client.insertLayer(g_hud_layer);
g_hud_layer.setViewport(g_viewport);

var g_camera = new Camera();
g_camera.setLoc(0,0);
g_main_layer.setCamera(g_camera);

var g_base_atlas = new Texture();
g_base_atlas.loadPNGMem( base_png );
g_base_deck = new TileDeck();
g_base_deck.setTexture(g_base_atlas);
g_base_deck.setSize(32,32,8,8 );

var g_bmpfont_atlas = new Texture();
g_bmpfont_atlas.loadPNGMem(font_only_png);
g_bmpfont_deck = new TileDeck();
g_bmpfont_deck.setTexture( g_bmpfont_atlas );
g_bmpfont_deck.setSize(32,32, 8,8 );

var tmplayer = new Layer();
g_moyai_client.insertLayer(tmplayer);
tmplayer.setViewport(g_viewport);

var t = new Texture();
t.loadPNGMem( base_png );
var t2 = new Texture();
t2.loadPNGMem( base_png );

var deck = new TileDeck();
deck.setTexture(t);
deck.setSize(32,32,8,8);
var d2 = new TileDeck();
d2.setTexture(t2);
d2.setSize(32,32,8,8 );


var sclpary=[];
for(var i=0;i<16;i++) {
    var sclp = new Prop2D();
    sclp.setDeck(deck);
    sclp.setIndex(1);
    sclp.setScl(16,16);
    sclp.setLoc(-200+20*i,0);
    sclp.setRot( Math.PI/8.0 );
    g_main_layer.insertProp(sclp);
    sclpary.push(sclp);
}



  
var sclprot = new Prop2D();
sclprot.setDeck(deck);
sclprot.setIndex(0);
sclprot.setScl(16,16);
sclprot.setLoc(-300,0);
sclprot.setRot( Math.PI/8 );
sclprot.setUVRot(true);
g_main_layer.insertProp(sclprot);    



var colp = new Prop2D();
colp.setColor(0,1,0,1);
colp.setDeck(d2);
colp.setIndex(1);
colp.setScl(24,24);
colp.setLoc(50,-20);
g_main_layer.insertProp(colp);


var statprimp = new Prop2D(); // a prop that has a prim with no changes
statprimp.setDeck(g_base_deck);
statprimp.setIndex(1);
statprimp.setColor(0,0,1,1);
statprimp.addLine(new Vec2(-1,1),new Vec2(1,-1),new Color(1,1,1,1), 3);
statprimp.setLoc(100,-100);
statprimp.debug = true;
g_main_layer.insertProp(statprimp);



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
        g.setColor( x,y, new Color(range(0.5,1), range(0.5,1), range(0.5,1), range(0.2,1) ));
        if(x==0) g.setXFlip(x,y,true);
        if(x==1) g.setYFlip(x,y,true);
        if(x==2) g.setUVRot(x,y,true);
        if(x==3) {
            var ofs = new Vec2(0.5,0.5);
            g.setTexOffset(x,y,ofs);
        }
        iii++;
    }
}
g.setUVRot(7,7,true);
gridp.addGrid(g);
g_main_layer.insertProp(gridp);






var p2 = new Prop2D(); // index is not set for this prop2d
p2.setColor(1,1,0,0.5);
p2.setDeck(d2);
p2.setScl(12,12);
p2.setLoc(-100,100);
p2.addGrid(g);
tmplayer.insertProp(p2);



// alpha
var dragontex = new Texture();
dragontex.loadPNGMem( dragon8_png );

for(var j=0;j<2;j++) {
    for(var i=0;i<6;i++) {
        var p = new Prop2D();
        p.setTexture(dragontex);
        p.setScl(32,32);
        p.setColor(1,1,1,0.3);
        p.setLoc(-SCRW/2+50 + i * 10,-SCRH/2+70 + (i%2)*10 + j*80);
        if(j==0) p.use_additive_blend = true;
        //p->setLoc(-200,-200);
        g_main_layer.insertProp(p);
    }
}





var charcodes = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~あいうえおぁぃぅぇぉかきくけこがぎぐげごさしすせそざじずぜぞたちつてとだぢづでどなにぬねのはひふへほばびぶべぼぱぴぷぺぽまみむめもやゆよゃゅょらりるれろわをん、。アイウエオァィゥェォカキクケコガギグゲゴサシスセソザジズゼゾタチツテトダヂヅデドナニヌネノハヒフヘホバビブベボパピプペポマミムメモヤユヨャュョラリルレロワヲンーッっ　「」";



var g_font = new Font();
g_font.loadFromMemTTF( cinecaption227_ttf, charcodes, 12 );



  
var g_tb = new TextBox();
g_tb.setFont(g_font);
g_tb.setString("!\"$%_-#dummyほげ");
g_tb.setScl(1);
g_tb.setLoc(22,22);
g_main_layer.insertProp(g_tb);




var t4 = new TextBox();

t4.setFont(g_font);
t4.setString( "ABC012あいうえお\nあいうえお(utf8)。" );
t4.setLoc(-100,-90);
t4.setScl(0.75);
g_main_layer.insertProp(t4);


  
// Check bottom line
var t5 = new TextBox();
t5.setFont(g_font);
t5.setString( "THIS SHOULDN'T SINK UNDER BOTTOM LINE : このもじはしたにしずまない1ぎょうめ\n"+
              "THIS SHOULDN'T SINK UNDER BOTTOM LINE : このもじはしたにしずまない2ぎょうめ"
            );
t5.setLoc(-SCRW/2,-SCRH/2);
t5.setScl(1);
g_main_layer.insertProp(t5);



  
// Image manipulation
var dragonimg = new Image();
dragonimg.loadPNGMem( dragon8_png );
for(var y=0;y<8;y++){
    for(var x=0;x<8;x++) {
        var pc = dragonimg.getPixelRaw(x,y);
        if( pc.r == 255 && pc.g == 255 && pc.b == 255 && pc.a == 255 ) {
//            console.log("setPixelRaw: replacing at: %d,%d",x,y);
            dragonimg.setPixelRaw( x,y, 255,0,0,255 );
        }
    }
}



var dragontex0 = new Texture();
dragontex0.setImage( dragonimg );
var dragontex1 = new Texture();
dragontex1.loadPNGMem( dragon8_png );


// white nose
var dragonp1 = new Prop2D();
dragonp1.setLoc( SCRW/2-80, 0);
dragonp1.setTexture(dragontex1);
dragonp1.setScl(32);
g_main_layer.insertProp(dragonp1);    


// red nose by replacing color
var dragonp0 = new Prop2D();
dragonp0.setLoc( SCRW/2-40, 0);
dragonp0.setTexture( dragontex0 );
dragonp0.setScl(32);
g_main_layer.insertProp(dragonp0);




// bitmap font
var scorep = new Prop2D();
scorep.setLoc( -SCRW/2+32,SCRH/2-100 );
var scoregrid = new CharGrid(8,8);
scoregrid.setDeck(g_bmpfont_deck );
scoregrid.setAsciiOffset(-32);
scoregrid.printf( 0,0, new Color(1,1,1,1), "SCORE: %d",1234 );
scoregrid.printf( 0,1, new Color(1,1,0,1), "$#!?()[%s]", "hoge" );
scoregrid.setColor( 3,0, new Color(0,1,1,1));
scorep.addGrid(scoregrid);
g_main_layer.insertProp(scorep);




// line prop
var g_linep = new Prop2D();
var g_narrow_line_prim = g_linep.addLine( new Vec2(0,0), new Vec2(100,100), new Color(1,0,0,1) );
//g_linep.addLine( new Vec2(0,0), new Vec2(range(-100,100),range(-50,50)), new Color(range(0,1),range(0,1),range(0,1),1), 5 );
g_linep.addLine( new Vec2(0,0), new Vec2(100,-50), new Color(0,1,1,1), 5 );
g_linep.addRect( new Vec2(0,0), new Vec2(-150,230), new Color(0.2,0,1,0.5) );
//for(var i=0;i<60;i++){
//   g_linep.addRect( new Vec2(0,0), new Vec2(range(-150,100),range(-200,230)), new Color(0.2,range(0,1),1,0.5) );
//}
g_linep.setLoc(0,200);
g_linep.setScl(1.0);
g_main_layer.insertProp(g_linep);



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




  
// dynamic images
var g_img = new Image();
g_img.setSize(256,256);
for(var i=0;i<256;i++){
    var c = new Color( Math.random(), Math.random(), Math.random(),1 );
    g_img.setPixel( i,i, c );
}
//g_img.writePNG( "dynamic_out.png");
var g_dyn_texture =  new Texture();
g_dyn_texture.setImage(g_img);


var p = new Prop2D();
var d = new TileDeck();
d.setTexture(g_dyn_texture);
d.setSize( 16,16,16,16);
p.setDeck(d);
p.setLoc(200,200);
p.setScl(128,128);
p.setIndex(0);
g_main_layer.insertProp(p);

var g_bullet = new Prop2D();
g_bullet.setDeck(g_base_deck);
g_bullet.setIndex(32);
g_bullet.setScl(32);
g_bullet.setLoc(0,0);
g_main_layer.insertProp(g_bullet);


var g_replacer_shader = new ColorReplacerShader();
var g_shader_prop = new Prop2D();
g_shader_prop.setFragmentShader(g_replacer_shader);
g_shader_prop.setDeck(g_base_deck);
g_shader_prop.setIndex(0);
g_shader_prop.setLoc(0,0);
g_shader_prop.setUVRot(true);
g_main_layer.insertProp(g_shader_prop);




var g_sound_system = new SoundSystem();
var g_explosion_sound = g_sound_system.newSoundFromMemory(blobloblll_wav,"wav");
var g_bgm_sound = g_sound_system.newBGMFromMemory(gymno1short_wav,"wav");
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
    if(!g_moyai_client) return;


    g_tb.setString("hoge:"+anim_cnt);
    

    g_bullet.setLoc(0, 100*Math.sin(anim_cnt/10));
    g_bullet.setIndex(32+(parseInt(anim_cnt/2)%4));


    g_linep.loc.y = Math.sin( anim_cnt/50 ) * 200;

    

    if( (anim_cnt % 100 ) == 0 ) {
        for(var i=0;i<1000;i++){
            g_img.setPixel( irange(0,256), irange(0,256), new Color( range(0,1), range(0,1), range(0,1),1 ));
        }
        g_dyn_texture.setImage(g_img);
    }
    


    g_replacer_shader.setColor( new Color(0xF7E26B), new Color( range(0,1),range(0,1),range(0,1),1), 0.02 );

    if( g_keyboard.getKey('z') ) {
        g_viewport.setScale2D( g_viewport.scl.x / 1.05, g_viewport.scl.y / 1.05 );
    }
    if( g_keyboard.getKey('x') ) {
        g_viewport.setScale2D( g_viewport.scl.x * 1.05, g_viewport.scl.y * 1.05 );
    }
    if( g_keyboard.getKey('a') ) {
        console.log(g_camera.loc);
        g_camera.setLoc( g_camera.loc.x-5, g_camera.loc.y );
    }
    if( g_keyboard.getKey('d') ) {
        g_camera.setLoc( g_camera.loc.x+5, g_camera.loc.y );
    }

    if( g_keyboard.getToggled( 'u' ) ) {
        g_keyboard.clearToggled('u');
        g_mem_sound.play();
    }
    if( g_keyboard.getToggled( 't' ) ) {
        g_keyboard.clearToggled('t');        
        g_bgm_sound.play();
    }
    if( g_keyboard.getToggled( 'y' ) ) {
        g_keyboard.clearToggled('y');
        g_explosion_sound.play();
    }

    g_narrow_line_prim.a.x = 0;
    g_narrow_line_prim.a.y = 0;    
    g_narrow_line_prim.b.x = 100;
    g_narrow_line_prim.b.y = range(100,150);



    // add/del prims
    if(g_linep){
        if( anim_cnt/10%3 == 0 ){
            var yl = g_linep.addLine( new Vec2(0,0), new Vec2( -30, -60), new Color(1,1,0,1), 3 );
            g_yellow_line_prim_id = yl.id;
        }else if( anim_cnt/10%3 == 1 ) {
            if( g_yellow_line_prim_id ) {
                var yl = g_linep.getPrimById(g_yellow_line_prim_id);
                if(yl) {
                    g_linep.deletePrim(yl.id);
                }
            }
        }
    }

    

    if( anim_cnt % 3==0 ) {
        if(anim_cnt%2==0) {
            for(var i in sclpary) sclpary[i].setDeck(g_bmpfont_deck);
        } else {
            for(var i in sclpary) sclpary[i].setDeck(g_base_deck);
        }
        for(var i in sclpary) {
            if(i%5==0) sclpary[i].setXFlip(anim_cnt%2);
            if(i%7==0) sclpary[i].setYFlip(anim_cnt%2);
        }
    }
        
    for(var i in sclpary) {
        sclpary[i].setIndex(anim_cnt%4);
    }

    
    gridp.grids[0].set(4,4, anim_cnt%4);

    if(anim_cnt%21==0) {
        if(anim_cnt%2==0) {
            gridp.grids[0].setDeck(g_base_deck);
        } else {
            gridp.grids[0].setDeck(g_bmpfont_deck);
        }
    }

    if(anim_cnt%9==0) {
        var pp = new Prop2D();
        pp.setDeck(g_base_deck);
        pp.setIndex( parseInt(range(0,4)));
        pp.setScl(8,8);
        pp.setLoc(range(-300,300),range(-300,300));;
        pp.propPoll = function(dt) {
            if(pp.accum_time>3) return false; else return true;
        }
        g_main_layer.insertProp(pp);                     
    }

    var now_time = new Date().getTime();
    var dt = now_time - last_anim_at;
    last_anim_at = now_time;
    g_moyai_client.poll(dt/1000.0);
    g_moyai_client.render();
    
}

    
animate();
