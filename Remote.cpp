
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


size_t Tracker2D::getDiffPacket( char *outpktbuf, size_t maxoutsize, PACKETTYPE *pktttype ) {
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
    *pktttype = PACKETTYPE_S2C_PROP2D_SNAPSHOT;
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
void RemoteHead::track2D() {
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Group *grp = target_moyai->getGroupByIndex(i);
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
                PACKETTYPE pkttype;
                pkt_size = p->tracker->getDiffPacket(pktbuf, sizeof(pktbuf), &pkttype );
                if(pkt_size>0) {
                    prt("%d ", pkt_size );
                    listener->broadcastUS1Bytes( pkttype, pktbuf, pkt_size );
                    //                    print("track2D: diff: prop[%d] changed. pkt size:%d", p->id, pkt_size );
                }
            }
            
            cur = cur->next;
        }
        
    }
}
// Send all IDs of tiledecks, layers, textures, fonts, viwports by scanning all props and grids.
// This occurs only when new player is comming in.
void RemoteHead::scanSendAllGraphicsPrerequisites( Conn *outco ) {
    // Layers(Groups) don't need scanning props
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Group *grp = target_moyai->getGroupByIndex(i);
        if(!grp)continue;
        outco->sendUS1UI1( PACKETTYPE_S2C_LAYER_CREATE, grp->id );
    }
    
    // 
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Group *grp = target_moyai->getGroupByIndex(i);
        if(!grp)continue;

        Prop *cur = grp->prop_top;
        while(cur) {
            Prop2D *p = (Prop2D*) cur;
            cur = cur->next;
        }
    }
}

void RemoteHead::heartbeat() {
    track2D();
    nw->heartbeat();
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
void HMPConn::onPacket( uint16_t funcid, char *argdata, size_t argdatalen ) {
    print("HMPConn::onfunction. id:%d fid:%d len:%d",id, funcid, argdatalen );
    switch(funcid) {
    case PACKETTYPE_C2S_GET_ALL_PREREQUISITES:
        print("PACKETTYPE_C2S_GET_ALL_PREREQUISITES");
        break;
    default:
        print("unhandled funcid: %d",funcid);
        break;
    }
}


#if 0

//////////////////
void sendS2RGridCreateSnapshot( conn_t *c, int prop_id, Grid *g ) {
    PacketGridCreateSnapshot pkt;
    pkt.id = g->id;
    pkt.width = g->width;
    pkt.height = g->height;
    pkt.tiledeck_id = g->deck->id;
    pkt.enfat_epsilon = g->enfat_epsilon;
    send_packet_i1_bytes( c, PACKETTYPE_S2R_GRID_CREATE_SNAPSHOT, prop_id, (char*)&pkt, sizeof(pkt) );
    if( g->index_table ) {
        int nbytes = g->width * g->height * sizeof(int);
        send_packet_i3_bytes( c, PACKETTYPE_S2R_GRID_TABLE_SNAPSHOT, prop_id, g->id, nbytes, (const char*) g->index_table, nbytes );
    }
    if( g->xflip_table ) {
        assert(false);
    }
    if( g->yflip_table ) {
        assert(false);        
    }
    if( g->texofs_table ) {
        assert(false);        
    }
    if( g->rot_table ) {
        assert(false);        
    }
    if( g->color_table ) {
        assert(false);        
    }
}
void sendS2RProp2DGrid( conn_t *c, Prop2D *p, Grid *g ) {
    if(p->debug) print("sendProp2DGrid. p:%d g:%d", p->id, g->id );
    send_packet_i2( c, PACKETTYPE_S2R_PROP2D_GRID, p->id, g->id );
}



void sendS2RLayerCreate( conn_t *c, int player_id, Layer *l, bool shared ) {
    print("sendS2RLayerCreate pl:%d id:%d", player_id, l->id);
    send_packet_i3( c, PACKETTYPE_S2R_LAYER_CREATE, player_id, l->id, shared );    
}
void sendS2RViewportCreate( conn_t *c, int player_id, Viewport *vp ) {
    print("sendS2RLayerSize pl:%d id:%d ", player_id, vp->id );
    send_packet_i2( c, PACKETTYPE_S2R_VIEWPORT_CREATE, player_id, vp->id );
}
void sendS2RViewportSize( conn_t *c, int player_id, Viewport *vp ) {
    print("sendS2RViewportSize. pl:%d id:%d w:%d h:%d", player_id, vp->id, vp->screen_width, vp->screen_height );
    send_packet_i4( c, PACKETTYPE_S2R_VIEWPORT_SIZE, player_id, vp->id, vp->screen_width, vp->screen_height );
}
void sendS2RViewportScale( conn_t *c, int player_id, Viewport *vp ) {
    //    print("sendS2RViewportScale. pl:%d id:%d x:%f y:%f", player_id, vp->id, vp->scl.x, vp->scl.y );
    send_packet_i2_f2( c, PACKETTYPE_S2R_VIEWPORT_SCALE, player_id, vp->id, vp->scl.x, vp->scl.y );
}
void sendS2RCameraCreate( conn_t *c, int player_id, Camera *cam ) {
    print("sendS2RCameraCreate. pl:%d id:%d", player_id, cam->id );
    send_packet_i2( c, PACKETTYPE_S2R_CAMERA_CREATE, player_id, cam->id );
}
void sendS2RCameraLoc( conn_t *c, int player_id, Camera *cam ) {
    //    print("sendS2RCameraLoc. id:%d x:%f y:%f", cam->id, cam->loc.x, cam->loc.y );
    send_packet_i2_f2( c, PACKETTYPE_S2R_CAMERA_LOC, player_id, cam->id, cam->loc.x, cam->loc.y );
}
void sendS2RLayerCamera( conn_t *c, int player_id, Layer *l ) {
    assert( l->camera );
    print("sendS2RLayerCamera: pl:%d id:%d cam:%d", player_id, l->id, l->camera->id );
    send_packet_i3( c, PACKETTYPE_S2R_LAYER_CAMERA, player_id, l->id, l->camera->id );
}
void sendS2RLayerViewport( conn_t *c, int player_id, Layer *l ) {
    assert( l->viewport );
    print("sendS2RLayerCamera: plid:%d lid:%d vp:%d", player_id, l->id, l->viewport->id );
    send_packet_i3( c, PACKETTYPE_S2R_LAYER_VIEWPORT, player_id, l->id, l->viewport->id );
}
void sendS2RFile( conn_t *c, const char *filename ) {
    char buf[64*1024];
    size_t sz = sizeof(buf);
    bool res = readFile( filename, buf, &sz );
    assertmsg(res, "sendFile: file '%s' read error", filename );
    assert( sz <= 65536-8 );
    print("sendFile: path:%s len:%d data:%x %x %x %x", filename, sz, buf[0], buf[1], buf[2], buf[3] );
    send_packet_str_bytes( c, PACKETTYPE_S2R_FILE, filename, buf, sz );
}
void sendS2RImageCreate( conn_t *c, Image *img ) {
    print("sendS2RImageCreate: id:%d", img->id );
    send_packet_i1( c, PACKETTYPE_S2R_IMAGE_CREATE, img->id );
}
void sendS2RImageLoadPNG( conn_t *c, Image *img, const char *filename ) {
    print("sendS2RImageLoadPNG: id:%d file:%s", img->id, filename );
    send_packet_i1_str( c, PACKETTYPE_S2R_IMAGE_LOAD_PNG, img->id, filename );
}
void sendS2RTextureCreate( conn_t *c, Texture *tex ) {
    print("sendS2RTextureCreate: id:%d", tex->id );
    send_packet_i1( c, PACKETTYPE_S2R_TEXTURE_CREATE, tex->id );
}
void sendS2RTextureImage( conn_t *c, Texture *tex, Image *img ) {
    print("sendS2RTextureImage: tex:%d img:%d", tex->id, img->id );
    send_packet_i2( c, PACKETTYPE_S2R_TEXTURE_IMAGE, tex->id, img->id );
}
void sendS2RTileDeckCreate( conn_t *c, TileDeck *dk ) {
    print("sendS2RTileDeckCreate id:%d", dk->id );
    send_packet_i1( c, PACKETTYPE_S2R_TILEDECK_CREATE, dk->id );
}
void sendS2RTileDeckTexture( conn_t *c, TileDeck *dk, Texture *tex ) {
    print("sendS2RTileDeckTexture dk:%d tex:%d", dk->id, tex->id );
    send_packet_i2( c, PACKETTYPE_S2R_TILEDECK_TEXTURE, dk->id, tex->id );
}
void sendS2RTileDeckSize( conn_t *c, TileDeck *dk, int sprw, int sprh, int cellw, int cellh ) {
    print("sendS2RTileDeckSize: id:%d %d,%d,%d,%d", dk->id, sprw, sprh, cellw, cellh );
    send_packet_i5( c, PACKETTYPE_S2R_TILEDECK_SIZE, dk->id, sprw, sprh, cellw, cellh );
}






void sendS2RProp2DCreateSnapshot( conn_t *c, Prop2D *p ) {
    PacketProp2DCreateSnapshot pkt;
    pkt.prop_id = p->id;
    Group *g = p->getParentGroup();
    pkt.layer_id = g->id;
    pkt.loc.x = p->getLocX();
    pkt.loc.y = p->getLocY();
    if(p->debug) print("sendProp2DCreateSnapshot: id:%d gid:%d loc:(%f,%f)",p->id, p->getParentGroup()->id, p->getLocX(), p->getLocY());
    pkt.scl.x = p->getSclX();
    pkt.scl.y = p->getSclY();
    pkt.index = p->getIndex();
    pkt.tiledeck_id = p->getDeckId();
    pkt.grid_id = p->getGridId();
    pkt.debug = p->debug;
    pkt.rot = p->getRot();
    pkt.xflip = p->getXFlip();
    pkt.yflip = p->getYFlip();
    Color col = p->getColor();
    pkt.color.r = col.r;
    pkt.color.g = col.g;
    pkt.color.b = col.b;
    pkt.color.a = col.a;    
    send_packet_bytes( c, PACKETTYPE_S2R_PROP2D_CREATE_SNAPSHOT, (char*)&pkt, sizeof(pkt) );
    Grid *grid = p->getGrid();
    if( grid ) {
        sendS2RGridCreateSnapshot( c, p->id, grid );
        sendS2RProp2DGrid( c, p, grid );
    }
}


#endif
