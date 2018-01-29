// demo2d server main

require("../web/moyai_common.js");
require("./moyai_node.js");
var net=require("net");

var g_moyai;

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
}

var g_last_update_at=0;

function gameUpdate() {
    var nt=new Date().getTime();
    var dt=nt-g_last_update_at;
    g_last_update_at=nt;
    g_moyai.poll(dt/1000.0);
}




////////////////////

var server = net.createServer(function(conn) {
    console.log("newconnection");
});
server.listen(22222);

gameInit();

setInterval( gameUpdate, 1000/60.0 );