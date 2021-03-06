
var g_stop_render=false;
function stopRender() {
    g_stop_render = true;
}

var g_fullscreen=false;

function makeFullscreen() {
    var doc = window.document;
    var docEl = doc.documentElement;
    var requestFullScreen = docEl.requestFullscreen || docEl.requestFullScreen || docEl.mozRequestFullScreen || docEl.webkitRequestFullScreen || docEl.msRequestFullscreen;
    g_fullscreen=true;
    var scr=document.getElementById("screen");
    window.onresize=onResizeWindow;
    requestFullScreen.call(scr);
}

function onResizeWindow() {
    var w=window.innerWidth, h=window.innerHeight;
    updateWindowSize(w,h);
}

function updateWindowSize(w,h){
    window.onresize=null;
    g_viewport3d.screen_width=w;
    g_viewport3d.screen_height=h;
    g_viewport2d.screen_width=w;
    g_viewport2d.screen_height=h;
    Moyai.width=w;
    Moyai.height=h;

//    Moyai.canvas.width=w;
//    g_moyai_client.renderer.domElement.height=h;
//    g_moyai_client.renderer.domElement.style="width:"+w+"px;height:"+w+"px;";
//    g_moyai_client.renderer.setSize(w,h);
}






////////////
var SCRW=window.innerWidth, SCRH=window.innerHeight;
var pixelRatio = window.devicePixelRatio || 1;
SCRW*=pixelRatio;
SCRH*=pixelRatio;
console.log("Screen size:",SCRW,SCRH);
Moyai.init(SCRW,SCRH);
Moyai.clearColor=Color.fromValues(0.5,0.5,0.5,1);
var screen = document.getElementById("screen");
var canvas=Moyai.getDomElement();
canvas.style="width:100%; height:100%";
screen.appendChild(canvas);


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

var g_main_camera = new PerspectiveCamera( 45*Math.PI/180 , SCRW / SCRH , 0.1, 1000);
g_main_camera.setLoc(-6,4,-3);
g_main_camera.setLookAt(vec3.fromValues(0,0,0), vec3.fromValues(0,1,0));
g_main_layer.setCamera(g_main_camera);

var g_base_tex = new Texture();
g_base_tex.loadPNG( "./assets/base.png", 256,256 );
//g_base_tex.min_filter=Moyai.gl.NEAREST_MIPMAP_NEAREST;  almost no perfomance improvement
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

    var p = new Prop3D();
    p.setGeom(geom);
    p.setMaterial(new PrimColorShaderMaterial());
    p.setScl(1,1,1);
    p.setLoc(0,0,0);
    p.prop3DPoll=function(dt) {
//        this.loc[1]+=0.02;//Math.cos(this.accum_time)*3;
//        this.loc[1]=Math.sin(this.accum_time)*3;
//        this.rot[0]+=0.1;
//        this.rot[1]+=0.1;        
        return true;
    }
    g_main_layer.insertProp(p);
}

if(1) {
    var geom = new FaceGeometry(6*6,6*2);
    var positions=[a,d,b, d,c,b, // bottom
                   h,f,g, e,f,h, // top
                   e,d,a, e,h,d, // left -x
                   f,b,c, f,c,g, // right
                   e,a,b, e,b,f, // front  +z
                   h,c,d, h,g,c, // rear
                  ];
    for(var i=0;i<36;i++) {
        geom.setPosition3v(i,positions[i]);
        //        geom.setColor(i,range(0,1),range(0,1),range(0,1),1);
        geom.setColor(i,1,1,1,1);
    }
    for(var i=0;i<12;i++) geom.setFaceInds(i,i*3,i*3+1,i*3+2);
    
    
    for(var i=0;i<6;i++) geom.setNormal(i,0,-1,0);
    for(var i=6;i<12;i++) geom.setNormal(i,0,1,0);
    for(var i=12;i<18;i++) geom.setNormal(i,-1,0,0);
    for(var i=18;i<24;i++) geom.setNormal(i,1,0,0);
    for(var i=24;i<30;i++) geom.setNormal(i,0,0,1);
    for(var i=30;i<36;i++) geom.setNormal(i,0,0,-1);

    var kk=1.0/256.0*8;
    var uv_lt=vec2.fromValues(0,0);
    var uv_rt=vec2.fromValues(kk,0);
    var uv_lb=vec2.fromValues(0,kk);
    var uv_rb=vec2.fromValues(kk,kk);
    
    var uvs = [ uv_lb, uv_lt, uv_rb, // adb
                uv_lt, uv_rt, uv_rb, // dcb
                uv_lt, uv_rb, uv_rt, // hfg
                uv_lb, uv_rb, uv_lt, // efh
                uv_rt, uv_lb, uv_rb, // eda
                uv_rt, uv_lt, uv_lb, // ehd
                uv_lt, uv_lb, uv_rb, // fbc
                uv_lt, uv_rb, uv_rt, // fcg
                uv_lt, uv_lb, uv_rb, // eab
                uv_lt, uv_rb, uv_rt, // ebf
                uv_rt, uv_lb, uv_rb, // hcd
                uv_rt, uv_lt, uv_lb, // hgc
              ];
    for(var i=0;i<uvs.length;i++) geom.setUV2v(i,uvs[i]);
    
    var p = new Prop3D();
    p.setGeom(geom);
    var shm = new DefaultColorLitShaderMaterial();
    shm.setAmbientColor(vec3.fromValues(0.2,0.2,0.2));
    shm.setLightDirection(vec3.fromValues(0,1,0));
    shm.setLightColor(vec3.fromValues(1,1,1));
    p.setMaterial(shm);
    p.setTexture(g_base_tex);    
    p.setScl(1,1,1);
    p.setLoc(1,2,0);
    p.t=0;
    p.prop3DPoll=function(dt) {
        this.t+=dt;
        this.setEulerRot(this.t,this.t/2,this.t/7);
        shm.setLightColor(vec3.fromValues(1,0.5+Math.sin(this.t)*0.5,0.5+Math.sin(this.t)*0.5));
        return true;
    }
    g_main_layer.insertProp(p);
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
    
    var p = new Prop3D();
    p.setGeom(geom);
    p.setMaterial(new DefaultColorShaderMaterial());
    p.setTexture(g_base_tex);
    p.setScl(1,1,1);
    p.setLoc(2,0,0);
    p.setColor(vec4.fromValues(1,1,1,1));
    g_main_layer.insertProp(p);
    p.prop3DPoll=function(dt) {
        this.setColor(1,1,1,0.5+Math.cos(this.accum_time)*0.5);
        if(this.poll_count%100==0) {
            this.setEulerRot(0,0.5,0);
        } else if(this.poll_count%100==50){
            this.setEulerRot(0,0,0);            
        }
        return true;
    }
    if(1) {
        var chp=new Prop3D();
        chp.setGeom(geom);
        chp.setMaterial(new DefaultColorShaderMaterial());
        chp.setTexture(g_base_tex);
        chp.setScl(0.3,0.3,0.3);
        chp.setLoc(1,0,0); // relative to parent
        chp.setColor(vec4.fromValues(1,1,1,1));
        p.addChild(chp);
    }
    
}


if(1) {

    function createNinjaChunk(sz) {
        var kk=1.0/256.0*8;
        var uv_lt=vec2.fromValues(0,0);
        var uv_rt=vec2.fromValues(kk,0);
        var uv_lb=vec2.fromValues(0,kk);
        var uv_rb=vec2.fromValues(kk,kk);

        /* need sort to use alpha-transparent tile
        if(range(0,1)<0.5) {
            uv_lt=vec2.fromValues(kk*4,0);
            uv_rt=vec2.fromValues(kk*5,0);
            uv_lb=vec2.fromValues(kk*4,kk);
            uv_rb=vec2.fromValues(kk*5,kk);                                    
        }        */
        var geom = new FaceGeometry(6*4*sz*sz*sz,6*2*sz*sz*sz);

        var a=vec3.create();
        var b=vec3.create();
        var c=vec3.create();
        var d=vec3.create();
        var e=vec3.create();
        var f=vec3.create();
        var g=vec3.create();
        var h=vec3.create();
        
        var cubei=0;
        for(var y=0;y<sz;y++) {
            for(var z=0;z<sz;z++) {
                for(var x=0;x<sz;x++) {
                    var vi=cubei*6*4;
                    var fi=cubei*6*2;

                    var  m=0.4;
                    vec3.set(a,x-m,-y-m,z+m);
                    vec3.set(b,x+m,-y-m,z+m);
                    vec3.set(c,x+m,-y-m,z-m);
                    vec3.set(d,x-m,-y-m,z-m);
                    vec3.set(e,x-m,-y+m,z+m);
                    vec3.set(f,x+m,-y+m,z+m);
                    vec3.set(g,x+m,-y+m,z-m);
                    vec3.set(h,x-m,-y+m,z-m);

                    
                    geom.setPosition3v(vi+0,a); geom.setPosition3v(vi+1,b); geom.setPosition3v(vi+2,c); geom.setPosition3v(vi+3,d);//-y
                    geom.setPosition3v(vi+4,e); geom.setPosition3v(vi+5,f); geom.setPosition3v(vi+6,g); geom.setPosition3v(vi+7,h);//+y
                    geom.setPosition3v(vi+8,a); geom.setPosition3v(vi+9,b); geom.setPosition3v(vi+10,f); geom.setPosition3v(vi+11,e);//+z
                    geom.setPosition3v(vi+12,c); geom.setPosition3v(vi+13,d); geom.setPosition3v(vi+14,h); geom.setPosition3v(vi+15,g);//-z
                    geom.setPosition3v(vi+16,b); geom.setPosition3v(vi+17,c); geom.setPosition3v(vi+18,g); geom.setPosition3v(vi+19,f);//+x
                    geom.setPosition3v(vi+20,d); geom.setPosition3v(vi+21,a); geom.setPosition3v(vi+22,e); geom.setPosition3v(vi+23,h);//-x
                    
                    geom.setUV2v(vi+0,uv_lb); geom.setUV2v(vi+1,uv_rb); geom.setUV2v(vi+2,uv_rt); geom.setUV2v(vi+3,uv_lt); // abcd
                    geom.setUV2v(vi+4,uv_lb); geom.setUV2v(vi+5,uv_rb); geom.setUV2v(vi+6,uv_rt); geom.setUV2v(vi+7,uv_lt); // efgh
                    geom.setUV2v(vi+8,uv_lb); geom.setUV2v(vi+9,uv_rb); geom.setUV2v(vi+10,uv_rt); geom.setUV2v(vi+11,uv_lt); // abfe
                    geom.setUV2v(vi+12,uv_lb); geom.setUV2v(vi+13,uv_rb); geom.setUV2v(vi+14,uv_rt); geom.setUV2v(vi+15,uv_lt); // cdhg
                    geom.setUV2v(vi+16,uv_lb); geom.setUV2v(vi+17,uv_rb); geom.setUV2v(vi+18,uv_rt); geom.setUV2v(vi+19,uv_lt); // bcgf
                    geom.setUV2v(vi+20,uv_lb); geom.setUV2v(vi+21,uv_rb); geom.setUV2v(vi+22,uv_rt); geom.setUV2v(vi+23,uv_lt); // daeh

                    var red=range(0,1), green=range(0,1), blue=range(0,1);
                    for(var i=0;i<24;i++) geom.setColor(vi+i, red,green,blue,1);

                    // bottom
                    geom.setFaceInds(fi+0, vi+0,vi+3,vi+1); // ADB
                    geom.setFaceInds(fi+1, vi+3,vi+2,vi+1); // DCB
                    // top
                    geom.setFaceInds(fi+2, vi+7,vi+5,vi+6); // HFG
                    geom.setFaceInds(fi+3, vi+4,vi+5,vi+7); // EFH
                    // +z abf, afe
                    geom.setFaceInds(fi+4, vi+8,vi+9,vi+10); // abf
                    geom.setFaceInds(fi+5, vi+8,vi+10,vi+11); // afe
                    // -z cdh, chg
                    geom.setFaceInds(fi+6, vi+12,vi+13,vi+14); // cdh
                    geom.setFaceInds(fi+7, vi+12,vi+14,vi+15); // chg
                    // front
                    geom.setFaceInds(fi+8, vi+16,vi+17,vi+18); // EAB
                    geom.setFaceInds(fi+9, vi+16,vi+18,vi+19); // EBF
                    // rear
                    geom.setFaceInds(fi+10, vi+20,vi+21,vi+22); // HCD
                    geom.setFaceInds(fi+11, vi+20,vi+22,vi+23); // HGC    

                    cubei++;
                }
            }
        }
        geom.setCullSize(vec3.fromValues(sz/2,sz/2,sz/2),sz*2);
        return geom;
    }

    var sz=8;// 8:512 9:729 10:1000
    var mat=new DefaultColorShaderMaterial();
    var chunknum=100;
    for(var i=0;i<chunknum;i++) {
        var chx=i%8;
        var chy=Math.floor(i/8)%8;
        var chz=Math.floor(i/64)%8;

        var p = new Prop3D();
        var geom = createNinjaChunk(sz);
        p.setGeom(geom);
        p.setMaterial(mat);
        p.setTexture(g_base_tex);
        p.setScl(1,1,1);
        var gap=1;
        p.setLoc(-33+chx*(sz+gap),-chy*(sz+gap),-33+chz*(sz+gap));
        p.setColor(vec4.fromValues(range(0,1),range(0,1),range(0,1),1));
        g_main_layer.insertProp(p);        
    }
    

    // 8x8x8=512voxel *12tri=6Ktri x 1000chk = 6Mtri/frame
    // 1ボクセルあたり12triangle
    // 16voxel x 3000chk = 45fps (2.1GB)  (576000tri/frame)
    // 256voxel x 730chk = 60fps (2GB) (2242560tri/frame)
    // 4096voxel x 64chk = 60fps (1.8GB) (3145728tri/frame)
    // 4096voxel * 128chk = 45fps (2.1GB) (6.2Mtri/frame)
    // 2GB超えるとだめ。 300万tri (1triあたり700byte食うのでメモリがボトルネックになった。)
}

tangentMax = function(theta,absmax) {
    var cs=Math.cos(theta);
    var sn=Math.sin(theta);
    if(cs==0) {
        if(theta>0)return absmax; else return -absmax;
    } else {
        return Math.tan(theta);
    }
}


var last_anim_at = new Date().getTime();
var last_print_at = new Date().getTime();
var fps=0;
var started_at=now();
var pitch=0, yaw=0;
function animate() {
    if(!g_stop_render) requestAnimationFrame(animate);

    fps++;

    var now_time = new Date().getTime();
    var dt = (now_time - last_anim_at) / 1000.0;

    if(now_time > last_print_at+1000) {
        last_print_at=now_time;
        document.getElementById("status").innerHTML = "FPS:"+fps+ "props:" + g_main_layer.props.length + "draw3d:" + Moyai.draw_count_3d + " skip3d:" + Moyai.skip_count_3d;
        fps=0;
    }

    if(g_main_camera) {
        var t=now()-started_at;
        if(0) {
            g_main_camera.loc[0]=Math.cos(t/3)*8;
            g_main_camera.loc[1]+=0.01;        
            g_main_camera.loc[2]=Math.sin(t/3)*8;
        }
        if(g_keyboard) {
            if(g_keyboard.getKey('a')) g_main_camera.loc[0]-=0.3;
            if(g_keyboard.getKey('d')) g_main_camera.loc[0]+=0.3;
            if(g_keyboard.getKey('w')) g_main_camera.loc[2]-=0.3;
            if(g_keyboard.getKey('s')) g_main_camera.loc[2]+=0.3;
            if(g_keyboard.getKey('k')) g_main_camera.loc[1]+=0.3;
            if(g_keyboard.getKey('l')) g_main_camera.loc[1]-=0.3;                        
        }
        
        if(g_mouse) {
            var dx=g_mouse.movement[0];
            var dy=g_mouse.movement[1];
            g_mouse.clearMovement();
            yaw-=dy/250;
            if(yaw>Math.PI/2) yaw=Math.PI/2;
            if(yaw<-Math.PI/2) yaw=-Math.PI/2;
            pitch+=dx/250;

            var nose=vec3.fromValues( g_main_camera.loc[0] + 1.0 * Math.cos(pitch),
                                      g_main_camera.loc[1] + tangentMax(yaw),
                                      g_main_camera.loc[2] + 1.0 * Math.sin(this.pitch) );

            g_main_camera.setLookAt(nose, vec3.fromValues(0,1,0));            
        }
    }
    
    
    last_anim_at = now_time;    
    Moyai.poll(dt);
    Moyai.render();
}

animate();

