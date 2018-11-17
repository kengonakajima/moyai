var gl; 
var SCRW=256, SCRH=128;

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
varying lowp vec4 vColor;
void main(void) {
    gl_FragColor = vColor; //vec4(1.0, 1.0, 1.0, 1.0);
}
`;

var g_vs_src = `
attribute vec3 aVertexPosition;
attribute vec4 aVertexColor;
uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
varying lowp vec4 vColor;
void main(void) {
    gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aVertexPosition, 1.0);
    vColor=aVertexColor;
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
        },
        uniformLocations: {
            projectionMatrix: gl.getUniformLocation(shaderProgram,"uProjectionMatrix"),
            modelViewMatrix: gl.getUniformLocation(shaderProgram,"uModelViewMatrix")
        },
    };
}



function initBuffers() {
    const positionBuffer=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
    var positions=[
     1.0,  1.0,
    -1.0,  1.0,
     1.0, -1.0,
    -1.0, -1.0,
    ];
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(positions), gl.STATIC_DRAW);
    
    var colors=[
        1.0,  1.0,  1.0,  1.0,    // 白
        1.0,  0.0,  0.0,  1.0,    // 赤
        0.0,  1.0,  0.0,  1.0,    // 緑
        0.0,  0.0,  1.0,  1.0     // 青
    ];
    const colorBuffer=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,colorBuffer);
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(colors), gl.STATIC_DRAW);

    return {
        position: positionBuffer,
        color: colorBuffer,
    };
}

var g_rot=0;

function drawScene(programInfo, buf, deltaTime) {
    //clear
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);// 近くにある物体は、遠くにある物体を覆い隠す
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    // prepare mat
    const fov=45*Math.PI/180;
    const aspect=SCRW/SCRH;
    const znear=0.1;
    const zfar=100;

    var projMat=mat4.create();
    mat4.perspective(projMat, fov, aspect, znear, zfar );
    const mvMat=mat4.create();
    mat4.translate(mvMat, mvMat, [-0.0, 0.0, -6.0 ]);

    g_rot+=deltaTime;
    mat4.rotate(mvMat, mvMat, g_rot, [0,0,1] );
    console.log("projMat:",projMat, "mvMat:",mvMat);


    // setup buf
    {
        const normalize=false;
        const stride=0;
        const offset=0;
        gl.bindBuffer(gl.ARRAY_BUFFER, buf.position);
        gl.vertexAttribPointer( programInfo.attribLocations.vertexPosition, 2, gl.FLOAT, normalize, stride, offset);
        gl.enableVertexAttribArray(programInfo.attribLocations.vertexPosition);
        gl.bindBuffer(gl.ARRAY_BUFFER, buf.color);
        gl.vertexAttribPointer( programInfo.attribLocations.vertexColor, 4, gl.FLOAT, false, 0,0 );
        gl.enableVertexAttribArray(programInfo.attribLocations.vertexColor);
    }

    // setup shader
    console.log("Pinfo:",programInfo);
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

    // draw!
    {
        const offset=0;
        const vertexCount=4;
        gl.drawArrays(gl.TRIANGLE_STRIP, offset, vertexCount);
    }    
}


function start() {
    var canvas = document.getElementById("glcanvas");
    initWebGL(canvas);
    gl.viewport(0, 0, canvas.width, canvas.height);

    console.log("gl init ok");
    
    const programInfo = initShaders();
    const buf=initBuffers();

    var then=0;
    function render(now) {
        now*=0.001;
        const dt=now-then;
        then=now;
        drawScene(programInfo,buf, dt);
        requestAnimationFrame(render);
    }
    requestAnimationFrame(render);    
}

