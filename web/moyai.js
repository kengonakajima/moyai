// moyai js port

function irange(a,b) {
    return parseInt(range(a,b));
}
function range(a,b) {
    var small=a,big=b;
    if(big<small) {
        var tmp = big;
        big=small;
        small=tmp;
    }
    return (small + (big-small)*Math.random());
}

//////////////
function createMeshBasicMaterial(objarg) {
    var m = new THREE.MeshBasicMaterial(objarg);
    m.shading = THREE.FlatShading;
    m.side = THREE.FrontSide;
    m.alphaTest = 0.5;
    m.needsUpdate = true;
    return m;
}

// Three.js material opacity can't render correctly under 0.5
function workaroundThreeOpacity(opa) {
    if(opa<0.5) return 0.5; else return opa;
}

//////////////

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
    return [ parseInt(this.r*255), parseInt(this.g*255), parseInt(this.b*255), parseInt(this.a*255) ];
}
Color.prototype.toCode = function() {
    return ( parseInt(this.r * 255) << 16 ) + ( parseInt(this.g * 255) << 8 ) + parseInt(this.b * 255);
}
Color.prototype.toTHREEColor = function() {
    return new THREE.Color(this.toCode());
}
     
///////////////
var g_moyais=[];
function MoyaiClient(w,h,pixratio){
    this.scene = new THREE.Scene();
    this.renderer = new THREE.WebGLRenderer();
    this.renderer.setPixelRatio( pixratio);
    this.renderer.setSize(w,h);
    this.renderer.setClearColor("#333");
    this.renderer.autoClear = false;

    this.z_per_layer = 100000;
    this.z_per_prop = 1;
    this.z_per_subprop = 1; // this causes some issue when dense sprites.. but no way to implement correct draw order
    this.max_z = this.z_per_layer*100; // use z to confirm render order ( renderOrder dont work for line prims..)
    
    this.camera = new THREE.OrthographicCamera( -w/2, w/2, h/2, -h/2,-1,this.max_z);
    this.camera.position.z = this.max_z+this.z_per_layer;

    this.scene = new THREE.Scene();
    
    this.layers=[];
    g_moyais.push(this);
}
MoyaiClient.prototype.resize = function(w,h) {
    this.renderer.setSize(w,h);
}
MoyaiClient.prototype.getHighestPriority = function() {
    var highp=0;
    for(var i in this.layers) {
        if(this.layers[i].priority>highp) highp = this.layers[i].priority;
    }
    return highp;
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
    for(var i in this.scene.children) {
        this.scene.remove( this.scene.children[i]);
    }
    if( this.scene.children.length>0) this.scene.remove( this.scene.children[0] ); // confirm remove all.. (TODO refactor)
//    this.scene.children.forEach(function(object){ this.scene.remove(object); });    
    for(var i in this.layers) {
        var layer = this.layers[i];
        for(var i in layer.props) {
            var prop = layer.props[i];
            prop.updateMesh();
            var prop_z = layer.priority * this.z_per_layer + prop.priority * this.z_per_prop;
            if(prop.mesh) {
                prop.mesh.position.x = prop.loc.x;
                prop.mesh.position.y = prop.loc.y;
                prop.mesh.position.z = prop_z;
                prop.mesh.scale.x = prop.scl.x;
                prop.mesh.scale.y = prop.scl.y;                
                prop.mesh.rotation.set(0,0,prop.rot);
                prop.material.opacity = workaroundThreeOpacity(prop.color.a);
                if( prop.use_additive_blend ) prop.material.blending = THREE.AdditiveBlending; else prop.material.blending = THREE.NormalBlending;
                this.scene.add(prop.mesh);
            }
            if(prop.grids) {
                for(var i in prop.grids) {
                    var grid = prop.grids[i];
                    grid.updateMesh();
                    grid.mesh.position.x = prop.loc.x;
                    grid.mesh.position.y = prop.loc.y;
                    grid.mesh.position.z = prop_z + (i+1) * this.z_per_subprop;
                    grid.mesh.scale.x = prop.scl.x;
                    grid.mesh.scale.y = prop.scl.y;
                    grid.mesh.rotation.set(0,0,prop.rot);
                    this.scene.add(grid.mesh);
                }
            }
            if(prop.children.length>0) {
                for(var i in prop.children) {
                    var chp = prop.children[i];
                    chp.updateMesh();
                    if( chp.mesh ) {
                        chp.mesh.position.x = chp.loc.x;
                        chp.mesh.position.y = chp.loc.y;
                        chp.mesh.position.z = prop_z + (i+1) * this.z_per_subprop;                        
                        chp.mesh.scale.x = chp.scl.x;
                        chp.mesh.scale.y = chp.scl.y;                
                        chp.mesh.rotation.set(0,0,chp.rot);
                        chp.material.opacity = workaroundThreeOpacity(chp.color.a);
                        if( chp.use_additive_blend ) chp.material.blending = THREE.AdditiveBlending; else chp.material.blending = THREE.NormalBlending;
                        this.scene.add(chp.mesh);
                    }
                }
            }
            if(prop.prim_drawer) {
                for(var i in prop.prim_drawer.prims) {
                    var prim = prop.prim_drawer.prims[i];
                    prim.updateMesh();
                    prim.mesh.position.x = prop.loc.x;
                    prim.mesh.position.y = prop.loc.y;
                    prim.mesh.position.z = prop_z + (i+1) * this.z_per_subprop;
                    prim.mesh.scale.x = prop.scl.x;
                    prim.mesh.scale.y = prop.scl.y;
                    prim.mesh.rotation.set(0,0,prop.rot);
                    prim.material.opacity = workaroundThreeOpacity(prim.color.a);                    
                    this.scene.add(prim.mesh);
                }
            }            
        }
    }
    this.renderer.clear();
    this.renderer.clearDepth();
    this.renderer.render( this.scene, this.camera );
}

MoyaiClient.prototype.insertLayer = function(l) {
    if(l.priority==null) {
        var highp = this.getHighestPriority();
        l.priority = highp+1;
    }
    this.layers.push(l);
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
    this.priority=null;// update when insert to moyai
}
Layer.prototype.setViewport = function(vp) { this.viewport = vp; }
Layer.prototype.setCamera = function(cam) { this.camera = cam; }
Layer.prototype.insertProp = function(p) {
    if(p.priority==null) {
        var highp = this.getHighestPriority();
        p.priority = highp+1;
    }
    this.props.push(p);
}
Layer.prototype.pollAllProps = function(dt) {
    var keep=[];
    for(var i in this.props) {
        var prop = this.props[i];
        var to_keep = prop.basePoll(dt);
        if(to_keep) {
            keep.push(prop);
        } else {
            prop.onDelete();
        }
    }
    this.props = keep;
    return this.props.length;
}
Layer.prototype.getHighestPriority = function() {
    var highp=0;
    for(var i in this.props) {
        if(this.props[i].priority>highp) highp = this.props[i].priority;
    }
    return highp;    
}
/////////////////////
Image.prototype.id_gen = 1;
function Image() {
    this.id = this.__proto__.id_gen++;
    this.data = [];
    this.png=null;
}
Image.prototype.loadPNGMem = function(u8adata) {
    var b = new Buffer(u8adata);
    this.png = pngParse(b);
    this.width = this.png.width;
    this.height = this.png.height;
    this.data = this.png.data;
}
Image.prototype.setSize = function(w,h) {
    this.width = w;
    this.height = h;
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
        this.data[index] = r;
        this.data[index+1] = g;
        this.data[index+2] = b;
        this.data[index+3] = a;
    }    
}
Image.prototype.setPixel = function(x,y,c) {
    var colary = c.toRGBA();
    var index = (x+y*this.width)*4;
    this.setPixelRaw(x,y,colary[0],colary[1],colary[2],colary[3]);
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
    this.update();
}
Texture.prototype.update = function() {
    if(!this.three_tex) {
        this.three_tex = new THREE.DataTexture( this.image.data, this.image.width, this.image.height, THREE.RGBAFormat );
        this.three_tex.magFilter = THREE.NearestFilter;
    } else {
        this.three_tex.image.data = this.image.data;
    }
    this.three_tex.needsUpdate = true;
}
Texture.prototype.getSize = function() {
    return this.image.getSize();
}
Texture.prototype.setImage = function(img) {
    this.image = img;
    this.update();
}
Texture.prototype.updateImage = function(img) {
    if(this.image.id == img.id ) {
        console.log("Tex.updateimage id:",img.id );
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
TileDeck.prototype.getUperCell = function() { return this.cell_width / this.moyai_tex.image.width; }
TileDeck.prototype.getVperCell = function() { return this.cell_height / this.moyai_tex.image.height; }    


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
    this.geom=null;
    this.material=null;
    this.mesh=null;
}
Prim.prototype.updateMesh = function() {
    if(this.type==PRIMTYPE_LINE) {
        if(this.geom) this.geom.dispose();
        this.geom = new THREE.Geometry();
        this.geom.vertices.push(new THREE.Vector3(this.a.x,this.a.y,0));
        this.geom.vertices.push(new THREE.Vector3(this.b.x,this.b.y,0));
        this.geom.verticesNeedUpdate=true;
        if(!this.material) {
            this.material = new THREE.LineBasicMaterial( { color: this.color.toCode(), linewidth: this.line_width, depthTest:true, transparent:true });
        }
        if(this.mesh) {
            this.mesh.geometry = this.geom;
            this.mesh.material = this.material;
        } else {
            this.mesh = new THREE.Line( this.geom, this.material);
        }        
    } else if(this.type==PRIMTYPE_RECTANGLE) {
        /*
          0--1
          |\ |  0:a 2:b
          | \|
          3--2
        */
        if(this.geom) this.geom.dispose();
        this.geom = new THREE.Geometry();
        this.geom.vertices.push(new THREE.Vector3(this.a.x,this.a.y,0));
        this.geom.vertices.push(new THREE.Vector3(this.b.x,this.a.y,0));
        this.geom.vertices.push(new THREE.Vector3(this.b.x,this.b.y,0));
        this.geom.vertices.push(new THREE.Vector3(this.a.x,this.b.y,0));
        this.geom.verticesNeedUpdate=true;        
        this.geom.faces.push(new THREE.Face3(0, 2, 1));
        this.geom.faces.push(new THREE.Face3(0, 3, 2));
        if(!this.material) {
            this.material = createMeshBasicMaterial({ color: this.color.toCode(),depthTest:true, transparent:true });
        }
        if(this.mesh) {
            this.mesh.geometry = this.geom;
            this.mesh.material = this.material;
        } else {
            this.mesh = new THREE.Mesh(this.geom,this.material);
        }        
    } else {
        console.log("invalid prim type",this.type)
    }
}
Prim.prototype.onDelete = function() {
    this.mesh.geometry.dispose();
    this.mesh.material.dispose();    
}

//////////////////
function PrimDrawer() {
    this.prims=[];
}
PrimDrawer.prototype.addLine = function(a,b,col,w) {
    var newprim = new Prim(PRIMTYPE_LINE,a,b,col,w);
    this.prims.push(newprim);
    return newprim;
}
PrimDrawer.prototype.addRect = function(a,b,col,w) {
    var newprim = new Prim(PRIMTYPE_RECTANGLE,a,b,col,w);
    this.prims.push(newprim);
    return newprim;
}
PrimDrawer.prototype.getPrimById = function(id) {
    for(var i in  this.prims) {
        if(this.prims[i].id == id ) return this.prims[i];
    }
    return null;
}
PrimDrawer.prototype.deletePrim = function(id) {
    for(var i in  this.prims) {
        if(this.prims[i].id == id ) {
            this.prims[i].onDelete();
            this.prims.splice(i,1);
            return;
        }
    }
}

//////////////////

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
    this.mesh=null;
    this.material=null;
    this.priority = null; // set when insertprop if kept null
    this.need_material_update=false;
    this.need_color_update=false;
    this.need_uv_update=false;
    this.xflip=false;
    this.yflip=false;
}
Prop2D.prototype.onDelete = function() {
    console.log("prp2d disposing ", this.id);
    this.mesh.geometry.dispose();
    this.mesh.material.dispose();
}
Prop2D.prototype.setDeck = function(dk) { this.deck = dk; this.need_material_update = true; }
Prop2D.prototype.setIndex = function(ind) { this.index = ind; this.need_uv_update = true; }
Prop2D.prototype.setScl = function(x,y) { this.scl.setWith2args(x,y);}
Prop2D.prototype.setLoc = function(x,y) { this.loc.setWith2args(x,y);}
Prop2D.prototype.setRot = function(r) { this.rot=r; }
Prop2D.prototype.setUVRot = function(flg) { this.uvrot=flg; this.need_uv_update = true; }
Prop2D.prototype.setColor = function(r,g,b,a) { this.color = new Color(r,g,b,a); this.need_color_update = true; }
Prop2D.prototype.setXFlip = function(flg) { this.xflip=flg; this.need_uv_update = true; }
Prop2D.prototype.setYFlip = function(flg) { this.yflip=flg; this.need_uv_update = true; }
Prop2D.prototype.addLine = function(p0,p1,col,w) {
    if(!this.prim_drawer) this.prim_drawer = new PrimDrawer();
    return this.prim_drawer.addLine(new Vec2(p0.x,p0.y),new Vec2(p1.x,p1.y),col,w);
}
Prop2D.prototype.addRect = function(p0,p1,col,w) {
    if(!this.prim_drawer) this.prim_drawer = new PrimDrawer();
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
    var newgrid={}; // Duplicate content of g for sharing same grid in other props
    Object.assign(newgrid,g);
    newgrid.__proto__ = g.__proto__;
    this.grids.push(newgrid);
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
Prop2D.prototype.updateMesh = function() {
    if(!this.deck)return;
    if( this.need_material_update ) {
        if(!this.material) {
            this.material = createMeshBasicMaterial({ map: this.deck.moyai_tex.three_tex, depthTest:true, transparent: true, vertexColors:THREE.VertexColors, blending: THREE.NormalBlending }); // materialはメモリを消耗しないようだ
        } else {
            this.material.map = this.deck.moyai_tex.three_tex;
        }
        this.need_material_update = false;
    }
    if( this.need_uv_update ) {
        var uvs = this.deck.getUVFromIndex(this.index,0,0,0);
        var u0 = uvs[0], v0 = uvs[1], u1 = uvs[2], v1 = uvs[3];
        if(this.xflip ) {
            var tmp = u1; u1 = u0; u0 = tmp;
        }
        if(this.yflip ) {
            var tmp = v1; v1 = v0; v0 = tmp;
        }        
        if(!this.geom) {
            this.geom = createRectGeometry(1,1);
        }
        var uvs = this.geom.faceVertexUvs[0][0];
        if(!uvs) {
            this.geom.faceVertexUvs[0].push([ new THREE.Vector2(u0,v0), new THREE.Vector2(u1,v1), new THREE.Vector2(u1,v0) ]);
            this.geom.faceVertexUvs[0].push([ new THREE.Vector2(u0,v0), new THREE.Vector2(u0,v1), new THREE.Vector2(u1,v1) ]);
        } else {
            uvs[0].x = u0; uvs[0].y = v0;
            uvs[1].x = u1; uvs[1].y = v1;
            uvs[2].x = u1; uvs[2].y = v0;
            uvs = this.geom.faceVertexUvs[0][1];
            uvs[0].x = u0; uvs[0].y = v0;
            uvs[1].x = u0; uvs[1].y = v1;
            uvs[2].x = u1; uvs[2].y = v1;
        }
        
        this.geom.verticesNeedUpdate = true;
        this.geom.uvsNeedUpdate = true;
        this.need_uv_update = false;
    }
    if( this.need_color_update ) {
        geom.faces[0].vertexColors[0] = this.color.toTHREEColor();
        geom.faces[0].vertexColors[1] = this.color.toTHREEColor();
        geom.faces[0].vertexColors[2] = this.color.toTHREEColor();
        geom.faces[1].vertexColors[0] = this.color.toTHREEColor();
        geom.faces[1].vertexColors[1] = this.color.toTHREEColor();
        geom.faces[1].vertexColors[2] = this.color.toTHREEColor();
        this.need_color_update = false;
    }

    if(!this.mesh) {
        this.mesh = new THREE.Mesh(this.geom,this.material);
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
    this.material=null;
    this.geom=null;
    this.need_material_update=false;
    this.need_geometry_update=false;
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
    this.need_geometry_update = true;
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
    this.need_geometry_update = true;
}
Grid.prototype.getXFlip = function(x,y) {
    if(!this.xflip_table) return false;
    return this.xflip_table[this.index(x,y)];
}
Grid.prototype.setYFlip = function(x,y,flg) {
    if(!this.yflip_table) this.yflip_table=[];
    this.yflip_table[this.index(x,y)]=flg;    
    this.need_geometry_update = true;
}
Grid.prototype.getYFlip = function(x,y) {
    if(!this.yflip_table) return false;
    return this.yflip_table[this.index(x,y)];
}
Grid.prototype.setTexOffset = function(x,y,uv) {
    if(!this.texofs_table) this.texofs_table=[];
    this.texofs_table[this.index(x,y)]=uv;
    this.need_geometry_update = true;
}
Grid.prototype.getTexOffset = function(x,y) {
    if(!this.texofs_table) return new Vec2(0,0);
    return this.texofs_table[this.index(x,y)];
}
Grid.prototype.setUVRot = function(x,y,flg) {
    if(!this.rot_table) this.rot_table=[];
    this.rot_table[this.index(x,y)]=flg;
    this.need_geometry_update = true;
}
Grid.prototype.getUVRot = function(x,y) {
    if(!this.rot_table) return false;
    return this.rot_table[this.index(x,y)];
}
Grid.prototype.setColor = function(x,y,col) {
    if(!this.color_table) this.color_table=[];
    this.color_table[this.index(x,y)]=col;
    this.need_geometry_update = true;
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
    this.need_geometry_update = true;
}
Grid.prototype.updateMesh = function() {
    if(!this.deck) return;
    if(!this.index_table) return;
    
    if(this.need_material_update) {
        this.need_material_update=false;
        if(!this.material) {
            this.material = createMeshBasicMaterial({map: this.deck.moyai_tex.three_tex, transparent:true, depthTest:true, vertexColors:THREE.VertexColors });
        } else {
            this.material.map = this.deck.moyai_tex.three_tex;
        }
    }
    if(this.need_geometry_update) {
        this.need_geometry_update = false;
        if(this.geom) {
            this.geom.dispose();
        }
        this.geom = new THREE.Geometry();

        var geom = this.geom;
        var quad_cnt=0;
        for(var y=0;y<this.height;y++) {
            for(var x=0;x<this.width;x++) {
                var ind = x+y*this.width;
                if( this.index_table[ind] == GRID_NOT_USED )continue;

                /*
                  0--1
                  |\ |
                  | \|
                  3--2

                  3の位置が(0,0)
                */

                // 1セルあたり4頂点づつ
                geom.vertices.push(new THREE.Vector3(x,y+1,0)); //0
                geom.vertices.push(new THREE.Vector3(x+1,y+1, 0)); //1
                geom.vertices.push(new THREE.Vector3(x+1,y, 0)); //2
                geom.vertices.push(new THREE.Vector3(x,y, 0)); //3
                // 1セルあたり2面づつ
                var face_start_vert_ind = quad_cnt*4;
                geom.faces.push(new THREE.Face3(face_start_vert_ind+0, face_start_vert_ind+2, face_start_vert_ind+1));
                geom.faces.push(new THREE.Face3(face_start_vert_ind+0, face_start_vert_ind+3, face_start_vert_ind+2));
                
                var left_bottom, right_top;
                var uvs = this.deck.getUVFromIndex(this.index_table[ind],0,0,0);
                var u0 = uvs[0], v0 = uvs[1], u1 = uvs[2], v1 = uvs[3];

                if(this.texofs_table && this.texofs_table[ind]) {
                    var u_per_cell = this.deck.getUperCell();
                    var v_per_cell = this.deck.getVperCell();
                    u0 += this.texofs_table[ind].x * u_per_cell;
                    v0 += this.texofs_table[ind].y * v_per_cell;
                    u1 += this.texofs_table[ind].x * u_per_cell;
                    v1 += this.texofs_table[ind].y * v_per_cell;
                }

                if(this.xflip_table && this.xflip_table[ind]) {
                    var tmp = u1; u1 = u0; u0 = tmp;
                }
                if(this.yflip_table && this.yflip_table[ind]) {
                    var tmp = v1; v1 = v0; v0 = tmp;
                }
                var uv_0 = new THREE.Vector2(u0,v0);
                var uv_1 = new THREE.Vector2(u1,v0);
                var uv_2 = new THREE.Vector2(u1,v1);
                var uv_3 = new THREE.Vector2(u0,v1);
                geom.faceVertexUvs[0].push([uv_0,uv_2,uv_1]);
                geom.faceVertexUvs[0].push([uv_0,uv_3,uv_2]);
                var col; 
                if( this.color_table && this.color_table[ind] ) col = this.color_table[ind].toTHREEColor(); else col = new THREE.Color("#fff");
                geom.faces[quad_cnt*2+0].vertexColors[0] = col;
                geom.faces[quad_cnt*2+0].vertexColors[1] = col;
                geom.faces[quad_cnt*2+0].vertexColors[2] = col;
                geom.faces[quad_cnt*2+1].vertexColors[0] = col;
                geom.faces[quad_cnt*2+1].vertexColors[1] = col;
                geom.faces[quad_cnt*2+1].vertexColors[2] = col;
                quad_cnt++;
            }
        }
        geom.verticesNeedUpdate = true;
        geom.uvsNeedUpdate = true;
        if(!this.mesh) {
            this.mesh = new THREE.Mesh(this.geom,this.material);
        } else {
            this.mesh.geometry = this.geom;
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
    this.image = null;
    this.moyai_tex=null;
}
TextureAtlas.prototype.dump = function(ofsx,ofsy, w,h) {
    for(var y=0;y<h;y++) {
        var line="";
        for(var x=0;x<w;x++) {
            var val = this.data[(ofsx+x)+(ofsy+y)*this.width];
            if(val>128) line+="*"; else if(val>60) line+="."; else line+=" ";
        }
        console.log(y,line);
    }
    console.log(this.data);
}
TextureAtlas.prototype.ensureTexture = function() {
    this.image = new Image();
    this.image.setSize(this.width,this.height);
    for(var y=0;y<this.height;y++) {
        for(var x=0;x<this.width;x++) {
            var pixdata = this.data[x+y*this.width]
            this.image.setPixelRaw(x,y,pixdata,pixdata,pixdata,pixdata);
        }
    }
    this.moyai_tex = new Texture();
    this.moyai_tex.setImage(this.image);
//    this.moyai_tex.three_tex.magFilter = THREE.LinearFilter;
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
function Glyph(l,t,w,h,adv,u0,v0,u1,v1,charcode,dbg) {
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
    this.debug = dbg;
    
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
//    this.atlas.dump(/*27*/0,0,100,20);
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
//                console.log("space char!:",ccode, FTFuncs.get_width(), FTFuncs.get_advance());
            } else {
                console.log("  get_bitmap failed for charcode:",ccode, "debug_code:", FTFuncs.get_debug_code(), "i:",i, "char:", codes[i] );
                continue;
            }            
        } 
        
        var w = FTFuncs.get_width();
        var h = FTFuncs.get_height();
        if(offset>0) {
            var buf = FTModule.HEAPU8.subarray(offset,offset+w*h);
//            console.log("BUF:",buf);
        }
        var start_x = (i % horiz_num) * this.pixel_size;
        var start_y = parseInt(i / horiz_num) * (this.pixel_size);

        var l = FTFuncs.get_left();
        var top = FTFuncs.get_top();        

        var pixelcnt=0;
        for(var ii=0;ii<w;ii++){
            for(var jj=0;jj<h;jj++) {
                var val = 0;
                if(offset>0) {
                    var val = buf[jj*w+ii]; // 0~255
                }
                if(val==0) {
                    continue; // 0 for no data
                }
                pixelcnt++;
                var ind_in_atlas = (start_y+jj+this.pixel_size-top)*this.atlas.width + (start_x+l+ii);
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

//        console.log("i:",i," charcode:",ccode," w,h:",w,h,"offset:",offset, "start:",start_x, start_y, "left:",l,"top:",top, "pixc:",pixelcnt , "firstind:", (start_y+0+this.pixel_size-t)*this.atlas.width+(start_x+0+l));

        // http://ncl.sakura.ne.jp/doc/ja/comp/freetype-memo.html
        // ここまでの結果、 face->glyph->bitmap_left、face->glyph->bitmap_top には現在位置から ビットマップにおける文字の左端と上端までの距離が格納される (現在位置はフォントのベースライン上の左端のことと思われる)。 face->glyph->bitmap (FT_Bitmap型)にビットマップ情報が格納される。
// ベースラインはstart_y+pixel_sizeなので、それ-top;

        var lt_x = start_x+l;
        var lt_y = start_y+this.pixel_size-top;
        var rb_x = start_x+l+w;
        var rb_y = start_y+this.pixel_size-top+h;
        
        var lt_u = lt_x / this.atlas.width;
        var lt_v = lt_y / this.atlas.height;
        var rb_u = rb_x / this.atlas.width;
        var rb_v = rb_y / this.atlas.height;
        var adv = FTFuncs.get_advance();
        this.glyphs[ccode] = new Glyph(l,top,w,h,adv,lt_u,lt_v,rb_u,rb_v,ccode, [lt_x,lt_y,rb_x,rb_y].join(","));
    }
    this.atlas.ensureTexture();
}
Font.prototype.getGlyph = function(code) {
    return this.glyphs[code];
}

//////////////////
function TextBox() {
    Prop2D.call(this);
    this.font = null;
    this.scl = new Vec2(1,1);
    this.str = null;
    this.geom=null;
    this.material=null;
    this.need_geometry_update=false;
    this.need_material_update=false;
}
TextBox.prototype = Object.create(Prop2D.prototype);
TextBox.prototype.constructor = TextBox;
TextBox.prototype.setFont = function(fnt) { this.font = fnt; this.need_material_update=true; }
TextBox.prototype.setString = function(s) {
    this.str = s;
    this.need_geometry_update=true;
}
TextBox.prototype.updateMesh = function() {
    if(!this.font)return;
    if(!this.need_geometry_update)return;
    this.need_geometry_update = false;
    
    if(this.geom) this.geom.dispose();
    var geom = new THREE.Geometry();
    this.geom = geom;
    var cur_x=0,cur_y=0;
    var used_chind=0;
    for(var chind = 0; chind <this.str.length;chind++) {
        // 1文字あたり4点, 2面,6インデックス
        // TODO: kerning
        // TODO: 改行
        var char_code = this.str.charCodeAt(chind);
        if(char_code==10) { // "\n"
            cur_y += this.font.pixel_size;
            cur_x = 0;
            continue;
        }
        var glyph = this.font.getGlyph( char_code );
        if(!glyph) {
            console.log("glyph not found for:", char_code, "char:", this.str.charAt(chind) );
        }
        // 座標の大きさはピクセルサイズ
        /*
          0--1
          |\ |
          | \|
          3--2 3の位置が(0,0) = (cur_x,cur_y)  幅がw,高さがh
        */
        // 1セルあたり4頂点づつ
        var w = glyph.width;
        var h = glyph.height;
        var l = glyph.left;
        var t = glyph.top;
        geom.vertices.push(new THREE.Vector3(cur_x+l,cur_y+t,0)); //0
        geom.vertices.push(new THREE.Vector3(cur_x+l+w,cur_y+t,0)); //1
        geom.vertices.push(new THREE.Vector3(cur_x+l+w,cur_y+t-h,0)); //2
        geom.vertices.push(new THREE.Vector3(cur_x+l,cur_y+t-h,0)); //3
        var face_start_vert_ind = used_chind*4;
        geom.faces.push(new THREE.Face3(face_start_vert_ind+0, face_start_vert_ind+2, face_start_vert_ind+1));
        geom.faces.push(new THREE.Face3(face_start_vert_ind+0, face_start_vert_ind+3, face_start_vert_ind+2));
        // uvは左上が0,右下が1
        geom.faceVertexUvs[0].push([ new THREE.Vector2(glyph.u0,glyph.v0),
                                     new THREE.Vector2(glyph.u1,glyph.v1),
                                     new THREE.Vector2(glyph.u1,glyph.v0)]);
        geom.faceVertexUvs[0].push([ new THREE.Vector2(glyph.u0,glyph.v0),
                                     new THREE.Vector2(glyph.u0,glyph.v1),
                                     new THREE.Vector2(glyph.u1,glyph.v1)]);

        geom.faces[used_chind*2+0].vertexColors[0] = this.color.toTHREEColor();
        geom.faces[used_chind*2+0].vertexColors[1] = this.color.toTHREEColor();
        geom.faces[used_chind*2+0].vertexColors[2] = this.color.toTHREEColor();
        geom.faces[used_chind*2+1].vertexColors[0] = this.color.toTHREEColor();
        geom.faces[used_chind*2+1].vertexColors[1] = this.color.toTHREEColor();
        geom.faces[used_chind*2+1].vertexColors[2] = this.color.toTHREEColor();
        cur_x += glyph.advance;
        used_chind++;
    }
    geom.verticesNeedUpdate = true;
    geom.uvsNeedUpdate = true;

    if(this.need_material_update) {
        this.need_material_update = false;
        if(!this.material) {
            this.material = createMeshBasicMaterial({ map: this.font.atlas.moyai_tex.three_tex,
                                                      transparent: true,
                                                      // antialias: true, three warns   'antialias' is not a property of this material.
                                                      vertexColors:THREE.VertexColors,
                                                      blending: THREE.NormalBlending });
        } else {
            this.material.map = this.font.atlas.moyai_tex.three_tex;
        }
    }
    if(this.mesh) {
        this.mesh.geometry = this.geom;
        this.mesh.material = this.material;
    } else {
        this.mesh = new THREE.Mesh(geom,this.material);
    }
}

/////////////////
function CharGrid(w,h) {
    Grid.call(this,w,h);
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

