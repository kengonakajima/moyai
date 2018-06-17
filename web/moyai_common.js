
DIR4_NONE=-1;
DIR4_UP=0;
DIR4_RIGHT=1;
DIR4_DOWN=2;
DIR4_LEFT=3;


randomDir = function() {
    return irange(DIR4_UP,DIR4_LEFT+1);
}
randomLRDir = function() {
    return [DIR4_LEFT,DIR4_RIGHT][irange(0,2)];
}
reverseDir = function(d){
    switch(d){
    case DIR4_UP: return DIR4_DOWN;
    case DIR4_DOWN: return DIR4_UP;
    case DIR4_RIGHT: return DIR4_LEFT;
    case DIR4_LEFT: return DIR4_RIGHT;
    default:
        return DIR4_NONE;
    }
}
turnDir = function(origdir,turndir) {
    switch(turndir) {
    case DIR4_LEFT: return leftDir(origdir);
    case DIR4_RIGHT: return rightDir(origdir);
    case DIR4_DOWN: return reverseDir(origdir);
    case DIR4_UP: return origdir;
    default:
        assert(false,"invalid dir");
    }
}
rightDir = function(d) {
    switch(d){
    case DIR4_UP: return DIR4_RIGHT; 
    case DIR4_DOWN: return DIR4_LEFT;
    case DIR4_RIGHT: return DIR4_DOWN;
    case DIR4_LEFT: return DIR4_UP;
    default:
        return DIR4_NONE;
    }
}
leftDir = function(d) {
    switch(d){
    case DIR4_UP: return DIR4_LEFT;
    case DIR4_DOWN: return DIR4_RIGHT;
    case DIR4_RIGHT: return DIR4_UP;
    case DIR4_LEFT: return DIR4_DOWN;
    default:
        return DIR4_NONE;
    }    
}

// 4方向のみ
dxdyToDir = function(dx,dy){
    if(dx>0&&dy==0){
        return DIR4_RIGHT;
    } else if(dx<0&&dy==0){
        return DIR4_LEFT;
    } else if(dy>0&&dx==0){
        return DIR4_UP;
    }else if(dy<0&&dx==0){
        return DIR4_DOWN;
    } else {
        return DIR4_NONE;
    }
}
clockDir = function(d) {
    switch(d) {
    case DIR4_NONE: return DIR4_NONE;
    case DIR4_UP: return DIR4_RIGHT;
    case DIR4_RIGHT: return DIR4_DOWN;
    case DIR4_DOWN: return DIR4_LEFT;
    case DIR4_LEFT: return DIR4_UP;
    default: console.assert( "clockDir: invalid direction:", d);
    }
    return DIR4_NONE;
}

dirToDXDY = function(d) {
    switch(d){
    case DIR4_NONE: return null;
    case DIR4_RIGHT: return {x:1,y:0};
    case DIR4_LEFT: return {x:-1,y:0};
    case DIR4_UP: return {x:0,y:1};
    case DIR4_DOWN: return {x:0,y:-1};
    default:
        console.assert("dirToDXDY: invalid direction:",d);
        return null;
    }
}




////

var moyai_rng_w = 123456789;
var moyai_rng_z = 987654321;
var moyai_rng_mask = 0xffffffff;

// Takes any integer
function moyai_rng_seed(i) {
    moyai_rng_w = i;
    moyai_rng_z = 987654321;
}

// Returns number between 0 (inclusive) and 1.0 (exclusive),
// just like Math.random().
function moyai_rng_random()
{
    moyai_rng_z = (36969 * (moyai_rng_z & 65535) + (moyai_rng_z >> 16)) & moyai_rng_mask;
    moyai_rng_w = (18000 * (moyai_rng_w & 65535) + (moyai_rng_w >> 16)) & moyai_rng_mask;
    var result = ((moyai_rng_z << 16) + moyai_rng_w) & moyai_rng_mask;
    result /= 4294967296;
    return result + 0.5;
}

irange = function(a,b) {
    return Math.floor(range(a,b));
}
range = function(a,b) {
    var small=a,big=b;
    if(big<small) {
        var tmp = big;
        big=small;
        small=tmp;
    }
    var out=(small + (big-small)*moyai_rng_random());
    if(out==b)return a; // in very rare case, out==b
    return out;
}
choose = function(ary) {
    return ary[ irange(0,ary.length) ];
}

sign = function(f) {
    if(f>0) return 1; else if(f<0)return -1; else return 0;
}
now = function() {
    var t = new Date().getTime();
    return t / 1000.0;
}
hrnow = function() {
    var t=process.hrtime();
    return t[0] + t[1]/1000000000.0;
}
lengthf = function(x0,y0,x1,y1) {
    return Math.sqrt( (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) );
}



//////////////

Vec2 = function(x,y) {
    this.x = x;
    this.y = y;
}
Vec2.prototype.setWith2args = function(x,y) {
    if(y==undefined) {
        if( (typeof x) == "number" ) {
            this.x=x;
            this.y=x;
        } else if( x.__proto__ == Vec2.prototype ) {
            this.x=x.x;
            this.y=x.y;            
        }
    } else {
        this.x=x;
        this.y=y;
    }
}
Vec2.prototype.normalize = function(l) {
    if(l==undefined)l=1;    
    var ll = Math.sqrt(this.x*this.x+this.y*this.y);
    if(ll==0) return new Vec2(0,0);
    return new Vec2(this.x/ll*l,this.y/ll*l);
}
Vec2.prototype.add = function(to_add) {
    return new Vec2( this.x + to_add.x, this.y + to_add.y );
}
Vec2.prototype.mul = function(to_mul) {
    return new Vec2( this.x * to_mul, this.y * to_mul );
}
Vec2.prototype.randomize = function(r) {
    if(r==undefined)r=1;    
    return new Vec2( this.x - r + range(0,r*2), this.y - r + range(0,r*2) );
}
Vec2.prototype.isEqual = function(v) {
    return (v.x!=this.x) || (v.y!=this.y);
}

Vec3 = function(x,y,z) {
    this.x = x;
    this.y = y;
    this.z = z;
}
Vec3.prototype.setWith3args = function(x,y,z) {
    if(x.constructor==Vec3){
        this.x=x.x; this.y=x.y; this.z=x.z;
    } else {
        this.x=x; this.y=y;this.z=z;
    }
}
Vec3.prototype.modify = function(v) {
    this.x += v.x;
    this.y += v.y;
    this.z += v.z;
}
Vec3.prototype.add = function(to_add) {
    return new Vec3( this.x + to_add.x, this.y + to_add.y, this.z + to_add.z );
}
Vec3.prototype.mul = function(m) {
    return new Vec3( this.x*m, this.y*m, this.z*m );
}
Vec3.prototype.set = function(x,y,z) {
    this.setWith3args(x,y,z);
    return this;
}
Vec3.prototype.randomize = function(r) {
    if(r==undefined)r=1;
    return new Vec3( this.x - r + range(0,r*2), this.y - r + range(0,r*2), this.z - r + range(0,r*2) );
}
Vec3.prototype.to_i = function() {
    return new Vec3( to_i(this.x), to_i(this.y), to_i(this.z) );
}
Vec3.prototype.normalize = function(l) {
    if(l==undefined)l=1;
    var ll = Math.sqrt(this.x*this.x+this.y*this.y+this.z*this.z);
    if(ll==0) return new Vec3(0,0,0);
    return new Vec3(this.x/ll*l,this.y/ll*l,this.z/ll*l);
}
Vec3.prototype.to = function(target) {
    return new Vec3(target.x-this.x, target.y-this.y, target.z-this.z);
}
Vec3.prototype.toTHREEVector3 = function() {
    return new THREE.Vector3(this.x,this.y,this.z);
}
Vec3.prototype.lengthf = function() {
    return Math.sqrt( this.x*this.x + this.y*this.y + this.z*this.z );
}
// 0 ~ 1
Color = function(r,g,b,a) {
    if(g==undefined || g==null) {
        var code = r; // color code
        this.r = ((code & 0xff0000)>>16)/255;
        this.g = ((code & 0xff00)>>8)/255;
        this.b = (code & 0xff)/255;
        this.a = 1.0;        
    } else {
        this.r = r;
        this.g = g;
        this.b = b;
        this.a = a;
    }
}
Color.prototype.toRGBA = function() {
    return [ Math.floor(this.r*255), Math.floor(this.g*255), Math.floor(this.b*255), Math.floor(this.a*255) ];
}
Color.prototype.fromRGBA = function(r,g,b,a) {
    this.r = r/255.0;
    this.g = g/255.0;
    this.b = b/255.0;    
    this.a = a/255.0;
}
    
Color.prototype.toCode = function() {
    return ( Math.floor(this.r * 255) << 16 ) + ( Math.floor(this.g * 255) << 8 ) + Math.floor(this.b * 255);
}
Color.prototype.toTHREEColor = function() {
    return new THREE.Color(this.toCode());
}
Color.prototype.equals = function(r,g,b,a) {
    return (this.r==r && this.g==g && this.b==b && this.a==a);
}
Color.prototype.adjust = function(v) {
    var rr = this.r*v;
    var gg = this.g*v;
    var bb = this.b*v;
    if(rr>1)rr=1;
    if(gg>1)gg=1;
    if(bb>1)bb=1;
    return new Color(rr,gg,bb,this.a);
}

///////////////////

Viewport.prototype.id_gen=1;
function Viewport() {
    this.id = this.__proto__.id_gen++;
    this.screen_width = null;
    this.screen_height = null;
    this.near_clip = null;
    this.far_clip = null;
    this.dimension = null;
}
Viewport.prototype.setSize = function(sw,sh) {
    this.screen_width = sw;
    this.screen_height = sh;
}
Viewport.prototype.setScale2D = function(sx,sy) {
    this.scl = new Vec2(sx,sy);
    this.dimension = 2;
}
Viewport.prototype.setClip3D = function(near,far) {
    this.near_clip = near;
    this.far_clip = far;
    this.dimension = 3;
}
Viewport.prototype.getMinMax = function() {
    return [ new Vec2(-this.scl.x/2,-this.scl.y/2), new Vec2(this.scl.x/2,this.scl.y/2) ];
}
Viewport.prototype.getRelativeScale = function() {
    return new Vec2(this.screen_width/this.scl.x,this.screen_height/this.scl.y);
}

////////////////////
Camera.prototype.id_gen=1;
function Camera() {
    this.id = this.__proto__.id_gen++;
    this.loc = null; // vec2 or vec3
    this.dimension=null;    
}
Camera.prototype.setLoc = function(x,y,z) {
    if(x.constructor==Vec3){ this.loc=x; this.dimension=3; }
    else if(x.constructor==Vec2){ this.loc=x; this.dimension=2; }
    else if(z==undefined) {
        this.loc=new Vec2(x,y);
        this.dimension=2;
    } else {
        this.loc = new Vec3(x,y,z);
        this.dimension=3;
    }
}
Camera.prototype.setLookAt = function(at,up) {
	this.look_at = at;
	this.look_up = up;
}


////////////////////
Layer.prototype.id_gen = 1;
function Layer() {
    this.id = this.__proto__.id_gen++;
    this.props=[];
    this.priority=null;// update when insert to moyai
    this.camera=null;
    this.viewport=null;
    this.light=null;
}
Layer.prototype.setViewport = function(vp) { this.viewport = vp; }
Layer.prototype.setCamera = function(cam) { this.camera = cam; }
Layer.prototype.setLight = function(lgt) { this.light = lgt; }
Layer.prototype.setAmbientLight = function(lgt) { this.ambient_light = lgt; }
Layer.prototype.insertProp = function(p) {
    if(p.priority==null) {
        var highp = this.getHighestPriority();
        p.priority = highp+1;
        p.parent_layer=this;
    }
    this.props.push(p);
}
Layer.prototype.hasProp = function(p) {
    for(var i=0;i<this.props.length;i++) {
        if(this.props[i].id==p.id) return true;
    }
    return false;
}
Layer.prototype.delProp = function(p) {
    for(var i=0;i<this.props.length;i++) {
        if(this.props[i].id==p.id) {
            this.props.splice(i,1);
            return true;
        }
    }
    return false;
}
Layer.prototype.pollAllProps = function(dt) {
    var keep=[];
    for(var i=0;i<this.props.length;i++) {
        var prop = this.props[i];
        var to_keep = prop.basePoll(dt);
        if(to_keep) {
            keep.push(prop);
        } else {
            if(prop.onDelete) prop.onDelete();
        }
    }
    this.props = keep;
    return this.props.length;
}
Layer.prototype.getHighestPriority = function() {
    var highp=0;
    for(var i=0;i<this.props.length;i++) {
        if(this.props[i].priority>highp) highp = this.props[i].priority;
    }
    return highp;    
}
Layer.prototype.getLowesetPriority = function() {
    var lowp=0;
    for(var i=0;i<this.props.length;i++) {
        if(this.props[i].priority<lowp) lowp = this.props[i].priority;
    }
    return lowp;
}
Layer.prototype.getPropById = function(id) {
    for(var i=0;i<this.props.length;i++) {
        if( this.props[i].id == id ) return this.props[i];
    }
    return null;
}
Layer.prototype.findByKey = function(keyname,val) {
    for(var i=0;i<this.props.length;i++) {
        var p = this.props[i];
        if( p[keyname] == val ) return p;
    }
    return null;
}
Layer.prototype.scan = function(cb) {
    for(var i=0;i<this.props.length;i++) {
        cb(this.props[i]);
    }
}
Layer.prototype.clean = function() {
    this.props=[];
}

/////////////////////
MoyaiImage.prototype.id_gen = 1;
function MoyaiImage() {
    this.id = this.__proto__.id_gen++;
    this.data = null;
    this.png=null;
}
MoyaiImage.prototype.loadPNGMem = function(u8adata) {
    var b = new Buffer(u8adata);
    this.png = pngParse(b);
    this.width = this.png.width;
    this.height = this.png.height;
    this.data = this.png.data;
}
MoyaiImage.prototype.setSize = function(w,h) {
    this.width = w;
    this.height = h;
    if(!this.data) {
        this.data = new Uint8Array(w*h*4);
    }
}
MoyaiImage.prototype.getSize = function() {
    return new Vec2(this.width,this.height);
}
MoyaiImage.prototype.getPixelRaw = function(x,y) {
// int x, int y, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a ) {
    var out={};
    if(x>=0&&y>=0&&x<this.width&&y<this.height){
        var index = ( x + y * this.width ) * 4;
        out.r = this.data[index];
        out.g = this.data[index+1];
        out.b = this.data[index+2];
        out.a = this.data[index+3];
    }
    return out;
}
MoyaiImage.prototype.setPixelRaw = function(x,y,r,g,b,a) {
    if(x>=0&&y>=0&&x<this.width&&y<this.height){
        var index = ( x + y * this.width ) * 4;
        this.data[index] = r;
        this.data[index+1] = g;
        this.data[index+2] = b;
        this.data[index+3] = a;
    }    
}
MoyaiImage.prototype.setPixel = function(x,y,c) {
    var colary = c.toRGBA();
    var index = (x+y*this.width)*4;
    this.setPixelRaw(x,y,colary[0],colary[1],colary[2],colary[3]);
}
MoyaiImage.prototype.getBufferSize = function() { return this.width * this.height * 4; }
MoyaiImage.prototype.setAreaRaw = function(x0,y0,w,h, data_u8a, insz ) {
    var reqsize = w*h*4;
    if( insz < reqsize ) {
        console.log("image.prototype.setAreaRaw input size too small required:",reqsize, "got:",insz);
        return;
    }
    for(var dy=0;dy<h;dy++) {
        for(var dx=0;dx<w;dx++) {
            var x = x0+dx;
            var y = y0+dy;
            if(x<0||y<0||x>=this.width||y>=this.height)continue;            
            var out_index = ( x + y * this.width ) * 4;
            var in_index = ( dx + dy * w ) * 4;
            this.data[out_index] = data_u8a[in_index]; // r
            this.data[out_index+1] = data_u8a[in_index+1]; // g
            this.data[out_index+2] = data_u8a[in_index+2]; // b
            this.data[out_index+3] = data_u8a[in_index+3]; // a            
        }
    }      
}


///////////////////
TileDeck.prototype.id_gen = 1;
function TileDeck() {
    this.id = this.__proto__.id_gen++;
}
TileDeck.prototype.setSize = function(sprw,sprh,cellw,cellh) {
    this.tile_width = sprw;
    this.tile_height = sprh;
    this.cell_width = cellw;
    this.cell_height = cellh;
}
TileDeck.prototype.setTexture = function(tex) {
    this.moyai_tex = tex;
}
TileDeck.prototype.getUVFromIndex = function(ind,uofs,vofs,eps) {
	var uunit = this.cell_width / this.moyai_tex.image.width;
	var vunit = this.cell_height / this.moyai_tex.image.height;
	var start_x = this.cell_width * Math.floor( Math.floor(ind) % Math.floor(this.tile_width) );
	var start_y = this.cell_height * Math.floor( Math.floor(ind) / Math.floor(this.tile_width ) );
    var u0 = start_x / this.moyai_tex.image.width + eps + uofs * uunit;
    var v0 = start_y / this.moyai_tex.image.height + eps + vofs * vunit;
    var u1 = u0 + uunit - eps*2;  // *2 because adding eps once for u0 and v0
	var v1 = v0 + vunit - eps*2;
    return [u0,v0,u1,v1];
}
TileDeck.prototype.getUperCell = function() { return this.cell_width / this.moyai_tex.image.width; }
TileDeck.prototype.getVperCell = function() { return this.cell_height / this.moyai_tex.image.height; }    

TileDeck.prototype.getPixelsFromIndex = function(ind) {
	var start_x = this.cell_width * Math.floor( Math.floor(ind) % Math.floor(this.tile_width) );
	var start_y = this.cell_height * Math.floor( Math.floor(ind) / Math.floor(this.tile_width ) );
    var out=[];
    for(var y=start_y;y<start_y+this.cell_height;y++) {
        for(var x=start_x;x<start_x+this.cell_width;x++) {
            var di=x*4+y*(this.cell_width*this.tile_width)*4;
            for(var i=0;i<4;i++) {
                out.push(this.moyai_tex.image.data[di+i]);
            }
        }
    }
    return out;
}

///////////////////////////

function UVRect(u0,v0,u1,v1) {
    this.u0=u0;
    this.u1=u1;
    this.v0=v0;
    this.v1=v1;
};
PackDeck.prototype.id_gen = 1;
function PackDeck() {
    this.id = this.__proto__.id_gen++;
}
PackDeck.prototype.setTexture = function(tex) {
    this.moyai_tex=tex;
}
PackDeck.prototype.getUVFromIndex = function(ind,uofs,vofs,eps) {
    var uvrect = this.rects[ind];
    if(!uvrect) return [0,0,1,1];
    return uvrect;
}
PackDeck.prototype.getUperCell = function() {
    return this.rects[0].u1 - this.rects[0].u0;
}
PackDeck.prototype.getVperCell = function() {
    return this.rects[0].v1 - this.rects[0].v0;    
}
PackDeck.prototype.setRects = function(uvrects) {
    this.rects=uvrects;
}

////////////////////////

try {
    if(global) {
        // classes
        global.Vec2=Vec2;
        global.Color=Color;
        global.Viewport=Viewport;
        global.Camera=Camera;
        global.Image=MoyaiImage;
        global.TileDeck = TileDeck;
        global.Layer = Layer;

        // funcs
        global.to_i=Math.floor;
        global.lengthf=lengthf;
        global.now=now;
        global.choose=choose;
        global.sign=sign;
        global.range=range;
        global.irange=irange;
        global.dirToDXDY=dirToDXDY;
        global.clockDir=clockDir;
        global.dxdyToDir=dxdyToDir;
        global.leftDir=leftDir;
        global.rightDir=rightDir;
        global.reverseDir=reverseDir;
        global.randomDir=randomDir;
        global.randomLRDir=randomLRDir;
        global.turnDir=turnDir;        
        // constants
        global.DIR4_NONE=DIR4_NONE;
        global.DIR4_UP=DIR4_UP;
        global.DIR4_RIGHT=DIR4_RIGHT;
        global.DIR4_DOWN=DIR4_DOWN;
        global.DIR4_LEFT=DIR4_LEFT;
    }
} catch(e) {}