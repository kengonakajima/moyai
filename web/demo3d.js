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
g_main_camera.setLoc(0.001,4,8);
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
var g_base_deck = new TileDeck();
g_base_deck.setTexture(g_base_tex);
g_base_deck.setSize(32,32,8,8);
    

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
function pushCubeVertFace(geom,d) {
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
            geom.faces[i].vertexColors[j] = c.toTHREEColor();
        }
    }
}
pushCubeVertFace(geom,0.2);

// 色
for(var i=0;i<12;i++){
    for(var j=0;j<3;j++){
        if(i>6 ) {
            c=new Color(1,0,0,1);
            geom.faces[i].vertexColors[j] = c.toTHREEColor();
        }
    }
}
var mat = createMeshBasicMaterial( { transparent:true,
                                     vertexColors: THREE.VertexColors,
                                     blending: THREE.NormalBlending } );

var g_colmesh = new THREE.Mesh(geom,mat);

var geom2 = new THREE.Geometry();
pushCubeVertFace(geom2,0.2);
var kk=1.0/256.0*8;
var uv_lt=new THREE.Vector2(0,0);
var uv_rt=new THREE.Vector2(kk,0);
var uv_lb=new THREE.Vector2(0,kk);
var uv_rb=new THREE.Vector2(kk,kk);


geom2.faceVertexUvs[0].push([uv_lb,uv_lt,uv_rb]);// ADB
geom2.faceVertexUvs[0].push([uv_lt,uv_rt,uv_rb]);// DCB
geom2.faceVertexUvs[0].push([uv_lt,uv_rb,uv_rt]);// HFG
geom2.faceVertexUvs[0].push([uv_lb,uv_rb,uv_lt]);// EFH
geom2.faces[2].vertexColors[0]= new Color(1,0,0,1).toTHREEColor();
//geom2.faces[2].vertexColors[1]= new Color(1,0,0,1).toTHREEColor();
//geom2.faces[2].vertexColors[2]= new Color(1,0,0,1).toTHREEColor();

console.log("geom2:",geom2);

geom2.uvsNeedUpdate = true;


var mat2 = createMeshBasicMaterial( {
    map: g_base_deck.moyai_tex.three_tex,
    depthTest: true,
    transparent:true,
    vertexColors: THREE.VertexColors,
    blending: THREE.NormalBlending
});

var g_texcolmesh = new THREE.Mesh(geom2,mat2);

var g_prop_col = new Prop3D();
g_prop_col.setMesh(g_colmesh);
g_prop_col.setScl(1,1,1);
g_prop_col.setLoc(0,0,0);
g_main_layer.insertProp(g_prop_col);

var g_prop_texcol = new Prop3D();
g_prop_texcol.setMesh(g_texcolmesh);
g_prop_texcol.setScl(1,1,1);
g_prop_texcol.setLoc(1.4,2,2);
g_main_layer.insertProp(g_prop_texcol);

var AIR=0;
var STONE=1;
function createFieldBlockData(sz) {
    // ex. 16*16*16=4096.
    // x>z>y 
    // 0: (0,0,0) 1:(1,0,0)... 16:(0,0,1), ... 256:(0,1,0) 4095:(15,15,15)
    var out=new Array(sz*sz*sz);
    for(var y=0;y<sz;y++) {
        for(var z=0;z<sz;z++) {
            for(var x=0;x<sz;x++) {
                var ind=x+z*sz+y*sz*sz;
                var val=AIR;
                if(x>y&&z>y)val=STONE;
                out[ind]=val;  
            }
        }
    }
    return out;
}
function createChunkGeometry(blks,sz) {
    var l=1.0;
    var geom = new THREE.Geometry();
    var vn=0, fn=0;
    for(var y=0;y<sz;y++) {
        for(var z=0;z<sz;z++) {
            for(var x=0;x<sz;x++) {
                var block_ind=x+z*sz+y*sz*sz;
                var blk = blks[block_ind];
                if(blk==AIR)continue;
                console.log("blk at",x,y,z,blk);
                
                geom.vertices.push(new THREE.Vector3(x,y,z+l));// A red
                geom.vertices.push(new THREE.Vector3(x+l,y,z+l) ); // B blue
                geom.vertices.push(new THREE.Vector3(x+l,y,z) ); // C yellow
                geom.vertices.push(new THREE.Vector3(x,y,z) ); // D green
                geom.vertices.push(new THREE.Vector3(x,y+l,z+l) ); // E white
                geom.vertices.push(new THREE.Vector3(x+l,y+l,z+l) ); // F purple
                geom.vertices.push(new THREE.Vector3(x+l,y+l,z) ); // G white
                geom.vertices.push(new THREE.Vector3(x,y+l,z) ); // H white


                // faces
                // bottom
                geom.faces.push(new THREE.Face3(vn+0,vn+3,vn+1)); // ADB
                geom.faces.push(new THREE.Face3(vn+3,vn+2,vn+1)); // DCB
                // top
                geom.faces.push(new THREE.Face3(vn+7,vn+5,vn+6)); // HFG
                geom.faces.push(new THREE.Face3(vn+4,vn+5,vn+7)); // EFH
                // left
                geom.faces.push(new THREE.Face3(vn+4,vn+3,vn+0)); // EDA
                geom.faces.push(new THREE.Face3(vn+4,vn+7,vn+3)); // EHD
                // right
                geom.faces.push(new THREE.Face3(vn+5,vn+1,vn+2)); // FBC
                geom.faces.push(new THREE.Face3(vn+5,vn+2,vn+6)); // FCG
                // front
                geom.faces.push(new THREE.Face3(vn+4,vn+0,vn+1)); // EAB
                geom.faces.push(new THREE.Face3(vn+4,vn+1,vn+5)); // EBF
                // rear
                geom.faces.push(new THREE.Face3(vn+7,vn+2,vn+3)); // HCD
                geom.faces.push(new THREE.Face3(vn+7,vn+6,vn+2)); // HGC

                // colors
                for(var i=fn;i<fn+12;i++){
                    for(var j=0;j<3;j++){
                        var c=new Color(1,1,1,1);
                        geom.faces[i].vertexColors[j] = c.toTHREEColor();
                    }
                }

                // uvs
                var uvrect=g_base_deck.getUVFromIndex(3,0,0,0);
                console.log("uvr:",uvrect);
                var uv_lt=new THREE.Vector2(uvrect[0],uvrect[1]);
                var uv_rt=new THREE.Vector2(uvrect[2],uvrect[1]);
                var uv_lb=new THREE.Vector2(uvrect[0],uvrect[3]);
                var uv_rb=new THREE.Vector2(uvrect[2],uvrect[3]);

                geom.faceVertexUvs[0].push([uv_lb,uv_lt,uv_rb]);// ADB
                geom.faceVertexUvs[0].push([uv_lt,uv_rt,uv_rb]);// DCB
                
                geom.faceVertexUvs[0].push([uv_lt,uv_rb,uv_rt]);// HFG
                geom.faceVertexUvs[0].push([uv_lb,uv_rb,uv_lt]);// EFH
                
                geom.faceVertexUvs[0].push([uv_rt,uv_lb,uv_rb]);//EDA
                geom.faceVertexUvs[0].push([uv_rt,uv_lt,uv_lb]);//EHD

                geom.faceVertexUvs[0].push([uv_lt,uv_lb,uv_rb]);//FBC
                geom.faceVertexUvs[0].push([uv_lt,uv_rb,uv_rt]);//FCG
                geom.faceVertexUvs[0].push([uv_lt,uv_lb,uv_rb]);//EAB
                geom.faceVertexUvs[0].push([uv_lt,uv_rb,uv_rt]);//EBF
                geom.faceVertexUvs[0].push([uv_lt,uv_rb,uv_lb]);// HCD
                geom.faceVertexUvs[0].push([uv_lt,uv_rt,uv_rb]);//HGC

                vn+=8;
                fn+=12;
                                
            }
        }
    }
    geom.verticesNeedUpdate=true;
    geom.uvsNeedUpdate = true;
    console.log("chgeom:",geom);
    return geom;
}
var blockdata = createFieldBlockData(8);
var chgeom = createChunkGeometry(blockdata,8);
var chmesh = new THREE.Mesh(chgeom,mat2);

var g_prop_voxel = new Prop3D();
g_prop_voxel.setMesh(chmesh);
g_prop_voxel.setScl(1,1,1);
g_prop_voxel.setLoc(-2,-2,2);
g_main_layer.insertProp(g_prop_voxel);

var last_anim_at = new Date().getTime();

function animate() {
    if(!g_stop_render) requestAnimationFrame(animate);
    if(!g_moyai_client)return;
    
    //    print("propx:%f r:%f", g_prop_0->loc.x, g_prop_0->rot3d.z );
    var now_time = new Date().getTime();
    var dt = (now_time - last_anim_at) / 1000.0;

    // props
    if( g_prop_col ){
        g_prop_col.loc.x += dt/10;
        g_prop_col.rot.z += dt;
        g_prop_col.rot.y += dt;
    }
    if( g_prop_texcol ){
        g_prop_texcol.rot.z += dt;
        g_prop_texcol.rot.y += dt;
    }
    if( g_prop_voxel ){
        g_prop_voxel.loc.z -= dt/5;        
//        g_prop_voxel.rot.z += dt;
//        g_prop_voxel.rot.y += dt;
    }
    
    last_anim_at = now_time;    
    g_moyai_client.poll(dt);
    g_moyai_client.render();


    //    g_main_camera.setLoc( g_main_camera.loc.x+0.1,0,3);
    
    
}

animate();

