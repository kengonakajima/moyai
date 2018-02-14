// moyai node.js server module
var fs=require("fs");
var net=require("net");

require("../web/moyai_common.js");
eval(fs.readFileSync("../web/packettypes.js").toString());



function MoyaiClient(win_w,win_h) {
    this.window_width=win_w;
    this.window_height=win_h;
    
    this.layers=[];
    this.remote_head;
}
MoyaiClient.prototype.insertLayer = function(l) {
    if(l.priority==null) {
        var highp = this.getHighestPriority();
        l.priority = highp+1;
    }
    this.layers.push(l);    
}
MoyaiClient.prototype.poll = function(dt) {
    var cnt=0;
    for(var i in this.layers) {
        var layer = this.layers[i];
        if( layer && (!layer.skip_poll) ) cnt += layer.pollAllProps(dt);
    }
    return cnt;   
}

MoyaiClient.prototype.getHighestPriority = function() {
    var highp=0;
    for(var i in this.layers) {
        if(this.layers[i].priority>highp) highp = this.layers[i].priority;
    }
    return highp;
}
MoyaiClient.prototype.setRemoteHead = function(rh) {
    this.remote_head=rh;
    rh.setWindowSize(this.window_width, this.window_height);
}

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
    this.image.last_load_file_path=path;
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
    this.parent_layer = null;
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
    this.priority = null; // set when insertprop if kept null
    this.xflip=false;
    this.yflip=false;
    this.fragment_shader= new DefaultColorShader();
    this.tracker=null;
}
Prop2D.prototype.getParentLayer = function() {
    return this.parent_layer;
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
Prop2D.prototype.onTrack = function(rh,parentprop) {
    if(!this.tracker) {
        this.tracker=new Tracker2D(rh,this);
    }
    this.tracker.scanProp2D(parentprop);
    this.tracker.broadcastDiff(false);
    this.tracker.flipCurrentBuffer();

    // TODO: track grids, shader, prims, dynamic images, children
    for(var i in this.grids) {
        var g=this.grids[i];
        if(!g.tracker) g.tracker = new TrackerGrid(rh,g);
        g.tracker.scanGrid();
        g.tracker.broadcastDiff(this,false);
        g.tracker.flipCurrentBuffer();
    }
}

///////////////
Grid.prototype.id_gen=1;

function Grid(w,h) {
    this.id=this.__proto__.id_gen++;
    this.width=w;
    this.height=h;
    this.index_table=null;
    this.xflip_table=null;
    this.yflip_table=null;
    this.texofs_table=null;
    this.rot_table=null;
    this.color_table=null;
    this.deck=null;
    this.visible=true;
    this.parent_prop=null;
}
Grid.prototype.setDeck =function(dk) { this.deck=dk; this.need_material_update=true;}
Grid.prototype.index = function(x,y) { return x+y*this.width; }
Grid.prototype.getCellNum = function() { return this.width * this.height; }
Grid.prototype._fill = function(tbl,val) {
    for(var y=0;y<this.height;y++) {
        for(var x=0;x<this.width;x++) {
            tbl[x+y*this.width] = val;
        }
    }
}
var GRID_NOT_USED = -1;
Grid.prototype.set = function(x,y,ind) {
    if(!this.index_table) this.index_table=[];
    this.index_table[this.index(x,y)] = ind;
}
Grid.prototype.get  =function(x,y) {
    if(!this.index_table) return GRID_NOT_USED;
    return this.index_table[ this.index(x,y) ];
}
Grid.prototype.bulkSetIndex = function(inds) {
    if(!this.index_table) this.index_table=[];
    var expect_len = this.width * this.height;
    if(inds.length < expect_len) {
        console.log("bulksetindex: data not enough. expect:",expect_len, "got:",inds.length);
    } else {
        for(var i=0;i<expect_len;i++) this.index_table[i] = inds[i];
    }
}
Grid.prototype.bulkSetFlipRotBits = function(xflbits,yflbits,uvrotbits) {
    var expect_len = this.width * this.height;
    var ind=0;
    for(var y=0;y<this.height;y++) {
        for(var x=0;x<this.width;x++) {
            this.setXFlip(x,y,xflbits[ind]);
            this.setYFlip(x,y,yflbits[ind]);
            this.setUVRot(x,y,uvrotbits[ind]);
            ind++;
        }
    }
}
Grid.prototype.bulkSetTexofs = function(ofsary) {
    var expect_len = this.width * this.height;
    if(ofsary.length < expect_len ) {
        console.log("bulksettexofs: data not enough. expect:", expect_len, "got:", ofsary.length );
    } else {
        var ind=0;
        for(var y=0;y<this.height;y++) {
            for(var x=0;x<this.width;x++) {
                this.setTexOffset(x,y,ofsary[ind]);
                ind++;
            }
        }
        
    }
}
Grid.prototype.bulkSetColor = function(colsary) {
    var expect_len = this.width * this.height;
    if(colsary.length < expect_len ) {
        console.log("bulksetcolor: data not enough. expect:", expect_len, "got:", colsary.length );
    } else {
        var ind=0;
        for(var y=0;y<this.height;y++) {
            for(var x=0;x<this.width;x++) {
                this.setColor(x,y,colsary[ind]);
                ind++;
            }
        }        
    }
}
Grid.prototype.setXFlip = function(x,y,flg) {
    if(!this.xflip_table) this.xflip_table=[];
    this.xflip_table[this.index(x,y)]=flg;
}
Grid.prototype.getXFlip = function(x,y) {
    if(!this.xflip_table) return false;
    return this.xflip_table[this.index(x,y)];
}
Grid.prototype.setYFlip = function(x,y,flg) {
    if(!this.yflip_table) this.yflip_table=[];
    this.yflip_table[this.index(x,y)]=flg;    
}
Grid.prototype.getYFlip = function(x,y) {
    if(!this.yflip_table) return false;
    return this.yflip_table[this.index(x,y)];
}
Grid.prototype.setTexOffset = function(x,y,uv) {
    if(!this.texofs_table) this.texofs_table=[];
    this.texofs_table[this.index(x,y)]=uv;
}
Grid.prototype.getTexOffset = function(x,y) {
    if(!this.texofs_table) return new Vec2(0,0);
    var o=this.texofs_table[this.index(x,y)];
    if(!o)return new Vec2(0,0); else return o;
}
Grid.prototype.setUVRot = function(x,y,flg) {
    if(!this.rot_table) this.rot_table=[];
    this.rot_table[this.index(x,y)]=flg;
}
Grid.prototype.getUVRot = function(x,y) {
    if(!this.rot_table) return false;
    return this.rot_table[this.index(x,y)];
}
Grid.prototype.setColor = function(x,y,col) {
    if(!this.color_table) this.color_table=[];
    this.color_table[this.index(x,y)]=col;
}
Grid.prototype.getColor = function(x,y) {
    if(!this.color_table) return new Color(1,1,1,1);
    var col=this.color_table[this.index(x,y)];
    if(!col) return new Color(1,1,1,1); else return col;
}
Grid.prototype.setVisible = function(flg) { this.visible=flg; }
Grid.prototype.getVisible = function() { return this.visible; }
Grid.prototype.clear = function(x,y) {
    if(x== (void 0) ) {
        if(this.index_table) this._fill(this.index_table,GRID_NOT_USED);
    } else {
        this.set(x,y,GRID_NOT_USED);
    }    
}
Grid.prototype.fillColor = function(c) {
    if(!this.color_table)this.color_table=[];
    if(this.color_table) {
        for(var y=0;y<this.height;y++) {
            for(var x=0;x<this.width;x++) {
                this.color_table[this.index(x,y)] = new Color(c.r,c.g,c.b,c.a);
            }
        }
    }
}
Grid.prototype.fill = function(ind) {
    this.fillRect(0,0,this.width-1,this.height-1,ind);
}
Grid.prototype.fillRect = function(x0,y0,x1,y1,ind) {
    for(var y=y0;y<=y1;y++) {
        for(var x=x0;x<=x1;x++) {
            this.set(x,y,ind);
        }
    }    
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
function Stream() {
//    console.log("Stream cons: given this:",this, "arg:",arguments);
    this.id=this.__proto__.id_gen++;
    this.conn=arguments[0];
    this.sendbuf = null; // いつもBuffer.lengthまでデータが満ちてるようにする
    this.recvbuf = null;
}
Stream.prototype.flushSendbuf = function(unitsize) {
    if(!this.sendbuf)return;
    var to_send=unitsize;
    if(this.sendbuf.length>to_send)to_send=this.sendbuf.length;
    var final_buf = this.sendbuf.slice(0,to_send);
    this.conn.write(final_buf);
    this.sendbuf = this.sendbuf.slice(to_send,this.sendbuf.length);
};
Stream.prototype.appendSendbuf = function(buffer) {
    console.log("appendSendbuf:",buffer);
    if(!this.sendbuf) {
        this.sendbuf=new Buffer(buffer);   
    } else {
        this.sendbuf=Buffer.concat([this.sendbuf,buffer]);
    }
}

function Client() {
    Client.super_.apply(this,arguments);
//    console.log("Client cons: this:",this, "arg:",arguments);
    this.parent_rh=null;
    this.target_camera=null;
    this.target_viewport=null;
    this.accum_time=0;
}
Client.super_=Stream;
Client.prototype=Object.create(Stream.prototype, {
    constructor: {
        value: Client,
        enumerable: false 
    }
    
})
Client.prototype.constructor = Client;
Client.prototype.canSee = null;

/////////////////
function sendUS1Bytes(s,us,b) {
    var l=2+4;
    var outb=new Buffer(4+l);
    outb.writeUInt32LE(l+b.length,0);
    outb.writeUInt16LE(us,4);
    outb.writeUInt32LE(b.length,6);
    var finb=Buffer.concat([outb,b]);
    s.appendSendbuf(finb);
}
function sendUS1UI1Bytes(s,us,ui,b) {
    var l=2+4+4;
    var outb=new Buffer(4+l);
    outb.writeUInt32LE(l+b.length,0);
    outb.writeUInt16LE(us,4);
    outb.writeUInt32LE(ui,6);
    outb.writeUInt32LE(b.length,10);    
    var finb=Buffer.concat([outb,b]);
    s.appendSendbuf(finb);    
}
function sendUS1UI1(s,us,ui) {
    var l=2+4;
    var b=new Buffer(4+l);
    b.writeUInt32LE(l,0)
    b.writeUInt16LE(us,4);
    b.writeUInt32LE(ui,6);
    s.appendSendbuf(b);    
}
function sendUS1UI1Str(s,us,ui,str) {
    var strblen=Buffer.byteLength(str,"utf8");
    if(strblen>255) throw "sendUS1UI1Str: string too long, cant send message";
    var l=2+4+1+strblen;
    var b=new Buffer(4+l);
    b.writeUInt32LE(l,0)
    b.writeUInt16LE(us,4);
    b.writeUInt32LE(ui,6);
    b.writeUInt8(strblen,10);
    b.write(str,11);
    s.appendSendbuf(b);    
}
function sendUS1StrBytes(s,us,str,sb) {
    var strblen=Buffer.byteLength(str,"utf8");    
    if(strblen>255) throw "sendUS1StrBytes: string too long, cant send message:"+str;
    var l=2+(1+strblen)+(4+sb.length);
    console.log("sendUS1StrBytes: strblen:",strblen, "buflen:",sb.length, "total:",l);
    var b=new Buffer(4+l);
    b.writeUInt32LE(l,0)
    b.writeUInt16LE(us,4);
    b.writeUInt8(strblen,6);
    b.write(str,7);
    b.writeUInt32LE(sb.length,7+strblen);
    sb.copy(b,7+strblen+4);
    s.appendSendbuf(b);    
}    

function sendUS1UI1F2(s,us,ui,f0,f1) {
    var l=2+4+4+4;
    var b=new Buffer(4+l);
    b.writeUInt32LE(l,0)
    b.writeUInt16LE(us,4);
    b.writeUInt32LE(ui,6);
    b.writeFloatLE(f0,10);
    b.writeFloatLE(f1,14);        
    s.appendSendbuf(b);        
}
function sendUS1UI2(s,us,ui0,ui1) {
    var l=2+4+4;
    var b=new Buffer(4+l);
    b.writeUInt32LE(l,0)
    b.writeUInt16LE(us,4);
    b.writeUInt32LE(ui0,6);
    b.writeUInt32LE(ui1,10);
    s.appendSendbuf(b);
}
function sendUS1UI3(s,us,ui0,ui1,ui2) {
    var l=2+4+4+4;
    var b=new Buffer(4+l);
    b.writeUInt32LE(l,0)
    b.writeUInt16LE(us,4);
    b.writeUInt32LE(ui0,6);
    b.writeUInt32LE(ui1,10);
    b.writeUInt32LE(ui2,14);    
    s.appendSendbuf(b);
}
function sendUS1UI5(s,us,ui0,ui1,ui2,ui3,ui4) {
    var l=2+4*5;
    var b=new Buffer(4+l);
    b.writeUInt32LE(l,0)
    b.writeUInt16LE(us,4);
    b.writeUInt32LE(ui0,6);
    b.writeUInt32LE(ui1,10);
    b.writeUInt32LE(ui2,14);
    b.writeUInt32LE(ui3,18);
    b.writeUInt32LE(ui4,22);    
    s.appendSendbuf(b);    
}
function sendWindowSize(s,w,h) {
    sendUS1UI2(s,PACKETTYPE_S2C_WINDOW_SIZE,w,h);
}
function sendViewportCreateScale(s,vp) {
    sendUS1UI1(s,PACKETTYPE_S2C_VIEWPORT_CREATE, vp.id );
    sendUS1UI1F2(s,PACKETTYPE_S2C_VIEWPORT_SCALE, vp.id, vp.scl.x, vp.scl.y );
}
function sendCameraCreateLoc(s,cam) {
    sendUS1UI1(s,PACKETTYPE_S2C_CAMERA_CREATE, cam.id );
    sendUS1UI1F2(s,PACKETTYPE_S2C_CAMERA_LOC, cam.id, cam.loc.x, cam.loc.y );
}
function sendLayerSetup(s,l) {
    console.log("sending layer_create,viewport,camera. layer:", l.id);
    sendUS1UI2(s,PACKETTYPE_S2C_LAYER_CREATE, l.id, l.priority );
    if(l.viewport) sendUS1UI2(s,PACKETTYPE_S2C_LAYER_VIEWPORT, l.id, l.viewport.id);
    if(l.camera ) sendUS1UI2(s,PACKETTYPE_S2C_LAYER_CAMERA, l.id, l.camera.id );
}
function sendDeckSetup(s,dk) {
    console.log("sending tiledeck_create:", dk );
    sendUS1UI1(s, PACKETTYPE_S2C_TILEDECK_CREATE, dk.id );
    sendUS1UI2(s, PACKETTYPE_S2C_TILEDECK_TEXTURE, dk.id, dk.moyai_tex.id );
    sendUS1UI5(s, PACKETTYPE_S2C_TILEDECK_SIZE, dk.id, dk.tile_width, dk.tile_height, dk.cell_width, dk.cell_height );    
}
function sendTextureCreateWithImage(s,tex) {
    console.log("sending texture_create, texture:", tex.id, "image:", tex.image.id );    
    sendUS1UI1(s, PACKETTYPE_S2C_TEXTURE_CREATE, tex.id );
    sendUS1UI2(s, PACKETTYPE_S2C_TEXTURE_IMAGE, tex.id, tex.image.id );
}
function sendImageSetup(s,moyimg) {
    console.log("sending image_create id:", moyimg.id );
    sendUS1UI1(s, PACKETTYPE_S2C_IMAGE_CREATE, moyimg.id );
    if( moyimg.last_load_file_path ) {
        console.log("sending image_load_png:", moyimg.last_load_file_path );
        sendUS1UI1Str(s, PACKETTYPE_S2C_IMAGE_LOAD_PNG, moyimg.id, moyimg.last_load_file_path );                
    }
    if( moyimg.width>0 && moyimg.buffer) {
        // this image is not from file, maybe generated.
        console.log("sending image_ensure_size id:", moyimg.id, moyimg.width, moyimg.height );        
        sendUS1UI3(s, PACKETTYPE_S2C_IMAGE_ENSURE_SIZE, moyimg.id, moyimg.width, moyimg.height );
    }
}
function sendFile(s,path) {
    var buf=fs.readFileSync(path)
    sendUS1StrBytes(s, PACKETTYPE_S2C_FILE, path, buf);
}


/////////////////

function copyColorToPacketColor(destpkt,srccol) {
    var c = srccol.toRGBA();
    destpkt.r=c.r; destpkt.g=c.g; destpkt.b=c.b; destpkt.a=c.a;
}

function toFlipRotBits(xflip,yflip,uvrot) {
    var out=0;
    if(xflip) out|=0x1;
    if(yflip) out|=0x2;
    if(uvrot) out|=0x4;
    return out;
}

function makePacketProp2DSnapshotInBuffer(p,parent) {
    var l=64;
    var b=new Buffer(l);
    b.writeUInt32LE(p.id,0);
    b.writeUInt32LE( parent ? 0 : p.getParentLayer().id,4);
    b.writeUInt32LE( parent ? p.parent_prop_id : 0, 8);
    b.writeFloatLE(p.loc.x,12);
    b.writeFloatLE(p.loc.y,16);
    b.writeFloatLE(p.scl.x,20);
    b.writeFloatLE(p.scl.y,24);    
    b.writeInt32LE(p.index,28);    
    b.writeUInt32LE( p.deck ? p.deck.id : 0, 32);    
    b.writeInt32LE(p.debug,36);    
    b.writeFloatLE(p.rot,40);
    var colary = p.color.toRGBA();
    b.writeUInt8(colary[0],44);
    b.writeUInt8(colary[1],45);
    b.writeUInt8(colary[2],46);
    b.writeUInt8(colary[3],47);
    b.writeUInt32LE(p.fragment_shader ? p.fragment_shader.id : 0,48);
    var optbits=0;
    if(p.use_additive_blend) optbits |= PROP2D_OPTBIT_ADDITIVE_BLEND;
    b.writeUInt32LE(optbits,52);
    b.writeInt32LE(p.priority,56);
    b.writeUInt32LE( toFlipRotBits(p.xflip,p.yflip,p.uvrot), 60 );
//    console.log( "makePacketProp2DSnapshotInBuffer outb:", b);
    return b;
}




function Tracker2D(rh,p) {
    this.parent_rh=rh;
    this.target_prop2d=p;
    this.cur_buffer_index=0;
    this.pktbuf=new Array(2); // flip-flop
    this.pktbuf[0]=new Buffer(0);
    this.pktbuf[1]=new Buffer(0);        
}
Tracker2D.prototype.checkDiff = function() {
    if(Buffer.compare(this.pktbuf[0],this.pktbuf[1])==0) {
        return false;
    } else {
        return true;
    }
}
Tracker2D.prototype.broadcastDiff = function(force) {
    var diff=this.checkDiff();
    if(diff||force) {
        // TODO: Reduce bandwidth.  locsyncmode, _LOC, _LOC_VEL, ...
        // TODO: reprecator
        console.log("t2d:curind:",this.cur_buffer_index, " bcdiff:", this.pktbuf[this.cur_buffer_index] );
        this.parent_rh.broadcastUS1Bytes( PACKETTYPE_S2C_PROP2D_SNAPSHOT, this.pktbuf[this.cur_buffer_index]);
    }
}

Tracker2D.prototype.scanProp2D = function(parentprop) {
    this.pktbuf[this.cur_buffer_index] = makePacketProp2DSnapshotInBuffer(this.target_prop2d,parentprop);
}
Tracker2D.prototype.flipCurrentBuffer = function() {
    this.cur_buffer_index = ( this.cur_buffer_index == 0 ? 1 : 0 );
}


////////////////////

Viewport.prototype.onTrack = function(rh) {
    if(!this.tracker) {
        this.tracker = new TrackerViewport(rh,this);
    }
    this.tracker.scanViewport();
    this.tracker.broadcastDiff(false);
    this.tracker.flipCurrentBuffer();
}
Camera.prototype.onTrack = function(rh) {
    if(!this.tracker) {
        this.tracker = new TrackerCamera(rh,this);
    }
    this.tracker.scanCamera();
    this.tracker.broadcastDiff(false);
    this.tracker.flipCurrentBuffer();    
}

////////////////////

function TrackerViewport(rh,vp) {
    this.parent_rh=rh;
    this.target_viewport=vp;
    this.cur_buffer_index=0;
    this.sclbuf=new Array(2); // flip-flop
    this.sclbuf[0]=new Vec2(0,0);
    this.sclbuf[1]=new Vec2(0,0);
}
TrackerViewport.prototype.scanViewport = function() {
    this.sclbuf[this.cur_buffer_index] = new Vec2( this.target_viewport.scl.x, this.target_viewport.scl.y );
}
TrackerViewport.prototype.flipCurrentBuffer = function() {
    this.cur_buffer_index = ( this.cur_buffer_index == 0 ? 1 : 0 );
}
TrackerViewport.prototype.checkDiff = function() {
    var curscl, prevscl;
    if( this.cur_buffer_index == 0 ) {
        curscl = this.sclbuf[0];
        prevscl = this.sclbuf[1];
    } else {
        curscl = this.sclbuf[1];
        prevscl = this.sclbuf[0];
    }
    return curscl.isEqual(prevscl);
}
TrackerViewport.prototype.broadcastDiff = function(force) {
    if( this.checkDiff() || force ) {
        this.parent_rh.broadcastUS1UI1F2( PACKETTYPE_S2C_VIEWPORT_SCALE,
                                          this.target_viewport.id,
                                          this.sclbuf[this.cur_buffer_index].x,
                                          this.sclbuf[this.cur_buffer_index].y );
    }
}

/////////////////////

function TrackerCamera(rh,cam) {
    this.target_camera = cam;
    this.cur_buffer_index=0;
    this.parent_rh=rh;
    this.locbuf = [new Vec2(0,0), new Vec2(0,0)];
}
TrackerCamera.prototype.scanCamera = function() {
    this.locbuf[this.cur_buffer_index] = new Vec2( this.target_camera.loc.x, this.target_camera.loc.y );
}
TrackerCamera.prototype.flipCurrentBuffer = function() {
    this.cur_buffer_index = ( this.cur_buffer_index == 0 ? 1 : 0 );
}
TrackerCamera.prototype.checkDiff = function() {
    var curloc, prevloc;
    if( this.cur_buffer_index == 0 ) {
        curloc = this.locbuf[0];
        prevloc = this.locbuf[1];
    } else {
        curloc = this.locbuf[1];
        prevloc = this.locbuf[0];
    }
    return (curloc.x != prevloc.x || curloc.y != prevloc.y );
}
TrackerCamera.prototype.broadcastDiff = function(force) {
    if( this.checkDiff() || force ) {
        this.parent_rh.broadcastUS1UI1F2( PACKETTYPE_S2C_CAMERA_LOC, this.target_camera.id, this.locbuf[this.cur_buffer_index].x, this.locbuf[this.cur_buffer_index].y );
    }
}


////////////////////

var GTT_INDEX = 1;
var GTT_FLIP = 2;
var GTT_TEXOFS = 3;
var GTT_COLOR = 4;

var GTT_FLIP_BIT_X = 0x01;
var GTT_FLIP_BIT_Y = 0x02;
var GTT_FLIP_BIT_UVROT = 0x04;

function TrackerGrid(rh,g) {
    this.target_grid=g;
    this.parent_rh=rh;
    this.cur_buffer_index=0;
    this.index_table=[new Array(),new Array()];
    this.flip_table=[new Array(),new Array()];
    this.texofs_table=[new Array(),new Array()];
    this.color_table=[new Array(),new Array()];
}

TrackerGrid.prototype.scanGrid = function() {
    for(var y=0;y<this.target_grid.height;y++){
        for(var x=0;x<this.target_grid.width;x++){
            var ind = this.target_grid.index(x,y);
            if(this.index_table[this.cur_buffer_index]) {
                this.index_table[this.cur_buffer_index][ind] = this.target_grid.get(x,y);
            }
            if(this.flip_table[this.cur_buffer_index] ) {
                var bits = 0;
                if( this.target_grid.getXFlip(x,y) ) bits |= GTT_FLIP_BIT_X;
                if( this.target_grid.getYFlip(x,y) ) bits |= GTT_FLIP_BIT_Y;
                if( this.target_grid.getUVRot(x,y) ) bits |= GTT_FLIP_BIT_UVROT;
                this.flip_table[this.cur_buffer_index][ind] = bits;
            }
            if(this.texofs_table[this.cur_buffer_index]) {
                var texofs=this.target_grid.getTexOffset(x,y);
                this.texofs_table[this.cur_buffer_index][ind] = new Vec2(texofs.x,texofs.y);
            }
            if(this.color_table[this.cur_buffer_index]) {
                var col = this.target_grid.getColor(x,y);
                this.color_table[this.cur_buffer_index][ind] = new Color(col.r,col.g,col.b,col.a);
            }
        }
    }
}
TrackerGrid.prototype.flipCurrentBuffer = function() {
    this.cur_buffer_index = ( this.cur_buffer_index == 0 ? 1 : 0 );    
}

TrackerGrid.prototype.checkDiff = function(tabletype) { 
    var curind, prevind;
    if(this.cur_buffer_index==0) {
        curind = 0;
        prevind = 1;
    } else {
        curind = 1;
        prevind = 0;
    }
    var curtbl,prevtbl;
    switch(tabletype) {
    case GTT_INDEX:
        curtbl = this.index_table[curind];
        prevtbl = this.index_table[prevind];
        break;
    case GTT_FLIP:
        curtbl = this.flip_table[curind];
        prevtbl = this.flip_table[prevind];
        break;
    case GTT_TEXOFS:
        curtbl = this.texofs_table[curind];
        prevtbl = this.texofs_table[prevind];
        break;
    case GTT_COLOR:
        curtbl = this.color_table[curind];
        prevtbl = this.color_table[prevind];            
        break;
    }

    var compsz;
    switch(tabletype){
    case GTT_INDEX:
        compsz = this.target_grid.getCellNum() * 4;//uint32
        break;
    case GTT_FLIP:
        compsz = this.target_grid.getCellNum() * 1; //uint8_t
        break;
    case GTT_TEXOFS:
        compsz = this.target_grid.getCellNum() * 8; // vec2
        break;
    case GTT_COLOR:
        compsz = this.target_grid.getCellNum() * 4; // color
        break;   
    }
    // return true if differ
    if(prevtbl.length==curtbl.length) {
        if(prevtbl.every( function(val,i) { return val==curtbl[i]; } ) ) {
            return false; 
        }
    }
    return true;
}

function makeInt32Buffer(ia,n) {
    var outb=new Buffer(n*4);
    for(var i=0;i<n;i++) outb.writeInt32LE(ia[i],i*4);
    return outb;
}
function makeUInt8Buffer(u8a,n) {
    var outb=new Buffer(n);
    for(var i=0;i<n;i++) outb.writeUInt8(u8a[i],i);
    return outb;
}
function makeVec2Buffer(va,n) {
    var outb=new Buffer(n*8);
    for(var i=0;i<n;i++) {
        outb.writeFloatLE(va[i].x,i*8);
        outb.writeFloatLE(va[i].y,i*8+4);
    }
    return outb;    
}
function makeColorBuffer(ca,n) {
    var outb=new Buffer(n*4);
    for(var i=0;i<n;i++) {
        var bin=ca[i].toRGBA();
        outb.writeUInt8(bin[0],i*4);
        outb.writeUInt8(bin[1],i*4+1);
        outb.writeUInt8(bin[2],i*4+2);
        outb.writeUInt8(bin[3],i*4+3);        
    }
    return outb;        
}
TrackerGrid.prototype.broadcastDiff = function(ownerprop,force) {
    var have_index_diff = this.checkDiff( GTT_INDEX );
    var have_flip_diff = this.checkDiff( GTT_FLIP );
    var have_texofs_diff = this.checkDiff( GTT_TEXOFS );
    var have_color_diff = this.checkDiff( GTT_COLOR );
    var have_any_diff = ( have_index_diff || have_flip_diff || have_texofs_diff || have_color_diff );

//    console.log("grid bc:",have_index_diff, have_flip_diff, have_texofs_diff, have_color_diff );
    
    if( force || have_any_diff ) {        
        this.broadcastGridConfs(ownerprop);
    }
    var cn=this.target_grid.getCellNum();
    if(have_index_diff||force) {
        this.parent_rh.broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT, this.target_grid.id,
                                             makeInt32Buffer(this.index_table[this.cur_buffer_index],cn));
    }
    if(have_flip_diff||force) {
        this.parent_rh.broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_FLIP_SNAPSHOT, this.target_grid.id,
                                             makeUInt8Buffer(this.flip_table[this.cur_buffer_index],cn));
    }
    if(have_texofs_diff||force) {
        this.parent_rh.broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_TEXOFS_SNAPSHOT, this.target_grid.id,
                                             makeVec2Buffer(this.texofs_table[this.cur_buffer_index],cn));
    }
    if(have_color_diff||force) {
        this.parent_rh.broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT, this.target_grid.id,
                                             makeColorBuffer(this.color_table[this.cur_buffer_index],cn));
    }
}

TrackerGrid.prototype.broadcastGridConfs = function(ownerprop) {
    this.parent_rh.broadcastUS1UI3( PACKETTYPE_S2C_GRID_CREATE, this.target_grid.id, this.target_grid.width, this.target_grid.height );
    var dk_id = 0;
    if(this.target_grid.deck) dk_id = this.target_grid.deck.id; else if(ownerprop.deck) dk_id = ownerprop.deck.id;
    if(dk_id) this.parent_rh.broadcastUS1UI2( PACKETTYPE_S2C_GRID_DECK, this.target_grid.id, dk_id );
    this.parent_rh.broadcastUS1UI2( PACKETTYPE_S2C_GRID_PROP2D, this.target_grid.id, ownerprop.id );    
}


////////////////////////

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

    this.prereq_decks=new Array();
}
RemoteHead.prototype.addClient = function(cl) {
    this.clients.push(cl);
}
RemoteHead.prototype.delClient = function(cl) {
    for(var i=0;i<this.clients.length;i++) {
        if(this.clients[i].id==cl.id) {
            this.clients.splice(i,1);
            console.log("delClient: deleted client id:",cl.id, this.clients.length);
            break;
        }
    }
}
RemoteHead.prototype.scanSendAllPrerequisites = function(s) {
    if( this.window_width==0 || this.window_height==0) {
        console.log( "remotehead: window size not set?");
    }
    console.log("scanSendAllPrerequisites: layers:", this.target_moyai.layers);
    // Viewport , Camera
    for(var i in this.target_moyai.layers) {
        var l=this.target_moyai.layers[i];
        console.log("scanSendAllPrerequisites: layer:",l);
        if(l.viewport) {
            sendViewportCreateScale(s,l.viewport);            
        }
        if(l.camera) {
            sendCameraCreateLoc(s,l.camera);
        }
    }
    
    // Layers(Groups) don't need scanning props
    for(var i in this.target_moyai.layers) {
        var l=this.target_moyai.layers[i];
        sendLayerSetup(s,l);
    }

    var decks={}, texs={}, imgs={};
    
    for(var i in this.prereq_decks) {
        var dk=this.prereq_decks[i];
//        console.log("RPERPEPREOPREOPROE:",dk);
        decks[dk.id]=dk;
        if(dk.moyai_tex) {
//            console.log("scanSendAllPrerequisites: deck", dk.id, "has tex:", dk.moyai_tex );
            texs[dk.moyai_tex.id]=dk.moyai_tex;
            if(dk.moyai_tex.image) {
//                console.log("scanSendAllPrerequisites: tex", dk.moyai_tex.id, "has img:", dk.moyai_tex.image );
                imgs[dk.moyai_tex.image.id]=dk.moyai_tex.image;
            }
        }
    }

    // scan all props
    for(var i in this.target_moyai.layers) {
        var l=this.target_moyai.layers[i];
        for(var j in l.props) {
            var p=l.props[j];
            if(p.deck) {
                decks[p.deck.id]=p.deck;
                if(p.deck.moyai_tex) {
                    texs[p.deck.moyai_tex.id]=p.deck.moyai_tex;
                    if(p.deck.moyai_tex.image) imgs[p.deck.moyai_tex.image.id]=p.deck.moyai_tex.image;
                }
            }
            if(p.grids) {
                for(var gi in p.grids) {
                    var g=p.grids[gi];
                    if(g.deck) {
                        decks[g.deck.id]=g.deck;
                        if(g.deck.moyai_tex) {
                            texs[g.deck.moyai_tex.id]=g.deck.moyai_tex;
                            if(g.deck.moyai_tex.image) imgs[g.deck.moyai_tex.image.id]=g.deck.moyai_tex.image;
                        }
                    }
                }
            }
        }
        
    }

    // image files
    for(var i in imgs) {
        var img=imgs[i];
        if(img.last_load_file_path) {
            console.log("sending file path:", img.last_load_file_path);
            sendFile(s,img.last_load_file_path);
        }
    }
    for(var i in imgs) {
        sendImageSetup(s,imgs[i]);
    }
    for(var i in texs) {
        sendTextureCreateWithImage(s,texs[i]);
    }
    for(var i in decks) {
        sendDeckSetup(s,decks[i]);
    }

    // TODO: send sounds
}
RemoteHead.prototype.scanSendAllProp2DSnapshots = function(s) {
    for(var i in this.target_moyai.layers) {
        var l=this.target_moyai.layers[i];
        for(var j in l.props) {
            var p=l.props[j];
            // prop body
            if(!p.tracker) {
                p.tracker = new Tracker2D(this,p);
                p.tracker.scanProp2D(null);
            }
            p.tracker.broadcastDiff(true);
            // grid
            for(var gi in p.grids) {
                var g=p.grids[gi];
                if(!g.tracker) {
                    g.tracker = new TrackerGrid(this,g);
                    g.tracker.scanGrid();                    
                }
                g.tracker.broadcastDiff(p, true );
            }
/*            
                // prims
                if(p->prim_drawer) {
                    if( !p->prim_drawer->tracker) p->prim_drawer->tracker = new TrackerPrimDrawer(this,p->prim_drawer);
                    p->prim_drawer->tracker->scanPrimDrawer();
                    p->prim_drawer->tracker->broadcastDiff(p, true );
                }
                // children
                for(int i=0;i<p->children_num;i++) {
                    Prop2D *chp = p->children[i];
                    if(!chp->tracker) {
                        chp->tracker = new Tracker2D(this,chp);
                        chp->tracker->scanProp2D(p);
                    }
                    chp->tracker->broadcastDiff(true);
                }
                */                
        }
    }    
}
RemoteHead.prototype.addPrerequisiteDeck = function(dk) {
    this.prereq_decks.push(dk);
}

RemoteHead.prototype.track2D = function() {
    //    if(enable_timestamp) broadcastTimestamp();
    for(var i in this.target_moyai.layers) {
        var layer = this.target_moyai.layers[i];
//        if(layer.hasDynamicCameras()) {
//            layer.onTrackDynamicCameras();
        //        } else
        if(layer.camera) layer.camera.onTrack(this);
//        if(layer->hasDynamicViewports()) {
//            layer->onTrackDynamicViewports();
        if(layer.viewport) layer.viewport.onTrack(this);
        for(var j in layer.props) {
            var p=layer.props[j];
            p.onTrack(this,null);
        }        
    }
}

RemoteHead.prototype.startServer = function(port) {
    var this_rh=this;
    this.server = net.createServer( function(conn) {
        console.log("rh:  newconnection");
        var ncl=new Client(conn);
        conn.client=ncl;
        this_rh.addClient(ncl);
        sendWindowSize(ncl,this_rh.window_width, this_rh.window_height);
        this_rh.scanSendAllPrerequisites(ncl);
        this_rh.scanSendAllProp2DSnapshots(ncl);
        if(this_rh.on_connect_cb) this_rh.on_connect_cb(this_rh,ncl);

        conn.on("data", function(data) {
            console.log("received data:",data);
        });
        conn.on("error", function(e) {
            console.log("socket error on client:",conn.client.id);
            this_rh.delClient(conn.client);
        });
        
    });
    this.server.listen(22222);
}
RemoteHead.prototype.setWindowSize = function(w,h) { this.window_width = w; this.window_height = h; }
RemoteHead.prototype.setOnConnectCallback = function(cb) { this.on_connect_cb=cb;}
RemoteHead.prototype.setOnDisconnectCallback = function(cb) { this.on_disconnect_cb = cb; }
RemoteHead.prototype.setOnKeyboardCallback = function(cb) { this.on_keyboard_cb = cb; }
RemoteHead.prototype.setOnMouseButtonCallback = function(cb) {this.on_mouse_button_cb = cb; }
RemoteHead.prototype.setOnMouseCursorCallback = function(cb) { this.on_mouse_cursor_cb = cb; }
RemoteHead.prototype.heartbeat = function(dt_sec) {
    this.track2D();
    for(var i in this.clients) {
        this.clients[i].flushSendbuf(1024*32);
    }
};
RemoteHead.prototype.flushBufferToNetwork = null; //(double dt);
//    void notifyProp2DDeleted( Prop2D *prop_deleted );
//    void notifyGridDeleted( Grid *grid_deleted );
//    void notifyChildCleared( Prop2D *owner_prop, Prop2D *child_prop );
//    void setTargetSoundSystem(SoundSystem*ss) { target_soundsystem = ss; }
RemoteHead.prototype.setTargetMoyai = function(moy) { this.target_moyai=moy; }
//    void notifySoundPlay( Sound *snd, float vol );
//    void notifySoundStop( Sound *snd );
    
RemoteHead.prototype.broadcastUS1Bytes = function(us,buf) {
    for(var i in this.clients) sendUS1Bytes(this.clients[i],us,buf);
}
RemoteHead.prototype.broadcastUS1UI1Bytes = function(usval,uival,buf) {
    for(var i in this.clients) sendUS1UI1Bytes(this.clients[i],usval,uival,buf);
}
//    void broadcastUS1UI1( uint16_t usval, uint32_t uival );
RemoteHead.prototype.broadcastUS1UI2 = function(usval,ui0,ui1) {
    for(var i in this.clients) sendUS1UI2(this.clients[i], usval,ui0,ui1);    
}
RemoteHead.prototype.broadcastUS1UI3 = function(usval,ui0,ui1,ui2) {
    for(var i in this.clients) sendUS1UI3(this.clients[i], usval,ui0,ui1,ui2);
}
//    void broadcastUS1UI4( uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2, uint32_t ui3 );    
//    void broadcastUS1UI1Wstr( uint16_t usval, uint32_t uival, wchar_t *wstr, int wstr_num_letters );
//    void broadcastUS1UI1F1( uint16_t usval, uint32_t uival, float f0 );
RemoteHead.prototype.broadcastUS1UI1F2 = function(usval,uival,f0,f1) {
    for(var i in this.clients) sendUS1UI1F2( this.clients[i],usval,uival,f0,f1);
}
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

global.MoyaiClient=MoyaiClient;
global.Texture=Texture;
global.Prop2D=Prop2D;
global.RemoteHead=RemoteHead;
global.Grid=Grid;