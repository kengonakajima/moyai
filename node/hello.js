// demo2d server main

require("./moyai_node.js");



var SCRW=600, SCRH=400;

var g_moyai_client;
var g_rh;

var g_p;
var g_grid;

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

    g_grid = new Grid(4,4);
    g_grid.setDeck(dk);
    g_grid.set(0,0,0);
    g_grid.set(1,1,1);
    g_grid.set(2,2,2);
    g_grid.set(3,3,3);            
    
    g_p = new Prop2D();
    g_p.setDeck(dk);
    g_p.setIndex(1);
    g_p.setScl(32);
    g_p.setLoc(0,0);
    g_p.addGrid(g_grid);
    l.insertProp(g_p);

    

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
    if(g_update_cnt%30==0) {
        g_p.loc.x=Math.sin(g_p.accum_time)*80;
        g_p.setIndex( Math.floor(g_p.accum_time*10)%2);
    }
    if(g_update_cnt%21==0) {
        g_grid.set(2,2,irange(0,3));
        g_grid.setXFlip(1,2,irange(0,2));
        g_grid.setColor(2,1,new Color(range(0,1),range(0,1),range(0,1),range(0,1)) );
        var texofs=new Vec2(range(0,1), range(0,1));
        g_grid.setTexOffset(3,2,texofs);
    }
    


    g_rh.heartbeat(dt/1000.0);    
}




////////////////////


gameInit();

setInterval( gameUpdate, 1000/60.0 );

