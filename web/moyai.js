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
    this.data = null;
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
Image.prototype.getSize = function() {
    return new Vec2(this.width,this.height);
}
Image.prototype.getPixelRaw = function(x,y) {
// int x, int y, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a ) {
    var out={};
    if(x>=0&&y>=0&&x<this.width&&y<this.height){
        var index = ( x + y * this.width ) * 4;
        out.r = buffer[index];
        out.g = buffer[index+1];
        out.b = buffer[index+2];
        out.a = buffer[index+3];
    }
    return out;
}
Image.prototype.setPixelRaw = function(x,y,r,g,b,a) {
    if(x>=0&&y>=0&&x<this.width&&y<this.height){
        var index = ( x + y * this.width ) * 4;
        buffer[index] = r;
        buffer[index+1] = g;
        buffer[index+2] = b;
        buffer[index+3] = a;
    }    
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
Texture.prototype.getSize = function() {
    return this.image.getSize();
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
    this.use_additive_blend = false;
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
Prop2D.prototype.setTexture = function(tex) {
    var td = new TileDeck();
    td.setTexture(tex);
    var sz = tex.getSize();
    td.setSize(1,1,sz.x,sz.y);
    this.setDeck(td);
    this.setIndex(0);    
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

/////////////////////
var FTFuncs={};
FTFuncs.monochrome	= FTModule.cwrap("monochrome", 'number', ['number']);
FTFuncs.load_font  = FTModule.cwrap("load_font", 'number', ['string','string','number']);
FTFuncs.load_mem_font_c = FTModule.cwrap("load_mem_font", "number", ['number','number','string','number']);
FTFuncs.find_font  = FTModule.cwrap("find_font", 'number', ['string']);
FTFuncs.get_bitmap = FTModule.cwrap("get_bitmap", 'number', ['number','number','number','number']);
FTFuncs.get_width = FTModule.cwrap("get_width", 'number', []);
FTFuncs.get_height = FTModule.cwrap("get_height", 'number', []);
FTFuncs.get_left = FTModule.cwrap("get_left", 'number', []);
FTFuncs.get_top = FTModule.cwrap("get_top", 'number', []);
FTFuncs.get_advance = FTModule.cwrap("get_advance", 'number', []);
FTFuncs.get_debug_code = FTModule.cwrap("get_debug_code", 'number', []);
FTFuncs.get_bitmap_opt_retcode = FTModule.cwrap("get_bitmap_opt_retcode","number",[]);

// freetype-gl's texture_atlas_t
function TextureAtlas(w,h,depth) {
    this.width = w;
    this.height = h;
    this.depth = depth;
    this.data = new Uint8Array(w*h*depth); 
    this.tex=null;
}
TextureAtlas.prototype.dump = function(w,h) {
    for(var y=0;y<h;y++) {
        var line="";
        for(var x=0;x<w;x++) {
            var val = this.data[x+y*this.width];
            if(val>128) line+="*"; else if(val>60) line+="."; else line+=" ";
        }
        console.log(y,line);
    }
    console.log(this.data);
}

Font.prototype.id_gen=1;
function Font() {
    this.id=this.id_gen++;
    this.font = null;
	this.atlas = null;
    this.charcode_table = [];
    this.glyphs={};
}
// 0:left-top 1:right-bottom
function Glyph(l,t,w,h,adv,u0,v0,u1,v1,charcode) {
    this.left = l;
    this.top = t;
    this.width = w;
    this.height = h;
    this.advance = adv;
    this.u0 = u0;
    this.v0 = v0;
    this.u1 = u1;
    this.v1 = v1;
    this.charcode = charcode;
    
//    console.log("glyph: ",u0,v0,u1,v1,charcode);
}
Font.prototype.loadFromMemTTF = function(u8a,codes,pxsz) {
    if(codes==null) codes = this.charcode_table; else this.charcode_table = codes;
    this.pixel_size = pxsz;

    this.atlas = new TextureAtlas(512,512,1);
    this.font_name = "font_"+this.id;
    
    // savefontして名前をID番号から自動で付けて loadfont する。
    var ret = FTModule.FS_createDataFile( "/", this.font_name, u8a, true,true,true);
    console.log("saving font:",this.font_name, "ret:",ret);
    
    ret = FTFuncs.load_font( this.font_name, this.font_name, 108);
    console.log("loading font ret:",ret);

    this.loadGlyphs(codes);
//    this.atlas.dump(80,80);
    return true;
}
Font.prototype.loadGlyphs = function(codes) {
    var horiz_num = parseInt(parseInt(this.atlas.width) / parseInt(this.pixel_size));
    var vert_num = parseInt(parseInt(this.atlas.height) / parseInt(this.pixel_size));
    var max_glyph_num = horiz_num * vert_num;
    console.log("max_glyph_num:",max_glyph_num, "horiz:",horiz_num, "vert:", vert_num, "pixel_size:",this.pixel_size );
    var font = FTFuncs.find_font(this.font_name);
    console.log("find_font result:",font);

    for(var i=0;i<codes.length;i++) {
        var ccode = codes.charCodeAt(i);
        var offset = FTFuncs.get_bitmap(font, ccode, this.pixel_size, this.pixel_size );
        if(offset==0) {
            if( FTFuncs.get_bitmap_opt_retcode()==1) {
                // space characers doesnt have buffer
                console.log("space char!:",ccode, FTFuncs.get_width(), FTFuncs.get_advance());
            } else {
                console.log("  get_bitmap failed for charcode:",ccode, "debug_code:", FTFuncs.get_debug_code(), "i:",i, "char:", codes[i] );
                continue;
            }            
        } 
        
        var w = FTFuncs.get_width();
        var h = FTFuncs.get_height();
        if(offset>0) var buf = FTModule.HEAPU8.subarray(offset,offset+w*h);
        var start_x = (i % horiz_num) * this.pixel_size;
        var start_y = parseInt(i / horiz_num) * this.pixel_size;
        var l = FTFuncs.get_left();
        var t = FTFuncs.get_top();        

        var pixelcnt=0;
        for(var ii=0;ii<w;ii++){
            for(var jj=0;jj<h;jj++) {
                var val = 0;
                if(offset>0) {
                    var val = buf[jj*w+ii]; // 0~255
                }
                if(val==0)continue; // 0 for no data
                pixelcnt++;
                var ind_in_atlas = (start_y+jj+this.pixel_size-t)*this.atlas.width + (start_x+ii+l);
                //                var final_val = Math.min( this.atlas.data[ind_in_atlas],val); 
                this.atlas.data[ind_in_atlas] = val;
                //                console.log("val:",val, "ii",ii,"jj",jj,"start:",start_x,start_y);
            }
        }
        /*
          (0,0)
          +-..--------------...-----+
          |                         |
          ..   (start_x,start_y)    |
          |                         |          
          |    A---------+          |
          |    | B  k    |          |
          |    | k k     |          |
          |    | kk      | h        |
          |    | k k     |          |  
          |    | k  C    |          |  
          |    +---------D          | 
          |         w               |  
          |                         |
          |                         |
          ...                       |
          |                         |
          +-------------------------+ (1,1)

          UVは左上が0
         */

        console.log("i:",i," charcode:",ccode," w,h:",w,h,"offset:",offset, "start:",start_x, start_y, "left:",l,"top:",t, "pixc:",pixelcnt , "firstind:", (start_y+0+this.pixel_size-t)*this.atlas.width+(start_x+0+l));
        
        var lt_u = start_x / this.atlas.width;
        var lt_v = start_y / this.atlas.height;
        var rb_u = (start_x+w) / this.atlas.width;
        var rb_v = (start_y+h) / this.atlas.height;
        var adv = FTFuncs.get_advance();
        this.glyphs[ccode] = new Glyph(l,t,w,h,adv,lt_u,lt_v,rb_u,rb_v,ccode);
    }
}


//////////////////
function TextBox() {
    Prop2D.call(this);
    this.font = null;
    this.scl = new Vec2(1,1);
    this.str = null;
}
TextBox.prototype = Object.create(Prop2D.prototype);
TextBox.prototype.constructor = TextBox;
TextBox.prototype.setFont = function(fnt) { this.font = fnt; }
TextBox.prototype.setString = function(s) { this.str = s; }
TextBox.prototype.setScl = function(scl_scalar) { this.scl = new Vec2(scl_scalar,scl_scalar); }

