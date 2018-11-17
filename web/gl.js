var gl; 
var SCRW=640, SCRH=320;

function initWebGL(canvas) {
    const opt={antialias:false};
    try {
        gl = canvas.getContext("webgl",opt) || canvas.getContext("experimental-webgl",opt);
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
void main(void) {
    //    gl_FragColor = texture2D(uSampler,vTextureCoord); //vColor; //vec4(1.0, 1.0, 1.0, 1.0);
    highp vec4 tcolor=texture2D(uSampler,vTextureCoord);
    gl_FragColor=vec4( tcolor.r*vColor.r*vLighting.r,
                       tcolor.g*vColor.g*vLighting.g,
                       tcolor.b*vColor.b*vLighting.b,
                       tcolor.a*vColor.a);    
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
function initShaders() {
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
            normalMatrix: gl.getUniformLocation(shaderProgram,"uNormalMatrix"),
        },
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
    };
}

function clearScene() {
    //clear
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);// 近くにある物体は、遠くにある物体を覆い隠す
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);    
}
function drawScene(programInfo, buf, tex, xtr,ytr,ztr, xrot,yrot,zrot ) {

    // prepare mat
    const fov=45*Math.PI/180;
    const aspect=SCRW/SCRH;
    const znear=0.1;
    const zfar=100;

    var projMat=mat4.create();
    mat4.perspective(projMat, fov, aspect, znear, zfar );
    const mvMat=mat4.create();
    mat4.translate(mvMat, mvMat, [xtr,ytr,ztr]);

    mat4.rotate(mvMat, mvMat, xrot, [1,0,0] );
    mat4.rotate(mvMat, mvMat, yrot, [0,1,0] );
    mat4.rotate(mvMat, mvMat, zrot, [0,0,1] );    
//    console.log("projMat:",projMat, "mvMat:",mvMat);

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


    gl.bindBuffer(gl.ARRAY_BUFFER, buf.normal );
    gl.vertexAttribPointer(programInfo.attribLocations.vertexNormal, 3, gl.FLOAT, false,0,0);
    gl.enableVertexAttribArray(programInfo.attribLocations.vertexNormal );
    
    // ind
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, buf.indices);


    // setup shader
//    console.log("Pinfo:",programInfo);
    gl.useProgram(programInfo.program);
    gl.uniformMatrix4fv(
        programInfo.uniformLocations.projectionMatrix,
        false,
        projMat
    );
    gl.uniformMatrix4fv(
        programInfo.uniformLocations.modelViewMatrix,
        false,
        mvMat
    );
    const normalMat = mat4.create();
    mat4.invert(normalMat,mvMat);
    mat4.transpose(normalMat, normalMat);
    gl.uniformMatrix4fv(
        programInfo.uniformLocations.normalMatrix,
        false,
        normalMat        
    );

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, tex);
    gl.uniform1i(programInfo.uniformLocations.uSampler,0);
    

    // draw!
    {
        const vertexCount=36;
        const type=gl.UNSIGNED_SHORT;
        const offset=0;
        gl.drawElements(gl.TRIANGLES, vertexCount, type, offset);
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


var g_t=0;

function start() {
    var canvas = document.getElementById("glcanvas");
    initWebGL(canvas);
    gl.viewport(0, 0, canvas.width, canvas.height);

    console.log("gl init ok");
    
    const programInfo = initShaders();
    const buf0=initBuffers(false);
    const buf1=initBuffers(true);    
    const tex = loadTexture("./cubetexture.png");
    
    var then=0;
    function render(now) {
        now*=0.001;
        const dt=now-then;
        then=now;

        clearScene();
        drawScene(programInfo,buf0,tex, Math.sin(g_t)*2,0,-8, g_t,g_t*0.7,0 );
        drawScene(programInfo,buf1,tex, 0,Math.sin(g_t)*2,-8, g_t*0.7,g_t,0 );        
        requestAnimationFrame(render);
        g_t+=1/60;
    }
    requestAnimationFrame(render);    
}

