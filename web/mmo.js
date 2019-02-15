

var g_stop_render=false;
function stopRender() {
    g_stop_render = true;
}

/////////// testing

var g_pixelratio = window.devicePixelRatio || 1;

var SCRW=Math.floor(window.innerWidth * g_pixelratio);
var SCRH=Math.floor(window.innerHeight * g_pixelratio);

Moyai.init(SCRW,SCRH);
var screen = document.getElementById("screen");
var canvas=Moyai.getDomElement();
screen.appendChild(canvas);
canvas.width=SCRW;
canvas.height=SCRH;

canvas.style="width:100%; height:100%";




var g_keyboard = new Keyboard();
g_keyboard.setupBrowser(window);

var g_viewport = new Viewport();
g_viewport.setSize(SCRW,SCRH);
g_viewport.setScale2D(SCRW,SCRH);

var g_main_layer = new Layer();
Moyai.insertLayer(g_main_layer);
g_main_layer.setViewport(g_viewport);

var g_camera = new OrthographicCamera(-SCRW/2,SCRW/2,SCRH/2,-SCRH/2);
g_camera.setLoc(0,0);
g_main_layer.setCamera(g_camera);

var g_base_atlas = new Texture();
g_base_atlas.loadPNG( "./assets/base.png", 256,256 );
g_base_deck = new TileDeck();
g_base_deck.setTexture(g_base_atlas);
g_base_deck.setSize(32,32,8,8 );

var g_base_deck = new TileDeck();
g_base_deck.setTexture(g_base_atlas);
g_base_deck.setSize(32,32,8,8);

var bg = new Prop2D();
bg.setLoc(-SCRW/2,-SCRH/2);
var g=new Grid(200,100);
g.setDeck(g_base_deck);
g.fill(3);
bg.addGrid(g);
var gt=new Grid(200,200);
gt.setDeck(g_base_deck);
for(var i=0;i<400;i++) gt.set(irange(0,200),irange(0,200),2);
bg.addGrid(gt);
bg.setScl(16,16);
g_main_layer.insertProp(bg);

var pc = new Prop2D();
pc.setLoc(-150,-50);
pc.setIndex(1);
pc.setScl(16,16);
pc.setDeck(g_base_deck);
g_main_layer.insertProp(pc);
pc.prop2DPoll = function(dt) {
    if(g_keyboard.getKey('a')) {
        this.loc[0]-=5;
    }
    if(g_keyboard.getKey('d')) {
        this.loc[0]+=5;
    }
    if(g_keyboard.getKey('w')) {
        this.loc[1]+=5;
    }
    if(g_keyboard.getKey('s')) {
        this.loc[1]-=5;
    }
    return true;
}



////////////////////

var last_anim_at = new Date().getTime();
function animate() {
	if(!g_stop_render) requestAnimationFrame( animate );
    var now_time = new Date().getTime();
    var dt = now_time - last_anim_at;
    last_anim_at = now_time;
    Moyai.poll(dt/1000.0);
    Moyai.render();    
}
    
animate();
