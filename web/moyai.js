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


// 0 ~ 1
function Color(r,g,b,a) {
    this.r = r;
    this.g = g;
    this.b = b;
    this.a = a;
}
Color.prototype.toRGBA = function() {
    return [ parseInt(r*255), parseInt(g*255), parseInt(b*255), parseInt(a*255) ];
}
    
///////////////
var g_moyais=[];
function MoyaiClient(w,h,pixratio){
    this.scene = new THREE.Scene();
    this.renderer = new THREE.WebGLRenderer();
    this.renderer.setPixelRatio( pixratio);
    this.renderer.setSize(w,h);
    this.renderer.autoClear = false;

    this.camera = new THREE.OrthographicCamera( -w/2, w/2, h/2, -h/2,-1,10);
    this.camera.position.z = 10;

    this.scene = new THREE.Scene();
    
    this.layers=[];
    g_moyais.push(this);
}
MoyaiClient.prototype.resize = function(w,h) {
    this.renderer.setSize(w,h);
}
MoyaiClient.prototype.poll = function(dt) {
    var cnt=0;
    for(var i in this.layers) {
        var layer = this.layers[i];
        if( layer && (!layer.skip_poll) ) cnt += layer.pollAllProps(dt);
    }
    return cnt;   
}

MoyaiClient.prototype.render = function() {
    console.log("render");
    for(var i in this.scene.children) {
        this.scene.remove( this.scene.children[i]);
    }
//    this.scene.children.forEach(function(object){ this.scene.remove(object); });    
    for(var i in this.layers) {
        var layer = this.layers[i];
        for(var i in layer.props) {
            var prop = layer.props[i];
            prop.ensureMesh();
            if(prop.mesh) {
                prop.mesh.position.x = prop.loc.x;
                prop.mesh.position.y = prop.loc.y;
                prop.mesh.rotation.set(0,0,prop.rot);
                console.log("adding prop.mesh:",prop);
                this.scene.add(prop.mesh);
            }
        }
    }
    this.renderer.clear();
    this.renderer.clearDepth();
    this.renderer.render( this.scene, this.camera );
    
/*
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
    SorterEntry tosort[128];
    int sort_n = 0;
    for(int i=0;i<elementof(groups);i++) {
        Group *g = groups[i];
		if( g && g->to_render ) {
            Layer *l = (Layer*)g;
            tosort[sort_n].val = l->priority;
            tosort[sort_n].ptr = l;
            sort_n++;
        }
    }
    quickSortF( tosort, 0, sort_n-1 );
        
	int render_n=0;    
	for(int i=0;i<sort_n;i++){
		Layer *l = (Layer*) tosort[i].ptr;
        render_n += l->render(&batch_list);
	}

    last_draw_call_count = batch_list.drawAll();    
	glfwSwapBuffers(window);
	glFlush();
	return render_n;
    */    
    
/*


  prop2dがsprite
  Textureがtextureloaderの返り値
  moyaiはmaterialという概念がない。

  prop2dがmaterialをもつ。
  textureがmapをもつ。
  imageがdatatexであり
  layer,gridやtiledeckは純粋に論理的なもの。
  cameraとviewport
  
    */
    
}
MoyaiClient.prototype.insertLayer = function(l) {
    this.layers.push(l);
}
MoyaiClient.prototype.updateAllImage = function(image) {
    for(var i in this.layers) {
        var layer = this.layers[i];
        for(var i in layer.props) {
            var prop = layer.props[i];
            console.log("updateAllImage prop id:",prop.id, image.id);
            if(prop.deck.moyai_tex) prop.deck.moyai_tex.updateImage(image);
        }
    }
}
///////////////////

Viewport.prototype.id_gen=1;
function Viewport() {
    this.id = this.__proto__.id_gen++;
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
    this.id = this.__proto__.id_gen++;
    this.loc = new Vec2(0,0);
}
Camera.prototype.setLoc = function(x,y) { this.loc.setWith2args(x,y); }

////////////////////
Layer.prototype.id_gen = 1;
function Layer() {
    this.id = this.__proto__.id_gen++;
    this.props=[];
}
Layer.prototype.setViewport = function(vp) { this.viewport = vp; }
Layer.prototype.setCamera = function(cam) { this.camera = cam; }
Layer.prototype.insertProp = function(p) { this.props.push(p); }
Layer.prototype.pollAllProps = function(dt) {
    var keep=[];
    for(var i in this.props) {
        var prop = this.props[i];
        var to_keep = prop.basePoll(dt);
        if(to_keep) keep.push(prop);
    }
    this.props = keep;
    return this.props.length;
}
/////////////////////
Image.prototype.id_gen = 1;
function Image() {
    this.id = this.__proto__.id_gen++;
    console.log("imgid:", this.id);
    this.data = [];
    this.png=null;
}
Image.prototype.loadPNGMem = function(u8adata) {
    this.png = new PNG(u8adata);
    this.width = this.png.width;
    this.height = this.png.height;
    var data_len = this.width*this.height*4;
    this.data = new Uint8Array(data_len);
    for(var i=0;i<data_len;i++) this.data[i] = 0xff; // white image
    var this_image = this;
    this.decode_callback = function(pixels) {
        console.log("decode_callback:",pixels, "image.id:", this_image.id );
        this_image.data = pixels;
        updateAllImage(this_image);
    }
    this.png.decode( this.decode_callback );
}
Image.prototype.setSize = function(w,h) {
    this.data = new Uint8Array(w*h*4);
}
Image.prototype.getSize = function() {
    return new Vec2(this.width,this.height);
}
Image.prototype.getPixelRaw = function(x,y) {
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
Image.prototype.setPixelRaw = function(x,y,r,g,b,a) {
    if(x>=0&&y>=0&&x<this.width&&y<this.height){
        var index = ( x + y * this.width ) * 4;
        this.buffer[index] = r;
        this.buffer[index+1] = g;
        this.buffer[index+2] = b;
        this.buffer[index+3] = a;
    }    
}
Image.prototype.setPixel = function(x,y,c) {
    var colary = c.toRGBA();
    var index = (x+y*this.width)*4;
    
}

//////////////////////////
Texture.prototype.id_gen = 1;
function Texture() {
    this.id = this.__proto__.id_gen++;
    this.image = null;
    this.three_tex = null;
    this.mat = null;
}
Texture.prototype.loadPNGMem = function(u8adata) {
    this.image = new Image();
    this.image.loadPNGMem(u8adata);
    this.three_tex = new THREE.DataTexture( this.image.data, this.image.width, this.image.height, THREE.RGBAFormat );
    this.three_tex.needsUpdate = true;
    this.mat = new THREE.MeshBasicMaterial({ map: this.three_tex /*,depthTest:true*/, transparent: true });
    this.mat.shading = THREE.FlatShading;
    this.mat.side = THREE.FrontSide;
    this.mat.alphaTest = 0.5;
    this.mat.needsUpdate = true;
}
Texture.prototype.getSize = function() {
    return this.image.getSize();
}
Texture.prototype.setImage = function(img) { this.image = img; }
Texture.prototype.updateImage = function(img) {
    if(this.image.id == img.id) {
        console.log("Texture.updateImage",this.three_tex);
        this.three_tex.image.data = img.data;
        this.three_tex.image.width = img.width;
        this.three_tex.image.height = img.height;
        this.three_tex.needsUpdate = true;
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
	var start_x = this.cell_width * parseInt( parseInt(ind) % parseInt(this.tile_width) );
	var start_y = this.cell_height * parseInt( parseInt(ind) / parseInt(this.tile_width ) );
    var u0 = start_x / this.moyai_tex.image.width + eps + uofs * uunit;
    var v0 = start_y / this.moyai_tex.image.height + eps + vofs * vunit;
    var u1 = u0 + uunit - eps*2;  // *2 because adding eps once for u0 and v0
	var v1 = v0 + vunit - eps*2;
    return [u0,v0,u1,v1];
}


///////

var PRIMTYPE_NONE = 0;
var PRIMTYPE_LINE = 1;
var PRIMTYPE_RECTANGLE = 2;

Prim.prototype.id_gen=1;
function Prim(t,a,b,col,lw) {
    this.id=this.__proto__.id_gen++;
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
    this.id=this.__proto__.id_gen++;
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
    this.children=[];
    this.accum_time=0;
    this.poll_count=0;
    this.mesh=null;
}
Prop2D.prototype.setDeck = function(dk) { this.deck = dk; }
Prop2D.prototype.setIndex = function(ind) { this.index = ind; }
Prop2D.prototype.setScl = function(x,y) { this.scl.setWith2args(x,y);}
Prop2D.prototype.setLoc = function(x,y) { this.loc.setWith2args(x,y);}
Prop2D.prototype.setRot = function(r) { this.rot=r; }
Prop2D.prototype.setUVRot = function(flg) { this.uvrot=flg;}
Prop2D.prototype.setColor = function(r,g,b,a) { this.color = new Color(r,g,b,a); }
Prop2D.prototype.addLine = function(p0,p1,col,w) {
    if(!this.prim_drawer) this.prim_drawer = new PrimDrawer();
    this.prim_drawer.addLine(p0,p1,col,w);
}
Prop2D.prototype.addRect = function(p0,p1,col,w) {
    if(!this.prim_drawer) this.prim_drawer = new PrimDrawer();
    this.prim_drawer.addRect(p0,p1,col,w);
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
Prop2D.prototype.addChild = function(chp) {
    this.children.push(chp);
}
Prop2D.prototype.clearChildren = function() {
    this.children=[];
}
Prop2D.prototype.clearChild = function(p) {
    var keep=[];
    for(var i in this.children) {
        if(this.children[i]!=p) keep.push( this.children[i]);
    }
    this.children = keep;
}
Prop2D.prototype.getChild = function(propid) {
    for(var i in this.children) {
        if( this.children[i].id == propid ) {
            return this.children[i];
        }
    }
    return null;
}
Prop2D.prototype.basePoll = function(dt) { // return false to clean
    this.poll_count++;
    this.accum_time+=dt;    
    if(this.to_clean) {
        return false;
    }
    if( this.propPoll && this.propPoll(dt) == false ) {
        return false;
    }
    return true;
}
function createRectGeometry(width,height) {
    var geometry = new THREE.Geometry();
    var sizeHalfX = width / 2;
    var sizeHalfY = height / 2;
    /*
      0--1
      |\ |
      | \|
      3--2
     */
    geometry.vertices.push(new THREE.Vector3(-sizeHalfX, sizeHalfY, 0)); //0
    geometry.vertices.push(new THREE.Vector3(sizeHalfX, sizeHalfY, 0)); //1
    geometry.vertices.push(new THREE.Vector3(sizeHalfX, -sizeHalfY, 0)); //2
    geometry.vertices.push(new THREE.Vector3(-sizeHalfX, -sizeHalfY, 0)); //3
    geometry.faces.push(new THREE.Face3(0, 2, 1));
    geometry.faces.push(new THREE.Face3(0, 3, 2));
    return geometry;
}
Prop2D.prototype.ensureMesh = function() {
    if(this.mesh==null) {
        var mat = new THREE.MeshBasicMaterial({ map: this.deck.moyai_tex.three_tex /*,depthTest:true*/, transparent: true, vertexColors:THREE.VertexColors });
        mat.shading = THREE.FlatShading;
        mat.side = THREE.FrontSide;
        mat.alphaTest = 0.5;
        mat.needsUpdate = true;
        var geom = createRectGeometry(1,1);
        var uvs = this.deck.getUVFromIndex(this.index,0,0,0);
        var u0 = uvs[0], v0 = uvs[1], u1 = uvs[2], v1 = uvs[3];
        geom.faceVertexUvs[0].push([ new THREE.Vector2(u0,v0), new THREE.Vector2(u1,v1), new THREE.Vector2(u1,v0) ]);
        geom.faceVertexUvs[0].push([ new THREE.Vector2(u0,v0), new THREE.Vector2(u0,v1), new THREE.Vector2(u1,v1) ]);
        geom.verticesNeedUpdate = true;
//        geom.elementsNeedUpdate = true;
        geom.uvsNeedUpdate = true;        
        geom.faces[0].vertexColors[0] = new THREE.Color(0xffffff);
        geom.faces[0].vertexColors[1] = new THREE.Color(0xffffff);
        geom.faces[0].vertexColors[2] = new THREE.Color(0xffffff);
        geom.faces[1].vertexColors[0] = new THREE.Color(0xffffff);
        geom.faces[1].vertexColors[1] = new THREE.Color(0xffffff);
        geom.faces[1].vertexColors[2] = new THREE.Color(0xffffff);
        
        this.mesh = new THREE.Mesh(geom,mat);
        this.mesh.scale.x = this.scl.x;
        this.mesh.scale.y = this.scl.y;
        console.log("ENSUREMESH:",this.mesh,uvs,this.mesh.scale);
    }
}
////////////////////////////

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
    this.id=this.__proto__.id_gen++;
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

//        console.log("i:",i," charcode:",ccode," w,h:",w,h,"offset:",offset, "start:",start_x, start_y, "left:",l,"top:",t, "pixc:",pixelcnt , "firstind:", (start_y+0+this.pixel_size-t)*this.atlas.width+(start_x+0+l));
        
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

/////////////////
function CharGrid() {
    Grid.call(this);
    this.ascii_offset = 0;
}
CharGrid.prototype = Object.create(Grid.prototype);
CharGrid.prototype.constructor = CharGrid;
CharGrid.prototype.setAsciiOffset = function(ofs) { this.ascii_offset = ofs; }

function str_repeat(i, m) {
    for (var o = []; m > 0; o[--m] = i);
    return o.join('');
}

function sprintf() {
    var i = 0, a, f = arguments[i++], o = [], m, p, c, x, s = '';
    while (f) {
        if (m = /^[^\x25]+/.exec(f)) {
            o.push(m[0]);
        }
        else if (m = /^\x25{2}/.exec(f)) {
            o.push('%');
        }
        else if (m = /^\x25(?:(\d+)\$)?(\+)?(0|'[^$])?(-)?(\d+)?(?:\.(\d+))?([b-fosuxX])/.exec(f)) {
            if (((a = arguments[m[1] || i++]) == null) || (a == undefined)) {
                throw('Too few arguments.');
            }
            if (/[^s]/.test(m[7]) && (typeof(a) != 'number')) {
                throw('Expecting number but found ' + typeof(a));
            }
            switch (m[7]) {
            case 'b': a = a.toString(2); break;
            case 'c': a = String.fromCharCode(a); break;
            case 'd': a = parseInt(a); break;
            case 'e': a = m[6] ? a.toExponential(m[6]) : a.toExponential(); break;
            case 'f': a = m[6] ? parseFloat(a).toFixed(m[6]) : parseFloat(a); break;
            case 'o': a = a.toString(8); break;
            case 's': a = ((a = String(a)) && m[6] ? a.substring(0, m[6]) : a); break;
            case 'u': a = Math.abs(a); break;
            case 'x': a = a.toString(16); break;
            case 'X': a = a.toString(16).toUpperCase(); break;
            }
            a = (/[def]/.test(m[7]) && m[2] && a >= 0 ? '+'+ a : a);
            c = m[3] ? m[3] == '0' ? '0' : m[3].charAt(1) : ' ';
            x = m[5] - String(a).length - s.length;
            p = m[5] ? str_repeat(c, x) : '';
            o.push(s + (m[4] ? a + p : p + a));
        }
        else {
            throw('Huh ?!');
        }
        f = f.substring(m[0].length);
    }
    return o.join('');
}
    
CharGrid.prototype.printf = function() {
    var args = Array.prototype.slice.call(arguments);    
    var x = args[0];
    var y = args[1];
    var col = args[2];
    var printf_args = args.slice(3);

    //void CharGrid::printf( int x, int y, Color c, const char *fmt, ...) 

    var s = sprintf.apply(this, printf_args );

	for(var i=0;i<s.length;i++){
		var ind = this.ascii_offset + s.charCodeAt(i);
		if(x+i>=this.width)break;
		this.set(x+i,y,ind);
		this.setColor(x+i,y,col);
	}    
}

	
////////////////////
function updateAllImage(image) {
    for(var i in g_moyais) {
        var moyai = g_moyais[i];
        moyai.updateAllImage(image);
    }
}


