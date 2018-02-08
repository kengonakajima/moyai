// demo2d server main

require("./moyai_node.js");

var SCRW=600, SCRH=400;

var g_moyai_client;
var g_rh;

function gameInit() {
    g_moyai_client = new MoyaiClient(SCRW,SCRH);

    var vp = new Viewport();
    vp.setSize(SCRW,SCRH);
    vp.setScale2D(SCRW,SCRH); 

    var cam = new Camera();
    cam.setLoc(0,0);
    var l = new Layer();
    l.setViewport(vp);
    l.setCamera(cam);
    g_moyai_client.insertLayer(l);
    

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
    g_rh.setTargetMoyai(g_moyai_client);
    g_moyai_client.setRemoteHead(g_rh);
    g_rh.startServer(22222);

    g_rh.addPrerequisiteDeck(dk);

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
    g_moyai_client.poll(dt/1000.0);
    if(nt>g_last_print+1000) {
        g_last_print=nt;
        console.log("loop:%d", g_update_cnt);
        
    }
    g_rh.heartbeat(dt/1000.0);    
}




////////////////////


gameInit();

setInterval( gameUpdate, 1000/60.0 );

