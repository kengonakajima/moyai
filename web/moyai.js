// moyai js port

function range(a,b) {
    var small=a,big=b;
    if(big<small) {
        var tmp = big;
        big=small;
        small=tmp;
    }
    return (small + (big-small)*Math.random());
}

function Vec2(x,y) {                                                                                                  
    this.x = x;                                                                                                       
    this.y = y;                                                                                                       
}

                                                                                                                      
// 0 ~ 1                                                                                                              
function Color(r,g,b,a) {                                                                                             
    this.r = r;                                                                                                       
    this.g = g;                                                                                                       
    this.b = b;                                                                                                       
    this.a = a;                                                                                                       
}

///////////////

function MoyaiClient(w,h,pixratio){
    this.scene = new THREE.Scene();
    this.renderer = new THREE.WebGLRenderer();
    this.renderer.setPixelRatio( pixratio);
    this.renderer.setSize(w,h);
    this.renderer.autoClear = false;

    this.layers=[];
    
}
MoyaiClient.prototype.resize = function(w,h) {
    this.renderer.setSize(w,h);
}
MoyaiClient.prototype.render = function() {
    console.log("render");
}
MoyaiClient.prototype.insertLayer = function(l) {
    this.layers.push(l);
}

///////////////////

Viewport.prototype.id_gen=1;
function Viewport() {
    this.id = this.id_gen++;
}
Viewport.prototype.setSize = function(sw,sh) {
    this.screen_width = sw;
    this.screen_height = sh;
}
Viewport.prototype.setScale2D = function(sx,sy) {
    this.scl = new Vec2(sx,sy);    
}
Viewport.prototype.getMinMax = function() {
    return [ new Vec2(-this.scl.x/2,-this.scl.y/2), new Vec2(this.scl.x/2,this.scl.y/2) ];
}

////////////////////
Camera.prototype.id_gen=1;
function Camera() {
    this.id = this.id_gen++;
    this.loc = new Vec2(0,0);
}
Camera.prototype.setLoc = function(x,y) { this.loc.x=x; this.loc.y=y; }

////////////////////
Layer.prototype.id_gen = 1;
function Layer() {
    this.id = this.id_gen++;
    this.props=[];
}
Layer.prototype.setViewport = function(vp) { this.viewport = vp; }
Layer.prototype.setCamera = function(cam) { this.camera = cam; }
Layer.prototype.insertProp = function(p) { this.props.push(p); }

/////////////////////
Image.prototype.id_gen = 1;
function Image() {
    this.id = this.id_gen++;    
}
Image.prototype.loadPNGMem = function(u8adata) {
    var png = new PNG(u8adata);
    console.log("png info:", png.width, png.height );
    this.width = png.width;
    this.height = png.height;
    png.decode( function(pixels) {
        console.log("png pixels:",pixels.length);
        this.data = pixels;
    });
    
}

//////////////////////////
Texture.prototype.id_gen = 1;
function Texture() {
    this.id = this.id_gen++;
}
Texture.prototype.loadPNGMem = function(u8adata) {
    this.image = new Image();
    this.image.loadPNGMem(u8adata);
}


///////////////////
TileDeck.prototype.id_gen = 1;
function TileDeck() {
    this.id = this.id_gen++;
}
TileDeck.prototype.setSize = function(sprw,sprh,cellw,cellh) {
    this.tile_width = sprw;
    this.tile_height = sprh;
    this.cell_width = cellw;
    this.cell_height = cellh;
}
TileDeck.prototype.setTexture = function(tex) {
    this.tex = tex;
}


/*
    virtual float getUperCell() { return (float) cell_width / (float) image_width; }
    virtual float getVperCell() { return (float) cell_height / (float) image_height; }    
	virtual void getUVFromIndex( int ind, float *u0, float *v0, float *u1, float *v1, float uofs, float vofs, float eps ) {
        assert( image_width > 0 && image_height > 0 );
		float uunit = (float) cell_width / (float) image_width;
		float vunit = (float) cell_height / (float) image_height;
		int start_x = cell_width * (int)( ind % tile_width );
		int start_y = cell_height * (int)( ind / tile_width );

		*u0 = (float) start_x / (float) image_width + eps + uofs * uunit; 
		*v0 = (float) start_y / (float) image_height + eps + vofs * vunit; 
		*u1 = *u0 + uunit - eps*2;  // *2 because adding eps once for u0 and v0
		*v1 = *v0 + vunit - eps*2;
	}
	// (x0,y0)-(x1,y1) : (0,0)-(16,16) for 16x16 sprite
	inline void getPixelPosition( int ind, int *x0, int *y0, int *x1, int *y1 ) {
		int start_x = cell_width * (int)( ind % tile_width );
		int start_y = cell_height * (int)( ind / tile_width );
		*x0 = start_x;
		*y0 = start_y;
		*x1 = start_x + cell_width;
		*y1 = start_y + cell_height;
	}
    */


///////

var PRIMTYPE_NONE = 0;
var PRIMTYPE_LINE = 1;
var PRIMTYPE_RECTANGLE = 2;

Prim.prototype.id_gen=1;
function Prim(t,a,b,col,lw) {
    this.id=this.id_gen++;
    this.type = t;
    this.a=a;
    this.b=b;
    this.color=col;
    if(!lw) lw=1;
    this.line_width=lw;    
}

function PrimDrawer() {
    this.prims=[];
}
PrimDrawer.prototype.addLine = function(a,b,col,w) {
    this.prims.push( new Prim(PRIMTYPE_LINE,a,b,col,w));
}
PrimDrawer.prototype.addRect = function(a,b,col,w) {
    this.prims.push( new Prim(PRIMTYPE_RECTANGLE,a,b,col,w));    
}

//////////////////

Prop2D.prototype.id_gen=1;
function Prop2D() {
    this.id=this.id_gen++;
    this.index = 0;
    this.scl = new Vec2(16,16);
    this.loc = new Vec2(0,0);
    this.rot = 0;
    this.deck = null;
    this.uvrot = false;
    this.color = new Color(1,1,1,1);
    this.prim_drawer = null;
    this.grids=null;
    this.visible=true;
}
Prop2D.prototype.setDeck = function(dk) { this.deck = dk; }
Prop2D.prototype.setIndex = function(ind) { this.index = ind; }
Prop2D.prototype.setScl = function(x,y) { this.scl.x=x; this.scl.y=y; }
Prop2D.prototype.setLoc = function(x,y) { this.loc.x=x; this.loc.y=y; }
Prop2D.prototype.setRot = function(r) { this.rot=r; }
Prop2D.prototype.setUVRot = function(flg) { this.uvrot=flg;}
Prop2D.prototype.setColor = function(r,g,b,a) { this.color = new Color(r,g,b,a); }
Prop2D.prototype.addLine = function(p0,p1,col,w) {
    if(!this.prim_drawer) this.prim_drawer = new PrimDrawer();
    this.prim_drawer.addLine(p0,p1,col,w);
}
Prop2D.prototype.addGrid = function(g) {
    if(!this.grids) this.grids=[];
    this.grids.push(g);
}

////////////////////////////

Grid.prototype.id_gen=1;
function Grid(w,h) {
    this.id=this.id_gen++;
    this.width=w;
    this.height=h;
    this.index_table=null;
    this.xflip_table=null;
    this.yflip_table=null;
    this.texofs_table=null;
    this.rot_table=null;
    this.color_table=null;
    this.deck=null;
    this.fragment_shader=null;
    this.visible=true;
    this.enfat_epsilon=0;
    this.parent_prop=null;
    this.mesh=null;
}
Grid.prototype.setDeck =function(dk) { this.deck=dk;}
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
}
Grid.prototype.get  =function(x,y) {
    if(!this.index_table) return GRID_NOT_USED;
    return this.index_table[ this.index(x,y) ];
}
Grid.prototype.bulkSetIndex = function(inds) {
    if(!this.index_table) this.index_table=[];
    for(var i in this.index_table) this.index_table[i] = inds[i];
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
    return this.texofs_table[this.index(x,y)];
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
    return this.color_table[this.index(x,y)];
}
Grid.prototype.setFragmentShader = function(s) { this.fragment_shader = s; }
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
    if(this.color_table) {
        for(var y=0;y<this.height;y++) {
            for(var x=0;x<this.width;x++) {
                this.color_table[this.index(x,y)] = new Color(c.r,c.g,c.b,c.a);
            }
        }
    }
}
