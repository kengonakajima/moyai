
var g_ws;

var g_moyai_client;

//var g_keyboard = new Keyboard();
//var g_mouse = new Mouse();
//var g_pad = new Pad();



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
document.getElementById("screen").appendChild( g_moyai_client.renderer.domElement );

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

var g_filedepo = new FileDepo();
g_filedepo.ensure("base.png",base_png);
g_filedepo.ensure("blobloblll.wav", blobloblll_wav );
//g_filedepo.ensure("cinecaption227.ttf", cinecaption227_ttf );
g_filedepo.ensure("dragon8.png", dragon8_png );
g_filedepo.ensure("font_only.png", font_only_png );
//g_filedepo.ensure("gymno1short.wav", gymno1short_wav );


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


/*
  
var sclprot = new Prop2D();
sclprot.setDeck(deck);
sclprot.setIndex(0);
sclprot.setScl(16,16);
sclprot.setLoc(-300,0);
sclprot.setRot( Math.PI/8 );
sclprot.setUVRot(true);
g_main_layer.insertProp(sclprot);    



var colp = new Prop2D();
colp.setColor(1,0,0,1);
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


*/

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
        g.setColor( x,y, new Color(range(0.5,1), range(0.5,1), range(0.5,1), range(0.5,1) ));
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
gridp.addGrid(g);
g_main_layer.insertProp(gridp);

/*

var p2 = new Prop2D();
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

*/



var charcodes = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~あいうえおぁぃぅぇぉかきくけこがぎぐげごさしすせそざじずぜぞたちつてとだぢづでどなにぬねのはひふへほばびぶべぼぱぴぷぺぽまみむめもやゆよゃゅょらりるれろわをん、。アイウエオァィゥェォカキクケコガギグゲゴサシスセソザジズゼゾタチツテトダヂヅデドナニヌネノハヒフヘホバビブベボパピプペポマミムメモヤユヨャュョラリルレロワヲンーッっ　「」";



var g_font = new Font();
g_font.loadFromMemTTF( cinecaption227_ttf, charcodes, 12 );



  
var g_tb = new TextBox();
g_tb.setFont(g_font);
g_tb.setString("!\"$%_-#dummy");
g_tb.setScl(1);
g_tb.setLoc(22,22);
g_main_layer.insertProp(g_tb);

/*
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
g_linep.addLine( new Vec2(0,0), new Vec2(100,-50), new Color(0,1,0,1), 5 );
g_linep.addRect( new Vec2(0,0), new Vec2(-150,230), new Color(0.2,0,1,0.5) );
g_linep.setLoc(0,200);
g_linep.setScl(1.0);
g_main_layer.insertProp(g_linep);
// add child to line prop
var childp = new Prop2D();
childp.setDeck(g_base_deck);
childp.setScl(16,44);
childp.setRot( Math.PI * 0.25 );
childp.setIndex(0);
childp.setLoc(-222,-222);
g_linep.addChild(childp);



  
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


*/

////////////////////

var anim_cnt=0;
var last_anim_at = new Date().getTime();
function animate() {
    if(anim_cnt<50000) {
        anim_cnt++;
	    if(!g_stop_render) requestAnimationFrame( animate );
    }
    if(!g_moyai_client) return;


    g_tb.setString("hoge:"+anim_cnt);
/*
    g_bullet.setLoc(0, 100*Math.sin(anim_cnt/10));
    g_bullet.setIndex(32+(parseInt(anim_cnt/2)%4));
    */    
    
//    g_linep->loc.y = sin( now() ) * 200;


    
/*
    if( (total_frame % 500 ) == 0 ) {
        for(int i=0;i<1000;i++){
            g_img->setPixel( irange(0,256), irange(0,256), Color( range(0,1), range(0,1), range(0,1),1 ) );
        }
    }
    g_dyn_texture->setImage(g_img);
    */


//    g_replacer_shader->setColor( Color(0xF7E26B), Color( range(0,1),range(0,1),range(0,1),1), 0.02 );


/*
    if( g_keyboard->getKey( 'Z' ) ) {
        g_viewport->setScale2D( g_viewport->scl.x / 1.1f, g_viewport->scl.y / 1.1f );
    }
    if( g_keyboard->getKey( 'X' ) ) {
        g_viewport->setScale2D( g_viewport->scl.x * 1.1f, g_viewport->scl.y * 1.1f );        
    }
*/
    
/*
    g_narrow_line_prim->a = Vec2(0,0);
    g_narrow_line_prim->b = Vec2(100, range(100,150));
*/    

/*
    // add/del prims
    {
        static int ylcnt=0;
        ylcnt++;
        static int yellow_line_prim_id=0;
        if( ylcnt%100 == 50 ){
            Prim *yl = g_linep->addLine( Vec2(0,0), Vec2( -30, range(-30,-100)), Color(0,1,1,1), 3 );
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
*/
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
    
    var now_time = new Date().getTime();
    var dt = now_time - last_anim_at;
    last_anim_at = now_time;
    g_moyai_client.poll(dt/1000.0);
    g_moyai_client.render();
    
}

    
animate();
