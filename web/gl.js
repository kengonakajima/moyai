var gl; 
var SCRW=1024, SCRH=512;

function initWebGL(canvas) {
    const opt={antialias:false};
    try {
        gl = canvas.getContext("webgl2",opt);// || canvas.getContext("experimental-webgl",opt);
    } catch(e) {    }
    if(!gl) {
        console.warn("no WebGL support");
        gl = null;
    }
}

var g_fs_src = `
varying highp vec2 vTextureCoord;
varying lowp vec4 vColor;
varying highp vec3 vLighting;
uniform sampler2D uSampler;
uniform lowp float uAlpha;
void main(void) {
    //    gl_FragColor = texture2D(uSampler,vTextureCoord); //vColor; //vec4(1.0, 1.0, 1.0, 1.0);
    highp vec4 tcolor=texture2D(uSampler,vTextureCoord);
    gl_FragColor=vec4( tcolor.r*vColor.r*vLighting.r,
                       tcolor.g*vColor.g*vLighting.g,
                       tcolor.b*vColor.b*vLighting.b,
                       tcolor.a*vColor.a * uAlpha);    
}
`;

var g_fs_nolight_src = `
varying highp vec2 vTextureCoord;
varying lowp vec4 vColor;
uniform sampler2D uSampler;
uniform lowp float uAlpha;
void main(void) {
    //    gl_FragColor = texture2D(uSampler,vTextureCoord); //vColor; //vec4(1.0, 1.0, 1.0, 1.0);
    highp vec4 tcolor=texture2D(uSampler,vTextureCoord);
    gl_FragColor=vec4( tcolor.r*vColor.r,
                       tcolor.g*vColor.g,
                       tcolor.b*vColor.b,
                       tcolor.a*vColor.a * uAlpha);    
}
`;


var g_vs_src = `
attribute vec3 aVertexPosition;
attribute vec4 aVertexColor;
attribute vec2 aTextureCoord;
attribute vec3 aVertexNormal;
uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat4 uNormalMatrix;
varying lowp vec4 vColor;
varying highp vec2 vTextureCoord;
varying highp vec3 vLighting;

void main(void) {
    gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aVertexPosition, 1.0);
    vColor=aVertexColor;
    vTextureCoord = aTextureCoord;

    highp vec3 ambientLight=vec3(0.3,0.3,0.3);
    highp vec3 directionalLightColor=vec3(1,1,1);
    highp vec3 directionalVector=normalize(vec3(0.85,0.8,0.75));
    highp vec4 transformedNormal=uNormalMatrix * vec4(aVertexNormal,1.0);
    highp float directional=max(dot(transformedNormal.xyz,directionalVector),0.0);
    vLighting=ambientLight+(directionalLightColor * directional);
}
`;

var g_vs_nolight_src = `
attribute vec3 aVertexPosition;
attribute vec4 aVertexColor;
attribute vec2 aTextureCoord;
attribute vec3 aVertexNormal;
uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
varying lowp vec4 vColor;
varying highp vec2 vTextureCoord;

void main(void) {
    //    gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aVertexPosition, 1.0);
    gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aVertexPosition, 1.0);    
    vColor=aVertexColor;
    vTextureCoord = aTextureCoord;
}
`;



function createShader(src,type) {
    var sh=gl.createShader(type);
    gl.shaderSource(sh,src);
    gl.compileShader(sh);
    if(!gl.getShaderParameter(sh, gl.COMPILE_STATUS)) {  
        console.warn("shader compile error:" + gl.getShaderInfoLog(sh));
        return null;
    }
    return sh;
}
function initLightShaders() {
    var fs=createShader(g_fs_src, gl.FRAGMENT_SHADER);
    var vs=createShader(g_vs_src, gl.VERTEX_SHADER);
  
    var shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vs);
    gl.attachShader(shaderProgram, fs);
    gl.linkProgram(shaderProgram);
  
    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        console.warn("cant init shader program");
    }

    return {
        program: shaderProgram,
        attribLocations: {
            vertexPosition: gl.getAttribLocation(shaderProgram,"aVertexPosition"),
            vertexColor: gl.getAttribLocation(shaderProgram,"aVertexColor"),
            textureCoord: gl.getAttribLocation(shaderProgram,"aTextureCoord"),
            vertexNormal: gl.getAttribLocation(shaderProgram,"aVertexNormal"),
        },
        uniformLocations: {
            projectionMatrix: gl.getUniformLocation(shaderProgram,"uProjectionMatrix"),
            modelViewMatrix: gl.getUniformLocation(shaderProgram,"uModelViewMatrix"),
            uSampler: gl.getUniformLocation(shaderProgram, "uSampler"),
            uAlpha: gl.getUniformLocation(shaderProgram,"uAlpha"),
            normalMatrix: gl.getUniformLocation(shaderProgram,"uNormalMatrix"),
        },
    };
}

function initNolightShaders() {
    var fs=createShader(g_fs_nolight_src, gl.FRAGMENT_SHADER);
    var vs=createShader(g_vs_nolight_src, gl.VERTEX_SHADER);
  
    var shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vs);
    gl.attachShader(shaderProgram, fs);
    gl.linkProgram(shaderProgram);
  
    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        console.warn("cant init shader program");
    }

    return {
        program: shaderProgram,
        attribLocations: {
            vertexPosition: gl.getAttribLocation(shaderProgram,"aVertexPosition"),
            vertexColor: gl.getAttribLocation(shaderProgram,"aVertexColor"),
            textureCoord: gl.getAttribLocation(shaderProgram,"aTextureCoord"),
        },
        uniformLocations: {
            projectionMatrix: gl.getUniformLocation(shaderProgram,"uProjectionMatrix"),
            modelViewMatrix: gl.getUniformLocation(shaderProgram,"uModelViewMatrix"),
            uSampler: gl.getUniformLocation(shaderProgram, "uSampler"),
            uAlpha: gl.getUniformLocation(shaderProgram,"uAlpha"),
        },
    };
}

function initBigBuffers() {
    var n=8; //32*32*32=32K cube, x4x6 verts1.5M verts

    const positions = [
        // Front face
            -1.0, -1.0,  1.0,
        1.0, -1.0,  1.0,
        1.0,  1.0,  1.0,
            -1.0,  1.0,  1.0,

        // Back face
            -1.0, -1.0, -1.0,
            -1.0,  1.0, -1.0,
        1.0,  1.0, -1.0,
        1.0, -1.0, -1.0,

        // Top face
            -1.0,  1.0, -1.0,
            -1.0,  1.0,  1.0,
        1.0,  1.0,  1.0,
        1.0,  1.0, -1.0,

        // Bottom face
            -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0, -1.0,  1.0,
            -1.0, -1.0,  1.0,

        // Right face
        1.0, -1.0, -1.0,
        1.0,  1.0, -1.0,
        1.0,  1.0,  1.0,
        1.0, -1.0,  1.0,

        // Left face
            -1.0, -1.0, -1.0,
            -1.0, -1.0,  1.0,
            -1.0,  1.0,  1.0,
            -1.0,  1.0, -1.0,
    ];

    const faceColors = [
        [1.0,  1.0,  1.0,  1.0],    // Front face: white
        [1.0,  0.0,  0.0,  1.0],    // Back face: red
        [0.0,  1.0,  0.0,  1.0],    // Top face: green
        [0.0,  0.0,  1.0,  1.0],    // Bottom face: blue
        [1.0,  1.0,  0.0,  1.0],    // Right face: yellow
        [1.0,  0.0,  1.0,  1.0],    // Left face: purple
    ];

    var colors = [];
    for (var j = 0; j < faceColors.length; ++j) {
        const c = faceColors[j];
        // Repeat each color four times for the four vertices of the face
        colors = colors.concat(c, c, c, c);
    }

    const indices = [
        0,  1,  2,      0,  2,  3,    // front
        4,  5,  6,      4,  6,  7,    // back
        8,  9,  10,     8,  10, 11,   // top
        12, 13, 14,     12, 14, 15,   // bottom
        16, 17, 18,     16, 18, 19,   // right
        20, 21, 22,     20, 22, 23,   // left
    ];

    const textureCoordinates = [
        // Front
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
        // Back
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
        // Top
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
        // Bottom
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
        // Right
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
        // Left
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
    ];
    

    var col_ary=new Float32Array(colors.length *n*n*n);
    var pos_ary=new Float32Array(positions.length *n*n*n);
    var ind_ary=new Uint32Array(indices.length *n*n*n);
    var uv_ary=new Float32Array(textureCoordinates.length *n*n*n);
    
    for(var y=0;y<n;y++) {
        for(var x=0;x<n;x++){
            for(var z=0;z<n;z++) {
                var ii=z+(x*n)+(y*n*n);
                var posbi=ii*positions.length;
                for(var i=0;i<positions.length;i+=3) {
                    pos_ary[posbi+i+0]=positions[i+0]*0.4+x;
                    pos_ary[posbi+i+1]=positions[i+1]*0.4+y;
                    pos_ary[posbi+i+2]=positions[i+2]*0.4+z;
                }
                var colbi=ii*colors.length;
                for(var i=0;i<colors.length;i++) {
                    col_ary[colbi+i]=colors[i];
                }
                var indbi=ii*indices.length;
                for(var i=0;i<indices.length;i++) {
                    ind_ary[indbi+i]=indices[i]+24*ii;
                }
                var uvbi=ii*textureCoordinates.length;
                for(var i=0;i<textureCoordinates.length;i++) {
                    uv_ary[uvbi+i]=textureCoordinates[i];
                }
            }
        }
    }
    const positionBuffer=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,pos_ary, gl.STATIC_DRAW);

    const colorBuffer=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,colorBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,col_ary, gl.STATIC_DRAW);
    
    const indexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, ind_ary, gl.STATIC_DRAW);

    const textureCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, textureCoordBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, uv_ary, gl.STATIC_DRAW);

    return {
        position: positionBuffer,
        color: colorBuffer,
        textureCoord: textureCoordBuffer,
        indices: indexBuffer,
        vertexCount: 36*n*n*n,        
    };    
}

function initBuffers() {
    const positionBuffer=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);

    const positions = [
        // Front face
            -1.0, -1.0,  1.0,
        1.0, -1.0,  1.0,
        1.0,  1.0,  1.0,
            -1.0,  1.0,  1.0,

        // Back face
            -1.0, -1.0, -1.0,
            -1.0,  1.0, -1.0,
        1.0,  1.0, -1.0,
        1.0, -1.0, -1.0,

        // Top face
            -1.0,  1.0, -1.0,
            -1.0,  1.0,  1.0,
        1.0,  1.0,  1.0,
        1.0,  1.0, -1.0,

        // Bottom face
            -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0, -1.0,  1.0,
            -1.0, -1.0,  1.0,

        // Right face
        1.0, -1.0, -1.0,
        1.0,  1.0, -1.0,
        1.0,  1.0,  1.0,
        1.0, -1.0,  1.0,

        // Left face
            -1.0, -1.0, -1.0,
            -1.0, -1.0,  1.0,
            -1.0,  1.0,  1.0,
            -1.0,  1.0, -1.0,
    ];
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(positions), gl.STATIC_DRAW);

    const faceColors = [
        [1.0,  1.0,  1.0,  1.0],    // Front face: white
        [1.0,  0.0,  0.0,  1.0],    // Back face: red
        [0.0,  1.0,  0.0,  1.0],    // Top face: green
        [0.0,  0.0,  1.0,  1.0],    // Bottom face: blue
        [1.0,  1.0,  0.0,  1.0],    // Right face: yellow
        [1.0,  0.0,  1.0,  1.0],    // Left face: purple
    ];

    var colors = [];
    for (var j = 0; j < faceColors.length; ++j) {
        const c = faceColors[j];
        // Repeat each color four times for the four vertices of the face
        colors = colors.concat(c, c, c, c);
    }

    const colorBuffer=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,colorBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(colors), gl.STATIC_DRAW);
    
    const indices = [
        0,  1,  2,      0,  2,  3,    // front
        4,  5,  6,      4,  6,  7,    // back
        8,  9,  10,     8,  10, 11,   // top
        12, 13, 14,     12, 14, 15,   // bottom
        16, 17, 18,     16, 18, 19,   // right
        20, 21, 22,     20, 22, 23,   // left
    ];
    const indexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
    // Now send the element array to GL
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indices), gl.STATIC_DRAW);


    //uv

    const textureCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, textureCoordBuffer);

    const textureCoordinates = [
        // Front
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
        // Back
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
        // Top
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
        // Bottom
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
        // Right
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
        // Left
        0.0,  0.0,
        1.0,  0.0,
        1.0,  1.0,
        0.0,  1.0,
    ];
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(textureCoordinates), gl.STATIC_DRAW);

    // normals
    const normalBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, normalBuffer);
    const vertexNormals = [
        // Front
        0.0,  0.0,  1.0,
        0.0,  0.0,  1.0,
        0.0,  0.0,  1.0,
        0.0,  0.0,  1.0,

        // Back
        0.0,  0.0, -1.0,
        0.0,  0.0, -1.0,
        0.0,  0.0, -1.0,
        0.0,  0.0, -1.0,

        // Top
        0.0,  1.0,  0.0,
        0.0,  1.0,  0.0,
        0.0,  1.0,  0.0,
        0.0,  1.0,  0.0,

        // Bottom
        0.0, -1.0,  0.0,
        0.0, -1.0,  0.0,
        0.0, -1.0,  0.0,
        0.0, -1.0,  0.0,

        // Right
        1.0,  0.0,  0.0,
        1.0,  0.0,  0.0,
        1.0,  0.0,  0.0,
        1.0,  0.0,  0.0,

        // Left
            -1.0,  0.0,  0.0,
            -1.0,  0.0,  0.0,
            -1.0,  0.0,  0.0,
            -1.0,  0.0,  0.0
    ];

    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertexNormals), gl.STATIC_DRAW);
    
    
    return {
        position: positionBuffer,
        color: colorBuffer,
        textureCoord: textureCoordBuffer,
        indices: indexBuffer,
        normal: normalBuffer,
        vertexCount: 36,
    };
}

function clearScene() {
    //clear
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);// 近くにある物体は、遠くにある物体を覆い隠す
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA,gl.ONE_MINUS_SRC_ALPHA);
}
function drawScene(type,vertexCount,use_light, projMat, mvMat, programInfo, buf, tex, alpha ) {

    // setup buf
    {
        const normalize=false;
        const stride=0;
        const offset=0;
        gl.bindBuffer(gl.ARRAY_BUFFER, buf.position);
        gl.vertexAttribPointer( programInfo.attribLocations.vertexPosition, 3, gl.FLOAT, normalize, stride, offset);
        gl.enableVertexAttribArray(programInfo.attribLocations.vertexPosition);
        gl.bindBuffer(gl.ARRAY_BUFFER, buf.color);
        gl.vertexAttribPointer( programInfo.attribLocations.vertexColor, 4, gl.FLOAT, false, 0,0 );
        gl.enableVertexAttribArray(programInfo.attribLocations.vertexColor);
    }

    // uv
    gl.bindBuffer(gl.ARRAY_BUFFER, buf.textureCoord);
    gl.vertexAttribPointer(programInfo.attribLocations.textureCoord, 2, gl.FLOAT, false,0,0 );
    gl.enableVertexAttribArray(programInfo.attribLocations.textureCoord );

    if(use_light) {
        gl.bindBuffer(gl.ARRAY_BUFFER, buf.normal );
        gl.vertexAttribPointer(programInfo.attribLocations.vertexNormal, 3, gl.FLOAT, false,0,0);
        gl.enableVertexAttribArray(programInfo.attribLocations.vertexNormal );
    }
    // ind
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, buf.indices);


    // setup shader
//    console.log("Pinfo:",programInfo);
    
    gl.uniformMatrix4fv(
        programInfo.uniformLocations.modelViewMatrix,
        false,
        mvMat
    );
    if(use_light) {
        const normalMat = mat4.create();
        mat4.invert(normalMat,mvMat);
        mat4.transpose(normalMat, normalMat);
        gl.uniformMatrix4fv(
            programInfo.uniformLocations.normalMatrix,
            false,
            normalMat        
        );
    }

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, tex);
    gl.uniform1i(programInfo.uniformLocations.uSampler,0);

    gl.uniform1f(programInfo.uniformLocations.uAlpha, alpha);

    // draw!
    {
        gl.drawElements(gl.TRIANGLES, vertexCount, type, 0);
//        const offset=0;
//        const vertexCount=4;
  //      gl.drawArrays(gl.TRIANGLE_STRIP, offset, vertexCount);
    }    
}


function loadTexture(url) {
  const texture = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, texture);

  // Because images have to be download over the internet
  // they might take a moment until they are ready.
  // Until then put a single pixel in the texture so we can
  // use it immediately. When the image has finished downloading
  // we'll update the texture with the contents of the image.
  const level = 0;
  const internalFormat = gl.RGBA;
  const width = 1;
  const height = 1;
  const border = 0;
  const srcFormat = gl.RGBA;
  const srcType = gl.UNSIGNED_BYTE;
  const pixel = new Uint8Array([0, 0, 255, 255]);  // opaque blue
  gl.texImage2D(gl.TEXTURE_2D, level, internalFormat,
                width, height, border, srcFormat, srcType,
                pixel);

  const image = new Image();
  image.onload = function() {
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, level, internalFormat,
                  srcFormat, srcType, image);

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
  };
  image.src = url;

  return texture;
}

function isPowerOf2(value) {
  return (value & (value - 1)) == 0;
}

/*
  original idea

  https://stackoverflow.com/questions/12836967/extracting-view-frustum-planes-hartmann-gribbs-method
  
  // Left clipping plane  
  p_planes[0].a = comboMatrix._41 + comboMatrix._11;
  p_planes[0].b = comboMatrix._42 + comboMatrix._12;
  p_planes[0].c = comboMatrix._43 + comboMatrix._13;
  p_planes[0].d = comboMatrix._44 + comboMatrix._14;
  // Right clipping plane
  p_planes[1].a = comboMatrix._41 - comboMatrix._11;
  p_planes[1].b = comboMatrix._42 - comboMatrix._12;
  p_planes[1].c = comboMatrix._43 - comboMatrix._13;
  p_planes[1].d = comboMatrix._44 - comboMatrix._14;
  // Top clipping plane
  p_planes[2].a = comboMatrix._41 - comboMatrix._21;
  p_planes[2].b = comboMatrix._42 - comboMatrix._22;
  p_planes[2].c = comboMatrix._43 - comboMatrix._23;
  p_planes[2].d = comboMatrix._44 - comboMatrix._24;
  // Bottom clipping plane
  p_planes[3].a = comboMatrix._41 + comboMatrix._21;
  p_planes[3].b = comboMatrix._42 + comboMatrix._22;
  p_planes[3].c = comboMatrix._43 + comboMatrix._23;
  p_planes[3].d = comboMatrix._44 + comboMatrix._24;
  // Near clipping plane
  p_planes[4].a = comboMatrix._41 + comboMatrix._31;
  p_planes[4].b = comboMatrix._42 + comboMatrix._32;
  p_planes[4].c = comboMatrix._43 + comboMatrix._33;
  p_planes[4].d = comboMatrix._44 + comboMatrix._34;
  // Far clipping plane
  p_planes[5].a = comboMatrix._41 - comboMatrix._31;
  p_planes[5].b = comboMatrix._42 - comboMatrix._32;
  p_planes[5].c = comboMatrix._43 - comboMatrix._33;
  p_planes[5].d = comboMatrix._44 - comboMatrix._34;
*/

/*
  threeはこんなふうになっている。
  
  			planes[ 0 ].setComponents( me3 - me0, me7 - me4, me11 - me8, me15 - me12 ).normalize();
			planes[ 1 ].setComponents( me3 + me0, me7 + me4, me11 + me8, me15 + me12 ).normalize();
			planes[ 2 ].setComponents( me3 + me1, me7 + me5, me11 + me9, me15 + me13 ).normalize();
			planes[ 3 ].setComponents( me3 - me1, me7 - me5, me11 - me9, me15 - me13 ).normalize();
			planes[ 4 ].setComponents( me3 - me2, me7 - me6, me11 - me10, me15 - me14 ).normalize();
			planes[ 5 ].setComponents( me3 + me2, me7 + me6, me11 + me10, me15 + me14 ).normalize();

 */
function extractPlanes2(m) {
    return [
        [ m[3]-m[0], m[7]-m[4], m[11]-m[8], m[15]-m[12] ],
        [ m[3]+m[0], m[7]+m[4], m[11]+m[8], m[15]+m[12] ],
        [ m[3]+m[1], m[7]+m[5], m[11]+m[9], m[15]+m[13] ],
        [ m[3]-m[1], m[7]-m[5], m[11]-m[9], m[15]-m[13] ],
        [ m[3]-m[2], m[7]-m[6], m[11]-m[10], m[15]-m[14] ],
        [ m[3]+m[2], m[7]+m[6], m[11]+m[10], m[15]+m[14] ]
    ];
}
// [a,b,c,d] => ax+by+cz+d=0
function extractPlanes(M, zNear, zFar) {
  var z  = zNear || 0.0
  var zf = zFar || 1.0
  return [
    [ M[12] + M[0], M[13] + M[1], M[14] + M[2], M[15] + M[3] ], // left
    [ M[12] - M[0], M[13] - M[1], M[14] - M[2], M[15] - M[3] ], // right
    [ M[12] + M[4], M[13] + M[5], M[14] + M[6], M[15] + M[7] ], // bottom
    [ M[12] - M[4], M[13] - M[5], M[14] - M[6], M[15] - M[7] ], // top
    [ z*M[12] + M[8], z*M[13] + M[9], z*M[14] + M[10], z*M[15] + M[11] ], // near
      [ zf*M[12] - M[8], zf*M[13] - M[9], zf*M[14] - M[10], zf*M[15] - M[11] ] // far
//    [ M[12] + M[8], M[13] + M[9], M[14] + M[10], M[15] + M[11] ], // near
//    [ M[12] - M[8], M[13] - M[9], M[14] - M[10], M[15] - M[11] ] // far      
  ]
}

//////////////////


var g_t=0;

var v3_100=vec3.fromValues(1,0,0);
var v3_010=vec3.fromValues(0,1,0);
var v3_001=vec3.fromValues(0,0,1);        

function start() {
    var canvas = document.getElementById("glcanvas");
    initWebGL(canvas);
    gl.viewport(0, 0, canvas.width, canvas.height);

    console.log("gl init ok");

    const use_light=true;//false;
    var programInfoLight=initLightShaders();
    var programInfoNolight=initNolightShaders();

    const bigbuf=initBigBuffers();
    const buf=initBuffers(use_light);
    const tex = loadTexture("./cubetexture.png");
    var pg_use;
    if(use_light) pg_use=programInfoLight; else pg_use=programInfoNolight;
    var then=0;
    var frame_cnt=0;
    var total_frame_cnt=0;
    var n=1000;
    var mvMatArray=new Array(n);
    var locArray=new Array(n);
    var rotArray=new Array(n);
    for(var i=0;i<n;i++) {
        mvMatArray[i]=mat4.create();
        locArray[i]=vec3.create();
        rotArray[i]=vec3.create();
    }
    var last_print_at=0;

    var camera=vec3.create();
    var center=vec3.fromValues(0,0,-1);
    var up=vec3.fromValues(0,1,0);
    var camMat=mat4.create();
    function render(now) {
        frame_cnt++;
        total_frame_cnt++;
        now*=0.001;
        const dt=now-then;
        then=now;

        if(last_print_at<now-1) {
            last_print_at=now;
            var e=document.getElementById("fps");
            e.innerHTML="fps:"+frame_cnt;
            frame_cnt=0;
        }
        

        clearScene();
        
        gl.useProgram(pg_use.program);

        // prepare mat
        const fov=45*Math.PI/180;
        const aspect=SCRW/SCRH;
        const znear=0.1;
        const zfar=100;


        camera[0]=Math.cos(g_t)*8; // 逆に動くけど、いちおうできてるぞ
        camera[1]        =3;
        camera[2]=Math.sin(g_t)*8; // 逆に動くけど、いちおうできてるぞ

//                center[1]+=0.01;        
        mat4.lookAt(camMat,camera,center,up);
        
        // http://ogldev.atspace.co.uk/www/tutorial12/tutorial12.html
        var projMat=mat4.create();
        mat4.perspective(projMat, fov, aspect, znear, zfar );

        var viewProjMat=mat4.create();
        mat4.multiply(viewProjMat,projMat,camMat);
        
        gl.uniformMatrix4fv(
            pg_use.uniformLocations.projectionMatrix,
            false,
            viewProjMat
        );

        var planes=extractPlanes2(viewProjMat,znear,zfar);        
        var xmargin=Math.sin(now/3)*130  ;
        xmargin=0;
        n=0;
        for(var i=0;i<n;i++) {
            var k=10,d=80;
            locArray[i][0]=range(-k,k)*2+xmargin;
            locArray[i][1]=range(-k,k);
            locArray[i][2]=range(-d,-d/2);
            rotArray[i][0]=range(1,5);
            rotArray[i][1]=range(1,5);

            // view frustum culling
            // http://www.sousakuba.com/Programming/gs_dot_plane_distance.html
            var to_skip=false;
            for(var j=0;j<6;j++) {
                var dot=vec3.dot(locArray[i],planes[j]);
                var distance=dot+planes[j][3]
                if(distance<0) to_skip=true; 
            }
            
            if(to_skip)continue;
            mat4.identity(mvMatArray[i]);
            mat4.translate(mvMatArray[i], mvMatArray[i], locArray[i]);
            mat4.rotate(mvMatArray[i], mvMatArray[i], rotArray[i][0], v3_100 );
            mat4.rotate(mvMatArray[i], mvMatArray[i], rotArray[i][1], v3_010 );
            mat4.rotate(mvMatArray[i], mvMatArray[i], rotArray[i][2], v3_001 );
            drawScene(gl.UNSIGNED_INT, bigbuf.vertexCount, use_light, viewProjMat, mvMatArray[i], pg_use, bigbuf,tex, range(0,1));
        }
        var mvMat0=mat4.create();
        mat4.identity(mvMat0);
        //        mat4.translate(mvMat0, mvMat0, [Math.sin(g_t)*2,0,-8]);
//        mat4.translate(mvMat0, mvMat0, [0,0,0]);
//        mat4.rotate(mvMat0,mvMat0, g_t, v3_100);
//        mat4.rotate(mvMat0,mvMat0, g_t*0.7, v3_010);
        drawScene(gl.UNSIGNED_SHORT,buf.vertexCount,use_light, viewProjMat, mvMat0, pg_use,buf,tex, 1.0 );
        var mvMat1=mat4.create();
        mat4.identity(mvMat1);
        //        mat4.translate(mvMat1,mvMat1, [0,Math.sin(g_t)*2,-8]);
        mat4.translate(mvMat1,mvMat1, [1,1,0]);
//        mat4.rotate(mvMat1,mvMat1, g_t*0.7, v3_100);
//        mat4.rotate(mvMat1,mvMat1, g_t, v3_010);
        drawScene(gl.UNSIGNED_SHORT,buf.vertexCount,use_light, viewProjMat, mvMat1, pg_use,buf,tex, 0.2 );
        requestAnimationFrame(render);
        g_t+=1/60;
    }
    requestAnimationFrame(render);    
}

