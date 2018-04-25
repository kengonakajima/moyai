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
p.addRect( new Vec2(0,0), new Vec2(5,5), new Color(1,1,1,0.3));
g_main_layer.insertProp(p);




var p_over = new Prop2D();
p_over.setDeck(deck);
p_over.setIndex(0);
p_over.setScl(48,48);
p_over.setLoc(20,20);
p_over.setColor(1,1,1,1);
g_main_layer.insertProp(p_over);


// grid test
if(1) {
    var p_grid_under = new Prop2D();
    p_grid_under.setLoc(-200,-100);
    var g=new Grid(4,4);
    g.setDeck(g_base_deck);
    for(var y=0;y<4;y++) for(var x=0;x<4;x++) g.set(x,y,0);
    p_grid_under.addGrid(g);
    p_grid_under.setScl(24);
    g_main_layer.insertProp(p_grid_under);

    var p_grid_over = new Prop2D();
    p_grid_over.setLoc(-150,-50);
    g=new Grid(4,4);
    g.setDeck(g_base_deck);
    for(var y=0;y<4;y++) for(var x=0;x<4;x++) g.set(x,y,32);
    p_grid_over.addGrid(g);
    p_grid_over.setScl(24);
    g_main_layer.insertProp(p_grid_over);
}

// custom mesh test
if(1) {
    var mat = createMeshBasicMaterial( { map: g_base_deck.moyai_tex.three_tex,
                                         depthTest:false,
                                         transparent:true,
                                         vertexColors: THREE.VertexColors,
                                         blending: THREE.NormalBlending } );
    var geom = new THREE.Geometry();
    geom.vertices.push(new THREE.Vector3(0,0,0));
    geom.vertices.push(new THREE.Vector3(2,0,0));
    geom.vertices.push(new THREE.Vector3(0,2,0));
    geom.faces.push(new THREE.Face3(0,1,2));
    geom.faces[0].vertexColors = [ new Color(1,1,1,1).toTHREEColor(), new Color(1,1,0,1).toTHREEColor(), new Color(0,1,1,1).toTHREEColor() ];
    geom.faceVertexUvs[0].push([new THREE.Vector2(0,0), new THREE.Vector2(0.1,0), new THREE.Vector2(0,0.1)]);
    
    var mesh = new THREE.Mesh(geom,mat);
    var p_custom = new Prop2D();
    p_custom.setLoc(200,-100);
    p_custom.setMesh(mesh);
    g_main_layer.insertProp(p_custom);
    
}
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
