// demo2d server main

require("../web/moyai_common.js");
require("./moyai_node.js");

var SCRW=600, SCRH=400;

var g_moyai;
var g_rh;

function gameInit() {
    g_moyai = new Moyai();

    var vp = new Viewport();
    vp.setSize(SCRW,SCRH);
    vp.setScale2D(SCRW,SCRH); 
    
    var l = new Layer();
    g_moyai.insertLayer(l);
    

    var tex = new Texture();
    tex.loadPNG( "../assets/base.png" );
    var dk = new TileDeck();
    dk.setTexture(tex);
    dk.setSize(32,32,8,8 );
    
    var p = new Prop2D();
    p.setDeck(dk);
    p.setIndex(1);
    p.setScl(32);
    p.setLoc(0,0);
    l.insertProp(p);

    // set up spritestream
    g_rh = new RemoteHead();
    g_rh.setTargetMoyai(g_moyai);
    g_moyai.setRemoteHead(g_rh);

    console.log("gameINit done");
}

var g_last_update_at=0;

var g_update_cnt=0;
var g_last_print=0;
function gameUpdate() {
    g_update_cnt++;

    var nt=new Date().getTime();
    var dt=nt-g_last_update_at;
    g_last_update_at=nt;
    g_moyai.poll(dt/1000.0);
    if(nt>g_last_print+1000) {
        g_last_print=nt;
        console.log("loop:%d", g_update_cnt);
        
    }
    
}




////////////////////


gameInit();

setInterval( gameUpdate, 1000/60.0 );

