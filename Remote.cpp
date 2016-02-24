
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
                    print("track2D: diff: prop[%d] changed. pkt size:%d", p->id, pkt_size );
                }
            }
            
            cur = cur->next;
        }
        
    }
}

// return false if can't
bool RemoteHead::startServer( int portnum ) {        
    tcp_port = portnum;

}
