// demo2d server main

require("../web/moyai_common.js");
require("./moyai_node.js");


var g_moyai;
var g_rh;

function gameInit() {
    g_moyai = new Moyai();
    var l = new Layer();
    console.log(l);
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
}

var g_last_update_at=0;

function gameUpdate() {
    var nt=new Date().getTime();
    var dt=nt-g_last_update_at;
    g_last_update_at=nt;
    g_moyai.poll(dt/1000.0);

    
}




////////////////////


gameInit();

setInterval( gameUpdate, 1000/60.0 );

