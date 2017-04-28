var g_moyai_client;

var g_stop_render=false;
function stopRender() {
    g_stop_render = true;
}

/////////// testing

var SCRW=960, SCRH=544;
g_moyai_client = new MoyaiClient(SCRW,SCRH,window.devicePixelRatio);
var screen = document.getElementById("screen");
screen.appendChild( g_moyai_client.renderer.domElement );

var g_viewport = new Viewport();
g_viewport.setSize(SCRW,SCRH);
g_viewport.setScale2D(SCRW,SCRH);

var g_main_layer = new Layer();
g_moyai_client.insertLayer(g_main_layer);
g_main_layer.setViewport(g_viewport);
var g_camera = new Camera();
g_camera.setLoc(0,0);
g_main_layer.setCamera(g_camera);

var g_base_atlas = new Texture();
g_base_atlas.loadPNGMem( base_png );
g_base_deck = new TileDeck();
g_base_deck.setTexture(g_base_atlas);
g_base_deck.setSize(32,32,8,8 );

var t = new Texture();
t.loadPNGMem( base_png );
var deck = new TileDeck();
deck.setTexture(t);
deck.setSize(32,32,8,8);

var p = new Prop2D();
p.setDeck(deck);
p.setIndex(1);
p.setScl(48,48);
p.setLoc(0,0);
g_main_layer.insertProp(p);

var p_over = new Prop2D();
p_over.setDeck(deck);
p_over.setIndex(0);
p_over.setScl(48,48);
p_over.setLoc(20,20);
p_over.setColor(1,1,1,0.5);
g_main_layer.insertProp(p_over);

////////////////////

var last_anim_at = new Date().getTime();
function animate() {
	if(!g_stop_render) requestAnimationFrame( animate );
    var now_time = new Date().getTime();
    var dt = now_time - last_anim_at;
    last_anim_at = now_time;
    g_moyai_client.poll(dt/1000.0);
    g_moyai_client.render();
    
}
    
animate();
