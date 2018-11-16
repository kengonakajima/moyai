var gl; 

function initWebGL(canvas) {
    try {
        gl = canvas.getContext("webgl") || canvas.getContext("experimental-webgl");
    } catch(e) {    }
    if(!gl) {
        console.warn("no WebGL support");
        gl = null;
    }
}

var g_fs_src =
    "void main(void) {\n"+
    "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"+
    "}\n";

var g_vs_src =
    "attribute vec3 aVertexPosition;\n"+
    "uniform mat4 uMVMatrix;\n"+
    "uniform mat4 uPMatrix;\n"+
    "void main(void) {\n"+
    "    gl_Position = uPMatrix * uMVMatrix * vec4(aVertexPosition, 1.0);\n"+
    "}\n";


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
    gl.useProgram(shaderProgram);
    vertexPositionAttribute = gl.getAttribLocation(shaderProgram, "aVertexPosition");
    gl.enableVertexAttribArray(vertexPositionAttribute);
}


function start() {
    var canvas = document.getElementById("glcanvas");
    initWebGL(canvas);
    
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.enable(gl.DEPTH_TEST);
    // 近くにある物体は、遠くにある物体を覆い隠す
    gl.depthFunc(gl.LEQUAL);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.viewport(0, 0, canvas.width, canvas.height);

    initShaders();
    console.log("gl init ok");
}
