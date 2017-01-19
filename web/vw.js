
var g_ws;

var g_moyai_client = new MoyaiClient();

//var g_keyboard = new Keyboard();
//var g_mouse = new Mouse();
//var g_pad = new Pad();



var g_scene = new THREE.Scene();
var g_renderer = new THREE.WebGLRenderer();
g_renderer.setPixelRatio( window.devicePixelRatio );
g_renderer.setSize( window.innerWidth, window.innerHeight );
g_renderer.autoClear = false;

document.getElementById("screen").appendChild( g_renderer.domElement );

function animate() {
	requestAnimationFrame( animate );
	render();
}
function render() {
    console.log("render");
}

function onWindowResize() {
    g_renderer.setSize(window.innerWidth, window.innerHeight);
    /*
	  camera.aspect = width / height;
	  camera.updateProjectionMatrix();

	  cameraOrtho.left = - width / 2;
	  cameraOrtho.right = width / 2;
	  cameraOrtho.top = height / 2;
	  cameraOrtho.bottom = - height / 2;
	  cameraOrtho.updateProjectionMatrix();

	  updateHUDSprites();
    */
}
window.addEventListener( "resize", onWindowResize, false );
    
animate();


function onPacket(ws,pkttype,argdata) {
    switch(pkttype) {
    case PACKETTYPE_ZIPPED_RECORDS:
        {
            //                console.log("zipped records:",argdata);
            var uncompressed = Snappy.uncompress(argdata.buffer);
            console.log(uncompressed);
            var dv = new DataView(uncompressed);
            ws.unzipped_rb.push(dv,uncompressed.byteLength);
            console.log("unzipped_rb:",ws.unzipped_rb);
        }
        break;
    case PACKETTYPE_S2C_WINDOW_SIZE:
        {
            var dv = new DataView(argdata.buffer);
            var w = dv.getUint32(0);
            var h = dv.getUint32(4);
            console.log("received window_size:",w,h);
        }
        break;
    }
}




// button funcs
function connectButton() {
    g_ws = createWSClient("ws://localhost:8888/");
    g_ws.onPacket = onPacket;
}
function disconnectButton() {
    g_ws.close();
}

