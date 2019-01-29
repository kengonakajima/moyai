// moyai js port

class OrthographicCamera {
    constructor(left,right,top,bottom,near,far) {
        this.left=left;
        this.right=right;
        this.top=top;
        this.bottom=bottom;
        this.near= near ? near:-1;
        this.far= far ? far : g_moyai_max_z;
        this.loc=vec2.create();
    }
    setLoc(x,y) {
        vec2.set(this.loc,x,y);
    }
};

class PerspectiveCamera {
    constructor(fov,aspect,near,far) {
        this.fov=fov;
        this.aspect=aspect;
        this.near=near;
        this.far=far;
	    this.look_at=vec3.create();
	    this.look_up=vec3.create();
        this.loc=vec3.create();
        this.camMat=mat4.create();
        this.invCamMat=mat4.create();
        this.invCamRotMat=mat4.create();
        this.invQuat=quat.create();
        this.projMat=mat4.create();
        this.viewProjMat=mat4.create();
        this.billboard=false;
    }
    setLoc(x,y,z) {
        if(y===undefined) vec3.copy(this.loc,x); else vec3.set(this.loc,x,y,z);
    }
    setLookAt(at,up) {
        vec3.copy(this.look_at,at);
        vec3.copy(this.look_up,up);
    }
    getDirection(outv) {
        vec3.subtract(outv, this.look_at,this.loc);
    }
    updateMatrix() {
        mat4.lookAt(this.camMat,this.loc,this.look_at,this.look_up);
        mat4.perspective(this.projMat, this.fov, this.aspect, this.near, this.far );
        // get quaternion for billboard
        mat4.invert(this.invCamMat,this.camMat);
        mat3.fromMat4(this.invCamRotMat,this.invCamMat);
        quat.fromMat3(this.invQuat,this.invCamRotMat);

        mat4.multiply(this.viewProjMat,this.projMat,this.camMat);
        if(!this.planes) { this.planes=[]; for(var i=0;i<6;i++) this.planes[i]=new Float32Array(4); }
        extractPlanes2(this.planes,this.viewProjMat);
    }    
};

// copied from three math
mat4.compose = function(out, position, quaternion, scale) {
	var x = quaternion[0], y = quaternion[1], z = quaternion[2], w = quaternion[3];
	var x2 = x + x,	y2 = y + y, z2 = z + z;
	var xx = x * x2, xy = x * y2, xz = x * z2;
	var yy = y * y2, yz = y * z2, zz = z * z2;
	var wx = w * x2, wy = w * y2, wz = w * z2;
	var sx = scale[0], sy = scale[1], sz = scale[2];

	out[ 0 ] = ( 1 - ( yy + zz ) ) * sx;
	out[ 1 ] = ( xy + wz ) * sx;
	out[ 2 ] = ( xz - wy ) * sx;
	out[ 3 ] = 0;

	out[ 4 ] = ( xy - wz ) * sy;
	out[ 5 ] = ( 1 - ( xx + zz ) ) * sy;
	out[ 6 ] = ( yz + wx ) * sy;
	out[ 7 ] = 0;

	out[ 8 ] = ( xz + wy ) * sz;
	out[ 9 ] = ( yz - wx ) * sz;
	out[ 10 ] = ( 1 - ( xx + yy ) ) * sz;
	out[ 11 ] = 0;

	out[ 12 ] = position[0];
	out[ 13 ] = position[1];
	out[ 14 ] = position[2];
	out[ 15 ] = 1;
	return this;
}
// from three
mat4.decompose = function(outpos,outquat,outscl, inmat) {
    var v=vec3.create();//TODO avoid allocation
    vec3.set(v, inmat[ 0 ], inmat[ 1 ], inmat[ 2 ] );
	var sx = vec3.length(v);
    vec3.set(v, inmat[ 4 ], inmat[ 5 ], inmat[ 6 ] );
	var sy = vec3.length(v);
    vec3.set(v, inmat[ 8 ], inmat[ 9 ], inmat[ 10 ] );
	var sz = vec3.length(v);

	// if determine is negative, we need to invert one scale
	var det = mat4.determinant(inmat);
	if ( det < 0 ) sx = - sx;

	outpos[0] = inmat[ 12 ];
	outpos[1] = inmat[ 13 ];
	outpos[2] = inmat[ 14 ];

	// scale the rotation part
    var m=mat3.create(); // TODO avoid allocation
    mat3.fromMat4(m,inmat);
    
	var invSX = 1 / sx;
	var invSY = 1 / sy;
	var invSZ = 1 / sz;

	m[ 0 ] *= invSX;
	m[ 1 ] *= invSX;
	m[ 2 ] *= invSX;

	m[ 3 ] *= invSY;
	m[ 4 ] *= invSY;
	m[ 5 ] *= invSY;

	m[ 6 ] *= invSZ;
	m[ 7 ] *= invSZ;
	m[ 8 ] *= invSZ;

	quat.fromMat3(outquat,m);

	outscl[0] = sx;
	outscl[1] = sy;
	outscl[2] = sz;
}


///////////////
Moyai ={ initialized:false }
Moyai.init = function(w,h){
    this.width=w;
    this.height=h;
    this.canvas=document.createElementNS( 'http://www.w3.org/1999/xhtml', 'canvas' );
    this.canvas.width=w;
    this.canvas.height=h;
    this.gl=this.canvas.getContext("webgl",{antialias: false});
    if(!this.gl) {
        console.warn("no WebGL support");
        this.gl = null;
    }
    this.clearColor=Color.fromValues(0.1,0.1,0.1,1);    
    this.enable_clear=true;
    this.layers=[];
    this.x_axis=vec3.fromValues(1,0,0);
    this.y_axis=vec3.fromValues(0,1,0);
    this.z_axis=vec3.fromValues(0,0,1);
    this.initialized=true;
}
Moyai.setSize = function(w,h) {
    this.width=w;
    this.height=h;
}
Moyai.setClearColor = function(col) {
    Color.fromValues(this.clearColor, col[0],col[1],col[2],col[3]);
}
Moyai.getDomElement = function() {
    return this.canvas;
}
Moyai.getHighestPriority = function() {
    var highp=0;
    for(var i=0;i<this.layers.length;i++) {
        if(this.layers[i].priority>highp) highp = this.layers[i].priority;
    }
    return highp;
}
Moyai.poll = function(dt) {
    if(!this.initialized)return 0;
    var cnt=0;
    for(var i=0;i<this.layers.length;i++) {
        var layer = this.layers[i];
        if( layer && (!layer.skip_poll) ) cnt += layer.pollAllProps(dt);
    }
    return cnt;   
}
Moyai.render = function() {
    if(!this.initialized)return;
    var gl=this.gl;
    if(this.enable_clear) {
        gl.clearColor(0.0, 0.0, 0.0, 1.0);
        gl.clearDepth(1.0);        
    }
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);// 近くにある物体は、遠くにある物体を覆い隠す
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.BLEND);

    this.draw_count_3d=this.skip_count_3d=0;
    for(var li=0;li<this.layers.length;li++) {
        var layer = this.layers[li];
        if(layer.viewport.dimension==3) {
            this.render3D(layer);
        }
    }

    gl.enable(gl.CULL_FACE);
    gl.cullFace(gl.BACK);    
    // then 2d            
    for(var li=0;li<this.layers.length;li++) {
        var layer = this.layers[li];
        if(layer.viewport.dimension==2) {
            this.render2D(layer);
        }
    }
}
// hartmann/gribbs loop expanded
function extractPlanes2(out,m) {
    out[0][0]=m[3]-m[0]; out[0][1]=m[7]-m[4]; out[0][2]=m[11]-m[8]; out[0][3]=m[15]-m[12]; // left
    out[1][0]=m[3]+m[0]; out[1][1]=m[7]+m[4]; out[1][2]=m[11]+m[8]; out[1][3]=m[15]+m[12]; // right
    out[2][0]=m[3]+m[1]; out[2][1]=m[7]+m[5]; out[2][2]=m[11]+m[9]; out[2][3]=m[15]+m[13]; // bottom
    out[3][0]=m[3]-m[1]; out[3][1]=m[7]-m[5]; out[3][2]=m[11]-m[9]; out[3][3]=m[15]-m[13]; // top
    out[4][0]=m[3]-m[2]; out[4][1]=m[7]-m[6]; out[4][2]=m[11]-m[10]; out[4][3]=m[15]-m[14]; // near
    out[5][0]=m[3]+m[2]; out[5][1]=m[7]+m[6]; out[5][2]=m[11]+m[10]; out[5][3]=m[15]+m[14]; // far
}
Moyai.cull_min_loc=vec3.create();
Moyai.cull_max_loc=vec3.create();
Moyai.workv0=vec3.create();
Moyai.workv1=vec3.create();
Moyai.workv2=vec3.create();
Moyai.workv3=vec3.create();
Moyai.render3D = function(layer) {
    var cam=layer.camera;
    cam.updateMatrix();
    
//    mat4.perspective(layer.projMat, 0.5, 1.3333, 1,1000);
//    mat4.ortho(layer.projMat, -3, 3, -5, 5, -1, 1000);
    for(var pi=0;pi<layer.props.length;pi++ ) {                    
        var prop = layer.props[pi];
        if(!prop.visible)continue;
        if(prop.to_clean)continue;

        // view frustum culling http://www.sousakuba.com/Programming/gs_dot_plane_distance.html
        var to_skip=false;
        if(prop.enable_frustum_culling ){
            if(prop.geom && prop.geom.cull_center) {
                vec3.add(this.workv0, prop.loc, prop.geom.cull_center);
//                console.log("cc:", this.workv0, prop.loc, prop.geom.cull_center);
                var outcnt=0;
                for(var j=0;j<6;j++) {
                    var dot=vec3.dot(this.workv0,cam.planes[j]);
                    var distance=dot+cam.planes[j][3];

                    if(distance<-prop.geom.cull_diameter*prop.geom.cull_diameter) {
                        to_skip=true;
                        break;
                    }
                }
            } else {
                for(var j=0;j<6;j++) {
                    var dot=vec3.dot(prop.loc,cam.planes[j]);
                    var distance=dot+cam.planes[j][3];
                    if(distance<0) { to_skip=true; break;}
                }
            }
        }
        
        if(to_skip) {
            this.skip_count_3d++;
            continue;
        }
        
        if(prop.billboard) quat.copy(prop.quaternion,cam.invQuat);
        
        prop.updateModelViewMatrix();
        if(prop.geom) {
            prop.geom.bless();
            this.draw(prop.geom, prop.mvMat, cam.viewProjMat, prop.material, prop.moyai_tex, prop.color, prop.use_additive_blend,prop.cull_face,prop.depth_mask);
            this.draw_count_3d++;
        }

        if(prop.children.length>0) {
            for(var i=0;i<prop.children.length;i++) {
                var chp = prop.children[i];
                if(!chp.visible)continue;
                chp.updateModelViewMatrix(prop.mvMat);                
                chp.geom.bless();
                this.draw(chp.geom, chp.mvMat, cam.viewProjMat, chp.material, chp.moyai_tex, chp.color, chp.use_additive_blend, chp.cull_face,chp.depth_mask);
                this.draw_count_3d++;
            }
        }
    }
}
Moyai.render2D = function(layer) {
    var cam=layer.camera;
    if(!cam)return;
    if(!layer.projMat) layer.projMat=mat4.create();
    mat4.ortho(layer.projMat, cam.left, cam.right, cam.bottom, cam.top, cam.near, cam.far );
    if(!cam.camMat) cam.camMat=mat4.create();
    mat4.identity(cam.camMat);
    if(!cam.loc3) cam.loc3=vec3.create();
    vec3.set(cam.loc3,cam.loc[0],cam.loc[1],0);
    mat4.translate(cam.camMat,cam.camMat, cam.loc3);
    var vp=layer.viewport;
    if(!vp.scl3) vp.scl3=vec3.create();
    vec3.set(vp.scl3,vp.scl[0]/vp.screen_width,vp.scl[1]/vp.screen_height,1);
    mat4.scale(cam.camMat,cam.camMat,vp.scl3);
    if(!this.viewProjMat)this.viewProjMat=mat4.create();
    mat4.multiply(this.viewProjMat,layer.projMat,cam.camMat);

    for(var pi=0;pi<layer.props.length;pi++ ) {                    
        var prop = layer.props[pi];
        if(!prop.visible)continue;
        if(prop.to_clean)continue;        
        prop.updateModelViewMatrix();
        if(!prop.use_custom_geometry) prop.updateGeom();
        if(prop.geom) prop.geom.bless();                    

        var prop_z = layer.priority * g_moyai_z_per_layer + prop.priority * g_moyai_z_per_prop;
        if(prop.grids) {
            for(var gi=0;gi<prop.grids.length;gi++) {
                var grid = prop.grids[gi];
                if(!grid.visible)continue;
                if(!grid.deck)grid.setDeck(prop.deck);
                grid.updateGeom();
                if(grid.geom) grid.geom.bless();
                var tex;
                if(grid.deck) tex=grid.deck.moyai_tex; else tex=prop.deck.moyai_tex;
                if(prop.debug) console.log("debug_moyai_prop_grid:",prop.geom,prop.deck);                
                this.draw(grid.geom, prop.mvMat, this.viewProjMat, prop.material, tex, prop.color, prop.use_additive_blend);
            }
        }
        if(prop.children.length>0) {
            for(var i=0;i<prop.children.length;i++) {
                var chp = prop.children[i];
                if(!chp.visible)continue;
                chp.updateModelViewMatrix();                
                chp.updateGeom();
                if(chp.geom) chp.geom.bless();
                if(chp.debug) console.log("debug_moyai_childprop:",chp.geom,chp.deck);                
                this.draw(chp.geom, chp.mvMat, this.viewProjMat, chp.material, chp.deck.moyai_tex, chp.color, chp.use_additive_blend);
            }
        }
        if(prop.debug) console.log("debug_moyai_prop:",prop.geom,prop.deck);
        
        if(prop.geom && prop.deck) {
            this.draw(prop.geom, prop.mvMat, this.viewProjMat, prop.material, prop.deck.moyai_tex, prop.color,prop.use_additive_blend);
        }            
        if(prop.prim_drawer) {
            for(var i=0;i<prop.prim_drawer.prims.length;i++) {
                var prim = prop.prim_drawer.prims[i];
                vec3.set(Moyai.workv0, prop.loc[0],prop.loc[1],0);
                vec3.set(Moyai.workv1, prop.scl[0],prop.scl[1],1);
                prim.updateModelViewMatrix(Moyai.workv0, Moyai.workv1);
                prim.updateGeom();
                if(prim.geom) prim.geom.bless();
                this.draw(prim.geom, prim.mvMat, this.viewProjMat, prim.material, null, null, prim.use_additive_blend);
            }
        }            
    }
}
Moyai.draw = function(geom,mvMat,projMat,material,moyai_tex,colv,additive_blend,cull_face,depth_mask) {
//    if(geom.stride_colors==3)  console.warn("draw:",geom,mvMat,projMat,material,moyai_tex,colv,additive_blend);
    var gl=Moyai.gl;
    gl.useProgram(material.glprog);
    gl.uniformMatrix4fv( material.uniformLocations.projectionMatrix, false, projMat ); // TODO: put it out        

    // pos
    if(!geom.positionBuffer) console.warn("no posbuf",geom);
    gl.bindBuffer(gl.ARRAY_BUFFER, geom.positionBuffer);
    gl.vertexAttribPointer( material.attribLocations.position, 3, gl.FLOAT, false,0,0);
    gl.enableVertexAttribArray(material.attribLocations.position);
    // color
    if(geom.colorBuffer) {
        if(!geom.colorBuffer) console.warn("no colbuf",geom);
        gl.bindBuffer(gl.ARRAY_BUFFER, geom.colorBuffer);
        gl.vertexAttribPointer( material.attribLocations.color, geom.stride_colors, gl.FLOAT, false,0,0 );
        gl.enableVertexAttribArray(material.attribLocations.color);
        Moyai.last_color_attr_loc=material.attribLocations.color;
    } else {
        gl.disableVertexAttribArray(Moyai.last_color_attr_loc);
    }
    // ind
    if(!geom.indexBuffer) console.warn("no indbuf",geom);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, geom.indexBuffer);
    // setup shader
    gl.uniformMatrix4fv( material.uniformLocations.modelViewMatrix, false, mvMat);
    if(moyai_tex) {
        var gltex=moyai_tex.gltex;
        // uv
        if(!geom.uvBuffer) console.warn("no uvbuf",geom);
        gl.bindBuffer(gl.ARRAY_BUFFER, geom.uvBuffer);
        gl.vertexAttribPointer(material.attribLocations.uv, 2, gl.FLOAT, false,0,0 );
        gl.enableVertexAttribArray(material.attribLocations.uv );
        Moyai.last_uv_attr_loc=material.attribLocations.uv;
        gl.activeTexture(gl.TEXTURE0);

        gl.bindTexture(gl.TEXTURE_2D, gltex);
        gl.uniform1i(material.uniformLocations.texture,0);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, moyai_tex.min_filter);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, moyai_tex.mag_filter);                            
    } else {
        gl.disableVertexAttribArray(Moyai.last_uv_attr_loc);
    }
    if(colv) gl.uniform4fv(material.uniformLocations.meshcolor, colv);

    if(material.applyUniforms) material.applyUniforms();
    // draw
    var indn=geom.indn_used;
    if(indn===undefined) indn=geom.indn;
    //    console.log("draw:",geom,mvMat,projMat,material,gltex,colv,fn);
    if(additive_blend) {
        gl.blendFunc(gl.ONE,gl.ONE);
    } else {
        gl.blendFuncSeparate(gl.ONE,gl.ONE_MINUS_SRC_ALPHA, gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
//        gl.blendFunc(gl.ONE,gl.ONE_MINUS_SRC_ALPHA);
    }
    if(!cull_face) {
        gl.disable(gl.CULL_FACE);
    } else {
        gl.enable(gl.CULL_FACE);
        gl.cullFace(cull_face);
    }
    if(depth_mask) {
        gl.depthMask(true);
    } else {
        gl.depthMask(false);
    }
    
    if(geom.primtype==gl.TRIANGLES) {
        gl.drawElements(gl.TRIANGLES, indn, gl.UNSIGNED_SHORT, 0);
    } else if(geom.primtype==gl.LINES) {
        gl.drawElements(gl.LINES, indn, gl.UNSIGNED_SHORT, 0);        
    }
    if(0) {
        var e=gl.getError();
        if(e!=gl.NO_ERROR) {
            var msg="unknown";
            switch(e) {
            case gl.INVALID_ENUM: msg="invalid enum"; break;
            case gl.INVALID_VALUE: msg="invalid value"; break;
            case gl.INVALID_OPERATION: msg="invalid operation"; break;
            case gl.INVALID_FRAMEBUFFER_OPERATION: msg="invalid fb op"; break;
            case gl.OUT_OF_MEMORY: msg="out of mem"; break;
            case gl.CONTEXT_LOST_WEBGL: msg="context lost webgl"; break;
            }
            console.log("glerror:",e,msg, geom );
        }
    }
}
    
Moyai.insertLayer = function(l) {
    if(l.priority==null) {
        var highp = this.getHighestPriority();
        l.priority = highp+1;
    }
    this.layers.push(l);
}


//////////////////////////
function isPowerOf2(value) {
  return (value & (value - 1)) == 0;
}
function createGLTextureFromPixels(gl,w,h,data) {
    var t=gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, t);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, w, h, 0, gl.RGBA, gl.UNSIGNED_BYTE, data);
    return t;        
}
function setImageToGLTexture(gl,texture,image) {
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);

    // WebGL1 has different requirements for power of 2 images
    // vs non power of 2 images so check if the image is a
    // power of 2 in both dimensions.
    if (isPowerOf2(image.width) && isPowerOf2(image.height)) {
        // Yes, it's a power of 2. Generate mips.
        gl.generateMipmap(gl.TEXTURE_2D);
    } else {
        // No, it's not a power of 2. Turn of mips and set
        // wrapping to clamp to edge
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    }
}

var g_moyai_texture_id_gen = 1;
class Texture {
    constructor() {
        this.id = g_moyai_texture_id_gen++;
        this.image = null;
        this.gltex = null;
        this.min_filter=Moyai.gl.NEAREST;
        this.mag_filter=Moyai.gl.NEAREST;
    }
    loadPNG(url,w,h) {
        if(w===undefined||h===undefined) console.warn("loadPNG require width and height currently");
        var gl=Moyai.gl;

        const pixel = new Uint8Array(w*h*4);
        for(var i=0;i<w*h*4;i++) pixel[i]=0xff;
        var texture=createGLTextureFromPixels(gl, w,h,pixel);
        var _this=this;
        var moyai_tex=this;
        var image = new Image();
        image.width=w;
        image.height=h;
        image.onload = function() {
            setImageToGLTexture(gl,texture,image);
            //        console.log("loadpng: onload:",texture,image,moyai_tex);
            if(moyai_tex.onload) moyai_tex.onload();
            // get pixel data and store
            var canvas=document.createElement("canvas");
            canvas.width=w;
            canvas.height=h;
            var ctx=canvas.getContext("2d");
            ctx.drawImage(this,0,0);
            var imgdata=ctx.getImageData(0,0,w,h);
            image.data=imgdata.data;
            image.canvas=canvas;
            moyai_tex.moyai_image=new MoyaiImage(image);            
        };
        image.src = url;
        this.gltex=texture;
        this.image=image;

    }    
    getSize(out) {
        return vec2.set(out,this.image.width,this.image.height);
    }
    setMoyaiImage(moimg) {
        var canvas = document.createElement('canvas'),
        ctx = canvas.getContext('2d');
        canvas.width = moimg.width;
        canvas.height = moimg.height;
        var imgdata = ctx.createImageData(moimg.width,moimg.height);
        imgdata.data.set(moimg.data);
        ctx.putImageData(imgdata,0,0);
        var datauri=canvas.toDataURL();
        var image=new Image();
        image.width=moimg.width;
        image.height=moimg.height;

        var gl=Moyai.gl;
        var texture;
        if(this.gltex) {
            if(image.width!=moimg.width || image.height!=moimg.height) {
                console.error("setMoyaiImage: updating image, but size differs! ",moimg.width,moimg.height,image.width,image.height);
                return;
            }
            texture=this.gltex;
        } else {
            texture= createGLTextureFromPixels(gl,moimg.width, moimg.height, moimg.data);            
        }
        var moyai_tex=this;
        image.onload = function() {
            setImageToGLTexture(gl,texture,image);
            //        console.log("loadpng: onload:",texture,image,moyai_tex);
            if(moyai_tex.onload) moyai_tex.onload();
        }
        image.src=datauri;
        this.image=image;
        this.moyai_image=moimg;
        this.gltex=texture;
    }
};


///////

var PRIMTYPE_NONE = 0;
var PRIMTYPE_LINE = 1;
var PRIMTYPE_RECTANGLE = 2;

var g_moyai_prim_id_gen=1;
class Prim {
    constructor(t,a,b,col,lw) {
        this.id=g_moyai_prim_id_gen++;
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
        this.material = getPrimColorShaderMaterial();
    }
    updateModelViewMatrix(locv3,sclv3) {
        if(!this.mvMat) this.mvMat=mat4.create();
        mat4.identity(this.mvMat);
        mat4.translate(this.mvMat,this.mvMat,locv3);
        mat4.scale(this.mvMat,this.mvMat,sclv3);
    }
    updateGeom() {
        if(this.type==PRIMTYPE_LINE) {
            if(!this.geom) this.geom=new LineGeometry(2,1);
            this.geom.setPosition(0, this.a[0],this.a[1],0);
            this.geom.setPosition(1, this.b[0],this.b[1],0);
            this.geom.need_positions_update=true;
            this.geom.setColor4v(0, this.color );
            this.geom.setColor4v(1, this.color );
            this.geom.need_colors_update=true;
            this.geom.setIndex(0,0);
            this.geom.setIndex(1,1);
            this.geom.need_inds_update=true;
        } else if(this.type==PRIMTYPE_RECTANGLE) {
            /*
              0--1
              |\ |  0:a 2:b
              | \|
              3--2
            */
            if(!this.geom) this.geom=new FaceGeometry(4,2);
            this.geom.setPosition(0, this.a[0],this.a[1],0);
            this.geom.setPosition(1, this.b[0],this.a[1],0);
            this.geom.setPosition(2, this.b[0],this.b[1],0);
            this.geom.setPosition(3, this.a[0],this.b[1],0);
            this.geom.need_positions_update=true;
            
            if( (this.a[0]<this.b[0] && this.a[1]<this.b[1]) || (this.a[0]>this.b[0] && this.a[1]>this.b[1]) ) {
                this.geom.setFaceInds(0, 0, 1, 2);
                this.geom.setFaceInds(1, 0, 2, 3);
            } else {
                this.geom.setFaceInds(0, 0, 2, 1);
                this.geom.setFaceInds(1, 0, 3, 2);
            }
            this.geom.setColor4v(0,this.color);
            this.geom.setColor4v(1,this.color);
            this.geom.setColor4v(2,this.color);
            this.geom.setColor4v(3,this.color);        
        } else {
            console.warn("invalid prim type",this.type)
        }
    }
    onDelete() {
    }
};


//////////////////
class PrimDrawer {
    constructor() {
        this.prims=[];        
    }
    addLine(a,b,col,w) {
        var newprim = new Prim(PRIMTYPE_LINE,a,b,col,w);
        this.prims.push(newprim);
        return newprim;
    }
    addRect(a,b,col,w) {
        var newprim = new Prim(PRIMTYPE_RECTANGLE,a,b,col,w);
        this.prims.push(newprim);
        return newprim;
    }
    getPrimById(id) {
        for(var i=0;i<this.prims.length;i++) {
            if(this.prims[i].id == id ) return this.prims[i];
        }
        return null;
    }
    deletePrim(id) {
        for(var i=0;i<this.prims.length;i++) {
            if(this.prims[i].id == id ) {
                this.prims[i].onDelete();
                this.prims.splice(i,1);
                return;
            }
        }
    }
    ensurePrim(p) {
        var existing = this.getPrimById(p.id);
        if(existing){
            existing.type = p.type;
            vec2.copy(existing.a,p.a);
            vec2.copy(existing.b,p.b);
            existing.color=p.color;
            existing.line_width=p.line_width;
            existing.updateGeom();
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
    clear() {
        for(var i=0;i<this.prims.length;i++) {
            this.prims[i].onDelete();
        }
        this.prims=[];
    }
};

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

class Geometry {
    constructor(vn,indn) {
        this.cull_center=vec3.create();
        this.cull_diameter=1;        
        if(vn===undefined || indn===undefined) return; // size is specified later
        this.ensureSize(vn,indn);
    }
    ensureSize(vn,indn) {
        this.vn=vn;
        this.indn=indn;
        this.positions=new Float32Array(vn*3);
        this.colors=new Float32Array(vn*4);
        this.stride_colors=4;
        this.inds=new Uint16Array(indn);
    }
    dispose() {
        var gl=Moyai.gl;
        if(this.positionBuffer) {
            gl.deleteBuffer(this.positionBuffer);
            this.positionBuffer=null;
            this.need_positions_update=true;
        }
        if(this.colorBuffer) {
            gl.deleteBuffer(this.colorBuffer);
            this.colorBuffer=null;
            this.need_colors_update=true;
        }
        if(this.indexBuffer) {
            gl.deleteBuffer(this.indexBuffer);
            this.indexBuffer=null;            
            this.need_inds_update=true;
        }
        if(this.uvBuffer) {
            gl.deleteBuffer(this.uvBuffer);
            this.uvBuffer=null;
            this.need_uvs_update=true;
        }
    }
    setPositionArray(ary,vn) { this.positions=ary; this.need_positions_update=true; this.vn=vn; }
    setColorArray(ary,stride) {this.colors=ary; this.stride_colors=stride; this.need_colors_update=true;}
    setUVArray(ary) { this.uvs=ary; this.need_uvs_update=true; }
    setIndexArray(ary) { this.inds=ary; this.indn=ary.length; this.need_inds_update=true; }
    setPosition3v(vind,p) {
        this.positions[vind*3]=p[0];
        this.positions[vind*3+1]=p[1];
        this.positions[vind*3+2]=p[2];        
    }
    setPosition(vind,x,y,z) {
        this.positions[vind*3]=x;
        this.positions[vind*3+1]=y;
        this.positions[vind*3+2]=z;
    }
    setIndex(ind,val) {
        this.inds[ind]=val;
    }
    setColor(vind,r,g,b,a) {
        this.colors[vind*4]=r;
        this.colors[vind*4+1]=g;
        this.colors[vind*4+2]=b;
        this.colors[vind*4+3]=a;
    }
    setColor4v(vind,v4) {
        this.colors[vind*4]=v4[0];
        this.colors[vind*4+1]=v4[1];
        this.colors[vind*4+2]=v4[2];
        this.colors[vind*4+3]=v4[3];        
    }
    fillColor4v(ofs,col,n) {
        for(var i=ofs;i<ofs+n;i++) this.setColor4v(i,col);
    }
    bless() {
        var gl=Moyai.gl;
        if(this.need_positions_update) {
            if(this.debug) console.log("bless position:", this );
            if(!this.positions) console.warn("bless: need positions!");
            if(!this.positionBuffer) this.positionBuffer=gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, this.positionBuffer);
            gl.bufferData(gl.ARRAY_BUFFER,this.positions, gl.STATIC_DRAW);
            this.need_positions_update=false;
        }
        if(this.need_colors_update) {
            if(!this.colors) console.warn("bless: need colors!");
            if(!this.colorBuffer)this.colorBuffer=gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER,this.colorBuffer);
            gl.bufferData(gl.ARRAY_BUFFER,this.colors, gl.STATIC_DRAW);
            this.need_colors_update=false;
        }
        if(this.need_inds_update) {
            if(!this.inds) console.warn("bless: need inds!");
            if(!this.indexBuffer)this.indexBuffer=gl.createBuffer();
            gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);
            gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, this.inds, gl.STATIC_DRAW);
            this.need_inds_update=false;
        }
        if(this.need_uvs_update) {
            if(!this.uvs) console.warn("bless: need uvs!");
            if(!this.uvBuffer)this.uvBuffer=gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, this.uvBuffer);
            gl.bufferData(gl.ARRAY_BUFFER, this.uvs, gl.STATIC_DRAW);
            this.need_uvs_update=false;
        }
    }
    setCullSize(center,diameter) {
        vec3.copy(this.cull_center,center);
        this.cull_diameter=diameter;
    }
}
class LineGeometry extends Geometry {
    constructor(vn,ln) {
        super(vn,ln*2)
        this.need_colors_update=true;
        this.need_positions_update=true;
        this.need_inds_update=true;
        this.primtype=Moyai.gl.LINES;        
    }
};
class FaceGeometry extends Geometry {
    constructor(vn,fn) {
        super(vn,fn*3);
        this.uvs=new Float32Array(vn*2);
        this.need_positions_update=true;
        this.need_inds_update=true;
        this.need_uvs_update=true;
        this.need_colors_update=true;
        this.primtype=Moyai.gl.TRIANGLES;        
    }
    setFaceInds(find,a,b,c) {
        this.inds[find*3]=a;
        this.inds[find*3+1]=b;
        this.inds[find*3+2]=c;        
    }
    setUV(vind,u,v) {
        this.uvs[vind*2]=u;
        this.uvs[vind*2+1]=v;            
    }
    setUV2v(vind,uv) {
        this.uvs[vind*2]=uv[0];
        this.uvs[vind*2+1]=uv[1];            
    }
};

class BoxGeometry extends Geometry {
    constructor(backface) {
        super(24,6*2*3);
        this.positions=new Float32Array([ 0.5, 0.5, 0.5,    // xp
                                          0.5, 0.5, -0.5,
                                          0.5, -0.5, 0.5,
                                          0.5, -0.5, -0.5,
                                          // xn
                                          -0.5, 0.5, -0.5, 
                                          -0.5, 0.5, 0.5,
                                          -0.5, -0.5, -0.5,
                                          -0.5,-0.5, 0.5,
                                          // yp
                                          -0.5, 0.5, -0.5, 
                                          0.5, 0.5, -0.5,
                                          -0.5, 0.5, 0.5,
                                          0.5, 0.5, 0.5,
                                          // yn
                                          -0.5, -0.5, 0.5, 
                                          0.5, -0.5, 0.5,
                                          -0.5, -0.5, -0.5,
                                          0.5, -0.5, -0.5,
                                          // zp
                                          -0.5, 0.5, 0.5, // E 
                                          0.5, 0.5, 0.5,  // F
                                          -0.5, -0.5, 0.5, //A
                                          0.5, -0.5, 0.5, // B
                                          // zn
                                          0.5, 0.5, -0.5, 
                                          -0.5, 0.5, -0.5,
                                          0.5, -0.5, -0.5,
                                          -0.5, -0.5, -0.5,
                                        ]);
        this.inds=new Uint16Array([
            0, 2, 1, // xp
            2, 3, 1,
            4, 6, 5, // xn
            6, 7, 5,
            8, 10, 9, //yp
            10, 11, 9,

            12, 14, 13, // yn
            14, 15, 13,
            16, 18, 17, // zp
            18, 19, 17,
            20, 22, 21, // zn
            22, 23, 21,
        ]);
        if(backface) {
            for(var i=0;i<12;i++) {
                var tmp=this.inds[i*3+2];
                this.inds[i*3+2]=this.inds[i*3+1];
                this.inds[i*3+1]=tmp;
            }
        }
        
        this.need_positions_update=true;
        this.need_inds_update=true;
        this.primtype=Moyai.gl.TRIANGLES;        
    }
}





function createRectGeometry(width,height) {
    var geometry = new FaceGeometry(4,2);
    var sizeHalfX = width / 2;
    var sizeHalfY = height / 2;
    /*
      0--1
      |\ |
      | \|
      3--2
     */
    geometry.setPosition(0, -sizeHalfX, sizeHalfY, 0);
    geometry.setPosition(1, sizeHalfX, sizeHalfY, 0); 
    geometry.setPosition(2, sizeHalfX, -sizeHalfY, 0);
    geometry.setPosition(3, -sizeHalfX, -sizeHalfY, 0);
    geometry.setColor(0,1,1,1,1);
    geometry.setColor(1,1,1,1,1);
    geometry.setColor(2,1,1,1,1);
    geometry.setColor(3,1,1,1,1);    
    geometry.setFaceInds(0, 0,2,1);
    geometry.setFaceInds(1, 0,3,2);
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
        this.material=null;
        this.priority = null; // set when insertprop if kept null
        this.need_color_update=false;
        this.need_uv_update=true;
        this.xflip=false;
        this.yflip=false;
        this.material= getDefaultColorShaderMaterial();
        this.remote_vel=null;
        this.draw_offset=vec2.create();
        this.geom=null;
        this.depth_mask=true;
        this.mvMat=mat4.create();
    }
    setVisible(flg) { this.visible=flg; }
    setDeck(dk) { this.deck = dk; }
    setGeom(g) {this.geom=g; this.use_custom_geometry=true; }    
    setMaterial(mat) { this.material=mat;}
    setIndex(ind) { this.index = ind; this.need_uv_update = true; this.use_custom_geometry=false; }
    setScl(x,y) {
        if(y===undefined) {
            if(isNaN(x) ) vec2.copy(this.scl,x); else vec2.set(this.scl,x,x);
        } else {
            vec2.set(this.scl,x,y);
        }
    }
    setLoc(x,y) {
        if(y===undefined) {
            if(isNaN(x)) vec2.copy(this.loc,x); else vec2.set(this.loc,x,x);
        } else {
            vec2.set(this.loc,x,y);
        }
    }
    setRot(r) { this.rot=r; }
    setUVRot(flg) { this.uvrot=flg; this.need_uv_update = true; }
    setColor(r,g,b,a) {
        if(Color.exactEqualsToValues(this.color,r,g,b,a)==false) {
            this.need_color_update = true;
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
    }
    propPoll(dt) { 
        if(this.remote_vel) {
            this.loc[0] += this.remote_vel[0]*dt;
            this.loc[1] += this.remote_vel[1]*dt;
        }
        if( this.prop2DPoll && this.prop2DPoll(dt) === false ) {
            return false;
        }
        return true;
    }
    updateModelViewMatrix() {
        mat4.identity(this.mvMat);
        vec3.set(Moyai.workv0, this.loc[0]+this.draw_offset[0],this.loc[1]+this.draw_offset[1],0);
        mat4.translate(this.mvMat,this.mvMat,Moyai.workv0);
        mat4.rotate(this.mvMat,this.mvMat,this.rot,Moyai.z_axis);
        vec3.set(Moyai.workv0, this.scl[0],this.scl[1],1);
        mat4.scale(this.mvMat,this.mvMat, Moyai.workv0 );
    }
    clearGeom() {
        this.geom=null;
        this.need_uv_update=this.need_color_update=true;
    }
    updateGeom() {
        if(!this.deck)return;
        if(this.index==-1)return;
        if(!this.geom) this.geom = createRectGeometry(1,1);
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
            if(!this.uv_p) {
                this.uv_p = vec2.fromValues(u0,v1);
                this.uv_q = vec2.fromValues(u0,v0);
                this.uv_r = vec2.fromValues(u1,v0);
                this.uv_s = vec2.fromValues(u1,v1);
            } else {
                vec2.set(this.uv_p,u0,v1);
                vec2.set(this.uv_q,u0,v0);
                vec2.set(this.uv_r,u1,v0);
                vec2.set(this.uv_s,u1,v1);
            }


            
            // Q (u0,v0) - R (u1,v0)      top-bottom upside down.
            //      |           |
            //      |           |                        
            // P (u0,v1) - S (u1,v1)        
            if(this.uvrot) {
                this.geom.setUV(0,this.uv_p[0],this.uv_p[1]);
                this.geom.setUV(1,this.uv_q[0],this.uv_q[1]);
                this.geom.setUV(2,this.uv_r[0],this.uv_r[1]);
                this.geom.setUV(3,this.uv_s[0],this.uv_s[1]);
            } else {
                this.geom.setUV(0,this.uv_q[0],this.uv_q[1]);
                this.geom.setUV(1,this.uv_r[0],this.uv_r[1]);
                this.geom.setUV(2,this.uv_s[0],this.uv_s[1]);
                this.geom.setUV(3,this.uv_p[0],this.uv_p[1]);
            }            
            this.geom.need_uvs_update=true;
            this.need_uv_update = false;
        }
        if( this.need_color_update ) {
            this.geom.setColor4v(0,this.color);
            this.geom.setColor4v(1,this.color);
            this.geom.setColor4v(2,this.color);
            this.geom.setColor4v(3,this.color);
            this.geom.need_colors_update=true;
            this.need_color_update = false;
        }
    }
    onDelete() {
        if(this.geom){
            this.geom.dispose();
            this.geom=null;
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
const GRID_NOT_USED = -1;
var g_moyai_grid_id_gen=1;
class Grid {
    constructor(w,h) {
        this.id=g_moyai_grid_id_gen++;
        this.width=w;
        this.height=h;
        this.index_table=new Int16Array(w*h);
        for(var i=0;i<w*h;i++) this.index_table[i]=GRID_NOT_USED;
        this.xflip_table=new Uint8Array(w*h); // 0,1
        this.yflip_table=new Uint8Array(w*h); // 0,1
        this.texofs_table=new Float32Array(w*h*2); // u,v,u,v,u,v,..
        this.rot_table=new Uint8Array(w*h); // 0,1
        this.color_table=new Float32Array(w*h*4); // r,g,b,a,r,g,b,a,..
        for(var i=0;i<w*h*4;i++) this.color_table[i]=1;
        this.deck=null;
        this.visible=true;
        this.enfat_epsilon=0;
        this.parent_prop=null;
        this.material=null;
        this.geom=null;
        this.need_geometry_update=true;
    }
    setDeck(dk) { this.deck=dk;}
    index(x,y) { return x+y*this.width; }
    getCellNum() { return this.width * this.height; }
    set(x,y,ind) {
        this.index_table[this.index(x,y)] = ind;
        this.need_geometry_update = true;
    }
    get(x,y) {
        return this.index_table[ this.index(x,y) ];
    }
    bulkSetIndex(inds) {
        var expect_len = this.width * this.height;
        if(inds.length < expect_len) {
            console.warn("bulksetindex: data not enough. expect:",expect_len, "got:",inds.length);
            return;
        }
        for(var i=0;i<expect_len;i++) this.index_table[i] = inds[i];
        this.need_geometry_update = true;
    }
    bulkSetFlipRotBits(xflbits,yflbits,uvrotbits) {
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
    bulkSetTexofs(ofsary) {
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
    bulkSetColor(colsary) {
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
    setXFlip(x,y,flg) {
        this.xflip_table[this.index(x,y)]=flg;
        this.need_geometry_update = true;
    }
    getXFlip(x,y) {
        return this.xflip_table[this.index(x,y)];
    }
    setYFlip(x,y,flg) {
        this.yflip_table[this.index(x,y)]=flg;
        this.need_geometry_update = true;
    }
    getYFlip(x,y) {
        return this.yflip_table[this.index(x,y)];
    }
    setTexOffset(x,y,uv) {
        this.texofs_table[this.index(x,y)*2+0]=uv[0];
        this.texofs_table[this.index(x,y)*2+1]=uv[1];
        this.need_geometry_update = true;
    }
    getTexOffset(outv,x,y) {
        outv[0]=this.texofs_table[this.index(x,y)*2+0];
        outv[1]=this.texofs_table[this.index(x,y)*2+1];
    }
    setUVRot(x,y,flg) {
        this.rot_table[this.index(x,y)]=flg;
        this.need_geometry_update = true;
    }
    getUVRot(x,y) {
        return this.rot_table[this.index(x,y)];
    }
    setColor(x,y,col) {
        var ind=this.index(x,y)*4;
        this.color_table[ind]=col[0];
        this.color_table[ind+1]=col[1];
        this.color_table[ind+2]=col[2];
        this.color_table[ind+3]=col[3];
        this.need_geometry_update = true;
    }
    getColor(outary,x,y) {
        var ind=this.index(x,y)*4;
        outary[0]=this.color_table[ind];
        outary[1]=this.color_table[ind+1];
        outary[2]=this.color_table[ind+2];
        outary[3]=this.color_table[ind+3];    
    }
    setVisible(flg) { this.visible=flg; }
    getVisible() { return this.visible; }
    clear(x,y) {
        if(x===undefined){
            for(var i=0;i<this.width*this.height;i++) this.index_table[i]=GRID_NOT_USED;
        } else {
            this.set(x,y,GRID_NOT_USED);
        }    
    }
    fillColor(col) {
        for(var i=0;i<this.width*this.height;i++) {
            this.color_table[i*4] = col[0];
            this.color_table[i*4+1] = col[1];
            this.color_table[i*4+2] = col[2];
            this.color_table[i*4+3] = col[3];        
        }
        this.need_geometry_update = true;
    }
    fill(ind) {
        this.fillRect(0,0,this.width-1,this.height-1,ind);
    }
    fillRect(x0,y0,x1,y1,ind) {
        for(var y=y0;y<=y1;y++) {
            for(var x=x0;x<=x1;x++) {
                this.set(x,y,ind);
            }
        }    
    }
    updateGeom() {
        if(!this.deck) return;
        if(!this.geom) this.geom = new FaceGeometry(this.width*this.height*4, this.width*this.height*2);
        if(!this.need_geometry_update) return;
        this.need_geometry_update = false;
        var geom=this.geom, eps=this.enfat_epsilon;
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
                geom.setPosition(quad_cnt*4+0, x-eps,y+1+eps,0);
                geom.setPosition(quad_cnt*4+1, x+eps+1,y+eps+1, 0);
                geom.setPosition(quad_cnt*4+2, x+eps+1,y-eps, 0); 
                geom.setPosition(quad_cnt*4+3, x-eps,y-eps, 0);
                
                // 1セルあたり2面づつ
                var face_start_vert_ind = quad_cnt*4;
                geom.setFaceInds(quad_cnt*2, face_start_vert_ind+0, face_start_vert_ind+2, face_start_vert_ind+1);
                geom.setFaceInds(quad_cnt*2+1, face_start_vert_ind+0, face_start_vert_ind+3, face_start_vert_ind+2);
                
                var left_bottom, right_top;
                if(!this.uvwork) this.uvwork=new Float32Array(4);
                this.deck.getUVFromIndex(this.uvwork,this.index_table[ind],0,0,0);
                var u0 = this.uvwork[0], v0 = this.uvwork[1], u1 = this.uvwork[2], v1 = this.uvwork[3];

                var u_per_cell = this.deck.getUperCell();
                var v_per_cell = this.deck.getVperCell();
                u0 += this.texofs_table[ind*2+0] * u_per_cell;
                v0 += this.texofs_table[ind*2+1] * v_per_cell;
                u1 += this.texofs_table[ind*2+0] * u_per_cell;
                v1 += this.texofs_table[ind*2+1] * v_per_cell;

                if(this.xflip_table[ind]) {
                    var tmp = u1; u1 = u0; u0 = tmp;
                }
                if(this.yflip_table[ind]) {
                    var tmp = v1; v1 = v0; v0 = tmp;
                }
                if(!this.uv_p) {
                    this.uv_p = vec2.create();
                    this.uv_q = vec2.create();
                    this.uv_r = vec2.create();
                    this.uv_s = vec2.create();
                }
                vec2.set(this.uv_p,u0,v1);
                vec2.set(this.uv_q,u0,v0);
                vec2.set(this.uv_r,u1,v0);
                vec2.set(this.uv_s,u1,v1);
                
                //            console.log("gridset:",x,y,this.uv_p,this.uv_q,this.uv_r, this.uv_s);
                if(this.rot_table[ind]) {
                    geom.setUV(quad_cnt*4+0,this.uv_p[0],this.uv_p[1]);
                    geom.setUV(quad_cnt*4+1,this.uv_q[0],this.uv_q[1]);
                    geom.setUV(quad_cnt*4+2,this.uv_r[0],this.uv_r[1]);
                    geom.setUV(quad_cnt*4+3,this.uv_s[0],this.uv_s[1]);
                } else {
                    geom.setUV(quad_cnt*4+0,this.uv_q[0],this.uv_q[1]);
                    geom.setUV(quad_cnt*4+1,this.uv_r[0],this.uv_r[1]);
                    geom.setUV(quad_cnt*4+2,this.uv_s[0],this.uv_s[1]);
                    geom.setUV(quad_cnt*4+3,this.uv_p[0],this.uv_p[1]);
                }
                var r=this.color_table[ind*4];
                var g=this.color_table[ind*4+1];
                var b=this.color_table[ind*4+2];
                var a=this.color_table[ind*4+3];
                
                geom.setColor(quad_cnt*4,r,g,b,a);
                geom.setColor(quad_cnt*4+1,r,g,b,a);
                geom.setColor(quad_cnt*4+2,r,g,b,a);
                geom.setColor(quad_cnt*4+3,r,g,b,a);
                quad_cnt++;
            }
            geom.indn_used=quad_cnt*2*3;
        }
        geom.need_positions_update=true;
        geom.need_inds_update=true;
        geom.need_uvs_update=true;
        geom.need_colors_update=true;
    }
};

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
class TextureAtlas {
    constructor(w,h,depth) {
        this.width = w;
        this.height = h;
        this.depth = depth;
        this.data = new Uint8Array(w*h*depth);
        this.moyai_image = null;
        this.moyai_tex=null;
    }
    dump(ofsx,ofsy, w,h) {
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
    ensureTexture() {
        this.moyai_image = new MoyaiImage();
        this.moyai_image.setSize(this.width,this.height);
        for(var y=0;y<this.height;y++) {
            for(var x=0;x<this.width;x++) {
                var pixdata = this.data[x+y*this.width]
                this.moyai_image.setPixelRaw(x,y,pixdata,pixdata,pixdata,pixdata);
            }
        }
        this.moyai_tex = new Texture();
        this.moyai_tex.setMoyaiImage(this.moyai_image);
        this.moyai_tex.mag_filter=Moyai.gl.LINEAR;
    }
};

    // 0:left-top 1:right-bottom
class Glyph {
    constructor(l,t,w,h,adv,u0,v0,u1,v1,charcode,dbg) {
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
    }
};

var g_moyai_font_id_gen=1;
class Font {
    constructor() {
        this.id=g_moyai_font_id_gen++;
        this.font = null;
	    this.atlas = null;
        this.charcode_table = [];
        this.glyphs={};
    }
    setCharCodes(codes_str) { this.charcode_table = codes_str; }

    loadFromTTFFile(url,codes,pxsz) {
        var _this=this;
        var req=new XMLHttpRequest();
        req.open("GET",url);
        req.responseType="arraybuffer";
        req.send();
        req.onload = function() {
            var res=req.response;
            var u8a=new Uint8Array(res);
            console.log("loadFromTTFFile res:",res,u8a);
            _this.loadFromMemTTF(u8a,codes,pxsz);
        }
    }
    
    loadFromMemTTF(u8a,codes,pxsz) {
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
    loadGlyphs(codes) {
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
    getGlyph(code) {
        return this.glyphs[code];
    }
};

//////////////////
class TextBox extends Prop2D {
    constructor() {
        super();
        this.font = null;
        this.deck = null;
        this.scl = vec2.fromValues(1,1);
        this.str = null;
        this.geom=null;
        this.material=getDefaultColorShaderMaterial();
        this.need_geometry_update=false;
        this.dimension=2;
        this.last_string_length=0;
    }
    setFont(fnt) { this.font = fnt;  }
    setString(s) {
        if(this.str) this.last_string_length=this.str.length;
        this.str = s;
        this.need_geometry_update=true;
    }
    getString(s) { return str; }
    updateGeom() {
        if(!this.font)return;
        if(!this.font.atlas)return;
        this.deck=this.font.atlas;        
        if(!this.need_geometry_update)return;        
        this.need_geometry_update = false;
        
        if(!this.geom || this.last_string_length < this.str.length ) {
            console.log("textbox is expanding. new str:",this.str);
            if(this.geom) this.geom.dispose();
            this.geom = new FaceGeometry(this.str.length*4,this.str.length*2);
        }
        var geom=this.geom;

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
                continue;
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
            geom.setPosition(used_chind*4, cur_x+l,cur_y+t,0); 
            geom.setPosition(used_chind*4+1, cur_x+l+w,cur_y+t,0); 
            geom.setPosition(used_chind*4+2, cur_x+l+w,cur_y+t-h,0);
            geom.setPosition(used_chind*4+3, cur_x+l,cur_y+t-h,0); 
            var face_start_vert_ind = used_chind*4;
            geom.setFaceInds(used_chind*2, face_start_vert_ind+0, face_start_vert_ind+2, face_start_vert_ind+1);
            geom.setFaceInds(used_chind*2+1, face_start_vert_ind+0, face_start_vert_ind+3, face_start_vert_ind+2);
            // uvは左上が0,右下が1
            geom.setUV(used_chind*4, glyph.u0,glyph.v0);
            geom.setUV(used_chind*4+1, glyph.u1,glyph.v0);
            geom.setUV(used_chind*4+2, glyph.u1,glyph.v1);
            geom.setUV(used_chind*4+3, glyph.u0,glyph.v1);

            geom.setColor4v(used_chind*4, this.color);
            geom.setColor4v(used_chind*4+1, this.color);
            geom.setColor4v(used_chind*4+2, this.color);
            geom.setColor4v(used_chind*4+3, this.color);
            
            cur_x += glyph.advance;
            used_chind++;
        }
        geom.need_positions_update=true;
        geom.need_uvs_update=true;
        geom.need_colors_update=true;
        geom.need_inds_update=true;
    }
}


/////////////////
function str_repeat(i, m) {
    for (var o = []; m > 0; o[--m] = i);
    return o.join('');
}

class CharGrid extends Grid {
    constructor(w,h) {
        super(w,h);
        this.ascii_offset = 0;
    }
    setAsciiOffset(ofs) { this.ascii_offset = ofs; }
    // good sprintf is not found in web.. please construct string by yourself
    print(x,y,col,s) {
	    for(var i=0;i<s.length;i++){
		    var ind = this.ascii_offset + s.charCodeAt(i);
		    if(x+i>=this.width)break;
		    this.set(x+i,y,ind);
		    this.setColor(x+i,y,col);
	    }    
    }
};

/////////////////////////////
var vertex_vcolor_glsl =
    "attribute vec4 color;\n"+
    "attribute vec3 position;\n"+
    "varying vec4 vColor;\n"+    
    "uniform mat4 modelViewMatrix;\n"+
    "uniform mat4 projectionMatrix;\n"+    
    "void main()\n"+
    "{\n"+
    "  vColor = color;\n"+    
    "  vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);\n"+
    "  gl_Position = projectionMatrix * mvPosition;\n"+
    "}\n";    
var fragment_vcolor_glsl = 
    "varying highp vec4 vColor;\n"+        
    "void main()\n"+
    "{\n"+
//    "  gl_FragColor = vColor;//vec4(1,0,1,1);\n"+
    "  gl_FragColor = vec4( vColor.r * vColor.a, vColor.g * vColor.a, vColor.b * vColor.a, vColor.a);\n"+    
    "}\n";
//    
var vertex_uv_color_glsl =
    "varying vec2 vUv;\n"+
    "varying vec4 vColor;\n"+
    "attribute vec4 color;\n"+
    "attribute vec2 uv;\n"+
    "attribute vec3 position;\n"+
    "uniform mat4 modelViewMatrix;\n"+
    "uniform mat4 projectionMatrix;\n"+    
    "void main()\n"+
    "{\n"+
    "  vUv = uv;\n"+
    "  vColor = color;\n"+
    "  vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);\n"+
    "  gl_Position = projectionMatrix * mvPosition;\n"+
    "}\n";
var fragment_uv_color_glsl =
    "uniform sampler2D texture;\n"+
    "uniform highp vec4 meshcolor;\n"+
    "varying highp vec2 vUv;\n"+
    "varying highp vec4 vColor;\n"+    
    "void main()\n"+
    "{\n"+
    "  highp vec4 tc = texture2D(texture,vUv);\n"+
    "  if(tc.a<0.01) discard; else gl_FragColor = vec4( tc.r * meshcolor.r * vColor.r * tc.a * vColor.a * meshcolor.a, tc.g * meshcolor.g * vColor.g * tc.a * vColor.a * meshcolor.a, tc.b * meshcolor.b * vColor.b * tc.a * vColor.a * meshcolor.a, tc.a * meshcolor.a * vColor.a);\n"+
    "}\n";

var fragment_replacer_glsl = 
	"uniform sampler2D texture;\n"+
    "varying highp vec2 vUv;\n"+
	"varying highp vec4 vColor;\n"+
	"uniform highp vec3 color1;\n"+    
	"uniform highp vec3 replace1;\n"+
	"uniform highp float eps;\n"+
	"void main() {\n"+
	"	highp vec4 pixel = texture2D(texture, vUv); \n"+
	"	if( pixel.r > color1.r - eps && pixel.r < color1.r + eps && pixel.g > color1.g - eps && pixel.g < color1.g + eps && pixel.b > color1.b - eps && pixel.b < color1.b + eps ){\n"+
	"		pixel = vec4(replace1, pixel.a );\n"+
	"    }\n"+
	"   pixel.r = vColor.r * pixel.r;\n"+
	"   pixel.g = vColor.g * pixel.g;\n"+
	"   pixel.b = vColor.b * pixel.b;\n"+
	"   pixel.a = vColor.a * pixel.a;\n" +   
	"	gl_FragColor = pixel;\n"+
	"}\n";
var g_shader_material_id_gen=1;
class ShaderMaterial {
    constructor() {
        var gl=Moyai.gl;
        this.vsh_src=vertex_uv_color_glsl; 
        this.fsh_src=null;
        this.glprog=null;
        this.vs=null;
        this.fs=null;
        this.id=g_shader_material_id_gen++;        
    }
    createShader(src,type) {
        var gl=Moyai.gl;
        var sh=gl.createShader(type);
        gl.shaderSource(sh,src);
        gl.compileShader(sh);
        if(!gl.getShaderParameter(sh, gl.COMPILE_STATUS)) {  
            console.warn("shader compile error:" + gl.getShaderInfoLog(sh) + src);
            return null;
        }
        return sh;
    }    
    compileAndLink() {
        var gl=Moyai.gl;
        if(!this.fsh_src) {
            console.warn("compileAndLink: need fs src set");
            return;
        }
        this.vs=this.createShader(this.vsh_src,gl.VERTEX_SHADER);
        this.fs=this.createShader(this.fsh_src,gl.FRAGMENT_SHADER);
        this.glprog=gl.createProgram();
        gl.attachShader(this.glprog,this.vs);
        gl.attachShader(this.glprog,this.fs);
        gl.linkProgram(this.glprog);
        if (!gl.getProgramParameter(this.glprog, gl.LINK_STATUS)) {
            console.warn("cant init shader program");
        }        
    }
}
class ColorReplacerShaderMaterial extends ShaderMaterial {
    constructor() {
        super();
        var gl=Moyai.gl;
        this.fsh_src = fragment_replacer_glsl;
        this.epsilon=0.02;
        this.from_color=vec3.fromValues(0,1,0,1);
        this.to_color=vec3.fromValues(1,0,0,1);
        this.setColor(vec3.fromValues(0,0,0),vec3.fromValues(0,1,0),0.01);
        this.compileAndLink();
        this.attribLocations = {
            position: gl.getAttribLocation(this.glprog,"position"),
            color: gl.getAttribLocation(this.glprog,"color"),
            uv: gl.getAttribLocation(this.glprog,"uv"),
        };
        this.uniformLocations = {
            projectionMatrix: gl.getUniformLocation(this.glprog,"projectionMatrix"),
            modelViewMatrix: gl.getUniformLocation(this.glprog,"modelViewMatrix"),
            texture: gl.getUniformLocation(this.glprog, "texture"),
            color1: gl.getUniformLocation(this.glprog,"color1"),
            replace1: gl.getUniformLocation(this.glprog,"replace1"),
            eps: gl.getUniformLocation(this.glprog,"eps"),
        };        
    }
    setColor(from,to,eps) {
        this.epsilon = eps;
        vec3.copy(this.from_color,from);
        vec3.copy(this.to_color,to);
    }
    applyUniforms() {
        Moyai.gl.uniform3fv(this.uniformLocations.color1,this.from_color);
        Moyai.gl.uniform3fv(this.uniformLocations.replace1,this.to_color);
        Moyai.gl.uniform1f(this.uniformLocations.eps,this.epsilon);
    }
};
class DefaultColorShaderMaterial extends ShaderMaterial {
    constructor() {
        super();
        var gl=Moyai.gl;
        this.fsh_src = fragment_uv_color_glsl;
        this.compileAndLink();
        this.attribLocations = {
            position: gl.getAttribLocation(this.glprog,"position"),
            color: gl.getAttribLocation(this.glprog,"color"),
            uv: gl.getAttribLocation(this.glprog,"uv"),
        };
        this.uniformLocations = {
            projectionMatrix: gl.getUniformLocation(this.glprog,"projectionMatrix"),
            modelViewMatrix: gl.getUniformLocation(this.glprog,"modelViewMatrix"),
            texture: gl.getUniformLocation(this.glprog, "texture"),
            meshcolor: gl.getUniformLocation(this.glprog,"meshcolor"),
        };
    }
};
var g_moyai_default_color_shader_material;
function getDefaultColorShaderMaterial() {
    if(!g_moyai_default_color_shader_material) {
        g_moyai_default_color_shader_material=new DefaultColorShaderMaterial();
    }
    return g_moyai_default_color_shader_material;
}
class PrimColorShaderMaterial extends ShaderMaterial {
    constructor() {
        super();        
        var gl=Moyai.gl;
        this.fsh_src = fragment_vcolor_glsl;
        this.vsh_src = vertex_vcolor_glsl;
        this.compileAndLink();
        this.attribLocations = {
            position: gl.getAttribLocation(this.glprog,"position"),
            color: gl.getAttribLocation(this.glprog,"color"),            
        };
        this.uniformLocations = {
            projectionMatrix: gl.getUniformLocation(this.glprog,"projectionMatrix"),
            modelViewMatrix: gl.getUniformLocation(this.glprog,"modelViewMatrix"),
        };                
    }
};
var g_moyai_prim_color_shader_material;
function getPrimColorShaderMaterial() {
    if(!g_moyai_prim_color_shader_material) {
        g_moyai_prim_color_shader_material=new PrimColorShaderMaterial();
    }
    return g_moyai_prim_color_shader_material;
}
    


//////////////////////
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

class Keyboard {
    constructor() {
        this.keys={};
        this.toggled={};
        this.mod_shift=false;
        this.mod_ctrl=false;
        this.mod_alt=false;
        this.prevent_default=false;
        this.to_read_event=true;
    }
    enableReadEvent(flg) { this.to_read_event=flg; }
    setKey(keycode,pressed) {
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
    getKey(keycode) {
        return this.keys[keycode];
    }
    getToggled(keycode) {
        return this.toggled[keycode];
    }
    clearToggled(keycode) {
        this.toggled[keycode]=false;
    }
    readBrowserEvent(e,pressed) {
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
    setupBrowser(w) {
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
    setPreventDefault(flg) { this.prevent_default=flg; }
};


/////////////////////
class Mouse {
    constructor() {
        this.cursor_pos=vec2.create();
        this.movement=vec2.create();
        this.buttons={};
        this.toggled={};
        this.mod_shift=false;
        this.mod_ctrl=false;
        this.mod_alt=false;
    }
    clearMovement() { vec2.set(this.movement,0,0); }
    setupBrowser(w,dom) {
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
    readButtonEvent(e,pressed) {
        if(pressed) {
            if(!this.buttons[e.button]) this.toggled[e.button] = true;
        }
        this.buttons[e.button] = pressed;
        this.mod_shift = e.shiftKey;
        this.mod_alt = e.altKey;
        this.mod_ctrl = e.ctrlKey;
    }
    getButton(btn_ind) {
        return this.buttons[btn_ind];
    }
    getToggled(btn_ind) {
        return this.toggled[btn_ind];
    }
    clearToggled(btn_ind) {
        this.toggled[btn_ind] = false;        
    }
    getCursorPos() { return this.cursor_pos; }
};


/////////////////////////
class Touch {
    constructor() {
        this.last_touch_pos=vec2.create();
        this.touching=false;        
    }
    readTouchEvent(e) {
        var rect=this.dom.getBoundingClientRect();        
        var x = Math.floor(e.touches[0].clientX - rect.left);
        var y = Math.floor(e.touches[0].clientY - rect.top);
        //        e.preventDefault();
   e.preventDefault();
   e.stopPropagation();
        
        vec2.set(this.last_touch_pos,x,y);
        //        vec2.set(this.movement,e.movementX, e.movementY);
        console.log("RRRR:",this.last_touch_pos,e);
    }
    
    setupBrowser(w,dom) {
        this.dom=dom;
        var _this=this;
        w.addEventListener("touchstart",function(e) {
            _this.touching=true;
            _this.readTouchEvent(e);
        },{passive: false, capture:false});
        w.addEventListener("touchend",function(e) {
            _this.touching=false;
        },{passive: false, capture:false});
        w.addEventListener("touchmove",function(e) {
            _this.readTouchEvent(e);
        },{passive: false, capture:false});
    }
}

/////////////////////////

class SoundSystem {
    constructor() {
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
    setMasterVolume(vol) { this.master_volume=vol; }
    getMasterVolume() { return this.master_volume; }
    // type: "float" or other, "wav", "mp3"..
    newBGMFromMemory(data,type) {
        var snd = this.createSound(data,true,type);
        this.sounds[snd.id] = snd;
        return snd;
    }
    newSoundFromMemory(data,type) {
        var snd = this.createSound(data,false,type);
        this.sounds[snd.id] = snd;
        return snd;
    }
    newSoundFromFile(url,type) {
        var snd = this.createSoundFromFile(url,true,type);
        this.sounds[snd.id]=snd;
        return snd;
    }
    newBGMFromFile(url,type) {
        var snd = this.createSoundFromFile(url,false,type);
        this.sounds[snd.id]=snd;
        return snd;
    }
    createSoundFromFile(url,loop,type) {
        var context=this.context;
        var request=new XMLHttpRequest();
        request.open("GET",url,true);
        request.responseType="arraybuffer";
        request.send();
        request.onload = function() {
            var res=request.response;
            context.decodeAudioData(res,function(buf) {
                snd.audiobuffer=buf;
            });
        }
        
        var snd=new Sound();
        snd.sound_system=this;
        snd.context=this.context;
        snd.setLoop(loop);
        return snd;
    }
    createSound(data,loop,type) {
        var snd = new Sound();
        snd.sound_system = this;
        snd.context=this.context;
        snd.setLoop(loop);
        snd.setData(data,type);
        return snd;
    }
};

var g_sound_id_gen=1;
class Sound {
    constructor() {
        this.id = g_sound_id_gen++;
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
    setLoop(loop) { this.loop=loop; }
    isReady() { return this.audiobuffer; }
    setDefaultVolume(v) { this.default_volume=v;}
    setData(data,type) {
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
    prepareSource(vol,detune) {
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
    play(vol,detune) {
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
    setTimePositionSec( pos_sec ) {
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
    isPlaying() {
        if(!this.context)return false;    
        if(this.source) {
            if(this.source.paused) return false;
            if(this.source.ended ) return false;  // set by moyai
            return true;
        } else {
            return false;
        }        
    }
    stop() {
        if(!this.context)return;    
        if(this.source) {
            console.log("stopping..", this.source);
            this.source.stop(0);
        } 
    }
};

/////////////////////////
class Prop3D extends Prop {
    constructor() {
        super();
        this.scl = vec3.fromValues(1,1,1);
        this.loc = vec3.fromValues(0,0,0);
        this.quaternion = quat.create();
        this.geom=null;
        this.material=null;
        this.color=Color.fromValues(1,1,1,1);
        this.sort_center = vec3.fromValues(0,0,0);
	    this.depth_mask=true;
        this.alpha_test=false;
        this.cull_back_face=true;
        this.draw_offset = vec3.fromValues(0,0,0);
        this.priority=this.id;
        this.dimension=3;
        this.visible=true;
        this.enable_frustum_culling=true;
        this.mvMat=mat4.create();
        this.localMat=mat4.create();
        this.finalLoc=vec3.create();
        this.rot=vec3.create(); // xyz-euler in radian
        this.cull_face=Moyai.gl.BACK;
        this.depth_mask=true;
    }
    propPoll(dt) {
        if(this.prop3DPoll && this.prop3DPoll(dt)===false) {
            return false;
        }
        return true;
    }
    setVisible(flg) { this.visible=flg; }
    updateModelViewMatrix(parentMat) {
        if(parentMat) {
            mat4.copy(this.mvMat,parentMat);
        } else {
            mat4.identity(this.mvMat);
        }
        vec3.set(this.finalLoc, this.loc[0]+this.draw_offset[0],this.loc[1]+this.draw_offset[1],this.loc[2]+this.draw_offset[2]);
        mat4.compose(this.localMat,this.finalLoc,this.quaternion,this.scl);
        mat4.multiply(this.mvMat,this.mvMat,this.localMat);
    }
    setTexture(moyai_tex) {
        this.moyai_tex=moyai_tex;
    }
    setGeom(g) {this.geom=g;}
    setMaterial(m) {this.material=m;}
    setScl(x,y,z) {
        if(y===undefined) {
            vec3.copy(this.scl,x);
        } else {
            vec3.set(this.scl,x,y,z);   
        }
    }
    setLoc(x,y,z) {
        if(y===undefined) vec3.copy(this.loc,x); else vec3.set(this.loc,x,y,z);
    }
    addLoc(x,y,z) {
        if(y===undefined) {
            vec3.add(this.loc,this.loc,x);
        } else {
            this.loc[0]+=x;
            this.loc[1]+=y;
            this.loc[2]+=z;            
        }
    }
    setEulerRot(x,y,z) {
        if(y===undefined) {
            quat.fromEuler(this.quaternion,x[0]/Math.PI*180,x[1]/Math.PI*180,x[2]/Math.PI*180);            
        } else {
            quat.fromEuler(this.quaternion,x/Math.PI*180,y/Math.PI*180,z/Math.PI*180);
        }
        vec3.set(this.rot,x,y,z);
    }
    setColor(r,g,b,a) {
        if(Color.exactEqualsToValues(this.color,r,g,b,a)==false) {
            this.need_color_update = true;
        }
        if(typeof r == 'object' ) {
            Color.copy(this.color,r);
        } else {
            Color.set(this.color,r,g,b,a); 
        }
    }
}




////////////////////