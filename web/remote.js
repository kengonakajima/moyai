// deps check
console.log("Snappy",Snappy);

//
function assert(cond,msg) {                                                                                             
  if(!cond) throw msg;                                                                                                  
}

function dump( dv, l) {
    var out = "";
    for(var i=0;i<l;i++) {
        var u8 = dv.getUint8(i);
//        if(u8<16) out += "0" + u8.toString(16)+ " "; else out += u8.toString(16)+ " ";
        out += u8.toString(10) + " ";
    }
    console.log( "dump:", out );
}

function RecvBuffer(sz) {
  this.buf = new ArrayBuffer(sz);
  this.dv = new DataView( this.buf );
  this.len = sz;
  this.used = 0;
}
RecvBuffer.prototype.push = function(dv,len) {
  //console.log( "bufpush:", len, "bytes", "used:", this.used );
  assert( this.used + len <= this.len, "recvbuffer exceeded. buflen:" + this.len + " used:" + this.used + " add:" + len  );
  for(var i=0;i<len;i++) {
    this.dv.setUint8( this.used + i, dv.getUint8(i) );
  }
  this.used += len;
}
RecvBuffer.prototype.shift = function(len) {
    assert( len <= this.used, "bufshift: too long. len:" + len + " used:" + this.used );    
    for(var i=0;i<this.used-len;i++) {
        this.dv.setUint8( i, this.dv.getUint8( len+i) );
    }
    this.used -= len;
}

///////////////////////////////////

var PACKETTYPE_PING = 1;
var PACKETTYPE_TIMESTAMP = 2;
var PACKETTYPE_ZIPPED_RECORDS = 8;
var PACKETTYPE_C2S_KEYBOARD = 100;
var PACKETTYPE_C2S_MOUSE_BUTTON = 102;
var PACKETTYPE_C2S_CURSOR_POS = 103;
var PACKETTYPE_C2S_TOUCH_BEGIN = 104;
var PACKETTYPE_C2S_TOUCH_MOVE = 105;
var PACKETTYPE_C2S_TOUCH_END = 106;
var PACKETTYPE_C2S_TOUCH_CANCEL = 107;
var PACKETTYPE_R2S_CLIENT_LOGIN = 150; // accepting new client; getting new id number of this client
var PACKETTYPE_R2S_CLIENT_LOGOUT = 151;
var PACKETTYPE_R2S_KEYBOARD = 155;
var PACKETTYPE_R2S_MOUSE_BUTTON = 156;
var PACKETTYPE_R2S_CURSOR_POS = 157;
var PACKETTYPE_S2R_NEW_CLIENT_ID = 170;
var PACKETTYPE_S2R_CAMERA_CREATE = 175;
var PACKETTYPE_S2R_CAMERA_DYNAMIC_LAYER = 176;
var PACKETTYPE_S2R_CAMERA_LOC = 177;
var PACKETTYPE_S2R_VIEWPORT_CREATE = 180;
var PACKETTYPE_S2R_VIEWPORT_DYNAMIC_LAYER = 181;
var PACKETTYPE_S2R_VIEWPORT_SCALE = 182;
var PACKETTYPE_S2C_PROP2D_SNAPSHOT = 200; 
var PACKETTYPE_S2C_PROP2D_LOC = 201;
var PACKETTYPE_S2C_PROP2D_GRID = 202;
var PACKETTYPE_S2C_PROP2D_INDEX = 203;
var PACKETTYPE_S2C_PROP2D_SCALE = 204;
var PACKETTYPE_S2C_PROP2D_ROT = 205;
var PACKETTYPE_S2C_PROP2D_FLIPROTBITS = 206;
var PACKETTYPE_S2C_PROP2D_COLOR = 208;
var PACKETTYPE_S2C_PROP2D_OPTBITS = 209;
var PACKETTYPE_S2C_PROP2D_PRIORITY = 210;
var PACKETTYPE_S2C_PROP2D_DELETE = 230;
var PACKETTYPE_S2C_PROP2D_CLEAR_CHILD = 240;
var PACKETTYPE_S2C_PROP2D_LOC_VEL = 250;
var PACKETTYPE_S2C_PROP2D_INDEX_LOC = 251;    
var PACKETTYPE_S2C_LAYER_CREATE = 300;
var PACKETTYPE_S2C_LAYER_VIEWPORT = 301;
var PACKETTYPE_S2C_LAYER_CAMERA = 302;
var PACKETTYPE_S2C_VIEWPORT_CREATE = 330;
var PACKETTYPE_S2C_VIEWPORT_SCALE = 332;
var PACKETTYPE_S2C_VIEWPORT_DYNAMIC_LAYER = 333;    
var PACKETTYPE_S2C_CAMERA_CREATE = 340;
var PACKETTYPE_S2C_CAMERA_LOC = 341;
var PACKETTYPE_S2C_CAMERA_DYNAMIC_LAYER = 342; // cam id; layer id: camera belongs to the layer's dynamic_cameras
var PACKETTYPE_S2C_TEXTURE_CREATE = 400;
var PACKETTYPE_S2C_TEXTURE_IMAGE = 401;
var PACKETTYPE_S2C_IMAGE_CREATE = 420;
var PACKETTYPE_S2C_IMAGE_LOAD_PNG = 421;
var PACKETTYPE_S2C_IMAGE_ENSURE_SIZE = 424;
var PACKETTYPE_S2C_IMAGE_RAW = 425;
var PACKETTYPE_S2C_TILEDECK_CREATE = 440;
var PACKETTYPE_S2C_TILEDECK_TEXTURE = 441;
var PACKETTYPE_S2C_TILEDECK_SIZE = 442;
var PACKETTYPE_S2C_GRID_CREATE = 460; // with its size (id;w;h)
var PACKETTYPE_S2C_GRID_DECK = 461; // with gid;tdid
var PACKETTYPE_S2C_GRID_PROP2D = 462; // with gid;propid    
var PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT = 464; // index table; array of int32_t
var PACKETTYPE_S2C_GRID_TABLE_FLIP_SNAPSHOT = 465; // xfl|yfl|uvrot bitfield in array of uint8_t
var PACKETTYPE_S2C_GRID_TABLE_TEXOFS_SNAPSHOT = 466; //  array of Vec2
var PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT = 467; // color table; array of PacketColor: 4 * float32    
var PACKETTYPE_S2C_GRID_DELETE = 470;
var PACKETTYPE_S2C_TEXTBOX_CREATE = 500; // tb_id; uint32_t
var PACKETTYPE_S2C_TEXTBOX_FONT = 501;    // tb_id; font_id
var PACKETTYPE_S2C_TEXTBOX_STRING = 502;    // tb_id; utf8str
var PACKETTYPE_S2C_TEXTBOX_LOC = 503;    // tb_id; x;y
var PACKETTYPE_S2C_TEXTBOX_SCL = 504;    // tb_id; x;y    
var PACKETTYPE_S2C_TEXTBOX_COLOR = 505;    // tb_id; PacketColor
var PACKETTYPE_S2C_TEXTBOX_PRIORITY = 506;    // tb_id, prio
var PACKETTYPE_S2C_TEXTBOX_LAYER = 510;     // tb_id; l_id
var PACKETTYPE_S2C_FONT_CREATE = 540; // fontid; utf8 string array
var PACKETTYPE_S2C_FONT_CHARCODES = 541; // fontid; utf8str
var PACKETTYPE_S2C_FONT_LOADTTF = 542; // fontid; filepath    
var PACKETTYPE_S2C_COLOR_REPLACER_SHADER_SNAPSHOT = 600; //
var PACKETTYPE_S2C_PRIM_BULK_SNAPSHOT = 610; // array of PacketPrim
var PACKETTYPE_S2C_SOUND_CREATE_FROM_FILE = 650;
var PACKETTYPE_S2C_SOUND_CREATE_FROM_SAMPLES = 651;
var PACKETTYPE_S2C_SOUND_DEFAULT_VOLUME = 653;
var PACKETTYPE_S2C_SOUND_PLAY = 660;
var PACKETTYPE_S2C_SOUND_STOP = 661;    
var PACKETTYPE_S2C_SOUND_POSITION = 662;
var PACKETTYPE_S2C_JPEG_DECODER_CREATE = 700;
var PACKETTYPE_S2C_CAPTURED_FRAME = 701;
var PACKETTYPE_S2C_CAPTURED_AUDIO = 710;
var PACKETTYPE_S2C_FILE = 800; // send file body and path
var PACKETTYPE_S2C_WINDOW_SIZE = 900; // u2
var PACKETTYPE_MAX = 1000;
var PACKETTYPE_ERROR = 2000; // error code


function createWSClient(url) {
    var ws = new WebSocket(url, ["binary"]);
    ws.binaryType = "arraybuffer";
    ws.rb = new RecvBuffer(8*1024*1024);
    ws.unzipped_rb = new RecvBuffer(8*1024*1024);
    ws.onopen = function(event) {
        console.log("wsopen");
    };
    ws.total_read_bytes=0;
    ws.total_read_count=0;
    ws.total_packet_count=0;
    ws.total_unzipped_read_bytes=0;
    ws.onmessage = function(ev) {
        var dv = new DataView(ev.data);
//        console.log("onmessage:", ev.data.byteLength);
        if(ws.log)dump( dv, ev.data.byteLength );
        ws.total_read_bytes+=ev.data.byteLength;
        ws.total_read_count++;
        ws.rb.push( dv, ev.data.byteLength );
        for(;;){
            if(!ws.parseDispatch(ws.rb))break;
        }
        for(;;) {
            if(!ws.parseDispatch(ws.unzipped_rb))break;
        }
    };
    ws.onerror = function (error) {
        console.log("wserr");
    };

    /*
        var out_u8b = new Uint8Array(total_record_len);
        for(var i=0;i<total_record_len;i++) {
            out_u8b[i] = dv.getUint8(i);                                                                                          
        }
        if(ws.log) console.log("outb:", out_u8b.buffer, "payload_len:", payload_len, "seqnum:", ws.seqnum, "payload_type:", payload_type, "data:", u8ary );
        ws.send( out_u8b.buffer );
        ws.seqnum+=1;
    }
    */
    
    ws.parseDispatch = function(recvbuf) {
        if( recvbuf.used < 4 ) return false;
        var pkt_len = recvbuf.dv.getUint32(0,true);
//        console.log("pkt_len:",pkt_len);
        if( recvbuf.used <4+pkt_len) return false;
        var argdata_len = pkt_len-2;
        var pkttype = recvbuf.dv.getUint16(4,true);
        var u8a = new Uint8Array(argdata_len);
        for(var i=0;i<argdata_len;i++) {
            u8a[i] = recvbuf.dv.getUint8(4+2+i);
        }        
        ws.onPacket(ws,pkttype,u8a);
        ws.total_packet_count++;
        recvbuf.shift(pkt_len+4);
        return true;
    };
    ws.onPacket = function(ws,pkttype,argdata) {
        console.log("onPacket not overwritten? pkttype:",pkttype,argdata.length);
    }
    ws.sendUS1UI2 = function(usval,ui0,ui1) {
        var totalsize = 4 + 2 + 4+4;
        var dv = new DataView(new ArrayBuffer(totalsize));
        dv.setUint32(0,totalsize-4,true);
        dv.setUint16(4,usval,true);
        dv.setUint32(6,ui0,true);
        dv.setUint32(10,ui1,true);
        ws.send(dv.buffer);        
    }
    ws.sendUS1UI3 = function(usval,ui0,ui1,ui2) {
        var totalsize = 4 + 2 + 4+4+4;
        var dv = new DataView(new ArrayBuffer(totalsize));
        dv.setUint32(0,totalsize-4,true);
        dv.setUint16(4,usval,true);
        dv.setUint32(6,ui0,true);
        dv.setUint32(10,ui1,true);
        dv.setUint32(14,ui2,true);
        ws.send(dv.buffer);
    }
    ws.sendUS1F2 = function(usval,f0,f1) {
        var totalsize = 4+2 + 4+4;
        var dv = new DataView(new ArrayBuffer(totalsize));
        dv.setUint32(0,totalsize-4,true);
        dv.setUint16(4,usval,true);
        dv.setFloat32(6,f0,true);
        dv.setFloat32(10,f1,true);
        ws.send(dv.buffer);
    }
    return ws;
}


//////////////////
function File(path,u8adata) {
    this.path = path;
    this.data = new Uint8Array(u8adata.length);
    for(var i in u8adata) this.data[i]=u8adata[i];
}
File.prototype.comparePath = function(path) {
    return (this.path==path);
}
function FileDepo() {
    this.files=[];
}
FileDepo.prototype.get = function(path) {
    for(var i in this.files) {
        if( this.files[i].comparePath(path) ) {
            return this.files[i];
        }
    }
    return null;
}
FileDepo.prototype.ensure = function(path,u8data) {
    var found = this.get(path);
    if(found!=null) return found;
    var f = new File(path,u8data);
    this.files.push(f);
}
FileDepo.prototype.getByIndex = function(ind) {
    return this.files[ind];
}

