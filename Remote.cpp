
#include "common.h"
#include "client.h"
#include "Remote.h"


void Tracker2D::scanProp2D() {
    PacketProp2DSnapshot *out = & pktbuf[cur_buffer_index];

    out->prop_id = target_prop2d->id;
    out->layer_id = target_prop2d->getParentLayer()->id;
    out->loc.x = target_prop2d->loc.x;
    out->loc.y = target_prop2d->loc.y;
    out->scl.x = target_prop2d->scl.x;
    out->scl.y = target_prop2d->scl.y;
    out->index = target_prop2d->index;
    out->tiledeck_id = target_prop2d->deck ? target_prop2d->deck->id : 0;
    if( target_prop2d->grid_used_num == 0 ) {
        out->grid_id = 0;
    } else if( target_prop2d->grid_used_num == 1 ) {
        out->grid_id = target_prop2d->grids[0]->id;
    } else {
        assertmsg(false, "Tracker2D: multiple grids are not implemented yet" );
    }
    out->debug = target_prop2d->debug_id;
    out->rot = target_prop2d->rot;
    out->xflip = target_prop2d->xflip;
    out->yflip = target_prop2d->yflip;
    out->color.r = target_prop2d->color.r;
    out->color.g = target_prop2d->color.g;
    out->color.b = target_prop2d->color.b;
    out->color.a = target_prop2d->color.a;    
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
Tracker2D::~Tracker2D() {
    parent_rh->notifyProp2DDeleted(target_prop2d);
}
void RemoteHead::notifyProp2DDeleted( Prop2D *deleted ) {
    listener->broadcastUS1UI1( PACKETTYPE_S2C_PROP2D_DELETE, deleted->id );
}
void RemoteHead::notifyGridDeleted( Grid *deleted ) {
    listener->broadcastUS1UI1( PACKETTYPE_S2C_GRID_DELETE, deleted->id );
}

// Assume all props in all layers are Prop2Ds.
void RemoteHead::track2D() {
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Group *grp = target_moyai->getGroupByIndex(i);
        if(!grp)continue;

        Prop *cur = grp->prop_top;
        while(cur) {
            Prop2D *p = (Prop2D*) cur;
            p->onTrack(this);
            cur = cur->next;
        }        
    }
}
// Send all IDs of tiledecks, layers, textures, fonts, viwports by scanning all props and grids.
// This occurs only when new player is comming in.
void RemoteHead::scanSendAllGraphicsPrerequisites( HMPConn *outco ) {
    std::unordered_map<int,Viewport*> vpmap;
    std::unordered_map<int,Camera*> cammap;
    
    // Viewport , Camera
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Group *grp = target_moyai->getGroupByIndex(i);
        if(!grp)continue;
        Layer *l = (Layer*) grp; // assume all groups are layers
        if(l->viewport) {
            vpmap[l->viewport->id] = l->viewport;
        }
        if(l->camera) {
            cammap[l->camera->id] = l->camera;
        }        
    }
    for( std::unordered_map<int,Viewport*>::iterator it = vpmap.begin(); it != vpmap.end(); ++it ) {
        Viewport *vp = it->second;
        print("sending viewport_create id:%d sz:%d,%d scl:%f,%f", vp->id, vp->screen_width, vp->screen_height, vp->scl.x, vp->scl.y );
        outco->sendUS1UI1( PACKETTYPE_S2C_VIEWPORT_CREATE, vp->id );
        outco->sendUS1UI1F2( PACKETTYPE_S2C_VIEWPORT_SIZE, vp->id, vp->screen_width, vp->screen_height );
        outco->sendUS1UI1F2( PACKETTYPE_S2C_VIEWPORT_SCALE, vp->id, vp->scl.x, vp->scl.y );
    }
    for( std::unordered_map<int,Camera*>::iterator it = cammap.begin(); it != cammap.end(); ++it ) {
        Camera *cam = it->second;
        print("sending camera_create id:%d", cam->id );
        outco->sendUS1UI1( PACKETTYPE_S2C_CAMERA_CREATE, cam->id );
        outco->sendUS1UI1F2( PACKETTYPE_S2C_CAMERA_LOC, cam->id, cam->loc.x, cam->loc.y );
    }
    
    // Layers(Groups) don't need scanning props
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Layer *l = (Layer*) target_moyai->getGroupByIndex(i);
        if(!l)continue;
        print("sending layer_create id:%d",l->id);
        outco->sendUS1UI1( PACKETTYPE_S2C_LAYER_CREATE, l->id );
        if( l->viewport ) outco->sendUS1UI2( PACKETTYPE_S2C_LAYER_VIEWPORT, l->id, l->viewport->id);
        if( l->camera ) outco->sendUS1UI2( PACKETTYPE_S2C_LAYER_CAMERA, l->id, l->camera->id );
    }
    
    // Image, Texture, tiledeck
    std::unordered_map<int,Image*> imgmap;
    std::unordered_map<int,Texture*> texmap;
    std::unordered_map<int,TileDeck*> tdmap;
    
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Group *grp = target_moyai->getGroupByIndex(i);
        if(!grp)continue;

        Prop *cur = grp->prop_top;
        while(cur) {
            Prop2D *p = (Prop2D*) cur;
            if(p->deck) {
                tdmap[p->deck->id] = p->deck;
                if( p->deck->tex) {
                    texmap[p->deck->tex->id] = p->deck->tex;
                    if( p->deck->tex->image ) {
                        imgmap[p->deck->tex->image->id] = p->deck->tex->image;
                    }
                }
            }
            for(int i=0;i<p->grid_used_num;i++) {
                Grid *g = p->grids[i];
                if(g->deck) {
                    tdmap[g->deck->id] = g->deck;
                    if( g->deck->tex) {
                        texmap[g->deck->tex->id] = g->deck->tex;
                        if( g->deck->tex->image ) {
                            imgmap[g->deck->tex->image->id] = g->deck->tex->image;
                        }
                    }
                }
            }
            cur = cur->next;
        }
    }
    // Files
    for( std::unordered_map<int,Image*>::iterator it = imgmap.begin(); it != imgmap.end(); ++it ) {
        Image *img = it->second;
        if( img->last_load_file_path[0] ) {
            print("sending file path:'%s' in image %d", img->last_load_file_path, img->id );
            outco->sendFile( img->last_load_file_path );
        }
    }
    for( std::unordered_map<int,Image*>::iterator it = imgmap.begin(); it != imgmap.end(); ++it ) {
        Image *img = it->second;
        print("sending image_create id:%d", img->id );
        outco->sendUS1UI1( PACKETTYPE_S2C_IMAGE_CREATE, img->id );
        if( img->last_load_file_path[0] ) {
            print("sending image_load_png: '%s'", img->last_load_file_path );
            outco->sendUS1UI1Str( PACKETTYPE_S2C_IMAGE_LOAD_PNG, img->id, img->last_load_file_path );                
        }
    }
    for( std::unordered_map<int,Texture*>::iterator it = texmap.begin(); it != texmap.end(); ++it ) {
        Texture *tex = it->second;
        print("sending texture_create id:%d", tex->id );
        outco->sendUS1UI1( PACKETTYPE_S2C_TEXTURE_CREATE, tex->id );
        outco->sendUS1UI2( PACKETTYPE_S2C_TEXTURE_IMAGE, tex->id, tex->image->id );
    }
    for( std::unordered_map<int,TileDeck*>::iterator it = tdmap.begin(); it != tdmap.end(); ++it ) {
        TileDeck *td = it->second;
        print("sending tiledeck_create id:%d", td->id );
        outco->sendUS1UI1( PACKETTYPE_S2C_TILEDECK_CREATE, td->id );
        outco->sendUS1UI2( PACKETTYPE_S2C_TILEDECK_TEXTURE, td->id, td->tex->id );
        //        print("sendS2RTileDeckSize: id:%d %d,%d,%d,%d", td->id, sprw, sprh, cellw, cellh );        
        outco->sendUS1UI5( PACKETTYPE_S2C_TILEDECK_SIZE, td->id, td->tile_width, td->tile_height, td->cell_width, td->cell_height );        
    }


}
// Send snapshots of all props and grids
void RemoteHead::scanSendAllProp2DSnapshots( HMPConn *c ) {
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Group *grp = target_moyai->getGroupByIndex(i);
        if(!grp)continue;

        Prop *cur = grp->prop_top;
        while(cur) {
            Prop2D *p = (Prop2D*) cur;

            // prop body
            if(!p->tracker) {
                p->tracker = new Tracker2D(this,p);
                p->tracker->scanProp2D();
            }
            char pktbuf[MAX_PACKET_SIZE];
            size_t pkt_size;
            pkt_size = p->tracker->getCurrentPacket( pktbuf, sizeof(pktbuf) );
            if( pkt_size > 0 ) {
                listener->broadcastUS1Bytes( PACKETTYPE_S2C_PROP2D_SNAPSHOT, pktbuf, pkt_size );
            }
            // grid
            for(int i=0;i<p->grid_used_num;i++) {
                Grid *g = p->grids[i];
                if(!g->tracker) {
                    g->tracker = new TrackerGrid(this,g);
                }
                g->tracker->scanGrid();
                broadcastGridConfs(p,g);
                pkt_size = g->tracker->getCurrentPacket( GTT_INDEX, pktbuf, sizeof(pktbuf) );
                if(pkt_size) listener->broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT, g->id, pktbuf, pkt_size );
                pkt_size = g->tracker->getCurrentPacket( GTT_COLOR, pktbuf, sizeof(pktbuf) );
                if(pkt_size) listener->broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT, g->id, pktbuf, pkt_size );
            }

            cur = cur->next;
        }
    }    
}

void RemoteHead::heartbeat() {
    track2D();
    nw->heartbeat();
}    


// return false if can't
bool RemoteHead::startServer( int portnum, bool to_log_syscall ) {        
    tcp_port = portnum;

    if(!nw) {
        nw = new Network();
        nw->syscall_log = to_log_syscall;
    }
    if(!listener) {
        listener = new HMPListener(nw,this);
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
    remote_head->scanSendAllGraphicsPrerequisites(c);
    remote_head->scanSendAllProp2DSnapshots(c);
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

void HMPConn::sendFile( const char *filename ) {
    char buf[64*1024];
    size_t sz = sizeof(buf);
    bool res = readFile( filename, buf, &sz );
    assertmsg(res, "sendFile: file '%s' read error", filename );
    assert( sz <= 65536-8 );
    print("sendFile: path:%s len:%d data:%x %x %x %x", filename, sz, buf[0], buf[1], buf[2], buf[3] );
    sendUS1StrBytes( PACKETTYPE_S2C_FILE, filename, buf, sz );
}


TrackerGrid::TrackerGrid( RemoteHead *rh, Grid *target ) : target_grid(target), cur_buffer_index(0), parent_rh(rh) {
    for(int i=0;i<2;i++) {
        index_table[i] = (int32_t*) MALLOC(target->getCellNum() * sizeof(int32_t) );
        color_table[i] = (PacketColor*) MALLOC(target->getCellNum() * sizeof(PacketColor) );
    }
}
TrackerGrid::~TrackerGrid() {
    parent_rh->notifyGridDeleted(target_grid);   
}
void TrackerGrid::scanGrid() {
    for(int y=0;y<target_grid->height;y++){
        for(int x=0;x<target_grid->width;x++){
            int ind = target_grid->index(x,y);
            index_table[cur_buffer_index][ind] = target_grid->get(x,y);
            Color col = target_grid->getColor(x,y);
            color_table[cur_buffer_index][ind].r = col.r;
            color_table[cur_buffer_index][ind].g = col.g;
            color_table[cur_buffer_index][ind].b = col.b;
            color_table[cur_buffer_index][ind].a = col.a;            
        }
    }
}
void TrackerGrid::flipCurrentBuffer() {
    cur_buffer_index = ( cur_buffer_index == 0 ? 1 : 0 );    
}
// packet data: Array of int32_t with length of width*height
size_t TrackerGrid::getCurrentPacket( GRIDTABLETYPE gtt, char *outpktbuf, size_t maxoutsize ) {
    switch(gtt) {
    case GTT_INDEX:
        {
            if( !target_grid->index_table ) return 0;            
            size_t sz_required = target_grid->width * target_grid->height * sizeof(int32_t);
            assert( maxoutsize >= sz_required );
            memcpy( outpktbuf, index_table[cur_buffer_index], sz_required );
            return sz_required;
        }
        break;
    case GTT_XFLIP:
        assertmsg(false,"not implemented");
        break;        
    case GTT_YFLIP:
        assertmsg(false,"not implemented");        
        break;        
    case GTT_TEXOFS:
        assertmsg(false,"not implemented");        
        break;        
    case GTT_UVROT:
        assertmsg(false,"not implemented");        
        break;
    case GTT_COLOR:
        {
            if( !target_grid->color_table ) return 0;
            size_t sz_required = target_grid->width * target_grid->height * sizeof(PacketColor);
            assert( maxoutsize >= sz_required );
            
            for(int y=0;y<target_grid->height;y++) {
                for(int x=0;x<target_grid->width;x++) {
                    int ind = target_grid->index(x,y);
                    PacketColor *outpktcol = (PacketColor*)( outpktbuf + sizeof(PacketColor) * ind );
                    Color *srccol = & target_grid->color_table[ind];
                    outpktcol->r = srccol->r;
                    outpktcol->g = srccol->g;
                    outpktcol->b = srccol->b;
                    outpktcol->a = srccol->a;
                    
                }
            }
            return sz_required;
        }
        break;
    default:
        assertmsg(false, "invalid gtt:%d",gtt);
        break;
    }
    return 0;    
}
// TODO: add a new packet type of sending changes in each cells.
size_t TrackerGrid::getDiffPacket( GRIDTABLETYPE gtt, char *outpktbuf, size_t maxoutsize ) {
    char *curtbl, *prevtbl;
    int curind, prevind;
    
    if(cur_buffer_index==0) {
        curind = 0;
        prevind = 1;
    } else {
        curind = 1;
        prevind = 0;
    }
    switch(gtt) {
    case GTT_INDEX:
        curtbl = (char*) index_table[curind];
        prevtbl = (char*) index_table[prevind];
        break;
    case GTT_XFLIP:
    case GTT_YFLIP:
    case GTT_TEXOFS:
    case GTT_UVROT:
        curtbl = prevtbl = NULL;
        assertmsg(false, "not implemented");
        break;
    case GTT_COLOR:
        curtbl = (char*) color_table[curind];
        prevtbl = (char*) color_table[prevind];            
        break;
    }

    size_t compsz;
    switch(gtt){
    case GTT_INDEX:
        compsz = target_grid->getCellNum() * sizeof(int32_t);
        break;
    case GTT_XFLIP:
    case GTT_YFLIP:
    case GTT_TEXOFS:
    case GTT_UVROT:
        compsz = 0;
        assertmsg(false, "not implemented");
        break;
    case GTT_COLOR:
        compsz = target_grid->getCellNum() * sizeof(PacketColor);
        break;   
    }
    int cmp = memcmp( curtbl, prevtbl, compsz );
    if( cmp ) {
        assert( maxoutsize >= compsz );
        memcpy( outpktbuf, curtbl, compsz );
        return compsz;
    } else {
        return 0;
    }
}

void RemoteHead::broadcastGridConfs( Prop2D *p, Grid *g ) {
    listener->broadcastUS1UI3( PACKETTYPE_S2C_GRID_CREATE, g->id, g->width, g->height );
    int dk_id = 0;
    if(g->deck) dk_id = g->deck->id; else if(p->deck) dk_id = p->deck->id;
    print("broadcastGridConfs: dk_id:%d gdeck:%d pdeck:%d",dk_id, g->deck?g->deck->id:0, p->deck?p->deck->id:0);
    if(dk_id) listener->broadcastUS1UI2( PACKETTYPE_S2C_GRID_DECK, g->id, dk_id );
    listener->broadcastUS1UI2( PACKETTYPE_S2C_GRID_PROP2D, g->id, p->id );    
}
