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
    for(var i=0;i<u8adata.length;i++) this.data[i]=u8adata[i];
}
File.prototype.comparePath = function(path) {
    return (this.path==path);
}
function FileDepo() {
    this.files=[];
}
FileDepo.prototype.get = function(path) {
    for(var i=0;i<this.files.length;i++) {
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

