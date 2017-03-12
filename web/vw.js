var g_ws;
var g_moyai_client;
var g_viewport_pool={};
var g_camera_pool={};

function onPacket(ws,pkttype,argdata) {
    if(pkttype==PACKETTYPE_ZIPPED_RECORDS) {
        //            console.log("zipped records:",argdata);
        var uncompressed = Snappy.uncompress(argdata.buffer);
        //            console.log(uncompressed);
        var dv = new DataView(uncompressed);
        ws.unzipped_rb.push(dv,uncompressed.byteLength);
        //            console.log("unzipped_rb:",ws.unzipped_rb);
        return;
    }

    var dv = new DataView(argdata.buffer);
    
    switch(pkttype) {
    case PACKETTYPE_S2C_WINDOW_SIZE:
        {
            var w = dv.getUint32(0,true);
            var h = dv.getUint32(4,true);
            console.log("received window_size:",w,h,argdata);
            if(!g_moyai_client) {
                g_moyai_client = new MoyaiClient(w,h,window.devicePixelRatio);
                var screen = document.getElementById("screen");
                screen.appendChild( g_moyai_client.renderer.domElement );
            }
        }
        break;
    case PACKETTYPE_S2C_VIEWPORT_CREATE:
        {
            var id = dv.getUint32(0,true);
            console.log("received vp creat:", id);
            var vp = new Viewport();
            vp.id = id;
            g_viewport_pool[id]=vp;
        }
        break;
    case PACKETTYPE_S2C_VIEWPORT_SCALE:
        {
            var id = dv.getUint32(0,true);
            var sclx = dv.getFloat32(4,true);
            var scly = dv.getFloat32(8,true);
            console.log("received vp scl:", id, sclx, scly );
            var vp = g_viewport_pool[id];
            if(!vp) { console.log("vp not found"); return; }
            vp.setScale2D(sclx,scly);
        }
        break;

    case PACKETTYPE_S2C_CAMERA_CREATE:
        {
            var id = dv.getUint32(0,true);
            console.log("received cam creat:",id);
            var cam = new Camera();
            g_camera_pool[id]=cam;            
        }
        break;
    case PACKETTYPE_S2C_CAMERA_LOC:
        {
            var id = dv.getUint32(0,true);
            var x = dv.getFloat32(4,true);
            var y = dv.getFloat32(8,true);
            console.log("received cam loc:",id,x,y);
            var cam = g_camera_pool[id];
            if(!cam) { console.log("cam not found"); return;}
            cam.setLoc(x,y);
        }
        break;        
        
/*
    PACKETTYPE_S2C_PROP2D_SNAPSHOT = 200, 
    PACKETTYPE_S2C_PROP2D_LOC = 201,
    PACKETTYPE_S2C_PROP2D_GRID = 202,
    PACKETTYPE_S2C_PROP2D_INDEX = 203,
    PACKETTYPE_S2C_PROP2D_SCALE = 204,
    PACKETTYPE_S2C_PROP2D_ROT = 205,
    PACKETTYPE_S2C_PROP2D_FLIPROTBITS = 206,
    PACKETTYPE_S2C_PROP2D_COLOR = 208,
    PACKETTYPE_S2C_PROP2D_OPTBITS = 209,
    PACKETTYPE_S2C_PROP2D_PRIORITY = 210,
    PACKETTYPE_S2C_PROP2D_DELETE = 230,
    PACKETTYPE_S2C_PROP2D_CLEAR_CHILD = 240,
    PACKETTYPE_S2C_PROP2D_LOC_VEL = 250,
    PACKETTYPE_S2C_PROP2D_INDEX_LOC = 251,    
    
    PACKETTYPE_S2C_LAYER_CREATE = 300,
    PACKETTYPE_S2C_LAYER_VIEWPORT = 301,
    PACKETTYPE_S2C_LAYER_CAMERA = 302,
    PACKETTYPE_S2C_VIEWPORT_CREATE = 330,
    //    PACKETTYPE_S2C_VIEWPORT_SIZE = 331,  not used now
    PACKETTYPE_S2C_VIEWPORT_SCALE = 332,
    PACKETTYPE_S2C_VIEWPORT_DYNAMIC_LAYER = 333,    
    PACKETTYPE_S2C_CAMERA_CREATE = 340,
    PACKETTYPE_S2C_CAMERA_LOC = 341,
    PACKETTYPE_S2C_CAMERA_DYNAMIC_LAYER = 342, // cam id, layer id: camera belongs to the layer's dynamic_cameras
    
    PACKETTYPE_S2C_TEXTURE_CREATE = 400,
    PACKETTYPE_S2C_TEXTURE_IMAGE = 401,
    PACKETTYPE_S2C_IMAGE_CREATE = 420,
    PACKETTYPE_S2C_IMAGE_LOAD_PNG = 421,
    PACKETTYPE_S2C_IMAGE_ENSURE_SIZE = 424,
    PACKETTYPE_S2C_IMAGE_RAW = 425,
    
    PACKETTYPE_S2C_TILEDECK_CREATE = 440,
    PACKETTYPE_S2C_TILEDECK_TEXTURE = 441,
    PACKETTYPE_S2C_TILEDECK_SIZE = 442,
    PACKETTYPE_S2C_GRID_CREATE = 460, // with its size (id,w,h)
    PACKETTYPE_S2C_GRID_DECK = 461, // with gid,tdid
    PACKETTYPE_S2C_GRID_PROP2D = 462, // with gid,propid    
    PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT = 464, // index table, array of int32_t
    PACKETTYPE_S2C_GRID_TABLE_FLIP_SNAPSHOT = 465, // xfl|yfl|uvrot bitfield in array of uint8_t
    PACKETTYPE_S2C_GRID_TABLE_TEXOFS_SNAPSHOT = 466, //  array of Vec2
    PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT = 467, // color table, array of PacketColor: 4 * float32    
    PACKETTYPE_S2C_GRID_DELETE = 470,

    PACKETTYPE_S2C_TEXTBOX_CREATE = 500, // tb_id, uint32_t
    PACKETTYPE_S2C_TEXTBOX_FONT = 501,    // tb_id, font_id
    PACKETTYPE_S2C_TEXTBOX_STRING = 502,    // tb_id, utf8str
    PACKETTYPE_S2C_TEXTBOX_LOC = 503,    // tb_id, x,y
    PACKETTYPE_S2C_TEXTBOX_SCL = 504,    // tb_id, x,y    
    PACKETTYPE_S2C_TEXTBOX_COLOR = 505,    // tb_id, PacketColor
    PACKETTYPE_S2C_TEXTBOX_LAYER = 510,     // tb_id, l_id
    PACKETTYPE_S2C_FONT_CREATE = 540, // fontid, utf8 string array
    PACKETTYPE_S2C_FONT_CHARCODES = 541, // fontid, utf8str
    PACKETTYPE_S2C_FONT_LOADTTF = 542, // fontid, filepath    

    PACKETTYPE_S2C_COLOR_REPLACER_SHADER_SNAPSHOT = 600, //
    PACKETTYPE_S2C_PRIM_BULK_SNAPSHOT = 610, // array of PacketPrim

    PACKETTYPE_S2C_SOUND_CREATE_FROM_FILE = 650,
    PACKETTYPE_S2C_SOUND_CREATE_FROM_SAMPLES = 651,
    PACKETTYPE_S2C_SOUND_DEFAULT_VOLUME = 653,
    PACKETTYPE_S2C_SOUND_PLAY = 660,
    PACKETTYPE_S2C_SOUND_STOP = 661,    
    PACKETTYPE_S2C_SOUND_POSITION = 662,

    PACKETTYPE_S2C_JPEG_DECODER_CREATE = 700,
    PACKETTYPE_S2C_CAPTURED_FRAME = 701,
    PACKETTYPE_S2C_CAPTURED_AUDIO = 710,
    
    PACKETTYPE_S2C_FILE = 800, // send file body and path



    */        
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
