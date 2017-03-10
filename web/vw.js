var g_ws;

function onPacket(ws,pkttype,argdata) {
    switch(pkttype) {
    case PACKETTYPE_ZIPPED_RECORDS:
        {
//            console.log("zipped records:",argdata);
            var uncompressed = Snappy.uncompress(argdata.buffer);
//            console.log(uncompressed);
            var dv = new DataView(uncompressed);
            ws.unzipped_rb.push(dv,uncompressed.byteLength);
//            console.log("unzipped_rb:",ws.unzipped_rb);
        }
        break;
    case PACKETTYPE_S2C_WINDOW_SIZE:
        {
            var dv = new DataView(argdata.buffer);
            var w = dv.getUint32(0,true);
            var h = dv.getUint32(4,true);
            console.log("received window_size:",w,h,argdata);
        }
        break;
    default:
        console.log("onPacket type:",pkttype);
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
var g_stop_render=false;
function stopRender() {
    g_stop_render = true;
}


////////////////////

var anim_cnt=0;
var last_anim_at = new Date().getTime();

function animate() {
    if(anim_cnt<50000) {
        anim_cnt++;
	    if(!g_stop_render) requestAnimationFrame( animate );
    }
}

    
animate();
