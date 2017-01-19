
var g_ws;

var g_moyai_client;

//var g_keyboard = new Keyboard();
//var g_mouse = new Mouse();
//var g_pad = new Pad();


function animate() {
	requestAnimationFrame( animate );
    if(g_moyai_client) {
        g_moyai_client.render();
    }
}


function onWindowResize() {
    g_moyai_client.resize(window.innerWidth, window.innerHeight);
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

/////////// testing

g_moyai_client = new MoyaiClient(400,400,window.devicePixelRatio);
document.getElementById("screen").appendChild( g_moyai_client.renderer.domElement );

