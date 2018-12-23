// moyai js port
//////////////
function createMeshBasicMaterial(objarg) {
    var m = new THREE.MeshBasicMaterial(objarg);
    //old    m.shading = THREE.FlatShading;
    m.flatShading=true;
    m.side = THREE.FrontSide;
    m.alphaTest = 0;
    m.needsUpdate = true;
    return m;
}



///////////////
var g_moyais=[];
function MoyaiClient(w,h,pixratio){
    this.width=w;
    this.height=h;
    this.renderer = new THREE.WebGLRenderer({preserveDrawingBuffer: true});
    this.renderer.setPixelRatio( pixratio);
    this.renderer.setSize(w,h);
    this.renderer.setClearColor("#000");
    this.renderer.autoClear = false;
    this.enable_clear=true;
    
    this.layers=[];
    g_moyais.push(this);
}
MoyaiClient.prototype.resize = function(w,h) {
    this.renderer.setSize(w,h);
}
MoyaiClient.prototype.getHighestPriority = function() {
    var highp=0;
    for(var i=0;i<this.layers.length;i++) {
        if(this.layers[i].priority>highp) highp = this.layers[i].priority;
    }
    return highp;
}
MoyaiClient.prototype.poll = function(dt) {
    var cnt=0;
    for(var i=0;i<this.layers.length;i++) {
        var layer = this.layers[i];
        if( layer && (!layer.skip_poll) ) cnt += layer.pollAllProps(dt);
    }
    return cnt;   
}
MoyaiClient.prototype.clearAll = function() {
    this.renderer.clear();
    this.renderer.clearDepth();            
}

MoyaiClient.prototype.render = function() {
    // clear
/*    
    for(var i=0;i<this.scene2d.children.length;i++) this.scene2d.remove( this.scene2d.children[i]);
    for(var i=0;i<this.scene3d.children.length;i++) this.scene3d.remove( this.scene3d.children[i]);
    
    if( this.scene2d.children.length>0) this.scene2d.remove( this.scene2d.children[0] ); // confirm remove all.. (TODO refactor)
    if( this.scene3d.children.length>0) this.scene3d.remove( this.scene3d.children[0] ); // confirm remove all.. (TODO refactor)    
*/
    this.scene2d=new THREE.Scene();
    this.scene3d=new THREE.Scene();
    if(this.enable_clear) {
        this.clearAll();
    }    
    // 3d first
    var camera3d=null; // 最初の3Dレイヤのカメラを採用する。(TODO:レイヤごとに別のカメラで描画できるようにする)
    for(var li=0;li<this.layers.length;li++) {
        var layer = this.layers[li];
        if(layer.viewport.dimension==3) {
            this.render3D(this.scene3d, layer);
            if(camera3d==null) {
                camera3d = new THREE.PerspectiveCamera( 45 , this.width / this.height , layer.viewport.near_clip , layer.viewport.far_clip );
                var lcam = layer.camera;
                camera3d.up[0] = lcam.look_up[0]; camera3d.up[1] = lcam.look_up[1]; camera3d.up[2] = lcam.look_up[2];
                camera3d.position.set( lcam.loc[0], lcam.loc[1], lcam.loc[2] );
                camera3d.lookAt(lcam.look_at[0], lcam.look_at[1], lcam.look_at[2] );
                lcam.three_camera=camera3d; // for billboard                
            }
        }
    }
    if(camera3d) {
        this.renderer.render(this.scene3d, camera3d );
    }


    // then 2d            
    for(var li=0;li<this.layers.length;li++) {
        var layer = this.layers[li];
        if(layer.viewport.dimension==2) {
            this.render2D(this.scene2d, layer);
        }
    }
    var camera2d = new THREE.OrthographicCamera( -this.width/2, this.width/2, this.height/2, -this.height/2,-1, g_moyai_max_z);
    camera2d.position.z = g_moyai_max_z;
    this.renderer.render( this.scene2d, camera2d );
}
MoyaiClient.prototype.render3D = function(scene,layer) {
    if(layer.light) {
        scene.add(layer.light);
    }
    if(layer.ambient_light) {
        scene.add(layer.ambient_light);
    }
    for(var pi=0;pi<layer.props.length;pi++ ) {                    
        var prop = layer.props[pi];
        if(!prop.visible)continue;
        if(prop.to_clean)continue;
        prop.mesh.position.set(prop.loc[0]+prop.draw_offset[0], prop.loc[1]+prop.draw_offset[1], prop.loc[2]+prop.draw_offset[2]);
        if(!prop.skip_default_rotation) prop.mesh.rotation.set(prop.rot[0], prop.rot[1], prop.rot[2]);
        if(prop.scl[0]!=1 || prop.scl[1]!=1 || prop.scl[2]!=1) {
            prop.mesh.scale.set(prop.scl[0], prop.scl[1], prop.scl[2]); 
        }
        scene.add(prop.mesh);
    }
}
MoyaiClient.prototype.render2D = function(scene, layer) {
    if(!this.relscl) this.relscl=vec2.fromValues(1,1);
    if(!this.camloc) this.camloc=vec2.create();
    if(layer.camera) {
        vec2.copy(this.camloc,layer.camera.loc);
    } else {
        vec2.set(this.camloc,0,0);
    }
    if(layer.viewport) {
        layer.viewport.getRelativeScale(this.relscl);
    }
    for(var pi=0;pi<layer.props.length;pi++ ) {                    
        var prop = layer.props[pi];
        if(!prop.visible)continue;

        if(prop.custom_mesh) {
            prop.mesh=prop.custom_mesh;
            prop.material=prop.custom_mesh.material;
        } else {
            prop.updateMesh();
        }
        
        var z_inside_prop=0;
        
        var prop_z = layer.priority * g_moyai_z_per_layer + prop.priority * g_moyai_z_per_prop;
        if(prop.grids) {
            for(var gi=0;gi<prop.grids.length;gi++) {
                var grid = prop.grids[gi];
                if(!grid.visible)continue;
                if(!grid.deck)grid.setDeck(prop.deck);
                grid.updateMesh();
                if(!grid.mesh) {
                    console.log("grid.mesh is null. grid_id:", grid.id, " skipping render");
                } else {
                    grid.mesh.position.x = (prop.loc[0]+prop.draw_offset[0]-this.camloc[0])*this.relscl[0];
                    grid.mesh.position.y = (prop.loc[1]+prop.draw_offset[1]-this.camloc[1])*this.relscl[1];
                    grid.mesh.position.z = prop_z + z_inside_prop;
                    grid.mesh.scale.x = prop.scl[0] * this.relscl[0];
                    grid.mesh.scale.y = prop.scl[1] * this.relscl[1];
                    grid.mesh.rotation.set(0,0,prop.rot);
                    scene.add(grid.mesh);
                    z_inside_prop += g_moyai_z_per_subprop;
                }
            }
        }
        if(prop.children.length>0) {
            for(var i=0;i<prop.children.length;i++) {
                var chp = prop.children[i];
                if(!chp.visible)continue;
                if(chp.custom_mesh) {
                    chp.mesh=chp.custom_mesh;
                    chp.material=chp.custom_mesh.material;
                } else {
                    chp.updateMesh();
                }
                if( chp.mesh ) {
                    chp.mesh.position.x = (chp.loc[0]-this.camloc[0])*this.relscl[0];
                    chp.mesh.position.y = (chp.loc[1]-this.camloc[1])*this.relscl[1];
                    chp.mesh.position.z = prop_z + z_inside_prop;
                    chp.mesh.scale.x = chp.scl[0] * this.relscl[0];
                    chp.mesh.scale.y = chp.scl[1] * this.relscl[1];
                    chp.mesh.rotation.set(0,0,chp.rot);
                    if( chp.use_additive_blend ) chp.material.blending = THREE.AdditiveBlending; else chp.material.blending = THREE.NormalBlending;
                    scene.add(chp.mesh);
                    z_inside_prop += g_moyai_z_per_subprop;
                }
            }
        }
        if(prop.mesh) {
            prop.mesh.position.x = (prop.loc[0]+prop.draw_offset[0] - this.camloc[0])*this.relscl[0];
            prop.mesh.position.y = (prop.loc[1]+prop.draw_offset[1] - this.camloc[1])*this.relscl[1];
            prop.mesh.position.z = prop_z + z_inside_prop;
            prop.mesh.scale.x = prop.scl[0] * this.relscl[0];
            prop.mesh.scale.y = prop.scl[1] * this.relscl[1];
            prop.mesh.rotation.set(0,0,prop.rot);
            if( prop.use_additive_blend ) prop.material.blending = THREE.AdditiveBlending; else prop.material.blending = THREE.NormalBlending;
            scene.add(prop.mesh);
            z_inside_prop += g_moyai_z_per_subprop;
        }            
        if(prop.prim_drawer) {
            for(var i=0;i<prop.prim_drawer.prims.length;i++) {
                var prim = prop.prim_drawer.prims[i];
                prim.updateMesh();
                prim.mesh.position.x = (prop.loc[0]+prop.draw_offset[0]-this.camloc[0])*this.relscl[0];
                prim.mesh.position.y = (prop.loc[1]+prop.draw_offset[1]-this.camloc[1])*this.relscl[1];
                prim.mesh.position.z = prop_z + z_inside_prop;
                prim.mesh.scale.x = prop.scl[0] * this.relscl[0];
                prim.mesh.scale.y = prop.scl[1] * this.relscl[1];
                prim.mesh.rotation.set(0,0,prop.rot);
                //                    console.log("adding prim:", prim, prim.a, prim.b, prim.mesh.position );
                scene.add(prim.mesh);
                z_inside_prop += g_moyai_z_per_subprop;
            }
        }            
    }
}

MoyaiClient.prototype.insertLayer = function(l) {
    if(l.priority==null) {
        var highp = this.getHighestPriority();
        l.priority = highp+1;
    }
    this.layers.push(l);
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
    this.image = new MoyaiImage();
    this.image.loadPNGMem(u8adata);
    this.update();
}
Texture.prototype.update = function() {
    if(!this.three_tex) {
//        console.log("texture.update: creating datatexture from image:", this.image,this );
        this.three_tex = new THREE.DataTexture( this.image.data, this.image.width, this.image.height, THREE.RGBAFormat );
        this.three_tex.magFilter = THREE.NearestFilter;
    } else {
        this.three_tex.image.data = this.image.data;
    }
    this.three_tex.needsUpdate = true;
}
Texture.prototype.getSize = function(out) {
    return this.image.getSize(out);
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


///////

var PRIMTYPE_NONE = 0;
var PRIMTYPE_LINE = 1;
var PRIMTYPE_RECTANGLE = 2;

Prim.prototype.id_gen=1;
function Prim(t,a,b,col,lw) {
    this.id=this.__proto__.id_gen++;
    this.type = t;
    this.a=vec2.create();
    vec2.copy(this.a,a);
    this.b=vec2.create();
    vec2.copy(this.b,b);
    this.color=Color.fromValues(col[0],col[1],col[2],col[3]);
    if(!lw) lw=1;
    this.line_width=lw;
    this.geom=null;
    this.material=null;
    this.mesh=null;
    if(t==PRIMTYPE_RECTANGLE) {
        this.fragment_shader = new PrimColorShader();
        this.need_material_update = true;        
    }
}
Prim.prototype.updateMesh = function() {
    if(this.type==PRIMTYPE_LINE) {
        if(this.geom) this.geom.dispose();
        this.geom = new THREE.Geometry();
        this.geom.vertices.push(new THREE.Vector3(this.a[0],this.a[1],0));
        this.geom.vertices.push(new THREE.Vector3(this.b[0],this.b[1],0));
        this.geom.verticesNeedUpdate=true;
        if(!this.material) {
            this.material = new THREE.LineBasicMaterial( { color: Color.toCode(this.color), linewidth: this.line_width, depthTest:true, transparent:true });
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
        this.geom.vertices.push(new THREE.Vector3(this.a[0],this.a[1],0));
        this.geom.vertices.push(new THREE.Vector3(this.b[0],this.a[1],0));
        this.geom.vertices.push(new THREE.Vector3(this.b[0],this.b[1],0));
        this.geom.vertices.push(new THREE.Vector3(this.a[0],this.b[1],0));
        this.geom.verticesNeedUpdate=true;
        if( (this.a[0]<this.b[0] && this.a[1]<this.b[1]) || (this.a[0]>this.b[0] && this.a[1]>this.b[1]) ) {
            this.geom.faces.push(new THREE.Face3(0, 1, 2));
            this.geom.faces.push(new THREE.Face3(0, 2, 3));
        } else {
            this.geom.faces.push(new THREE.Face3(0, 2, 1));
            this.geom.faces.push(new THREE.Face3(0, 3, 2));
        }
        this.geom.faces[0].vertexColors[0] = Color.toTHREEColor(this.color);
        this.geom.faces[0].vertexColors[1] = Color.toTHREEColor(this.color);
        this.geom.faces[0].vertexColors[2] = Color.toTHREEColor(this.color);
        this.geom.faces[1].vertexColors[0] = Color.toTHREEColor(this.color);
        this.geom.faces[1].vertexColors[1] = Color.toTHREEColor(this.color);
        this.geom.faces[1].vertexColors[2] = Color.toTHREEColor(this.color);
        
        if(this.need_material_update ) {
            if(!this.material) {
                this.fragment_shader.updateUniforms(this.color);
                this.material = this.fragment_shader.material;
            } else {
                this.fragment_shader.updateUniforms(this.color);
            }
            this.need_material_update = false;
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
    if(this.mesh) {
        this.mesh.geometry.dispose();
        this.mesh.material.dispose();
    }
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
    for(var i=0;i<this.prims.length;i++) {
        if(this.prims[i].id == id ) return this.prims[i];
    }
    return null;
}
PrimDrawer.prototype.deletePrim = function(id) {
    for(var i=0;i<this.prims.length;i++) {
        if(this.prims[i].id == id ) {
            this.prims[i].onDelete();
            this.prims.splice(i,1);
            return;
        }
    }
}
PrimDrawer.prototype.ensurePrim = function(p) {
    var existing = this.getPrimById(p.id);
    if(existing){
        existing.type = p.type;
        vec2.copy(existing.a,p.a);
        vec2.copy(existing.b,p.b);
        existing.color=p.color;
        existing.line_width=p.line_width;
        existing.updateMesh();
    } else {
        if(p.type==PRIMTYPE_LINE) {
            var newprim = this.addLine(p.a,p.b,p.color,p.line_width);
            newprim.id=p.id;
        } else if(p.type == PRIMTYPE_RECTANGLE) {
            var newprim = this.addRect(p.a,p.b,p.color,p.line_width);
            newprim.id=p.id;
        }        
    }
}
PrimDrawer.prototype.clear = function() {
    for(var i=0;i<this.prims.length;i++) {
        this.prims[i].onDelete();
    }
    this.prims=[];
}
//////////////////
var moyai_id_gen=1;
class Prop {
    constructor() {
        this.id=moyai_id_gen++;
        this.poll_count=0;
        this.accum_time=0;
        this.children=[];        
    }
    basePoll(dt) {
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
    addChild(chp) { this.children.push(chp); }
    clearChildren() { this.children=[]; }
    clearChild(p) {
        var keep=[];
        for(var i=0;i<this.children.length;i++) {
            if(this.children[i]!=p) keep.push( this.children[i]);
        }
        this.children = keep;
    }
    getChild(propid) {
        for(var i=0;i<this.children.length;i++) {
            if( this.children[i].id == propid ) {
                return this.children[i];
            }
        }
        return null;
    }    
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

class Prop2D extends Prop {
    constructor() {
        super();
        this.index = 0;
        this.scl = vec2.fromValues(32,32);
        this.loc = vec2.create();
        this.rot = 0;
        this.deck = null;
        this.uvrot = false;
        this.color = Color.fromValues(1,1,1,1);
        this.prim_drawer = null;
        this.grids=null;
        this.visible=true;
        this.use_additive_blend = false;
        this.mesh=null;
        this.material=null;
        this.priority = null; // set when insertprop if kept null
        this.need_material_update=false;
        this.need_color_update=false;
        this.need_uv_update=true;
        this.xflip=false;
        this.yflip=false;
        this.fragment_shader= new DefaultColorShader();
        this.remote_vel=null;
        this.draw_offset=vec2.create();
    }
    setVisible(flg) { this.visible=flg; }
    setDeck(dk) { this.deck = dk; this.need_material_update = true; }
    setIndex(ind) { this.index = ind; this.need_uv_update = true; }
    setScl(x,y) { if(y===undefined) vec2.copy(this.scl,x); else vec2.set(this.scl,x,y); }
    setLoc(x,y) { if(y===undefined) vec2.copy(this.loc,x); else vec2.set(this.loc,x,y); }
    setRot(r) { this.rot=r; }
    setUVRot(flg) { this.uvrot=flg; this.need_uv_update = true; }
    setColor(r,g,b,a) {
        if(Color.exactEqualsToValues(this.color,r,g,b,a)==false) {
            this.need_color_update = true;
            if(this.fragment_shader) this.need_material_update = true;
        }
        if(typeof r == 'object' ) {
            Color.copy(this.color,r);
        } else {
            Color.set(this.color,r,g,b,a); 
        }
    }
    setXFlip(flg) { this.xflip=flg; this.need_uv_update = true; }
    setYFlip(flg) { this.yflip=flg; this.need_uv_update = true; }
    setPriority(prio) { this.priority = prio; }
    ensurePrimDrawer() {
        if(!this.prim_drawer) this.prim_drawer = new PrimDrawer();
    }
    addLine(p0,p1,col,w) {
        this.ensurePrimDrawer();
        return this.prim_drawer.addLine(p0,p1,col,w);
    }
    addRect(p0,p1,col,w) {
        this.ensurePrimDrawer();
        return this.prim_drawer.addRect(p0,p1,col,w);
    }
    getPrimById(id) {
        if(!this.prim_drawer)return null;
        return this.prim_drawer.getPrimById(id);
    }
    deletePrim(id) {
        if(this.prim_drawer) this.prim_drawer.deletePrim(id);
    }
    clearPrims() {
        if(this.prim_drawer) this.prim_drawer.clear();
    }
    clearMesh() {
        this.custom_mesh=null;
        this.mesh=null;
    }
    setMesh(mesh) {
        this.custom_mesh=mesh;
    }
    addGrid(g) {
        if(!this.grids) this.grids=[];
        this.grids.push(g);
    }
    setGrid(g) {
        if(this.grids) {
            for(var i=0;i<this.grids.length;i++) {
                if(this.grids[i].id==g.id) {
                    return;
                }
            }
        }
        this.addGrid(g);
    }
    setTexture(tex) {
        var td = new TileDeck();
        td.setTexture(tex);
        var sz = vec2.create();
        tex.getSize(sz);
        td.setSize(1,1,sz[0],sz[1]);
        this.setDeck(td);
        this.setIndex(0);
        this.need_material_update = true;
    }
    setFragmentShader(s) { this.fragment_shader = s;}
    propPoll(dt) { 
        if(this.remote_vel) {
            this.loc[0] += this.remote_vel[0]*dt;
            this.loc[1] += this.remote_vel[1]*dt;
        }
        if( this.prop2DPoll && this.prop2DPoll(dt) == false ) {
            return false;
        }
        return true;
    }
    updateMesh() {
        if(!this.deck)return;
        if(this.index==-1)return;
        if( this.need_material_update ) {
            if(!this.material) {
                if(this.fragment_shader) {
                    this.fragment_shader.updateUniforms(this.deck.moyai_tex.three_tex,this.color);
                    this.material = this.fragment_shader.material;
                } else {
                    this.material = createMeshBasicMaterial({ map: this.deck.moyai_tex.three_tex, depthTest:true, transparent: true, vertexColors:THREE.VertexColors, blending: THREE.NormalBlending });
                }
            } else {
                this.material.map = this.deck.moyai_tex.three_tex;
                if(this.fragment_shader) {
                    this.fragment_shader.updateUniforms(this.deck.moyai_tex.three_tex,this.color);
                }
            }
            this.need_material_update = false;
        }  
        if( this.need_uv_update ) {
            if(!this.uvwork) this.uvwork=new Float32Array(4);
            this.deck.getUVFromIndex(this.uvwork,this.index,0,0,0);
            var u0 = this.uvwork[0], v0 = this.uvwork[1], u1 = this.uvwork[2], v1 = this.uvwork[3];
            if(this.xflip ) {
                var tmp = u1; u1 = u0; u0 = tmp;
            }
            if(this.yflip ) {
                var tmp = v1; v1 = v0; v0 = tmp;
            }
            var uv_p = new THREE.Vector2(u0,v1);
            var uv_q = new THREE.Vector2(u0,v0);
            var uv_r = new THREE.Vector2(u1,v0);
            var uv_s = new THREE.Vector2(u1,v1);
            // Q (u0,v0) - R (u1,v0)      top-bottom upside down.
            //      |           |
            //      |           |                        
            // P (u0,v1) - S (u1,v1)        
            if(this.uvrot) {
                var tmp = uv_p;
                uv_p = uv_s;
                uv_s = uv_r;
                uv_r = uv_q;
                uv_q = tmp;
            }
            
            if(!this.geom) {
                this.geom = createRectGeometry(1,1);
            }
            var uvs = this.geom.faceVertexUvs[0][0];
            if(!uvs) {
                this.geom.faceVertexUvs[0].push([uv_q,uv_s,uv_r]);
                this.geom.faceVertexUvs[0].push([uv_q,uv_p,uv_s]);
            } else {
                uvs[0].x = uv_q.x; uvs[0].y = uv_q.y;
                uvs[1].x = uv_s.x; uvs[1].y = uv_s.y;
                uvs[2].x = uv_r.x; uvs[2].y = uv_r.y;
                uvs = this.geom.faceVertexUvs[0][1];
                uvs[0].x = uv_q.x; uvs[0].y = uv_q.y;
                uvs[1].x = uv_p.x; uvs[1].y = uv_p.y;
                uvs[2].x = uv_s.x; uvs[2].y = uv_s.y;
            }
            
            this.geom.verticesNeedUpdate = true;
            this.geom.uvsNeedUpdate = true;
            this.need_uv_update = false;
        }
        if( this.need_color_update ) {
            //        this.color.r = this.color.g = this.color.b = this.color.a = 1;
            this.geom.faces[0].vertexColors[0] = Color.toTHREEColor(this.color);
            this.geom.faces[0].vertexColors[1] = Color.toTHREEColor(this.color);
            this.geom.faces[0].vertexColors[2] = Color.toTHREEColor(this.color);
            this.geom.faces[1].vertexColors[0] = Color.toTHREEColor(this.color);
            this.geom.faces[1].vertexColors[1] = Color.toTHREEColor(this.color);
            this.geom.faces[1].vertexColors[2] = Color.toTHREEColor(this.color);
            this.need_color_update = false;
        }

        if(!this.mesh) {
            this.mesh = new THREE.Mesh(this.geom,this.material);
        }
    }
    onDelete() {
        if(this.mesh){
            if(this.mesh.geometry) this.mesh.geometry.dispose();
            if(this.mesh.material) this.mesh.material.dispose();
        }
    }
	hit(at,margin) {
        if(margin==undefined)margin=0;
		return ( at[0] >= this.loc[0] - this.scl[0]/2 - margin ) && ( at[0] <= this.loc[0] + this.scl[0]/2 + margin) && ( at[1] >= this.loc[1] - this.scl[1]/2 - margin) && ( at[1] <= this.loc[1] + this.scl[1]/2 + margin );
	}
    hitGrid(at,margin) {
        if(margin==undefined)margin=0;
        for(var i in this.grids) {
            var g = this.grids[i];
            var rt_x = this.scl[0] * g.width;
            var rt_y = this.scl[1] * g.height;
            if( (at[0] >= this.loc[0]-margin) && (at[0] <= this.loc[0]+rt_x+margin) &&
                (at[1] >= this.loc[1]-margin) && (at[1] <= this.loc[1]+rt_y+margin) ) {
                return true;
            }
        }
        return false;
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
    this.visible=true;
    this.enfat_epsilon=0;
    this.parent_prop=null;
    this.mesh=null;
    this.material=null;
    this.geom=null;
    this.need_material_update=false;
    this.need_geometry_update=false;
    // this.fragment_shader  TODO:currently each vertex color alpha is not supported, because of three.js only have vec3 attribute color
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
    this.need_geometry_update = true;
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
    var expect_len = this.width * this.height;
    if(inds.length < expect_len) {
        console.log("bulksetindex: data not enough. expect:",expect_len, "got:",inds.length);
    } else {
        for(var i=0;i<expect_len;i++) this.index_table[i] = inds[i];
        this.need_geometry_update = true;
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
    if(y==undefined) {
        // fill texofs
        for(var i=0;i<this.width*this.height;i++) {
            if(this.texofs_table[i]) vec2.set(this.texofs_table[i],uv); else this.texofs_table[i]=vec2.fromValues(uv[0],uv[1]);
        } 
    } else {
        var uvout=vec2.create();
        vec2.set(uvout,uv);
        this.texofs_table[this.index(x,y)]=uvout;
    }
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
    if(this.color_table[this.index(x,y)]) {
        Color.copy(this.color_table[this.index(x,y)],col);
    } else {
        this.color_table[this.index(x,y)]=Color.fromValues(col[0],col[1],col[2],col[3]);
    }
    this.need_geometry_update = true;
}
Grid.prototype.getColor = function(outary,x,y) {
    if(!this.color_table) outary[0]=outary[1]=outary[2]=outary[3]=1;
    var col=this.color_table[this.index(x,y)];
    if(col) {
        Color.copy(outary,col);
    } else {
        outary[0]=outary[1]=outary[2]=outary[3]=1;
    }
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
                this.color_table[this.index(x,y)] = Color.fromValues(c[0],c[1],c[2],c[3]);
            }
        }
    }
    this.need_geometry_update = true;
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

var g_debug_grid_alpha_message=false;
Grid.prototype.updateMesh = function() {
    if(!this.deck) {
//        console.log("grid.updateMesh: deck is null?", this.deck, this.id );
        return;
    }
    if(!this.index_table) {
//        console.log("grid.updateMesh: index_table is null?", this, "grid_id:",this.id );
        return;
    }
    
    if(this.need_material_update) {
        this.need_material_update=false;
        if(!this.material) {
            this.material = createMeshBasicMaterial({map: this.deck.moyai_tex.three_tex, transparent:true, depthTest:true, vertexColors:THREE.VertexColors, blending: THREE.NormalBlending });
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
                geom.vertices.push(new THREE.Vector3(x-this.enfat_epsilon,y+1+this.enfat_epsilon,0)); //0
                geom.vertices.push(new THREE.Vector3(x+this.enfat_epsilon+1,y+this.enfat_epsilon+1, 0)); //1
                geom.vertices.push(new THREE.Vector3(x+this.enfat_epsilon+1,y-this.enfat_epsilon, 0)); //2
                geom.vertices.push(new THREE.Vector3(x-this.enfat_epsilon,y-this.enfat_epsilon, 0)); //3
                // 1セルあたり2面づつ
                var face_start_vert_ind = quad_cnt*4;
                geom.faces.push(new THREE.Face3(face_start_vert_ind+0, face_start_vert_ind+2, face_start_vert_ind+1));
                geom.faces.push(new THREE.Face3(face_start_vert_ind+0, face_start_vert_ind+3, face_start_vert_ind+2));
                
                var left_bottom, right_top;
                if(!this.uvwork) this.uvwork=new Float32Array(4);
                this.deck.getUVFromIndex(this.uvwork,this.index_table[ind],0,0,0);
                var u0 = this.uvwork[0], v0 = this.uvwork[1], u1 = this.uvwork[2], v1 = this.uvwork[3];

                if(this.texofs_table && this.texofs_table[ind]) {
                    var u_per_cell = this.deck.getUperCell();
                    var v_per_cell = this.deck.getVperCell();
                    u0 += this.texofs_table[ind][0] * u_per_cell;
                    v0 += this.texofs_table[ind][1] * v_per_cell;
                    u1 += this.texofs_table[ind][0] * u_per_cell;
                    v1 += this.texofs_table[ind][1] * v_per_cell;
                }

                if(this.xflip_table && this.xflip_table[ind]) {
                    var tmp = u1; u1 = u0; u0 = tmp;
                }
                if(this.yflip_table && this.yflip_table[ind]) {
                    var tmp = v1; v1 = v0; v0 = tmp;
                }
                var uv_p = new THREE.Vector2(u0,v1);
                var uv_q = new THREE.Vector2(u0,v0);
                var uv_r = new THREE.Vector2(u1,v0);
                var uv_s = new THREE.Vector2(u1,v1);
                if(this.rot_table && this.rot_table[ind]) {
                    var tmp = uv_p;
                    uv_p = uv_s;
                    uv_s = uv_r;
                    uv_r = uv_q;
                    uv_q = tmp;
                }                
                geom.faceVertexUvs[0].push([uv_q,uv_s,uv_r]);
                geom.faceVertexUvs[0].push([uv_q,uv_p,uv_s]);
                var col; 
                if( this.color_table && this.color_table[ind] ) {
                    col = Color.toTHREEColor(this.color_table[ind]);
                    if(this.color_table[ind].a < 1.0 ) {
                        if(!g_debug_grid_alpha_message) {
                            console.log("alpha blending in grid cell is not implemented yet (THREE.js dont have vert color alpha)");
                            g_debug_grid_alpha_message=true;
                        }
                    }
                } else {
                    col = new THREE.Color("#fff");
                }
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

try {
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
} catch(e) {
    console.log("Can't init FTFuncs. no freetype available");
}


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
    this.image = new MoyaiImage();
    this.image.setSize(this.width,this.height);
    for(var y=0;y<this.height;y++) {
        for(var x=0;x<this.width;x++) {
            var pixdata = this.data[x+y*this.width]
            this.image.setPixelRaw(x,y,pixdata,pixdata,pixdata,pixdata);
        }
    }
    this.moyai_tex = new Texture();
    this.moyai_tex.setImage(this.image);
    this.moyai_tex.three_tex.magFilter = THREE.LinearFilter;
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
Font.prototype.setCharCodes = function(codes_str) { this.charcode_table = codes_str; }
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
    var horiz_num = Math.floor(Math.floor(this.atlas.width) / Math.floor(this.pixel_size));
    var vert_num = Math.floor(Math.floor(this.atlas.height) / Math.floor(this.pixel_size));
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
        var start_y = Math.floor(i / horiz_num) * (this.pixel_size);

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
class TextBox extends Prop2D {
    constructor() {
        super();
        this.font = null;
        this.scl = vec2.fromValues(1,1);
        this.str = null;
        this.geom=null;
        this.material=null;
        this.need_geometry_update=false;
        this.need_material_update=false;
        this.dimension=2;
    }
    setFont(fnt) { this.font = fnt; this.need_material_update=true; }
    setString(s) {
        this.str = s;
        this.need_geometry_update=true;
    }
    getString(s) { return str; }
    updateMesh() {
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

            geom.faces[used_chind*2+0].vertexColors[0] = Color.toTHREEColor(this.color);
            geom.faces[used_chind*2+0].vertexColors[1] = Color.toTHREEColor(this.color);
            geom.faces[used_chind*2+0].vertexColors[2] = Color.toTHREEColor(this.color);
            geom.faces[used_chind*2+1].vertexColors[0] = Color.toTHREEColor(this.color);
            geom.faces[used_chind*2+1].vertexColors[1] = Color.toTHREEColor(this.color);
            geom.faces[used_chind*2+1].vertexColors[2] = Color.toTHREEColor(this.color);
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

// good sprintf is not found in web.. please construct string by yourself
CharGrid.prototype.print = function(x,y,col,s) {
	for(var i=0;i<s.length;i++){
		var ind = this.ascii_offset + s.charCodeAt(i);
		if(x+i>=this.width)break;
		this.set(x+i,y,ind);
		this.setColor(x+i,y,col);
	}    
}

/////////////////////////////
var vertex_vcolor_glsl =
    "varying vec4 vColor;\n"+
    "attribute vec3 color;\n"+
    "void main()\n"+
    "{\n"+
    "  vColor = vec4(color,1);\n"+
    "  vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);\n"+
    "  gl_Position = projectionMatrix * mvPosition;\n"+
    "}\n";    
var fragment_vcolor_glsl = 
    "uniform vec4 meshcolor;\n"+
    "varying vec4 vColor;\n"+    
    "void main()\n"+
    "{\n"+
    "  gl_FragColor = meshcolor;//vec4(1,0,1,1);\n"+
    "}\n";
//    
var vertex_uv_color_glsl =
    "varying vec2 vUv;\n"+
    "varying vec4 vColor;\n"+
    "attribute vec3 color;\n"+
    "void main()\n"+
    "{\n"+
    "  vUv = uv;\n"+
    "  vColor = vec4(color,1);\n"+
    "  vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);\n"+
    "  gl_Position = projectionMatrix * mvPosition;\n"+
    "}\n";
var fragment_uv_color_glsl =
    "uniform sampler2D texture;\n"+
    "uniform vec4 meshcolor;\n"+
    "varying vec2 vUv;\n"+
    "varying vec4 vColor;\n"+    
    "void main()\n"+
    "{\n"+
    "  vec4 tc = texture2D(texture,vUv);\n"+
    "  gl_FragColor = vec4( tc.r * meshcolor.r, tc.g * meshcolor.g, tc.b * meshcolor.b, tc.a * meshcolor.a );\n"+
    "}\n";

var fragment_replacer_glsl = 
	"uniform sampler2D texture;\n"+
    "varying vec2 vUv;\n"+
	"varying vec4 vColor;\n"+
	"uniform vec3 color1;\n"+    
	"uniform vec3 replace1;\n"+
	"uniform float eps;\n"+
	"void main() {\n"+
	"	vec4 pixel = texture2D(texture, vUv); \n"+
	"	if( pixel.r > color1.r - eps && pixel.r < color1.r + eps && pixel.g > color1.g - eps && pixel.g < color1.g + eps && pixel.b > color1.b - eps && pixel.b < color1.b + eps ){\n"+
	"		pixel = vec4(replace1, pixel.a );\n"+
	"    }\n"+
	"   pixel.r = vColor.r * pixel.r;\n"+
	"   pixel.g = vColor.g * pixel.g;\n"+
	"   pixel.b = vColor.b * pixel.b;\n"+
	"   pixel.a = vColor.a * pixel.a;\n" +   
	"	gl_FragColor = pixel;\n"+
	"}\n";

FragmentShader.prototype.id_gen=1;
function FragmentShader() {
    this.id=this.__proto__.id_gen++;
    this.uniforms=null;
    this.material=null;
    this.vsh_src=vertex_uv_color_glsl; 
    this.fsh_src=null;
}
FragmentShader.prototype.updateMaterial = function() {
    if(!this.material) {
        this.material = new THREE.ShaderMaterial( {
            uniforms : this.uniforms,
            vertexShader : this.vsh_src,
            fragmentShader : this.fsh_src,
            blending : THREE.NormalBlending,
            transparent: true
        });
    } else {
        this.material.uniforms = this.uniforms;
        this.material.needsUpdate = true;
    }
}
ColorReplacerShader.prototype = Object.create(FragmentShader.prototype);
ColorReplacerShader.prototype.constructor = ColorReplacerShader;
function ColorReplacerShader() {
    FragmentShader.call(this);
    this.fsh_src = fragment_replacer_glsl;
    this.setColor(new THREE.Vector3(0,0,0),new THREE.Vector3(0,1,0),0.01);
}
// updateUniforms(tex) called when render
ColorReplacerShader.prototype.updateUniforms = function(texture) {
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
    this.updateMaterial();    
}
ColorReplacerShader.prototype.setColor = function(from,to,eps) {
    this.epsilon = eps;
    this.from_color = new THREE.Vector3(from[0],from[1],from[2]);
    this.to_color = new THREE.Vector3(to[0],to[1],to[2]);
    this.updateUniforms();
}
DefaultColorShader.prototype = Object.create(FragmentShader.prototype);
DefaultColorShader.prototype.constructor = DefaultColorShader;
function DefaultColorShader() {
    FragmentShader.call(this);
    this.fsh_src = fragment_uv_color_glsl;
}
DefaultColorShader.prototype.updateUniforms = function(texture,moyaicolor) {
    if(this.uniforms) {
        if(texture) this.uniforms["texture"]["value"] = texture;
        this.uniforms["meshcolor"]["value"] = new THREE.Vector4(moyaicolor[0], moyaicolor[1], moyaicolor[2], moyaicolor[3] );
    } else {
        this.uniforms = {
            "texture" : { type: "t", value: texture },
            "meshcolor" : { type: "v4", value: new THREE.Vector4(moyaicolor[0], moyaicolor[1], moyaicolor[2], moyaicolor[3] ) }
        };
    }
    this.updateMaterial();
}
PrimColorShader.prototype = Object.create(FragmentShader.prototype);
PrimColorShader.prototype.constructor = PrimColorShader;
function PrimColorShader() {
    FragmentShader.call(this);
    this.fsh_src = fragment_vcolor_glsl;
    this.vsh_src = vertex_vcolor_glsl;
}
PrimColorShader.prototype.updateUniforms = function(moyaicolor) {
    if(this.uniforms) {
        this.uniforms["meshcolor"]["value"] = new THREE.Vector4(moyaicolor[0], moyaicolor[1], moyaicolor[2], moyaicolor[3] );
    } else {
        this.uniforms = {
            "meshcolor" : { type: "v4", value: new THREE.Vector4(moyaicolor[0], moyaicolor[1], moyaicolor[2], moyaicolor[3] ) }
        };
    }
    this.updateMaterial();    
}
//////////////////////
function Keyboard() {
    this.keys={};
    this.toggled={};
    this.mod_shift=false;
    this.mod_ctrl=false;
    this.mod_alt=false;
    this.prevent_default=false;
    this.to_read_event=true;
}
Keyboard.prototype.enableReadEvent = function(flg) { this.to_read_event=flg; }
Keyboard.prototype.setKey = function(keycode,pressed) {
    this.keys[keycode] = pressed;
    if(!pressed) {
        this.keys[keycode.toUpperCase()]=false;
        this.keys[keycode.toLowerCase()]=false;        
    }
    if(pressed &&  (!this.toggled[keycode]) ) {
        this.toggled[keycode]=true;
    } else {
        this.toggled[keycode]=false;
    }
}
Keyboard.prototype.getKey = function(keycode) {
    return this.keys[keycode];
}
Keyboard.prototype.getToggled = function(keycode) {
    return this.toggled[keycode];
}
Keyboard.prototype.clearToggled = function(keycode) {
    this.toggled[keycode]=false;
}
function safariKey(e) {
    console.log("safari:",e);    
    if(e.keyIdentifier=="Enter") return "Enter";
    if(e.keyIdentifier=="Right") return "ArrowRight";
    if(e.keyIdentifier=="Left") return "ArrowLeft";
    if(e.keyIdentifier=="Down") return "ArrowDown";
    if(e.keyIdentifier=="Up") return "ArrowUp";        
    if(e.keyIdentifier=="U+0008") return "Backspace";
    if(e.keyIdentifier=="U+001B") return "Escape";    
    var k=String.fromCharCode(e.keyCode);
    if(!e.shiftKey)k=k.toLowerCase();
    return k;
}
Keyboard.prototype.readBrowserEvent = function(e,pressed) {
    var id=e.key;
    if(!id)id=safariKey(e);
    if(this.onKeyEvent) {
        var keep=this.onKeyEvent(id,pressed,e);
        if(!keep)return;
    }
    this.setKey(id,pressed);
    if(e.key=="Control") this.mod_ctrl = pressed;
    if(e.key=="Shift") this.mod_shift = pressed;
    if(e.key=="Alt") this.mod_alt = pressed;

}
Keyboard.prototype.setupBrowser = function(w) {
    var _this = this;
    w.addEventListener("keydown", function(e) {
        if(_this.prevent_default) e.preventDefault();
        if(_this.to_read_event) _this.readBrowserEvent(e,true);
    }, false);
    w.addEventListener("keyup", function(e) {
        if(_this.preventDefault) e.preventDefault();
        if(_this.to_read_event) _this.readBrowserEvent(e,false);    
    });
}
Keyboard.prototype.setPreventDefault = function(flg) { this.prevent_default=flg; }

/////////////////////
function Mouse() {
    this.cursor_pos=vec2.create();
    this.movement=vec2.create();
    this.buttons={};
    this.toggled={};
    this.mod_shift=false;
    this.mod_ctrl=false;
    this.mod_alt=false;
}
Mouse.prototype.clearMovement = function() { vec2.set(this.movement,0,0); }
Mouse.prototype.setupBrowser = function(w,dom) {
    var _this = this;
    w.addEventListener("mousedown", function(e) {
//        e.preventDefault();
        _this.readButtonEvent(e,true);
    },false);
    w.addEventListener("mouseup", function(e)  {
//        e.preventDefault();
        _this.readButtonEvent(e,false);        
    },false);
    w.addEventListener("mousemove", function(e)  {
        var rect = dom.getBoundingClientRect();
        var x = Math.floor(e.clientX - rect.left);
        var y = Math.floor(e.clientY - rect.top);
//        e.preventDefault();
        vec2.set(_this.cursor_pos,x,y);
        vec2.set(_this.movement,e.movementX, e.movementY);
    },false);    
}
Mouse.prototype.readButtonEvent = function(e,pressed) {
    if(pressed) {
        if(!this.buttons[e.button]) this.toggled[e.button] = true;
    }
    this.buttons[e.button] = pressed;
    this.mod_shift = e.shiftKey;
    this.mod_alt = e.altKey;
    this.mod_ctrl = e.ctrlKey;
}
Mouse.prototype.getButton = function(btn_ind) {
    return this.buttons[btn_ind];
}
Mouse.prototype.getToggled = function(btn_ind) {
    return this.toggled[btn_ind];
}
Mouse.prototype.clearToggled = function(btn_ind) {
    this.toggled[btn_ind] = false;        
}
Mouse.prototype.getCursorPos = function() { return this.cursor_pos; }

/////////////////////////

function SoundSystem() {
    var AudioContext = window.AudioContext // Default
    || window.webkitAudioContext // Safari and old versions of Chrome
    || false; 

    if (AudioContext) {
        this.context = new AudioContext();
    } else {
        console.log("AudioContext is not available in this browser");
        this.context = null;
    }
    this.sounds={};
    this.master_volume = 1;
}
SoundSystem.prototype.setMasterVolume = function(vol) { this.master_volume=vol; }
SoundSystem.prototype.getMasterVolume = function() { return this.master_volume; }
// type: "float" or other, "wav", "mp3"..
SoundSystem.prototype.newBGMFromMemory = function(data,type) {
    var snd = this.createSound(data,true,type);
    this.sounds[snd.id] = snd;
    return snd;
}
SoundSystem.prototype.newSoundFromMemory = function(data,type) {
    var snd = this.createSound(data,false,type);
    this.sounds[snd.id] = snd;
    return snd;
}
SoundSystem.prototype.createSound = function(data,loop,type) {
    var snd = new Sound();
    snd.sound_system = this;
    snd.context=this.context;
    snd.setLoop(loop);
    snd.setData(data,type);
    return snd;
}

Sound.prototype.id_gen=1;
function Sound(data,loop,type) {
    this.id = this.__proto__.id_gen++;
    this.type=null;
    this.data=null;
    this.loop=false;
    this.audiobuffer=null;
    this.context=null;
    this.default_volume=1;
    this.source=null;
    this.play_volume=null;
    this.sound_system=null;
}
Sound.prototype.setLoop = function(loop) { this.loop=loop; }
Sound.prototype.isReady = function() { return this.audiobuffer; }
Sound.prototype.setDefaultVolume = function(v) { this.default_volume=v;}
Sound.prototype.setData = function(data,type) {
    if(!this.context)return;
    this.type = type;
    this.data = data;
    if(type=="float") {
        this.audiobuffer = this.context.createBuffer( 1, data.length, this.context.sampleRate );
        var b = this.audiobuffer.getChannelData(0); // channel 0
        for (var i = 0; i < data.length; i++) {
            b[i] = data[i];
        }
    } else {
        var _this = this;
        this.context.decodeAudioData(data.buffer, function(decoded) {
            _this.audiobuffer = decoded;
        })
    }
}
Sound.prototype.prepareSource = function(vol,detune) {
    if(!this.context)return;
    if(!detune)detune=0;
    
    if(this.source) {
        this.source.stop();
    }
    this.source = this.context.createBufferSource();
    this.source.buffer = this.audiobuffer;
    if(this.source.detune) this.source.detune.value=detune; // browser dependent
    var thissnd=this;
    this.source.onended = function() { thissnd.source.ended=true; }
    this.gain_node = this.context.createGain();
    this.source.connect(this.gain_node);
    this.gain_node.connect(this.context.destination);
    this.gain_node.gain.value = this.default_volume * vol * this.sound_system.master_volume;
}
Sound.prototype.play = function(vol,detune) {
    if(!this.context)return;
    if(vol==undefined)vol=1;
    if(this.audiobuffer) {
        this.prepareSource(vol,detune);
        this.source.start(0);
        this.play_volume=vol;
    } else {
        console.log("Sound.play: audiobuffer is not ready");
    }
}
Sound.prototype.setTimePositionSec = function( pos_sec ) {
    if(!this.context)return;    
    if(this.source) {
        if(this.source.paused) {
            return;
        } else {
            this.source.stop();
            this.prepareSource(this.play_volume);
            this.source.start(0,pos_sec);
        }
    }
}
Sound.prototype.isPlaying = function() {
    if(!this.context)return false;    
    if(this.source) {
        if(this.source.paused) return false;
        if(this.source.ended ) return false;  // set by moyai
        return true;
    } else {
        return false;
    }        
}
Sound.prototype.stop = function() {
    if(!this.context)return;    
    if(this.source) {
        console.log("stopping..", this.source);
        this.source.stop(0);
    } 
}
///////////////////////

function FileDepo() {
    this.files = {};
}
FileDepo.prototype.get = function(path) {
    return this.files[path];
}
FileDepo.prototype.ensure = function(path,data) {
    return this.files[path] = data;
}


/////////////////////////
class Prop3D extends Prop {
    constructor() {
        super();
        this.scl = vec3.fromValues(1,1,1);
        this.loc = vec3.fromValues(0,0,0);
        this.rot = vec3.fromValues(0,0,0);
        this.mesh=null;
        this.sort_center = vec3.fromValues(0,0,0);
	    this.depth_mask=true;
        this.alpha_test=false;
        this.cull_back_face=true;
        this.draw_offset = vec3.fromValues(0,0,0);
        this.priority=this.id;
        this.dimension=3;
        this.visible=true;        
    }
    propPoll(dt) {
        if(this.prop3DPoll && this.prop3DPoll(dt)==false) {
            return false;
        }
        return true;
    }
    setVisible(flg) { this.visible=flg; }    
}
Prop3D.prototype.setMesh = function(m) {this.mesh=m;}
Prop3D.prototype.setGroup = function(g) { this.mesh=g;}
Prop3D.prototype.setScl = function(x,y,z) {
    if(y===undefined) {
        vec3.copy(this.scl,x);
    } else {
        vec3.set(this.scl,x,y,z);   
    }
}
Prop3D.prototype.setLoc = function(x,y,z) {
    if(y===undefined) vec3.copy(this.loc,x); else vec3.set(this.loc,x,y,z);
}
Prop3D.prototype.setRot = function(x,y,z) {
    if(y===undefined) vec3.copy(this.rot,x); else vec3.set(this.rot,x,y,z); 
}


