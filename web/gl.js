var gl; 

function initWebGL(canvas) {
    try {
        gl = canvas.getContext("webgl") || canvas.getContext("experimental-webgl");
    } catch(e) {    }
    if(!gl) {
        alert("no WebGL support");
        gl = null;
    }
}

function start() {
    var canvas = document.getElementById("glcanvas");
    initWebGL(canvas);
    
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.enable(gl.DEPTH_TEST);
    // 近くにある物体は、遠くにある物体を覆い隠す
    gl.depthFunc(gl.LEQUAL);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    console.log("gl init ok");
}
