
var g_stop_render=false;
function stopRender() {
    g_stop_render = true;
}

var SCRW=1024, SCRH=512;

Moyai.init(SCRW,SCRH);
var screen = document.getElementById("screen");
screen.appendChild( Moyai.getDomElement());


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
Moyai.insertLayer(g_main_layer);
g_main_layer.setViewport(g_viewport3d);

var g_main_camera = new PerspectiveCamera( 45*Math.PI/180 , SCRW / SCRH , 0.1, 100);
g_main_camera.setLoc(3,5,0);
g_main_camera.setLookAt(vec3.fromValues(0,0,0), vec3.fromValues(0,1,0));
g_main_layer.setCamera(g_main_camera);

var g_base_tex = new Texture();
g_base_tex.loadPNG( "./assets/base.png" );
var g_base_deck = new TileDeck();
g_base_deck.setTexture(g_base_tex);
g_base_deck.setSize(32,32,8,8);
    






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


var sz=0.5;
var a=vec3.fromValues(-sz,-sz,sz);
var b=vec3.fromValues(sz,-sz,sz);
var c=vec3.fromValues(sz,-sz,-sz);
var d=vec3.fromValues(-sz,-sz,-sz);
var e=vec3.fromValues(-sz,sz,sz);
var f=vec3.fromValues(sz,sz,sz);
var g=vec3.fromValues(sz,sz,-sz);
var h=vec3.fromValues(-sz,sz,-sz);

if(1) {
    var geom = new FaceGeometry(8,6*2);
    
    geom.setPosition3v(0,a);// A red
    geom.setPosition3v(1,b); // B blue
    geom.setPosition3v(2,c); // C yellow
    geom.setPosition3v(3,d); // D green
    geom.setPosition3v(4,e); // E white
    geom.setPosition3v(5,f); // F purple
    geom.setPosition3v(6,g); // G white
    geom.setPosition3v(7,h); // H white    

    geom.setColor(0, 1,0,0,1);
    geom.setColor(1, 0,0,1,1);
    geom.setColor(2, 1,1,0,1);
    geom.setColor(3, 0,1,0,1);
    geom.setColor(4, 1,1,1,1);
    geom.setColor(5, 1,0,1,1);
    geom.setColor(6, 1,1,1,1);
    geom.setColor(7, 1,1,1,1);                        
    
    // bottom
    geom.setFaceInds(0, 0,3,1); // ADB
    geom.setFaceInds(1, 3,2,1); // DCB
    // top
    geom.setFaceInds(2, 7,5,6); // HFG
    geom.setFaceInds(3, 4,5,7); // EFH
    // left
    geom.setFaceInds(4, 4,3,0); // EDA
    geom.setFaceInds(5, 4,7,3); // EHD
    // right
    geom.setFaceInds(6, 5,1,2); // FBC
    geom.setFaceInds(7, 5,2,6); // FCG
    // front
    geom.setFaceInds(8, 4,0,1); // EAB
    geom.setFaceInds(9, 4,1,5); // EBF
    // rear
    geom.setFaceInds(10, 7,2,3); // HCD
    geom.setFaceInds(11, 7,6,2); // HGC

    var g_prop_col = new Prop3D();
    g_prop_col.setGeom(geom);
    g_prop_col.setMaterial(new PrimColorShaderMaterial());
    g_prop_col.setScl(1,1,1);
    g_prop_col.setLoc(0,0,0);
    g_prop_col.prop3DPoll=function(dt) {
//        this.loc[1]+=0.02;//Math.cos(this.accum_time)*3;
//        this.loc[1]=Math.sin(this.accum_time)*3;
//        this.rot[0]+=0.1;
//        this.rot[1]+=0.1;        
        return true;
    }
    g_main_layer.insertProp(g_prop_col);
}

function setNinja(geom) {
    var kk=1.0/256.0*8;
    var uv_lt=vec2.fromValues(0,0);
    var uv_rt=vec2.fromValues(kk,0);
    var uv_lb=vec2.fromValues(0,kk);
    var uv_rb=vec2.fromValues(kk,kk);

    geom.setPosition3v(0,a); geom.setPosition3v(1,b); geom.setPosition3v(2,c); geom.setPosition3v(3,d);//-y
    geom.setPosition3v(4,e); geom.setPosition3v(5,f); geom.setPosition3v(6,g); geom.setPosition3v(7,h);//+y
    geom.setPosition3v(8,a); geom.setPosition3v(9,b); geom.setPosition3v(10,f); geom.setPosition3v(11,e);//+z
    geom.setPosition3v(12,c); geom.setPosition3v(13,d); geom.setPosition3v(14,h); geom.setPosition3v(15,g);//-z
    geom.setPosition3v(16,b); geom.setPosition3v(17,c); geom.setPosition3v(18,g); geom.setPosition3v(19,f);//+x
    geom.setPosition3v(20,d); geom.setPosition3v(21,a); geom.setPosition3v(22,e); geom.setPosition3v(23,h);//-x

    geom.setUV2v(0,uv_lb); geom.setUV2v(1,uv_rb); geom.setUV2v(2,uv_rt); geom.setUV2v(3,uv_lt); // abcd
    geom.setUV2v(4,uv_lb); geom.setUV2v(5,uv_rb); geom.setUV2v(6,uv_rt); geom.setUV2v(7,uv_lt); // efgh
    geom.setUV2v(8,uv_lb); geom.setUV2v(9,uv_rb); geom.setUV2v(10,uv_rt); geom.setUV2v(11,uv_lt); // abfe
    geom.setUV2v(12,uv_lb); geom.setUV2v(13,uv_rb); geom.setUV2v(14,uv_rt); geom.setUV2v(15,uv_lt); // cdhg
    geom.setUV2v(16,uv_lb); geom.setUV2v(17,uv_rb); geom.setUV2v(18,uv_rt); geom.setUV2v(19,uv_lt); // bcgf
    geom.setUV2v(20,uv_lb); geom.setUV2v(21,uv_rb); geom.setUV2v(22,uv_rt); geom.setUV2v(23,uv_lt); // daeh
    
    for(var i=0;i<24;i++) geom.setColor(i, 1,1,1,1);
    
    // bottom
    geom.setFaceInds(0, 0,3,1); // ADB
    geom.setFaceInds(1, 3,2,1); // DCB
    // top
    geom.setFaceInds(2, 7,5,6); // HFG
    geom.setFaceInds(3, 4,5,7); // EFH
    // +z abf, afe
    geom.setFaceInds(4, 8,9,10); // abf
    geom.setFaceInds(5, 8,10,11); // afe
    // -z cdh, chg
    geom.setFaceInds(6, 12,13,14); // cdh
    geom.setFaceInds(7, 12,14,15); // chg
    // front
    geom.setFaceInds(8, 16,17,18); // EAB
    geom.setFaceInds(9, 16,18,19); // EBF
    // rear
    geom.setFaceInds(10, 20,21,22); // HCD
    geom.setFaceInds(11, 20,22,23); // HGC    
}
        
if(1) {
    var geom = new FaceGeometry(6*4,6*2);
    setNinja(geom);
    geom.setColor(8, 1,0,0,1);
    geom.setColor(16, 1,1,1,0.1);    
    
    var g_prop_texcol = new Prop3D();
    g_prop_texcol.setGeom(geom);
    g_prop_texcol.setMaterial(new DefaultColorShaderMaterial());
    g_prop_texcol.setTexture(g_base_tex);
    g_prop_texcol.setScl(1,1,1);
    g_prop_texcol.setLoc(2,0,0);
    g_prop_texcol.setColor(vec4.fromValues(1,1,1,1));
    g_main_layer.insertProp(g_prop_texcol);
}

if(0) {
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
                    if(y<8)val=STONE; // 2048vox per chunk
                    // if(y==0)val=STONE; // 256vox per chunk
                    //if(x==z && z==y)val=STONE; // 16vox per chunk
                    out[ind]=val;  
                }
            }
        }
        return out;
    }

    // precalc
    var white=new Color(1,1,1,1).toTHREEColor();
    var dark=new Color(0.8,0.8,0.8,1).toTHREEColor();

    var uvrect=new Float32Array(4);
    g_base_deck.getUVFromIndex(uvrect,3,0,0,0);
    var uv_lt=new THREE.Vector2(uvrect[0],uvrect[1]);
    var uv_rt=new THREE.Vector2(uvrect[2],uvrect[1]);
    var uv_lb=new THREE.Vector2(uvrect[0],uvrect[3]);
    var uv_rb=new THREE.Vector2(uvrect[2],uvrect[3]);
    var uv_adb=[uv_lb,uv_lt,uv_rb];// ADB
    var uv_dcb=[uv_lt,uv_rt,uv_rb];// DCB
    var uv_hfg=[uv_lt,uv_rb,uv_rt];// HFG
    var uv_efh=[uv_lb,uv_rb,uv_lt];// EFH
    var uv_eda=[uv_rt,uv_lb,uv_rb];//EDA
    var uv_ehd=[uv_rt,uv_lt,uv_lb];//EHD
    var uv_fbc=[uv_lt,uv_lb,uv_rb];//FBC
    var uv_fcg=[uv_lt,uv_rb,uv_rt];//FCG
    var uv_eab=[uv_lt,uv_lb,uv_rb];//EAB
    var uv_ebf=[uv_lt,uv_rb,uv_rt];//EBF
    var uv_hcd=[uv_lt,uv_rb,uv_lb];// HCD
    var uv_hgc=[uv_lt,uv_rt,uv_rb];//HGC


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
                    var cols=[ dark,dark,dark,  dark,dark,dark, white,white,white, white,white,white, // ADB DCB  HFG EFH
                               white,dark,dark, white,white,dark, white,dark,dark, white,dark,white, // EDA EHD  FBC  FCG
                               white,dark,dark, white,dark,white, white,dark,dark, white,white,dark // EAB EBF  HCD HGC
                             ];
                    for(var i=fn;i<fn+12;i++){
                        for(var j=0;j<3;j++){
                            geom.faces[i].vertexColors[j] = cols[j+(i-fn)*3];
                            
                        }
                    }

                    // uvs
                    geom.faceVertexUvs[0].push(uv_adb);// ADB
                    geom.faceVertexUvs[0].push(uv_dcb);// DCB
                    geom.faceVertexUvs[0].push(uv_hfg);// HFG
                    geom.faceVertexUvs[0].push(uv_efh);// EFH
                    geom.faceVertexUvs[0].push(uv_eda);//EDA
                    geom.faceVertexUvs[0].push(uv_ehd);//EHD
                    geom.faceVertexUvs[0].push(uv_fbc);//FBC
                    geom.faceVertexUvs[0].push(uv_fcg);//FCG
                    geom.faceVertexUvs[0].push(uv_eab);//EAB
                    geom.faceVertexUvs[0].push(uv_ebf);//EBF
                    geom.faceVertexUvs[0].push(uv_hcd);// HCD
                    geom.faceVertexUvs[0].push(uv_hgc);//HGC

                    vn+=8;
                    fn+=12;
                    
                }
            }
        }
        geom.verticesNeedUpdate=true;
        geom.uvsNeedUpdate = true;
        return geom;
    }
    var g_blockdata = createFieldBlockData(16);

    var g_chk_sz = 5;
    var g_chk_x = 0, g_chk_y = 0, g_chk_z = 0;

    setInterval(function() {
        if(g_chk_y==g_chk_sz)return;
        
        var chgeom = createChunkGeometry(g_blockdata,16);
        var chmesh = new THREE.Mesh(chgeom,g_mat2);
        var chkp = new Prop3D();
        chkp.setMesh(chmesh);
        chkp.setScl(1,1,1);
        chkp.setLoc(g_chk_x*16,g_chk_y*16,g_chk_z*16);
        g_chk_x++;
        if(g_chk_x==g_chk_sz) {
            g_chk_x=0;
            g_chk_z++;
            if(g_chk_z==g_chk_sz) {
                g_chk_y++;
                g_chk_z=0;
            }
        }
        g_main_layer.insertProp(chkp);    
    }, 20 );


    // 1ボクセルあたり12triangle
    // 16voxel x 3000chk = 45fps (2.1GB)  (576000tri/frame)
    // 256voxel x 730chk = 60fps (2GB) (2242560tri/frame)
    // 4096voxel x 64chk = 60fps (1.8GB) (3145728tri/frame)
    // 4096voxel * 128chk = 45fps (2.1GB) (6.2Mtri/frame)
    // 2GB超えるとだめ。 300万tri (1triあたり700byte食うのでメモリがボトルネックになった。)
}

var last_anim_at = new Date().getTime();
var last_print_at = new Date().getTime();
var fps=0;
function animate() {
    if(!g_stop_render) requestAnimationFrame(animate);

    fps++;

    var now_time = new Date().getTime();
    var dt = (now_time - last_anim_at) / 1000.0;

    if(now_time > last_print_at+1000) {
        last_print_at=now_time;
        document.getElementById("status").innerHTML = "FPS:"+fps+ "props:" + g_main_layer.props.length;
        fps=0;
    }

    // props

//    if( g_prop_texcol ){
//        g_prop_texcol.rot[2] += dt;
//        g_prop_texcol.rot[1] += dt;
//    }
    if(g_main_camera) {
        var t=now();
        g_main_camera.loc[0]=Math.cos(t)*8;
        g_main_camera.loc[2]=Math.sin(t)*8;
    }
//    if( g_prop_voxel ){
//        g_prop_voxel.loc[2] -= dt/5;        
//        g_prop_voxel.rot[2] += dt;
//        g_prop_voxel.rot[1] += dt;
//    }
    
    last_anim_at = now_time;    
    Moyai.poll(dt);
    Moyai.render();

    //    g_main_camera.setLoc( g_main_camera.loc[0]+0.1,0,3);
    
    
}

animate();

