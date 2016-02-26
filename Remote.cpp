
#include "common.h"
#include "client.h"
#include "Remote.h"


void Tracker2D::scanProp2D( Prop2D *inp ) {
    PacketProp2DSnapshot *out = & pktbuf[cur_buffer_index];

    out->prop_id = inp->id;
    out->layer_id = inp->getParentLayer()->id;
    out->loc.x = inp->loc.x;
    out->loc.y = inp->loc.y;
    out->scl.x = inp->scl.x;
    out->scl.y = inp->scl.y;
    out->index = inp->index;
    out->tiledeck_id = inp->deck ? inp->deck->id : 0;
    if( inp->grid_used_num == 0 ) {
        out->grid_id = 0;
    } else if( inp->grid_used_num == 1 ) {
        out->grid_id = inp->grids[0]->id;
    } else {
        assertmsg(false, "Tracker2D: multiple grids are not implemented yet" );
    }
    out->debug = inp->debug_id;
    out->rot = inp->rot;
    out->xflip = inp->xflip;
    out->yflip = inp->yflip;
    out->color.r = inp->color.r;
    out->color.g = inp->color.g;
    out->color.b = inp->color.b;
    out->color.a = inp->color.a;    
}

void Tracker2D::flipCurrentBuffer() {
    cur_buffer_index = ( cur_buffer_index == 0 ? 1 : 0 );
}

static const int CHANGED_LOC = 0x1;
static const int CHANGED_INDEX = 0x2;
static const int CHANGED_SCL = 0x4;
static const int CHANGED_ROT = 0x8;
static const int CHANGED_XFLIP = 0x10;
static const int CHANGED_YFLIP = 0x20;
static const int CHANGED_COLOR = 0x40;


size_t Tracker2D::getDiffPacket( char *outpktbuf, size_t maxoutsize ) {
    PacketProp2DSnapshot *curpkt, *prevpkt;
    if(cur_buffer_index==0) {
        curpkt = & pktbuf[0];
        prevpkt = & pktbuf[1];
    } else {
        curpkt = & pktbuf[1];
        prevpkt = & pktbuf[0];        
    }
    int changes = 0;
    if(curpkt->loc.x != prevpkt->loc.x) changes |= CHANGED_LOC;
    if(curpkt->loc.y != prevpkt->loc.y) changes |= CHANGED_LOC;
    if(curpkt->index != prevpkt->index ) changes |= CHANGED_INDEX;
    if(curpkt->scl.x != prevpkt->scl.x) changes |= CHANGED_SCL;
    if(curpkt->scl.y != prevpkt->scl.y) changes |= CHANGED_SCL;
    if(curpkt->rot != prevpkt->rot ) changes |= CHANGED_ROT;
    if(curpkt->xflip != prevpkt->xflip ) changes |= CHANGED_XFLIP;
    if(curpkt->yflip != prevpkt->yflip ) changes |= CHANGED_YFLIP;
    if(curpkt->color.r != prevpkt->color.r ) changes |= CHANGED_COLOR;
    if(curpkt->color.r != prevpkt->color.r ) changes |= CHANGED_COLOR;    
    if(curpkt->color.r != prevpkt->color.r ) changes |= CHANGED_COLOR;
    if(curpkt->color.r != prevpkt->color.r ) changes |= CHANGED_COLOR;    

    if( changes == 0 ) {
        return 0;
    }
    size_t outsz = sizeof(PacketProp2DSnapshot);
    assertmsg( outsz <= maxoutsize, "buffer too small" );
    memcpy(outpktbuf, curpkt, outsz );
    return outsz;    
}

size_t Tracker2D::getCurrentPacket( char *outpktbuf, size_t maxoutsize ) {
    PacketProp2DSnapshot *curpkt = & pktbuf[cur_buffer_index];
    size_t outsz = sizeof(PacketProp2DSnapshot);
    assertmsg( outsz <= maxoutsize, "buffer too small" );
    memcpy(outpktbuf, curpkt, outsz );
    return outsz;
}

// Assume all props in all layers are Prop2Ds.
void RemoteHead::track2D( Moyai *m ) {
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Group *grp = m->getGroupByIndex(i);
        if(!grp)continue;

        Prop *cur = grp->prop_top;
        while(cur) {
            Prop2D *p = (Prop2D*) cur;

            if( !p->tracker ) {
                // A new prop!
                p->tracker = new Tracker2D();
                p->tracker->scanProp2D(p);
                char pktbuf[1024];
                size_t pkt_size;
                pkt_size = p->tracker->getCurrentPacket(pktbuf, sizeof(pktbuf) );
                if( pkt_size>0) {
                    print("track2D: new: prop[%d] pkt size:%d", p->id, pkt_size );
                }
            } else {
                p->tracker->flipCurrentBuffer();
                p->tracker->scanProp2D(p);
                char pktbuf[1024];
                size_t pkt_size;
                pkt_size = p->tracker->getDiffPacket(pktbuf, sizeof(pktbuf));
                if(pkt_size>0) {
                    //                    print("track2D: diff: prop[%d] changed. pkt size:%d", p->id, pkt_size );
                }
            }
            
            cur = cur->next;
        }
        
    }
}



// return false if can't
bool RemoteHead::startServer( int portnum ) {        
    tcp_port = portnum;

    if(!nw) {
        nw = new Network();
        nw->syscall_log = true;
    }
    if(!listener) {
        listener = new HMPListener(nw);
        bool success = listener->startListen( "", DEFAULT_PORT);
        if(!success) {
            print("RemoteHead::startServer: listen failed on port %d", DEFAULT_PORT );
            delete listener;
            return false;
        }
    }
    return true;
}

void HMPListener::onAccept( int newfd ) {
    HMPConn *c = new HMPConn(this->parent_nw,newfd);
    addConn(c);
    print("HMPListener::onAccept. newcon id:%d", c->id );
}

void HMPConn::onError( NET_ERROR e, int eno ) {
    print("HMPConn::onError: id:%d e:%d eno:%d",id,e,eno);
}
void HMPConn::onClose() {
    print("HMPConn::onClose. id:%d",id);
}
void HMPConn::onConnect() {
    print("HMPConn::onConnect. id:%d",id);
}
void HMPConn::onFunction( int funcid, char *argdata, size_t argdatalen ) {
    print("HMPConn::onfunction. id:%d fid:%d len:%d",id, funcid, argdatalen );    
}


int sendPacket_noarg( Conn *c, unsigned short pkttype ) {
    if(!c)return 0;
    size_t totalsize = 2 + 2;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
}
int sendPacket_i1( Conn *c, unsigned short pkttype, int iarg0 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    assert( sizeof(int) == 4 );
    c->sendbuf.push( (char*)&iarg0, 4 );
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
}
int sendPacket_i2( Conn *c, unsigned short pkttype, int iarg0, int iarg1 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    assert( sizeof(int) == 4 );
    c->sendbuf.push( (char*)&iarg0, 4 );
    c->sendbuf.push( (char*)&iarg1, 4 );
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
}
int sendPacket_i3( Conn *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    assert( sizeof(int) == 4 );
    c->sendbuf.push( (char*)&iarg0, 4 );
    c->sendbuf.push( (char*)&iarg1, 4 );
    c->sendbuf.push( (char*)&iarg2, 4 );    
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
}
int sendPacket_i4( Conn *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2, int iarg3 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    assert( sizeof(int) == 4 );
    c->sendbuf.push( (char*)&iarg0, 4 );
    c->sendbuf.push( (char*)&iarg1, 4 );
    c->sendbuf.push( (char*)&iarg2, 4 );
    c->sendbuf.push( (char*)&iarg3, 4 );        
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
}
int sendPacket_i5( Conn *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2, int iarg3, int iarg4 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4 + 4 + 4;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    assert( sizeof(int) == 4 );
    c->sendbuf.push( (char*)&iarg0, 4 );
    c->sendbuf.push( (char*)&iarg1, 4 );
    c->sendbuf.push( (char*)&iarg2, 4 );
    c->sendbuf.push( (char*)&iarg3, 4 );
    c->sendbuf.push( (char*)&iarg4, 4 );            
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
}
int sendPacket_ints( Conn *c, unsigned short pkttype, int *iargs, int argn ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4*argn;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    assert( sizeof(int) == 4 );
    c->sendbuf.push( (char*)iargs, argn * sizeof(int) );
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
}

int sendPacket_f2( Conn *c, unsigned short pkttype, float f0, float f1 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    assert( sizeof(int) == 4 );
    c->sendbuf.push( (char*)&f0, 4 );
    c->sendbuf.push( (char*)&f1, 4 );
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
}
int sendPacket_i1_f1( Conn *c, unsigned short pkttype, unsigned int i0, float f0 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    assert( sizeof(unsigned int) == 4 );
    c->sendbuf.push( (char*)&i0, 4 );    
    c->sendbuf.push( (char*)&f0, 4 );
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
    
}
int sendPacket_i1_f2( Conn *c, unsigned short pkttype, unsigned int i0, float f0, float f1 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    assert( sizeof(unsigned int) == 4 );
    c->sendbuf.push( (char*)&i0, 4 );    
    c->sendbuf.push( (char*)&f0, 4 );
    c->sendbuf.push( (char*)&f1, 4 );
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
}
int sendPacket_i2_f2( Conn *c, unsigned short pkttype, unsigned int i0, unsigned int i1, float f0, float f1 ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4 + 4;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    assert( sizeof(unsigned int) == 4 );
    c->sendbuf.push( (char*)&i0, 4 );
    c->sendbuf.push( (char*)&i1, 4 );        
    c->sendbuf.push( (char*)&f0, 4 );
    c->sendbuf.push( (char*)&f1, 4 );
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;    
}
int sendPacket_i1_floats( Conn *c, unsigned short pkttype, unsigned int i0, float *fargs, int argn ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 * argn ;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype ); // packet type
    c->sendbuf.push( (char*)&i0, 4 );
    assert( sizeof(float) == 4 );
    c->sendbuf.push( (char*)fargs, argn * sizeof(float) );
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;    
}

int sendPacket_bytes( Conn *c, unsigned short pkttype, char *buf, size_t buflen ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + buflen;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype );
    c->sendbuf.push( buf, buflen );
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
}
// [record-len:16][pkttype:16][cstr-len:8][cstr-body][data-len:16][data-body]
int sendPacket_str_bytes( Conn *c, unsigned short pkttype, const char *cstr, const char *data, unsigned short datalen ) {
    if(!c)return 0;    
    int cstrlen = strlen(cstr);
    assert( cstrlen <= 255 );
    size_t totalsize = 2 + 2 + 1 + cstrlen + 2 + datalen;
    assertmsg( totalsize <= 65535, "datalen too big? : %d", datalen );
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype );
    c->sendbuf.pushU8( (unsigned char) cstrlen );
    c->sendbuf.push( cstr, cstrlen );
    c->sendbuf.pushU16( datalen );
    c->sendbuf.push( data, datalen );
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    //    print("sendPacket_str_bytes: cstrlen:%d datalen:%d totallen:%d", cstrlen, datalen, totalsize );
    return totalsize;
}
void parse_packet_str_bytes( char *inptr, char *outcstr, char **outptr, size_t *outsize ) {
    unsigned char slen = get_u8(inptr);
    char *s = inptr + 1;
    unsigned short datalen = get_u16(inptr+1+slen);
    *outptr = inptr + 1 + slen + 2;
    memcpy( outcstr, s, slen );
    outcstr[slen]='\0';
    *outsize = (size_t) datalen;
}
// [record-len:16][pkttype:16][i0:32][cstr-len:8][cstr-body]
int sendPacket_i1_str( Conn *c, unsigned short pkttype, int i0, const char *cstr ) {
    if(!c)return 0;    
    int cstrlen = strlen(cstr);
    assert( cstrlen <= 255 );
    size_t totalsize = 2 + 2 + 4 + 1 + cstrlen;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype );
    c->sendbuf.pushU32( i0 );
    c->sendbuf.pushU8( (unsigned char) cstrlen );
    c->sendbuf.push( cstr, cstrlen );
    ev_io_start( c->parent_nw->evloop, c->write_watcher );
    return totalsize;
}
int sendPacket_i1_bytes( Conn *c, unsigned short pkttype, int iarg0, const char *buf, unsigned short datalen ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + datalen;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype );
    c->sendbuf.pushU32( iarg0 );
    c->sendbuf.push( buf, datalen );
    return totalsize;
}
int sendPacket_i2_bytes( Conn *c, unsigned short pkttype, int iarg0, int iarg1, const char *buf, unsigned short datalen ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + datalen;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype );
    c->sendbuf.pushU32( iarg0 );
    c->sendbuf.pushU32( iarg1 );
    c->sendbuf.push( buf, datalen );
    return totalsize;
}
int sendPacket_i3_bytes( Conn *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2, const char *buf, unsigned short datalen ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4 + datalen;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype );
    c->sendbuf.pushU32( iarg0 );
    c->sendbuf.pushU32( iarg1 );
    c->sendbuf.pushU32( iarg2 );    
    c->sendbuf.push( buf, datalen );
    return totalsize;
}
int sendPacket_i4_bytes( Conn *c, unsigned short pkttype, int iarg0, int iarg1, int iarg2, int iarg3, const char *buf, unsigned short datalen ) {
    if(!c)return 0;    
    size_t totalsize = 2 + 2 + 4 + 4 + 4 + 4 + datalen;
    if( c->getSendbufRoom() < totalsize ) return 0;
    c->sendbuf.pushU16( totalsize - 2 ); // record-len
    c->sendbuf.pushU16( pkttype );
    c->sendbuf.pushU32( iarg0 );
    c->sendbuf.pushU32( iarg1 );
    c->sendbuf.pushU32( iarg2 );
    c->sendbuf.pushU32( iarg3 );    
    c->sendbuf.push( buf, datalen );
    return totalsize;
}
