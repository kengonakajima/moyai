#include "common.h"
#include "client.h"
#include "Remote.h"

void setupPacketColorReplacerShaderSnapshot( PacketColorReplacerShaderSnapshot *outpkt, ColorReplacerShader *crs ) {
    outpkt->shader_id = crs->id;
    outpkt->epsilon = crs->epsilon;
    copyColorToPacketColor( &outpkt->from_color, &crs->from_color );
    copyColorToPacketColor( &outpkt->to_color, &crs->to_color );
}

//////////////

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
    out->shader_id = target_prop2d->fragment_shader ? target_prop2d->fragment_shader->id : 0;
    out->optbits = 0;
    if( target_prop2d->use_additive_blend ) out->optbits |= PROP2D_OPTBIT_ADDITIVE_BLEND;
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
static const int CHANGED_SHADER = 0x80;
static const int CHANGED_ADDITIVE_BLEND = 0x100;


int getPacketProp2DSnapshotDiff( PacketProp2DSnapshot *s0, PacketProp2DSnapshot *s1 ) {
    int changes = 0;
    if(s0->loc.x != s1->loc.x) changes |= CHANGED_LOC;
    if(s0->loc.y != s1->loc.y) changes |= CHANGED_LOC;
    if(s0->index != s1->index ) changes |= CHANGED_INDEX;
    if(s0->scl.x != s1->scl.x) changes |= CHANGED_SCL;
    if(s0->scl.y != s1->scl.y) changes |= CHANGED_SCL;
    if(s0->rot != s1->rot ) changes |= CHANGED_ROT;
    if(s0->xflip != s1->xflip ) changes |= CHANGED_XFLIP;
    if(s0->yflip != s1->yflip ) changes |= CHANGED_YFLIP;
    if(s0->color.r != s1->color.r ) changes |= CHANGED_COLOR;
    if(s0->color.r != s1->color.r ) changes |= CHANGED_COLOR;    
    if(s0->color.r != s1->color.r ) changes |= CHANGED_COLOR;
    if(s0->color.r != s1->color.r ) changes |= CHANGED_COLOR;
    if(s0->shader_id != s1->shader_id ) changes |= CHANGED_SHADER;
    if( (s0->optbits & PROP2D_OPTBIT_ADDITIVE_BLEND) != (s1->optbits & PROP2D_OPTBIT_ADDITIVE_BLEND) ) changes |= CHANGED_ADDITIVE_BLEND;
    return changes;    
}

// send packet if necessary
bool Tracker2D::checkDiff() {
    PacketProp2DSnapshot *curpkt, *prevpkt;
    if(cur_buffer_index==0) {
        curpkt = & pktbuf[0];
        prevpkt = & pktbuf[1];
    } else {
        curpkt = & pktbuf[1];
        prevpkt = & pktbuf[0];        
    }
    return getPacketProp2DSnapshotDiff( curpkt, prevpkt );
}
void Tracker2D::broadcastDiff( Listener *listener, bool force ) {
    if( checkDiff() || force ) {
        listener->broadcastUS1Bytes( PACKETTYPE_S2C_PROP2D_SNAPSHOT, (const char*)&pktbuf[cur_buffer_index], sizeof(PacketProp2DSnapshot) );
    }
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
    std::unordered_map<int,Font*> fontmap;
    std::unordered_map<int,ColorReplacerShader*> crsmap;
    
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
            if(p->fragment_shader) {
                ColorReplacerShader *crs = dynamic_cast<ColorReplacerShader*>(p->fragment_shader); // TODO: avoid using dyncast..
                if(crs) {
                    crsmap[crs->id] = crs;
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
            TextBox *tb = dynamic_cast<TextBox*>(cur); // TODO: refactor this!
            if(tb) {
                if(tb->font) {
                    fontmap[tb->font->id] = tb->font;
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
        if( img->width>0 && img->buffer) {
            // this image is not from file, maybe generated.
            outco->sendUS1UI3( PACKETTYPE_S2C_IMAGE_ENSURE_SIZE, img->id, img->width, img->height );
        }
        if( img->modified_pixel_num > 0 ) {
            // modified image (includes loadPNG case)
            outco->sendUS1UI3( PACKETTYPE_S2C_IMAGE_ENSURE_SIZE, img->id, img->width, img->height );
            listener->broadcastUS1UI1Bytes( PACKETTYPE_S2C_IMAGE_RAW,
                                            img->id, (const char*) img->buffer, img->getBufferSize() );                
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
    for( std::unordered_map<int,Font*>::iterator it = fontmap.begin(); it != fontmap.end(); ++it ) {
        Font *f = it->second;
        print("sending font id:%d path:%s", f->id, f->last_load_file_path );
        outco->sendUS1UI1( PACKETTYPE_S2C_FONT_CREATE, f->id );
        // utf32toutf8
        outco->sendUS1UI1Wstr( PACKETTYPE_S2C_FONT_CHARCODES, f->id, f->charcode_table, f->charcode_table_used_num );
        outco->sendFile( f->last_load_file_path );
        outco->sendUS1UI2Str( PACKETTYPE_S2C_FONT_LOADTTF, f->id, f->pixel_size, f->last_load_file_path );
    }
    for( std::unordered_map<int,ColorReplacerShader*>::iterator it = crsmap.begin(); it != crsmap.end(); ++it ) {
        ColorReplacerShader *crs = it->second;
        print("sending col repl shader id:%d", crs->id );
        PacketColorReplacerShaderSnapshot ss;
        setupPacketColorReplacerShaderSnapshot(&ss,crs);
        outco->sendUS1Bytes( PACKETTYPE_S2C_COLOR_REPLACER_SHADER_SNAPSHOT, (const char*)&ss, sizeof(ss) );        
    }

    // sounds
    for(int i=0;i<elementof(target_soundsystem->sounds);i++){
        Sound *snd = target_soundsystem->sounds[i];
        if(!snd)continue;

        if( snd->last_load_file_path[0] ) {
            print("sending sound load file: %d, '%s'", snd->id, snd->last_load_file_path );
            outco->sendUS1UI1Str( PACKETTYPE_S2C_SOUND_CREATE_FROM_FILE, snd->id, snd->last_load_file_path );
        } else if( snd->last_samples ){
            outco->sendUS1UI1Bytes( PACKETTYPE_S2C_SOUND_CREATE_FROM_SAMPLES, snd->id,
                                    (const char*) snd->last_samples,
                                    snd->last_samples_num * sizeof(snd->last_samples[0]) );
        }
        outco->sendUS1UI1F1( PACKETTYPE_S2C_SOUND_DEFAULT_VOLUME, snd->id, snd->default_volume );
        if(snd->isPlaying()) {
            outco->sendUS1UI1F2( PACKETTYPE_S2C_SOUND_POSITION, snd->id, snd->getTimePositionSec(), snd->last_play_volume );
        }
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

            TextBox *tb = dynamic_cast<TextBox*>(p);
            if(tb) {
                if(!tb->tracker) {
                    tb->tracker = new TrackerTextBox(this,tb);
                    tb->tracker->scanTextBox();
                }
                tb->tracker->broadcastDiff(listener,true );
            } else {
                // prop body
                if(!p->tracker) {
                    p->tracker = new Tracker2D(this,p);
                    p->tracker->scanProp2D();
                }
                p->tracker->broadcastDiff(listener, true );
                // grid
                for(int i=0;i<p->grid_used_num;i++) {
                    Grid *g = p->grids[i];
                    if(!g->tracker) {
                        g->tracker = new TrackerGrid(this,g);
                        g->tracker->scanGrid();                    
                    }
                    g->tracker->broadcastDiff(p, listener, true );
                }
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
    const size_t MAXBUFSIZE = 1024*1024*4;
    char *buf = (char*) MALLOC(MAXBUFSIZE);
    assert(buf);
    size_t sz = MAXBUFSIZE;
    bool res = readFile( filename, buf, &sz );
    assertmsg(res, "sendFile: file '%s' read error", filename );
    int r = sendUS1StrBytes( PACKETTYPE_S2C_FILE, filename, buf, sz );
    assert(r>0);
    print("sendFile: path:%s len:%d data:%x %x %x %x sendres:%d", filename, sz, buf[0], buf[1], buf[2], buf[3], r );
    FREE(buf);
}

////////////////

TrackerGrid::TrackerGrid( RemoteHead *rh, Grid *target ) : target_grid(target), cur_buffer_index(0), parent_rh(rh) {
    for(int i=0;i<2;i++) {
        index_table[i] = (int32_t*) MALLOC(target->getCellNum() * sizeof(int32_t) );
        color_table[i] = (PacketColor*) MALLOC(target->getCellNum() * sizeof(PacketColor) );
    }
}
TrackerGrid::~TrackerGrid() {
    parent_rh->notifyGridDeleted(target_grid);
    for(int i=0;i<2;i++) {
        FREE( index_table[i] );
        FREE( color_table[i] );
    }
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

// TODO: add a new packet type of sending changes in each cells.
bool TrackerGrid::checkDiff( GRIDTABLETYPE gtt ) { 
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
    return cmp;
}

void TrackerGrid::broadcastDiff( Prop2D *owner, Listener *listener, bool force ) {
    if( checkDiff( GTT_INDEX ) || force ) {
        broadcastGridConfs(owner, listener); // TODO: not necessary to send every time
        listener->broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT, target_grid->id,
                                        (const char*) index_table[cur_buffer_index],
                                        target_grid->getCellNum() * sizeof(int32_t) );
    }
    if( checkDiff( GTT_COLOR ) || force ) {
        broadcastGridConfs(owner, listener); // TODO: not necessary to send every time
        listener->broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT, target_grid->id,
                                        (const char*) color_table[cur_buffer_index],
                                        target_grid->getCellNum() * sizeof(PacketColor) );
    }
}

void TrackerGrid::broadcastGridConfs( Prop2D *owner, Listener *listener ) {
    listener->broadcastUS1UI3( PACKETTYPE_S2C_GRID_CREATE, target_grid->id, target_grid->width, target_grid->height );
    int dk_id = 0;
    if(target_grid->deck) dk_id = target_grid->deck->id; else if(owner->deck) dk_id = owner->deck->id;
    if(dk_id) listener->broadcastUS1UI2( PACKETTYPE_S2C_GRID_DECK, target_grid->id, dk_id );
    listener->broadcastUS1UI2( PACKETTYPE_S2C_GRID_PROP2D, target_grid->id, owner->id );    
}

/////////////

TrackerTextBox::TrackerTextBox(RemoteHead *rh, TextBox *target) : target_tb(target), cur_buffer_index(0), parent_rh(rh) {
    memset( pktbuf, 0, sizeof(pktbuf) );
    memset( strbuf, 0, sizeof(strbuf) );
}
TrackerTextBox::~TrackerTextBox() {
    parent_rh->notifyProp2DDeleted(target_tb);
}
void TrackerTextBox::scanTextBox() {
    PacketProp2DSnapshot *out = & pktbuf[cur_buffer_index];
    out->prop_id = target_tb->id;
    out->layer_id = target_tb->getParentLayer()->id;
    out->loc.x = target_tb->loc.x;
    out->loc.y = target_tb->loc.y;
    out->scl.x = target_tb->scl.x;
    out->scl.y = target_tb->scl.y;
    out->index = 0; // fixed
    out->tiledeck_id = 0; // fixed
    out->grid_id = 0; // fixed
    out->debug = target_tb->debug_id;
    out->rot = 0; // fixed
    out->xflip = 0; // fixed
    out->yflip = 0; // fixed
    out->color.r = target_tb->color.r;
    out->color.g = target_tb->color.g;
    out->color.b = target_tb->color.b;
    out->color.a = target_tb->color.a;

    size_t copy_sz = (target_tb->len_str + 1) * sizeof(wchar_t);
    assertmsg( copy_sz <= MAX_STR_LEN, "textbox string too long" );

    memcpy( strbuf[cur_buffer_index], target_tb->str, copy_sz );
    str_bytes[cur_buffer_index] = copy_sz;
    //    print("scantb: cpsz:%d id:%d s:%s l:%d cbi:%d",copy_sz, target_tb->id, target_tb->str, target_tb->len_str, cur_buffer_index );
}
bool TrackerTextBox::checkDiff() {
    PacketProp2DSnapshot *curpkt, *prevpkt;
    size_t cur_str_bytes, prev_str_bytes;
    uint8_t *cur_str, *prev_str;
    if(cur_buffer_index==0) {
        curpkt = & pktbuf[0];
        prevpkt = & pktbuf[1];
        cur_str_bytes = str_bytes[0];
        prev_str_bytes = str_bytes[1];
        cur_str = strbuf[0];
        prev_str = strbuf[1];        
    } else {
        curpkt = & pktbuf[1];
        prevpkt = & pktbuf[0];
        cur_str_bytes = str_bytes[1];
        prev_str_bytes = str_bytes[0];        
        cur_str = strbuf[1];
        prev_str = strbuf[0];                
    }
    int pktchanges = getPacketProp2DSnapshotDiff( curpkt, prevpkt );
    bool str_changed = false;
    if( cur_str_bytes != prev_str_bytes ) {
        str_changed = true;
    } else if( memcmp( cur_str, prev_str, cur_str_bytes ) ){
        str_changed = true;
    }

    if( str_changed ) {
        //        print("string changed! id:%d l:%d", target_tb->id, cur_str_bytes );
    }
    
    return pktchanges || str_changed;    
}
void TrackerTextBox::flipCurrentBuffer() {
    cur_buffer_index = ( cur_buffer_index == 0 ? 1 : 0 );        
}
void TrackerTextBox::broadcastDiff( Listener *listener, bool force ) {
    if( checkDiff() || force ) {
        listener->broadcastUS1UI1( PACKETTYPE_S2C_TEXTBOX_CREATE, target_tb->id );
        listener->broadcastUS1UI2( PACKETTYPE_S2C_TEXTBOX_LAYER, target_tb->id, target_tb->getParentLayer()->id );        
        listener->broadcastUS1UI2( PACKETTYPE_S2C_TEXTBOX_FONT, target_tb->id, target_tb->font->id );
        listener->broadcastUS1UI1Wstr( PACKETTYPE_S2C_TEXTBOX_STRING, target_tb->id, target_tb->str, target_tb->len_str );
        listener->broadcastUS1UI1F2( PACKETTYPE_S2C_TEXTBOX_LOC, target_tb->id, target_tb->loc.x, target_tb->loc.y );
        listener->broadcastUS1UI1F2( PACKETTYPE_S2C_TEXTBOX_SCL, target_tb->id, target_tb->scl.x, target_tb->scl.y );        
        PacketColor pc;
        pc.r = target_tb->color.r;
        pc.g = target_tb->color.g;
        pc.b = target_tb->color.b;
        pc.a = target_tb->color.a;        
        listener->broadcastUS1UI1Bytes( PACKETTYPE_S2C_TEXTBOX_COLOR, target_tb->id, (const char*)&pc, sizeof(pc) );            
    }
}

//////////////

TrackerColorReplacerShader::~TrackerColorReplacerShader() {
}
void TrackerColorReplacerShader::scanShader() {
    PacketColorReplacerShaderSnapshot *out = &pktbuf[cur_buffer_index];
    out->epsilon = target_shader->epsilon;
    copyColorToPacketColor( &out->from_color, &target_shader->from_color );
    copyColorToPacketColor( &out->to_color, &target_shader->to_color );
}
void TrackerColorReplacerShader::flipCurrentBuffer() {
    cur_buffer_index = ( cur_buffer_index == 0 ? 1 : 0 );            
}
bool TrackerColorReplacerShader::checkDiff() {
    PacketColorReplacerShaderSnapshot *curpkt, *prevpkt;
    if(cur_buffer_index==0) {
        curpkt = &pktbuf[0];
        prevpkt = &pktbuf[1];
    } else {
        curpkt = &pktbuf[1];
        prevpkt = &pktbuf[0];        
    }
    if( ( curpkt->epsilon != prevpkt->epsilon ) ||
        ( curpkt->from_color.r != prevpkt->from_color.r ) ||
        ( curpkt->from_color.g != prevpkt->from_color.g ) ||
        ( curpkt->from_color.b != prevpkt->from_color.b ) ||
        ( curpkt->from_color.a != prevpkt->from_color.a ) ||        
        ( curpkt->to_color.r != prevpkt->to_color.r ) ||
        ( curpkt->to_color.g != prevpkt->to_color.g ) ||
        ( curpkt->to_color.b != prevpkt->to_color.b ) ||
        ( curpkt->to_color.a != prevpkt->to_color.a ) ) {        
        return true;
    } else {
        return false;
    }
}
void TrackerColorReplacerShader::broadcastDiff( Listener *listener, bool force ) {
    if( checkDiff() || force ) {
        PacketColorReplacerShaderSnapshot pkt;
        setupPacketColorReplacerShaderSnapshot( &pkt, target_shader );
        listener->broadcastUS1Bytes( PACKETTYPE_S2C_COLOR_REPLACER_SHADER_SNAPSHOT, (const char*)&pkt, sizeof(pkt) );
    }
}
//////////////////
TrackerPrimDrawer::~TrackerPrimDrawer() {
    if( pktbuf[0] ) FREE(pktbuf[0]);
    if( pktbuf[1] ) FREE(pktbuf[1]);
}
void TrackerPrimDrawer::flipCurrentBuffer() {
    cur_buffer_index = ( cur_buffer_index == 0 ? 1 : 0 );                
}
void TrackerPrimDrawer::scanPrimDrawer() {
    // ensure buffer
    if( pktmax[cur_buffer_index] < target_pd->prim_num ) {
        if( pktbuf[cur_buffer_index] ) {
            print("free buf:%d", cur_buffer_index );
            FREE( pktbuf[cur_buffer_index] );
        }
        
        size_t sz = target_pd->prim_num * sizeof(PacketPrim);
        pktbuf[cur_buffer_index] = (PacketPrim*) MALLOC(sz);
        pktmax[cur_buffer_index] = target_pd->prim_num;
        print( "scanPrimDrawer: malloc buf:%d pktmax:%d sz:%d", cur_buffer_index, pktmax[cur_buffer_index], sz);
    }
    // 
    // scan
    pktnum[cur_buffer_index] = target_pd->prim_num;
    for(int i=0;i<pktnum[cur_buffer_index];i++){
        PacketPrim *outpkt = &pktbuf[cur_buffer_index][i];
        Prim *srcprim = target_pd->prims[i];
        outpkt->prim_id = srcprim->id;
        outpkt->prim_type = srcprim->type;
        outpkt->a.x = srcprim->a.x;
        outpkt->a.y = srcprim->a.y;
        outpkt->b.x = srcprim->b.x;
        outpkt->b.y = srcprim->b.y;
        copyColorToPacketColor( &outpkt->color, &srcprim->color );
        outpkt->line_width = srcprim->line_width;
    }
}
bool TrackerPrimDrawer::checkDiff() {
    PacketPrim *curary, *prevary;
    int curnum, prevnum;
    if(cur_buffer_index==0) {
        curary = pktbuf[0];
        curnum = pktnum[0];
        prevary = pktbuf[1];
        prevnum = pktnum[1];
    } else {
        curary = pktbuf[1];
        curnum = pktnum[1];        
        prevary = pktbuf[0];
        prevnum = pktnum[0];        
    }

    if( prevnum != curnum ) return true;

    for(int i=0;i<curnum;i++ ) {
        PacketPrim *curpkt = &curary[i];
        PacketPrim *prevpkt = &prevary[i];
        if( ( curpkt->prim_id != prevpkt->prim_id ) ||
            ( curpkt->prim_type != prevpkt->prim_type ) ||
            ( curpkt->a.x != prevpkt->a.x ) ||
            ( curpkt->a.y != prevpkt->a.y ) ||
            ( curpkt->b.x != prevpkt->b.x ) ||
            ( curpkt->b.y != prevpkt->b.y ) ||            
            ( curpkt->color.r != prevpkt->color.r ) ||
            ( curpkt->color.g != prevpkt->color.g ) ||
            ( curpkt->color.b != prevpkt->color.b ) ||
            ( curpkt->color.a != prevpkt->color.a ) ||
            ( curpkt->line_width != prevpkt->line_width ) ) {
            return true;
        }
    }
    return false;
}
void TrackerPrimDrawer::broadcastDiff( Prop2D *owner, Listener *listener, bool force ) {
    if( checkDiff() || force ) {
        if( pktnum[cur_buffer_index] > 0 ) {
            //            print("sending %d prims for prop %d", pktnum[cur_buffer_index], owner->id );
            //            for(int i=0;i<pktnum[cur_buffer_index];i++) print("#### primid:%d", pktbuf[cur_buffer_index][i].prim_id );
            listener->broadcastUS1UI1Bytes( PACKETTYPE_S2C_PRIM_BULK_SNAPSHOT,
                                         owner->id,
                                         (const char*) pktbuf[cur_buffer_index],
                                         pktnum[cur_buffer_index] * sizeof(PacketPrim) );
        }
    }
}
//////////////////

TrackerImage::TrackerImage( RemoteHead *rh, Image *target ) : target_image(target), cur_buffer_index(0), parent_rh(rh) {
    size_t sz = target->getBufferSize();
    for(int i=0;i<2;i++) {
        imgbuf[i] = (uint8_t*) MALLOC(sz);
        assert(imgbuf[i]);
    }
}
TrackerImage::~TrackerImage() {
    for(int i=0;i<2;i++) if( imgbuf[i] ) FREE(imgbuf[i]);
}
void TrackerImage::scanImage() {
    uint8_t *dest = imgbuf[cur_buffer_index];
    memcpy( dest, target_image->buffer, target_image->getBufferSize() );
}
void TrackerImage::flipCurrentBuffer() {
    cur_buffer_index = ( cur_buffer_index == 0 ? 1 : 0 );
}
bool TrackerImage::checkDiff() {
    uint8_t *curimg, *previmg;
    if( cur_buffer_index==0) {
        curimg = imgbuf[0];
        previmg = imgbuf[1];
    } else {
        curimg = imgbuf[1];
        previmg = imgbuf[0];
    }
    if( memcmp( curimg, previmg, target_image->getBufferSize() ) != 0 ) {
        return true;
    } else {
        return false;
    }
}
void TrackerImage::broadcastDiff( TileDeck *owner_dk, Listener *listener, bool force ) {
    if( checkDiff() || force ) {
        //        print("TrackerImage::broadcastDiff bufsz:%d", target_image->getBufferSize() );
        listener->broadcastUS1UI3( PACKETTYPE_S2C_IMAGE_ENSURE_SIZE,
                                   target_image->id, target_image->width, target_image->height );
        listener->broadcastUS1UI1Bytes( PACKETTYPE_S2C_IMAGE_RAW,
                                        target_image->id, (const char*) imgbuf[cur_buffer_index], target_image->getBufferSize() );
        listener->broadcastUS1UI2( PACKETTYPE_S2C_TEXTURE_IMAGE, owner_dk->tex->id, target_image->id );
        listener->broadcastUS1UI2( PACKETTYPE_S2C_TILEDECK_TEXTURE, owner_dk->id, owner_dk->tex->id ); // to update tileeck's image_width/height
    }
}
