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

function Buffer(sz) {
  this.buf = new ArrayBuffer(sz);
  this.dv = new DataView( this.buf );
  this.len = sz;
  this.used = 0;
}
Buffer.prototype.push = function(dv,len) {
  //console.log( "bufpush:", len, "bytes", "used:", this.used );
  assert( this.used + len <= this.len, "buffer exceeded. buflen:" + this.len + " used:" + this.used + " add:" + len  );
  for(var i=0;i<len;i++) {
    this.dv.setUint8( this.used + i, dv.getUint8(i) );
  }
  this.used += len;
}
Buffer.prototype.shift = function(len) {
    assert( len <= this.used, "bufshift: too long. len:" + len + " used:" + this.used );    
    for(var i=0;i<this.used-len;i++) {
        this.dv.setUint8( i, this.dv.getUint8( len+i) );
    }
    this.used -= len;
}

///////////////////////////////////

function createMoyaiClient(url) {
    var ws = new WebSocket(url, ["binary"]);
    ws.binaryType = "arraybuffer";
    ws.rb = new Buffer(1024*1024);
    ws.onopen = function(event) {
        console.log("wsopen");
    };

    ws.onmessage = function(ev) {
        var dv = new DataView(ev.data);
        if(ws.log)dump( dv, ev.data.byteLength );
        ws.rb.push( dv, ev.data.byteLength );
        for(;;){
            if(!ws.parseDispatch())break;
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
    
    ws.parseDispatch = function() {
        if( ws.rb.used < 4 ) return false;
        var pkt_len = ws.rb.dv.getUint32(0,true);
        if( ws.rb.used <4+pkt_len) return false;
        var argdata_len = pkt_len-4-2;
        var pkttype = ws.rb.dv.getUint16(4+2,true);
        console.log( "pkttype:", pkttype, "argdata_len", argdata_len );
        var u8a = new Uint8Array(argdata_len);
        for(var i=0;i<argdata_len;i++) {
            u8a[i] = ws.rb.dv.getUint8(4+2+i);
        }        
        ws.onPacket(pkttype,u8a);
        ws.rb.shift(pkt_len+4);
        return true;
    };
    ws.onPacket = function(pkttype,argdata) {
        console.log("onPacket not set");
    }

    return ws;
}