// moyai node.js server module
var fs=require("fs");

function Moyai() {
    this.layers=[];
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


global.pngParse = require("pngparse-sync");

global.Moyai=Moyai;
global.Texture=Texture;
global.Prop2D=Prop2D;