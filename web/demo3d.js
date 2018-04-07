var g_moyai_client;

var g_stop_render=false;
function stopRender() {
    g_stop_render = true;
}

var SCRW=480, SCRH=320;

g_moyai_client = new MoyaiClient(SCRW,SCRH,window.devicePixelRatio);
var screen = document.getElementById("screen");
screen.appendChild( g_moyai_client.renderer.domElement );


var g_keyboard = new Keyboard();
g_keyboard.setupBrowser(window);
var g_mouse = new Mouse();
g_mouse.setupBrowser(window,screen);


var g_viewport3d = new Viewport();
g_viewport3d.setSize(SCRW,SCRH);
g_viewport3d.setClip3D( 0.01, 100 );

var g_viewport2d = new Viewport();
g_viewport2d.setSize(SCRW,SCRH);
g_viewport2d.setScale2D(SCRW,SCRH);

var g_main_layer = new Layer();
g_moyai_client.insertLayer(g_main_layer);
g_main_layer.setViewport(g_viewport3d);

var g_main_camera = new Camera();
g_main_camera.setLoc(0.001,0,3);
g_main_camera.setLookAt(new Vec3(0,0,0), new Vec3(0,1,0));
g_main_layer.setCamera(g_main_camera);

var g_hud_layer = new Layer();
g_hud_layer.setViewport(g_viewport2d);
g_moyai_client.insertLayer(g_hud_layer);
g_hud_camera = new Camera();
g_hud_camera.setLoc(0,0);
g_hud_layer.setCamera( g_hud_camera );

var g_base_tex = new Texture();
g_base_tex.loadPNGMem( base_png );
g_deck = new TileDeck();
g_deck.setTexture(g_base_tex);

    

var g_light = new Light();
g_light.pos = new Vec3(-50,50,100);
g_main_layer.setLight(g_light);

var g_material = createMeshBasicMaterial();
g_material.ambient =Color(0.3,0.3,0.3,1);




// 
//
//   +y
//    ^
//                     d,d,-d
//     H ------------- G
//    /|              /|
//   / |             / |
//  E ------------- F  |
//  |  |            |  |      -z               7   6
//  |  |            |  |      /               4   5
//  |  D -----------|- C
//  | /             | /
//  |/              |/                         3   2
//  A ------------- B     >   +x              0   1
//  -d,-d,d


var geom = new THREE.Geometry();
var d = 0.2;

geom.vertices.push(new THREE.Vector3(-d,-d,d));// A red
geom.vertices.push(new THREE.Vector3(d,-d,d) ); // B blue
geom.vertices.push(new THREE.Vector3(d,-d,-d) ); // C yellow
geom.vertices.push(new THREE.Vector3(-d,-d,-d) ); // D green
geom.vertices.push(new THREE.Vector3(-d,d,d) ); // E white
geom.vertices.push(new THREE.Vector3(d,d,d) ); // F purple
geom.vertices.push(new THREE.Vector3(d,d,-d) ); // G white
geom.vertices.push(new THREE.Vector3(-d,d,-d) ); // H white    
geom.verticesNeedUpdate=true;
// bottom
geom.faces.push(new THREE.Face3(0,3,1)); // ADB
geom.faces.push(new THREE.Face3(3,2,1)); // DCB
// top
geom.faces.push(new THREE.Face3(7,5,6)); // HFG
geom.faces.push(new THREE.Face3(4,5,7)); // EFH
// left
geom.faces.push(new THREE.Face3(4,3,0)); // EDA
geom.faces.push(new THREE.Face3(4,7,3)); // EHD
// right
geom.faces.push(new THREE.Face3(5,1,2)); // FBC
geom.faces.push(new THREE.Face3(5,2,6)); // FCG
// front
geom.faces.push(new THREE.Face3(4,0,1)); // EAB
geom.faces.push(new THREE.Face3(4,1,5)); // EBF
// rear
geom.faces.push(new THREE.Face3(7,2,3)); // HCD
geom.faces.push(new THREE.Face3(7,6,2)); // HGC

for(var i=0;i<12;i++){
    for(var j=0;j<3;j++){
        var c=new Color(1,1,1,1);
        if(i>6 ) c=new Color(1,0,0,1);
        geom.faces[i].vertexColors[j] = c.toTHREEColor();
    }
}
var mat = createMeshBasicMaterial( { transparent:true,
                                     vertexColors: THREE.VertexColors,
                                     blending: THREE.NormalBlending } );

var g_colmesh = new THREE.Mesh(geom,mat);


var g_prop_col = new Prop3D();
g_prop_col.setMesh(g_colmesh);
g_prop_col.setScl(new Vec3(1,1,1));
g_prop_col.setLoc(new Vec3(0,0,0));
g_main_layer.insertProp(g_prop_col);

var last_anim_at = new Date().getTime();

function animate() {
    if(!g_stop_render) requestAnimationFrame(animate);
    if(!g_moyai_client)return;
    
    // props
    if( g_prop_col ){
        g_prop_col.loc.x += dt/10;
        g_prop_col.rot.z += dt*100;
        g_prop_col.rot.y += dt*100;        
    }
    //    print("propx:%f r:%f", g_prop_0->loc.x, g_prop_0->rot3d.z );
    var now_time = new Date().getTime();
    var dt = now_time - last_anim_at;
    last_anim_at = now_time;    
    g_moyai_client.poll(dt/1000.0);
    g_moyai_client.render();


    g_main_camera.setLoc( g_main_camera.loc.x+0.1,0,3);
    
}

animate();

