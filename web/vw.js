var g_ws;
var g_moyai_client;
var g_viewport_pool={};
var g_camera_pool={};
var g_layer_pool={};
var g_filedepo = new FileDepo();
var g_image_pool={};
var g_texture_pool={};
var g_tiledeck_pool={};
var g_sound_system = new SoundSystem();
var g_sound_pool={};
var g_prop2d_pool={};
var g_crshader_pool={};

var g_window_width=null;
var g_window_height=null;

function getString8FromDataView(dv,ofs) {
    var len = dv.getUint8(ofs);
    var u8a=new Uint8Array(len);
    for(var i=0;i<len;i++) {
        u8a[i]=dv.getUint8(ofs+1+i);
    }
    return String.fromCharCode.apply(null,u8a);
}
function getPacketColor(dv,ofs) {
    var r = dv.getUint8(ofs);
    var g = dv.getUint8(ofs+1);
    var b = dv.getUint8(ofs+2);
    var a = dv.getUint8(ofs+3);
    return new Color( r/255.0, g/255.0, b/255.0,a/255.0)
}
function getProp2DSnapshot(dv) {
    var out={};
    var sz = dv.getUint32(0,true);
    out.prop_id = dv.getUint32(4,true);
    out.layer_id = dv.getUint32(8,true);
    out.parent_prop_id = dv.getUint32(12,true);
    out.loc = new Vec2( dv.getFloat32(16,true), dv.getFloat32(20,true));
    out.scl = new Vec2( dv.getFloat32(24,true), dv.getFloat32(28,true));
    out.index = dv.getInt32(32,true);
    out.tiledeck_id = dv.getUint32(36,true);
    out.debug = dv.getInt32(40,true);
    out.rot = dv.getFloat32(44,true);
    out.color = getPacketColor(dv,48);
    out.shader_id = dv.getUint32(52,true);
    out.optbits = dv.getUint32(56,true);
    out.priority = dv.getInt32(60,true);
    out.fliprotbits = dv.getUint8(64);
    return out;
}
function getXFlipFromFlipRotBits(bits) { return bits & 0x01; }
function getYFlipFromFlipRotBits(bits) { return bits & 0x02; }
function getUVRotFromFlipRotBits(bits) { return bits & 0x04; }
    
var PROP2D_OPTBIT_ADDITIVE_BLEND = 0x00000001;
    
    
    /*
      typedef struct  {
      uint32_t prop_id; // non-zero
      uint32_t layer_id; // non-zero for layer, zero for child props
      uint32_t parent_prop_id; // non-zero for child props, zero for layer props
      PacketVec2 loc;
      PacketVec2 scl;
      int32_t index;
      uint32_t tiledeck_id; // non-zero
      int32_t debug;
      float rot;
      PacketColor color;
      uint32_t shader_id;
      uint32_t optbits;
      int32_t priority;
      uint8_t fliprotbits;
      } PacketProp2DSnapshot;
    */


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
    case PACKETTYPE_TIMESTAMP:
        {
        }
        break;
        
    case PACKETTYPE_S2C_WINDOW_SIZE:
        {
            var w = g_window_width = dv.getUint32(0,true);
            var h = g_window_height = dv.getUint32(4,true);
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
            vp.setSize(g_window_width,g_window_height);
        }
        break;

    case PACKETTYPE_S2C_CAMERA_CREATE:
        {
            var id = dv.getUint32(0,true);
            console.log("received cam creat:",id);
            var cam = new Camera();
            cam.id=id;
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

    case PACKETTYPE_S2C_LAYER_CREATE:
        {
            var id = dv.getUint32(0,true);
            console.log("received layer creat",id);
            var l = new Layer();
            l.id=id;
            g_layer_pool[id]=l;
            if(g_moyai_client) g_moyai_client.insertLayer(l);
        }
        break;
    case PACKETTYPE_S2C_LAYER_VIEWPORT:
        {
            var lid = dv.getUint32(0,true);
            var vid = dv.getUint32(4,true);
            console.log("received layer vp",lid,vid);
            var ly = g_layer_pool[lid]
            var vp = g_viewport_pool[vid];
            if(ly && vp) {
                ly.setViewport(vp);
            } else {
                console.log("vp or ly not found:",lid,vid);
            }
        }
        break;
    case PACKETTYPE_S2C_LAYER_CAMERA:
        {
            var lid = dv.getUint32(0,true);
            var cid = dv.getUint32(4,true);
            console.log("received layer cam",lid,cid);
            var ly = g_layer_pool[lid]
            var cam = g_camera_pool[cid];
            if(ly && cam) {
                ly.setCamera(cam);
            } else {
                console.log("cam or ly not found:",lid,vid);
            }
        }
        break;

    case PACKETTYPE_S2C_FILE:
        {
            var pathstr = getString8FromDataView(dv,0);
            var data_len = dv.getUint32(1+pathstr.length,true);
            var data_u8a=new Uint8Array(data_len);
            for(var i=0;i<data_len;i++) {
                data_u8a[i] = dv.getUint8(1+pathstr.length+4+i);
            }

            console.log("received file. path:",pathstr, "data:",data_u8a );
            g_filedepo.ensure(pathstr,data_u8a);
        }
        break;
    case PACKETTYPE_S2C_IMAGE_CREATE:
        {
            var id = dv.getUint32(0,true);
            var img = new Image();
            img.id=id;
            console.log("received image creat:",id);
            g_image_pool[id]=img;
        }
        break;
    case PACKETTYPE_S2C_IMAGE_LOAD_PNG:
        {
            var id = dv.getUint32(0,true);
            var pathstr = getString8FromDataView(dv,4);
            console.log("received image loadpng", id, pathstr);
            var u8a = g_filedepo.get(pathstr);
            var img = g_image_pool[id];
            if(img && u8a) {
                img.loadPNGMem(u8a);
//                console.log("loadpng done:", img,u8a);
            }
        }
        break;
    case PACKETTYPE_S2C_IMAGE_ENSURE_SIZE:
        {
            var id = dv.getUint32(0,true);
            var w = dv.getUint32(4,true);
            var h = dv.getUint32(8,true);
            var img = g_image_pool[id];
            console.log("received image ensure size",id,w,h,img);
            if(img) {
                img.setSize(w,h);
            }
        }
        break;
    case PACKETTYPE_S2C_TEXTURE_CREATE:
        {
            var id = dv.getUint32(0,true);
            console.log("received texture create",id);
            var t = new Texture();
            t.id=id;
            g_texture_pool[id]=t;
        }
        break;
    case PACKETTYPE_S2C_TEXTURE_IMAGE:
        {
            var tex_id = dv.getUint32(0,true);
            var img_id = dv.getUint32(4,true);
            console.log("received teximage", tex_id,img_id);
            var tex = g_texture_pool[tex_id];
            var img = g_image_pool[img_id];
            if(tex&&img) {
                tex.setImage(img);
            } else {
                console.log("tex or img not found?", tex,img);
            }
        }
        break;
    case PACKETTYPE_S2C_TILEDECK_CREATE:
        {
            var id = dv.getUint32(0,true);
            console.log("received tiledeck creat",id);
            var td = new TileDeck();
            td.id=id;
            g_tiledeck_pool[id]=td;
        }
        break;
    case PACKETTYPE_S2C_TILEDECK_TEXTURE:
        {
            var td_id = dv.getUint32(0,true);
            var tex_id = dv.getUint32(4,true);
            console.log("received td tex", td_id, tex_id);
            var td = g_tiledeck_pool[td_id];
            var tex = g_texture_pool[tex_id];
            if(td&&tex) {
                td.setTexture(tex);
            } else {
                console.log("td or tex not found", td,tex);
            }
        }
        break;
    case PACKETTYPE_S2C_TILEDECK_SIZE:
        {
            var td_id = dv.getUint32(0,true);
            var sprw = dv.getUint32(4,true);
            var sprh = dv.getUint32(8,true)
            var cellw = dv.getUint32(12,true)
            var cellh = dv.getUint32(16,true)
            console.log("received tiledeck_size.",td_id,sprw,sprh,cellw,cellh);
            var dk = g_tiledeck_pool[td_id];
            dk.setSize( sprw, sprh, cellw, cellh );            
        }
        break;
    case PACKETTYPE_S2C_SOUND_CREATE_FROM_FILE:
        {
            var id = dv.getUint32(0,true);
            var path = getString8FromDataView(dv,4);
            console.log("received sound create from file:",id,path);
            var data_u8a = g_filedepo.get(path);
            var snd = g_sound_system.newSoundFromMemory(data_u8a, "file");
            snd.id=id;
            g_sound_pool[id]=snd;            
        }
        break;
    case PACKETTYPE_S2C_SOUND_DEFAULT_VOLUME:
        {
            var id = dv.getUint32(0,true);
            var vol = dv.getFloat32(4,true);
            var snd = g_sound_pool[id];
            console.log("received sound default volume",id,vol);
            snd.setDefaultVolume(vol);            
        }
        break;
    case PACKETTYPE_S2C_SOUND_POSITION:
        {
            var id = dv.getUint32(0,true);
            var pos_sec = dv.getFloat32(4);
            var last_play_vol = dv.getFloat32(8);
            var snd = g_sound_pool[id];
            console.log("received sound_position:",id,pos_sec,last_play_vol,snd);
            if(snd) {
                if(snd.isPlaying() == false ) snd.play(last_play_vol);
                snd.setTimePositionSec( pos_sec );
            }
        }
        break;

    case PACKETTYPE_S2C_PROP2D_SNAPSHOT:
        {
            var pkt = getProp2DSnapshot(dv);
//            console.log( "received prop2d pkt", dv,pkt);
            var layer;            
            if( pkt.layer_id > 0 ) {
                layer = g_layer_pool[pkt.layer_id];
            } else if( pkt.parent_prop_id > 0 ) {
                var parent_prop = g_prop2d_pool[pkt.parent_prop_id];
                if(!parent_prop) {
                    console.log("Warning: can't find parent prop", pkt.prop_id, pkt.parent_prop_id );
                } 
            }
            if( !(layer || parent_prop ) ) {
                console.log("no layer nor parent", pkt.prop_id, pkt.layer_id, pkt.parent_prop_id );
                break;
            }

            var dk = g_tiledeck_pool[pkt.tiledeck_id]; // deck can be null (may have grid,textbox)
            if(!dk && pkt.tiledeck_id!=0) {
                console.log("TileDeck is not initialized yet! td:",pkt.tiledeck_id);
                break;
            }
            var prop = g_prop2d_pool[pkt.prop_id];
            if(!prop) {
                prop = new Prop2D();
                prop.id=pkt.prop_id;
                g_prop2d_pool[prop.id]=prop;
                if(layer) {
                    layer.insertProp(prop);
                } else if(parent_prop) {
                    var found_prop = prop.getChild( pkt.prop_id );
                    if(!found_prop) {
                        console.log("  adding a child to a prop", pkt.prop_id, pkt.parent_prop_id );
                        parent_prop.addChild(prop);
                    }
                } else {
                    console.log("Warning: this prop has no parent?",pkt.prop_id, pkt.layer_id );
                }
            }
            if(dk) prop.setDeck(dk);
            prop.setIndex(pkt.index);
            prop.setScl(pkt.scl.x,pkt.scl.y);
            prop.setLoc(pkt.loc.x, pkt.loc.y);
            prop.setRot( pkt.rot );
            prop.setXFlip( getXFlipFromFlipRotBits(pkt.fliprotbits) );
            prop.setYFlip( getYFlipFromFlipRotBits(pkt.fliprotbits) );
            prop.setUVRot( getUVRotFromFlipRotBits(pkt.fliprotbits) );
            prop.setColor(pkt.color);
            prop.use_additive_blend = pkt.optbits & PROP2D_OPTBIT_ADDITIVE_BLEND;
            if(pkt.shader_id != 0 ) {
                var crs = g_crshader_pool[pkt.shader_id];
                if(crs) {
                    prop.setFragmentShader(crs);
                } else {
                    console.log("  colorreplacershader not found", pkt.shader_id);
                }
            }
            prop.priority = pkt.priority;
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
    PACKETTYPE_S2C_SOUND_POSITION = 662,    
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

var screen = document.getElementById("screen");
                

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

var last_anim_at = new Date().getTime();

function animate() {
	if(!g_stop_render) requestAnimationFrame( animate );
    if(!g_moyai_client)return;
    var now_time = new Date().getTime();
    var dt = now_time - last_anim_at;        
    g_moyai_client.poll(dt/1000.0);
    g_moyai_client.render();
}

    
animate();
