// moyai node.js server module
var fs=require("fs");
var net=require("net");

function Moyai() {
    this.layers=[];
    this.remote_head;
}
Moyai.prototype.insertLayer = function(l) {
    if(l.priority==null) {
        var highp = this.getHighestPriority();
        l.priority = highp+1;
    }
    this.layers.push(l);    
}
Moyai.prototype.poll = function(dt) {
    var cnt=0;
    for(var i=0;i<this.layers.length;i++) {
        var layer = this.layers[i];
        if( layer && (!layer.skip_poll) ) cnt += layer.pollAllProps(dt);
    }
    return cnt;   
}

Moyai.prototype.getHighestPriority = function() {
    var highp=0;
    for(var i=0;i<this.layers.length;i++) {
        if(this.layers[i].priority>highp) highp = this.layers[i].priority;
    }
    return highp;
}
Moyai.prototype.setRemoteHead = function(rh) { this.remote_head=rh; }

///////////////

Texture.prototype.id_gen = 1;
function Texture() {
    this.id = this.__proto__.id_gen++;
    this.image = null;
}
Texture.prototype.loadPNG = function(path) {
    this.image = new Image();
    var data = fs.readFileSync(path);
    this.image.loadPNGMem(data);
}
Texture.prototype.getSize = function() {
    return this.image.getSize();
}
Texture.prototype.setImage = function(img) {
    this.image = img;
}
Texture.prototype.updateImage = function(img) {
    console.log("no impl in node");
}


/////////////////////

Prop2D.prototype.id_gen=1;
function Prop2D() {
    this.id=this.__proto__.id_gen++;
    this.index = 0;
    this.scl = new Vec2(32,32);
    this.loc = new Vec2(0,0);
    this.rot = 0;
    this.deck = null;
    this.uvrot = false;
    this.color = new Color(1,1,1,1);
    this.prim_drawer = null;
    this.grids=null;
    this.visible=true;
    this.use_additive_blend = false;
    this.children=[];
    this.accum_time=0;
    this.poll_count=0;
//    this.mesh=null;
//    this.material=null;
    this.priority = null; // set when insertprop if kept null
//    this.need_material_update=false;
//    this.need_color_update=false;
//    this.need_uv_update=true;
    this.xflip=false;
    this.yflip=false;
    this.fragment_shader= new DefaultColorShader();
//    this.remote_vel=null; 
}
Prop2D.prototype.onDelete = function() {
    if(this.mesh){
        if(this.mesh.geometry) this.mesh.geometry.dispose();
        if(this.mesh.material) this.mesh.material.dispose();
    }
}
Prop2D.prototype.setVisible = function(flg) { this.visible=flg; }
Prop2D.prototype.setDeck = function(dk) { this.deck = dk; this.need_material_update = true; }
Prop2D.prototype.setIndex = function(ind) { this.index = ind; this.need_uv_update = true; }
Prop2D.prototype.setScl = function(x,y) { this.scl.setWith2args(x,y);}
Prop2D.prototype.setLoc = function(x,y) { this.loc.setWith2args(x,y);}
Prop2D.prototype.setRot = function(r) { this.rot=r; }
Prop2D.prototype.setUVRot = function(flg) { this.uvrot=flg; this.need_uv_update = true; }
Prop2D.prototype.setColor = function(r,g,b,a) {
    if(this.color.equals(r,g,b,a)==false) {
        this.need_color_update = true;
        if(this.fragment_shader) this.need_material_update = true;
    }
    if(typeof r == 'object' ) {
        this.color = r ;
    } else {
        this.color = new Color(r,g,b,a); 
    }
}
Prop2D.prototype.setXFlip = function(flg) { this.xflip=flg; this.need_uv_update = true; }
Prop2D.prototype.setYFlip = function(flg) { this.yflip=flg; this.need_uv_update = true; }
Prop2D.prototype.setPriority = function(prio) { this.priority = prio; }
Prop2D.prototype.ensurePrimDrawer = function() {
    if(!this.prim_drawer) this.prim_drawer = new PrimDrawer();
}
Prop2D.prototype.addLine = function(p0,p1,col,w) {
    this.ensurePrimDrawer();
    return this.prim_drawer.addLine(new Vec2(p0.x,p0.y),new Vec2(p1.x,p1.y),col,w);
}
Prop2D.prototype.addRect = function(p0,p1,col,w) {
    this.ensurePrimDrawer();
    return this.prim_drawer.addRect(new Vec2(p0.x,p0.y),new Vec2(p1.x,p1.y),col,w);
}
Prop2D.prototype.getPrimById = function(id) {
    if(!this.prim_drawer)return null;
    return this.prim_drawer.getPrimById(id);
}
Prop2D.prototype.deletePrim = function(id) {
    if(this.prim_drawer) this.prim_drawer.deletePrim(id);
}
Prop2D.prototype.addGrid = function(g) {
    if(!this.grids) this.grids=[];
    this.grids.push(g);
//    console.log("addGrid: grid id:",g.id,g.width, g.height, "propid:",this.id, "grids:",this.grids);
}
Prop2D.prototype.setGrid = function(g) {
    if(this.grids) {
        for(var i=0;i<this.grids.length;i++) {
            if(this.grids[i].id==g.id) {
                return;
            }
        }
    }
    this.addGrid(g);
}
Prop2D.prototype.setTexture = function(tex) {
    var td = new TileDeck();
    td.setTexture(tex);
    var sz = tex.getSize();
    td.setSize(1,1,sz.x,sz.y);
    this.setDeck(td);
    this.setIndex(0);
    this.need_material_update = true;
}
Prop2D.prototype.addChild = function(chp) {
    this.children.push(chp);
}
Prop2D.prototype.clearChildren = function() {
    this.children=[];
}
Prop2D.prototype.clearChild = function(p) {
    var keep=[];
    for(var i=0;i<this.children.length;i++) {
        if(this.children[i]!=p) keep.push( this.children[i]);
    }
    this.children = keep;
}
Prop2D.prototype.getChild = function(propid) {
    for(var i=0;i<this.children.length;i++) {
        if( this.children[i].id == propid ) {
            return this.children[i];
        }
    }
    return null;
}
Prop2D.prototype.setFragmentShader = function(s) {    this.fragment_shader = s;}
Prop2D.prototype.basePoll = function(dt) { // return false to clean
    this.poll_count++;
    this.accum_time+=dt;    
    if(this.to_clean) {
        return false;
    }
    if( this.prop2DPoll && this.prop2DPoll(dt) == false ) {
        return false;
    }
    return true;
}

///////////////

FragmentShader.prototype.id_gen=1;
function FragmentShader() {
    this.id=this.__proto__.id_gen++;
    this.uniforms=null;
}
ColorReplacerShader.prototype = Object.create(FragmentShader.prototype);
ColorReplacerShader.prototype.constructor = ColorReplacerShader;
function ColorReplacerShader() {
    FragmentShader.call(this);
    this.setColor(new THREE.Vector3(0,0,0),new THREE.Vector3(0,1,0),0.01);
}
// updateUniforms(tex) called when render
ColorReplacerShader.prototype.updateUniforms = function(texture,moyaicolor) {
    if(this.uniforms) {
        if(texture) this.uniforms["texture"]["value"] = texture;
        this.uniforms["color1"]["value"] = this.from_color;
        this.uniforms["replace1"]["value"] = this.to_color;
        this.uniforms["eps"]["value"] = this.epsilon;
    } else {
        this.uniforms = {
            "texture" : { type: "t", value: texture },        
            "color1" : { type: "v3", value: this.from_color },
            "replace1" : { type: "v3", value: this.to_color },
            "eps" : { type: "f", value: this.epsilon }
        }
    }
    //    console.log("colrep: updated uniforms. tex:", texture, this.from_color, this.to_color );    
}
ColorReplacerShader.prototype.setColor = function(from,to,eps) {
    this.epsilon = eps;
    this.from_color = new THREE.Vector3(from.r,from.g,from.b);
    this.to_color = new THREE.Vector3(to.r,to.g,to.b);
    this.updateUniforms();
}
DefaultColorShader.prototype = Object.create(FragmentShader.prototype);
DefaultColorShader.prototype.constructor = DefaultColorShader;
function DefaultColorShader() {
    FragmentShader.call(this);
}
DefaultColorShader.prototype.updateUniforms = function(texture,moyaicolor) {
    if(this.uniforms) {
        if(texture) this.uniforms["texture"]["value"] = texture;
        this.uniforms["meshcolor"]["value"] = new THREE.Vector4(moyaicolor.r, moyaicolor.g, moyaicolor.b, moyaicolor.a );
    } else {
        this.uniforms = {
            "texture" : { type: "t", value: texture },
            "meshcolor" : { type: "v4", value: new THREE.Vector4(moyaicolor.r, moyaicolor.g, moyaicolor.b, moyaicolor.a ) }
        };
    }
}
PrimColorShader.prototype = Object.create(FragmentShader.prototype);
PrimColorShader.prototype.constructor = PrimColorShader;
function PrimColorShader() {
    FragmentShader.call(this);
}
PrimColorShader.prototype.updateUniforms = function(moyaicolor) {
    if(this.uniforms) {
        this.uniforms["meshcolor"]["value"] = new THREE.Vector4(moyaicolor.r, moyaicolor.g, moyaicolor.b, moyaicolor.a );
    } else {
        this.uniforms = {
            "meshcolor" : { type: "v4", value: new THREE.Vector4(moyaicolor.r, moyaicolor.g, moyaicolor.b, moyaicolor.a ) }
        };
    }
}

///////////////////////

Stream.prototype.id_gen=1;
function Stream(conn) {
    this.id=this.__proto__.id_gen++;
    this.conn=conn;
    this.sendbuf = new Buffer();
    this.recvbuf = new Bufffer();
}
Stream.prototype.flushSendbuf = function(unitsize) {
    var to_send=unitsize;
    if(this.sendbuf.length>to_send)to_send=this.sendbuf.length;
    var final_buf = this.sendbuf.slice(0,to_send);
    conn.write(final_buf);
    this.sendbuf = this.sendbuf.slice(to_send,this.sendbuf.length);
};

function Client(conn) {
    this.__proto__ = Stream.prototype;
    this.parent_rh=null;
    this.target_camera=null;
    this.target_viewport=null;
    this.accum_time=0;
}
Client.prototype.canSee = null;

function RemoteHead() {

    this.target_moyai=null;
    this.target_soundsystem=null;
    this.window_width=0;
    this.window_height=0;

    this.on_connect_cb=null;
    this.on_disconnect_cb=null;
    this.on_keyboard_cb=null;
    this.on_mouse_button_cb=null;
    this.on_mouse_cursor_cb=null;

    this.server = null;

    this.clients=new Array();
}
RemoteHead.prototype.addClient = function(cl) {
    this.clients.push(cl);
}
RemoteHead.prototype.delClient = function(cl) {
    for(var i=0;i<this.clients.length;i++) {
        if(this.clients[i].id==cl.id) {
            this.clients = this.clients.splice(i,1);
            break;
        }
    }
}
RemoteHead.prototype.track2D = null;
RemoteHead.prototype.startServer = function(port) {
    this.server = net.createServer( function(conn) {
        console.log("rh:  newconnection");
    });
    this.server.listen(22222);
}
RemoteHead.prototype.setWindowSize = function(w,h) { this.window_width = w; this.window_height = h; }
RemoteHead.prototype.setOnConnectCallback = function(cb) { this.on_connect_cb=cb;}
RemoteHead.prototype.setOnDisconnectCallback = function(cb) { this.on_disconnect_cb = cb; }
RemoteHead.prototype.setOnKeyboardCallback = function(cb) { this.on_keyboard_cb = cb; }
RemoteHead.prototype.setOnMouseButtonCallback = function(cb) {this.on_mouse_button_cb = cb; }
RemoteHead.prototype.setOnMouseCursorCallback = function(cb) { this.on_mouse_cursor_cb = cb; }
RemoteHead.prototype.heartbeat = null; // (double dt);
RemoteHead.prototype.flushBufferToNetwork = null; //(double dt);
RemoteHead.prototype.scanSendAllPrerequisites = null; //( Stream *outstream );
RemoteHead.prototype.scanSendAllProp2DSnapshots = null; // ( Stream *outstream );
//    void notifyProp2DDeleted( Prop2D *prop_deleted );
//    void notifyGridDeleted( Grid *grid_deleted );
//    void notifyChildCleared( Prop2D *owner_prop, Prop2D *child_prop );
//    void setTargetSoundSystem(SoundSystem*ss) { target_soundsystem = ss; }
RemoteHead.prototype.setTargetMoyai = function(moy) { this.target_moyai=moy; }
//    void notifySoundPlay( Sound *snd, float vol );
//    void notifySoundStop( Sound *snd );
    
//    void broadcastUS1Bytes( uint16_t usval, const char *data, size_t datalen );
//    void broadcastUS1UI1Bytes( uint16_t usval, uint32_t uival, const char *data, size_t datalen );    
//    void broadcastUS1UI1( uint16_t usval, uint32_t uival );
//    void broadcastUS1UI2( uint16_t usval, uint32_t ui0, uint32_t ui1, bool reprecator_only = false );
//    void broadcastUS1UI3( uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2 );
//    void broadcastUS1UI4( uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2, uint32_t ui3 );    
//    void broadcastUS1UI1Wstr( uint16_t usval, uint32_t uival, wchar_t *wstr, int wstr_num_letters );
//    void broadcastUS1UI1F1( uint16_t usval, uint32_t uival, float f0 );
//    void broadcastUS1UI1F2( uint16_t usval, uint32_t uival, float f0, float f1 );
//    void broadcastUS1UI2F2( uint16_t usval, uint32_t ui0, uint32_t ui1, float f0, float f1 );    
//    void broadcastUS1UI1F4( uint16_t usval, uint32_t uival, float f0, float f1, float f2, float f3 );
//    void broadcastUS1UI1UC1( uint16_t usval, uint32_t uival, uint8_t ucval );    

//    void nearcastUS1UI1F2( Prop2D *p, uint16_t usval, uint32_t uival, float f0, float f1 );
//    void nearcastUS1UI3( Prop2D *p, uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2, bool reprecator_only = false );
//    void nearcastUS1UI3F2( Prop2D *p, uint16_t usval, uint32_t uival, uint32_t u0, uint32_t u1, float f0, float f1 );

//    void broadcastUS1UI2ToReprecator( uint16_t usval, uint32_t ui0, uint32_t ui1 );
    
//    void broadcastTimestamp();

//    static const char *funcidToString(PACKETTYPE pkt);




///////////////////////


global.pngParse = require("pngparse-sync");

global.Moyai=Moyai;
global.Texture=Texture;
global.Prop2D=Prop2D;
global.RemoteHead=RemoteHead;