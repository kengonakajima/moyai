var g_moyai_client;

var g_stop_render=false;
function stopRender() {
    g_stop_render = true;
}

var SCRW=800, SCRH=600;

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


var g_main_layer = new Layer();
g_moyai_client.insertLayer(g_main_layer);
g_main_layer.setViewport(g_viewport3d);

var g_main_camera = new Camera();
g_main_camera.setLoc(-8,5,8);
g_main_camera.setLookAt(new Vec3(0,0,0), new Vec3(0,1,0));
g_main_layer.setCamera(g_main_camera);


var g_base_tex = new Texture();
g_base_tex.loadPNGMem( base_png );
var g_base_deck = new TileDeck();
g_base_deck.setTexture(g_base_tex);
g_base_deck.setSize(32,32,8,8);
    

var g_light = new Light();
g_light.pos = new Vec3(-50,50,100);
g_main_layer.setLight(g_light);




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

function pushBox8Verts(geom,xsz,ysz,zsz,xofs,yofs,zofs) {
    if(xofs==undefined)xofs=0;
    if(yofs==undefined)yofs=0;
    if(zofs==undefined)zofs=0;    
    geom.vertices.push(new THREE.Vector3(-xsz+xofs,-ysz+yofs,zsz+zofs) );// A
    geom.vertices.push(new THREE.Vector3(xsz+xofs,-ysz+yofs,zsz+zofs) ); // B
    geom.vertices.push(new THREE.Vector3(xsz+xofs,-ysz+yofs,-zsz+zofs) ); // C 
    geom.vertices.push(new THREE.Vector3(-xsz+xofs,-ysz+yofs,-zsz+zofs) ); // D
    geom.vertices.push(new THREE.Vector3(-xsz+xofs,ysz+yofs,zsz+zofs) ); // E 
    geom.vertices.push(new THREE.Vector3(xsz+xofs,ysz+yofs,zsz+zofs) ); // F 
    geom.vertices.push(new THREE.Vector3(xsz+xofs,ysz+yofs,-zsz+zofs) ); // G 
    geom.vertices.push(new THREE.Vector3(-xsz+xofs,ysz+yofs,-zsz+zofs) ); // H
    geom.verticesNeedUpdate=true;    
}

function pushCubeVertFace(geom,d) {
    return pushBoxVertFace(geom,d,d,d,new Color(1,1,1,1));
}
//xsz:辺の半分の長さ
function pushBoxVertFace(geom,xsz,ysz,zsz,moyaicol) {
    pushBox8Verts(geom,xsz,ysz,zsz);
    pushBox12Faces(geom);
    for(var i=0;i<12;i++){
        for(var j=0;j<3;j++){
            geom.faces[i].vertexColors[j] = moyaicol.toTHREEColor();
        }
    }
}
function pushBox12Faces(geom,ofs) {
    if(ofs==undefined)ofs=0;
    var o=ofs;
    // bottom
    geom.faces.push(new THREE.Face3(0+o,3+o,1+o)); // ADB
    geom.faces.push(new THREE.Face3(3+o,2+o,1+o)); // DCB
    // top
    geom.faces.push(new THREE.Face3(7+o,5+o,6+o)); // HFG
    geom.faces.push(new THREE.Face3(4+o,5+o,7+o)); // EFH
    // left
    geom.faces.push(new THREE.Face3(4+o,3+o,0+o)); // EDA
    geom.faces.push(new THREE.Face3(4+o,7+o,3+o)); // EHD
    // right
    geom.faces.push(new THREE.Face3(5+o,1+o,2+o)); // FBC
    geom.faces.push(new THREE.Face3(5+o,2+o,6+o)); // FCG
    // front
    geom.faces.push(new THREE.Face3(4+o,0+o,1+o)); // EAB
    geom.faces.push(new THREE.Face3(4+o,1+o,5+o)); // EBF
    // rear
    geom.faces.push(new THREE.Face3(7+o,2+o,3+o)); // HCD
    geom.faces.push(new THREE.Face3(7+o,6+o,2+o)); // HGC
}


function makeCubeUVSet(ind,uofs,vofs) {
    var uvrect=g_base_deck.getUVFromIndex(ind,uofs,vofs,0);
    if(!uvrect) throw "bad index";
    var uv_lt=new THREE.Vector2(uvrect[0],uvrect[1]);
    var uv_rt=new THREE.Vector2(uvrect[2],uvrect[1]);
    var uv_lb=new THREE.Vector2(uvrect[0],uvrect[3]);
    var uv_rb=new THREE.Vector2(uvrect[2],uvrect[3]);
    var out={};
    out.adb=[uv_lb,uv_lt,uv_rb];
    out.dcb=[uv_lt,uv_rt,uv_rb];
    out.hfg=[uv_lt,uv_rb,uv_rt];
    out.efh=[uv_lb,uv_rb,uv_lt];
    out.eda=[uv_rt,uv_lb,uv_rb];
    out.ehd=[uv_rt,uv_lt,uv_lb];
    out.fbc=[uv_lt,uv_lb,uv_rb];
    out.fcg=[uv_lt,uv_rb,uv_rt];
    out.eab=[uv_lt,uv_lb,uv_rb];
    out.ebf=[uv_lt,uv_rb,uv_rt];
    out.hcd=[uv_lt,uv_rb,uv_lb];
    out.hgc=[uv_lt,uv_rt,uv_rb];
    // 水の裏面用に2つ
    out.gfh=[uv_rt,uv_rb,uv_lt];
    out.hfe=[uv_lt,uv_rb,uv_lb];    
    return out;
}


// pushBoxVertFaceで作った箱にUVをつける
function pushBoxTileDeckUV(geom,ind) {
    var uvset=makeCubeUVSet(ind,0,0);
    var ary=[
        uvset.adb, uvset.dcb,
        uvset.hfg, uvset.efh,
        uvset.eda, uvset.ehd,
        uvset.fbc, uvset.fcg,
        uvset.eab, uvset.ebf,
        uvset.hcd, uvset.hgc
    ];
    for(var i in ary) geom.faceVertexUvs[0].push(ary[i]);
}


var mat = createMeshBasicMaterial( { map: g_base_deck.moyai_tex.three_tex,
                                     depthTest:true,
                                     transparent:true,
                                     vertexColors: THREE.VertexColors,
                                     blending: THREE.NormalBlending } );


////////////

function makePartCube(ind,szx,szy,szz, ofsx,ofsy,ofsz) {
    var geom = new THREE.Geometry();
    pushBox8Verts(geom,szx,szy,szz, ofsx,ofsy,ofsz);
    pushBox12Faces(geom);
    pushBoxTileDeckUV(geom,ind);
    return geom;
}


var g_head_mesh = new THREE.Mesh(makePartCube(0, 0.5,0.5,0.5, 0,0.5,0),mat);
g_head_mesh.position.set(0,0,0);
var g_body_mesh = new THREE.Mesh(makePartCube(1, 0.5,0.6,0.3, 0,0,0),mat);
g_body_mesh.position.set(0,-0.6,0);
var g_lfoot_mesh = new THREE.Mesh(makePartCube(2, 0.2,0.5,0.2, 0,-0.5,0),mat);
g_lfoot_mesh.position.set(0.25,-1.2,0);
var g_rfoot_mesh = new THREE.Mesh(makePartCube(3,0.2,0.5,0.2, 0,-0.5,0),mat);
g_rfoot_mesh.position.set(-0.25,-1.2,0);
var g_lhand_mesh = new THREE.Mesh(makePartCube(1,0.2,0.5,0.2, 0,-0.5,0),mat);
g_lhand_mesh.position.set(-0.7,0,0);
var g_rhand_mesh = new THREE.Mesh(makePartCube(2,0.2,0.5,0.2, 0,-0.5,0),mat);
g_rhand_mesh.position.set(+0.7,0,0);

var grp = new THREE.Group();
grp.add(g_head_mesh,g_body_mesh, g_lfoot_mesh, g_rfoot_mesh, g_lhand_mesh, g_rhand_mesh);

var g_char = new Prop3D();
g_char.setGroup(grp);
g_char.prop3DPoll = function(dt) {
    this.setLoc( new Vec3( Math.sin(this.accum_time),0,0));
    this.setRot( new Vec3( 0,this.accum_time,0) );

    g_head_mesh.rotation.set(Math.sin(this.accum_time)/2, this.accum_time,0);    
    g_lfoot_mesh.rotation.set(Math.cos(this.accum_time*8),0,0);
    g_rfoot_mesh.rotation.set(Math.cos(this.accum_time*8+Math.PI),0,0);

    g_lhand_mesh.rotation.set(Math.cos(this.accum_time*8),0,0);
    g_rhand_mesh.rotation.set(Math.cos(this.accum_time*8+Math.PI),0,Math.PI/3);
    return true;
}


g_main_layer.insertProp(g_char);


/////////////

var last_anim_at = new Date().getTime();
var last_print_at = new Date().getTime();
var fps=0;
function animate() {
    if(!g_stop_render) requestAnimationFrame(animate);
    if(!g_moyai_client)return;

    fps++;

    var now_time = new Date().getTime();
    var dt = (now_time - last_anim_at) / 1000.0;

    if(now_time > last_print_at+1000) {
        last_print_at=now_time;
        document.getElementById("status").innerHTML = "FPS:"+fps+ "props:" + g_main_layer.props.length;
        fps=0;
    }
    // props
    
    last_anim_at = now_time;    
    g_moyai_client.poll(dt);
    g_moyai_client.render();
}

animate();

