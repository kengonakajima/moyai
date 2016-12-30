#include "common.h"
#include "client.h"
#include "Remote.h"
#include "JPEGCoder.h"


#ifdef USE_UNTZ
#include "threading/Threading.h" // To implement lock around send buffer inside libuv
RCriticalSection g_lock;
#endif

#include "ConvertUTF.h"



////////

void Moyai::globalInitNetwork() {
    static bool g_global_init_done = false;
    
    if( g_global_init_done ) return;
    
#ifdef WIN32
    WSADATA data;
    WSAStartup(MAKEWORD(2,0), &data);
#endif
#ifndef WIN32
    signal( SIGPIPE, SIG_IGN );
#endif        

}

void uv_run_times( int maxcount ) {
    for(int i=0;i<maxcount;i++) {
        uv_run( uv_default_loop(), UV_RUN_NOWAIT );
    }
}

///////////

void setupPacketColorReplacerShaderSnapshot( PacketColorReplacerShaderSnapshot *outpkt, ColorReplacerShader *crs ) {
    outpkt->shader_id = crs->id;
    outpkt->epsilon = crs->epsilon;
    copyColorToPacketColor( &outpkt->from_color, &crs->from_color );
    copyColorToPacketColor( &outpkt->to_color, &crs->to_color );
}

//////////////
void copyPrimToPacketPrim( PacketPrim*out, Prim *src ) {
    out->prim_id = src->id;
    out->prim_type = src->type;
    out->a.x = src->a.x;
    out->a.y = src->a.y;
    out->b.x = src->b.x;
    out->b.y = src->b.y;
    copyColorToPacketColor( &out->color, &src->color );
    out->line_width = src->line_width;
}
    
void makePacketProp2DSnapshot( PacketProp2DSnapshot *out, Prop2D *tgt, Prop2D *parent ) {
    out->prop_id = tgt->id;
    if( parent ) {
        out->layer_id = 0;
        out->parent_prop_id = parent->id;
    } else {
        out->layer_id = tgt->getParentLayer()->id;
        out->parent_prop_id = 0;
    }
    out->loc.x = tgt->loc.x;
    out->loc.y = tgt->loc.y;
    out->scl.x = tgt->scl.x;
    out->scl.y = tgt->scl.y;
    out->index = tgt->index;
    out->tiledeck_id = tgt->deck ? tgt->deck->id : 0;
    out->debug = tgt->debug_id;
    out->rot = tgt->rot;
    out->xflip = tgt->xflip;
    out->yflip = tgt->yflip;
    out->uvrot = tgt->uvrot;
    out->color.r = tgt->color.r;
    out->color.g = tgt->color.g;
    out->color.b = tgt->color.b;
    out->color.a = tgt->color.a;
    out->shader_id = tgt->fragment_shader ? tgt->fragment_shader->id : 0;
    out->optbits = 0;
    if( tgt->use_additive_blend ) out->optbits |= PROP2D_OPTBIT_ADDITIVE_BLEND;
    out->priority = tgt->priority;
    
}
void Tracker2D::scanProp2D( Prop2D *parentprop ) {
    PacketProp2DSnapshot *out = & pktbuf[cur_buffer_index];

    makePacketProp2DSnapshot(out,target_prop2d,parentprop);
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
static const int CHANGED_OPTBITS = 0x100;
static const int CHANGED_PRIORITY = 0x200;

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
    if(s0->color.g != s1->color.g ) changes |= CHANGED_COLOR;    
    if(s0->color.b != s1->color.b ) changes |= CHANGED_COLOR;
    if(s0->color.a != s1->color.a ) changes |= CHANGED_COLOR;
    if(s0->shader_id != s1->shader_id ) changes |= CHANGED_SHADER;
    if(s0->optbits != s1->optbits ) changes |= CHANGED_OPTBITS;
    if(s0->priority != s1->priority ) changes |= CHANGED_PRIORITY;
    return changes;    
}

// send packet if necessary
int Tracker2D::checkDiff() {
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
void Tracker2D::broadcastDiff( bool force ) {
    int diff = checkDiff();
    if( diff || force ) {
        if( diff == CHANGED_LOC && (!force) ) {
            int prev_buffer_index = cur_buffer_index==0?1:0;
            Vec2 v0(pktbuf[prev_buffer_index].loc.x,pktbuf[prev_buffer_index].loc.y);
            Vec2 v1(pktbuf[cur_buffer_index].loc.x,pktbuf[cur_buffer_index].loc.y);
            float l = v0.len(v1);
            target_prop2d->loc_sync_score+= l;
            
            // only location changed!
            if( target_prop2d->locsync_mode == LOCSYNCMODE_LINEAR ) {
                bool to_send = true;                
                if(target_prop2d->loc_sync_score>40 ) {
                    target_prop2d->loc_sync_score=0;
                } else if( target_prop2d->poll_count>2 ){
                    to_send = false;
                }
                if(to_send) {
                    Vec2 vel = (v1 - v0) / target_prop2d->getParentLayer()->last_dt;
                    parent_rh->broadcastUS1UI1F4( PACKETTYPE_S2C_PROP2D_LOC_VEL,
                                                  pktbuf[cur_buffer_index].prop_id,
                                                  pktbuf[cur_buffer_index].loc.x, pktbuf[cur_buffer_index].loc.y,
                                                  vel.x, vel.y
                                                  );                    
                }
                //                print("l:%f lss:%f id:%d", l, target_prop2d->loc_sync_score, target_prop2d->id);
            } else {
                target_prop2d->loc_sync_score+=1; // avoid missing syncing stopped props
                if( target_prop2d->loc_sync_score < 50 ) {
                    if( !parent_rh->appendChangelist( target_prop2d, &pktbuf[cur_buffer_index] ) ) {
                        // must send if changelist is full
                        parent_rh->nearcastUS1UI1F2( target_prop2d, PACKETTYPE_S2C_PROP2D_LOC,
                                                     pktbuf[cur_buffer_index].prop_id,
                                                     pktbuf[cur_buffer_index].loc.x, pktbuf[cur_buffer_index].loc.y );                    
                    }
                } else {
                    target_prop2d->loc_sync_score=0;                                
                    // dont use changelist sorting for big changes
                    parent_rh->nearcastUS1UI1F2( target_prop2d, PACKETTYPE_S2C_PROP2D_LOC,
                                                 pktbuf[cur_buffer_index].prop_id,
                                                 pktbuf[cur_buffer_index].loc.x, pktbuf[cur_buffer_index].loc.y );
                }
            }
        } else if( diff == CHANGED_SCL && (!force) ) {
            parent_rh->broadcastUS1UI1F2( PACKETTYPE_S2C_PROP2D_SCALE,
                                          pktbuf[cur_buffer_index].prop_id,
                                          pktbuf[cur_buffer_index].scl.x, pktbuf[cur_buffer_index].scl.y );
        } else if( diff == CHANGED_ROT && (!force) ) {
            parent_rh->broadcastUS1UI1F1( PACKETTYPE_S2C_PROP2D_ROT,
                                          pktbuf[cur_buffer_index].prop_id,
                                          pktbuf[cur_buffer_index].rot );
        } else if( diff == CHANGED_COLOR && (!force) ) {
            parent_rh->broadcastUS1UI1Bytes( PACKETTYPE_S2C_PROP2D_COLOR,
                                             pktbuf[cur_buffer_index].prop_id,
                                             (const char*)&pktbuf[cur_buffer_index].color, sizeof(PacketColor));
        } else if( diff == CHANGED_INDEX && (!force) ) {
            parent_rh->broadcastUS1UI2( PACKETTYPE_S2C_PROP2D_INDEX, pktbuf[cur_buffer_index].prop_id, pktbuf[cur_buffer_index].index );
        } else if( diff == CHANGED_XFLIP && (!force) ) {
            prt("XFL ");
            parent_rh->broadcastUS1UI2( PACKETTYPE_S2C_PROP2D_XFLIP, pktbuf[cur_buffer_index].prop_id, pktbuf[cur_buffer_index].xflip );
        } else if( diff == CHANGED_YFLIP && (!force) ) {
            prt("YFL ");
            parent_rh->broadcastUS1UI2( PACKETTYPE_S2C_PROP2D_YFLIP, pktbuf[cur_buffer_index].prop_id, pktbuf[cur_buffer_index].yflip );            
        } else if( diff == CHANGED_OPTBITS && (!force) ) {
            prt("OPT ");
            parent_rh->broadcastUS1UI2( PACKETTYPE_S2C_PROP2D_OPTBITS, pktbuf[cur_buffer_index].prop_id, pktbuf[cur_buffer_index].optbits );
        } else if( diff == CHANGED_PRIORITY && (!force) ) {
            prt("PRI ");
            parent_rh->broadcastUS1UI2( PACKETTYPE_S2C_PROP2D_PRIORITY, pktbuf[cur_buffer_index].prop_id, pktbuf[cur_buffer_index].priority );
        } else {
            //                        prt("SS%d ",diff);            
            parent_rh->broadcastUS1Bytes( PACKETTYPE_S2C_PROP2D_SNAPSHOT, (const char*)&pktbuf[cur_buffer_index], sizeof(PacketProp2DSnapshot) );
        }        
    }
}

Tracker2D::~Tracker2D() {
    parent_rh->notifyProp2DDeleted(target_prop2d);
}
void RemoteHead::notifyProp2DDeleted( Prop2D *deleted ) {
    broadcastUS1UI1( PACKETTYPE_S2C_PROP2D_DELETE, deleted->id );
}
void RemoteHead::notifyChildCleared( Prop2D *owner_prop, Prop2D *child_prop ) {
    broadcastUS1UI2( PACKETTYPE_S2C_PROP2D_CLEAR_CHILD, owner_prop->id, child_prop->id );
}
void RemoteHead::notifyGridDeleted( Grid *deleted ) {
    broadcastUS1UI1( PACKETTYPE_S2C_GRID_DELETE, deleted->id );
}

void RemoteHead::addClient( Client *cl ) {
    Client *stored = cl_pool.get(cl->id);
    if(!stored) {
        cl->parent_rh = this;
        cl_pool.set(cl->id,cl);
    }
}
void RemoteHead::delClient( Client *cl ) {
    cl_pool.del(cl->id);
}
Client *RemoteHead::getFirstClient() {
    return cl_pool.getFirst();
}
int RemoteHead::getClientCount() {
    return cl_pool.size();
}
// Assume all props in all layers are Prop2Ds.
void RemoteHead::track2D() {
    broadcastTimestamp();
    clearChangelist();
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Layer *layer = (Layer*) target_moyai->getGroupByIndex(i);
        if(!layer)continue;
        if(layer->hasDynamicCameras()) {
            layer->onTrackDynamicCameras();
        } else if(layer->camera) layer->camera->onTrack(this);
        if(layer->hasDynamicViewports()) {
            layer->onTrackDynamicViewports();
        } else if(layer->viewport) layer->viewport->onTrack(this);
        Prop *cur = layer->prop_top;
        while(cur) {
            Prop2D *p = (Prop2D*) cur;
            p->onTrack(this, NULL);
            cur = cur->next;
        }        
    }
    broadcastSortedChangelist();
}
// Send all IDs of tiledecks, layers, textures, fonts, viwports by scanning all props and grids.
// This occurs only when new player is comming in.
void RemoteHead::scanSendAllPrerequisites( uv_stream_t *outstream ) {
    if( window_width==0 || window_height==0) {
        assertmsg( false, "remotehead: window size not set?");
    }
    
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
        sendViewportCreateScale(outstream,vp);
    }
    for( std::unordered_map<int,Camera*>::iterator it = cammap.begin(); it != cammap.end(); ++it ) {
        Camera *cam = it->second;
        print("sending camera_create id:%d", cam->id );
        sendCameraCreateLoc(outstream,cam);
    }
    
    // Layers(Groups) don't need scanning props
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Layer *l = (Layer*) target_moyai->getGroupByIndex(i);
        if(!l)continue;
        print("sending layer_create id:%d",l->id);
        sendLayerSetup(outstream,l);
    }
    
    // Image, Texture, tiledeck
    std::unordered_map<int,Image*> imgmap;
    std::unordered_map<int,Texture*> texmap;
    std::unordered_map<int,Deck*> dkmap;
    std::unordered_map<int,Font*> fontmap;
    std::unordered_map<int,ColorReplacerShader*> crsmap;
    
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Group *grp = target_moyai->getGroupByIndex(i);
        if(!grp)continue;

        Prop *cur = grp->prop_top;
        while(cur) {
            Prop2D *p = (Prop2D*) cur;
            if(p->deck) {
                dkmap[p->deck->id] = p->deck;
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
                    dkmap[g->deck->id] = g->deck;
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
            sendFile( outstream, img->last_load_file_path );
        }
    }
    for( std::unordered_map<int,Image*>::iterator it = imgmap.begin(); it != imgmap.end(); ++it ) {
        Image *img = it->second;
        sendImageSetup(outstream,img);
    }
    for( std::unordered_map<int,Texture*>::iterator it = texmap.begin(); it != texmap.end(); ++it ) {
        Texture *tex = it->second;
        //        print("sending texture_create id:%d", tex->id );
        sendTextureCreateWithImage(outstream,tex);
    }
    for( std::unordered_map<int,Deck*>::iterator it = dkmap.begin(); it != dkmap.end(); ++it ) {
        Deck *dk = it->second;
        sendDeckSetup(outstream,dk);
    }
    for( std::unordered_map<int,Font*>::iterator it = fontmap.begin(); it != fontmap.end(); ++it ) {
        Font *f = it->second;
        sendFontSetupWithFile(outstream,f);
    }
    for( std::unordered_map<int,ColorReplacerShader*>::iterator it = crsmap.begin(); it != crsmap.end(); ++it ) {
        ColorReplacerShader *crs = it->second;
        sendColorReplacerShaderSetup(outstream,crs);
    }

    // sounds
    for(int i=0;i<elementof(target_soundsystem->sounds);i++){
        if(!target_soundsystem)break;
        Sound *snd = target_soundsystem->sounds[i];
        if(!snd)continue;
        sendSoundSetup(outstream,snd);
    }
}

// Send snapshots of all props and grids
void RemoteHead::scanSendAllProp2DSnapshots( uv_stream_t *outstream ) {
    for(int i=0;i<Moyai::MAXGROUPS;i++) {
        Layer *layer = (Layer*) target_moyai->getGroupByIndex(i);
        if(!layer)continue;

        Prop *cur = layer->prop_top;
        while(cur) {
            Prop2D *p = (Prop2D*) cur;

            TextBox *tb = dynamic_cast<TextBox*>(p);
            if(tb) {
                if(!tb->tracker) {
                    tb->tracker = new TrackerTextBox(this,tb);
                    tb->tracker->scanTextBox();
                }
                tb->tracker->broadcastDiff(true);
            } else {
                // prop body
                if(!p->tracker) {
                    p->tracker = new Tracker2D(this,p);
                    p->tracker->scanProp2D(NULL);
                }
                p->tracker->broadcastDiff(true);
                // grid
                for(int i=0;i<p->grid_used_num;i++) {
                    Grid *g = p->grids[i];
                    if(!g->tracker) {
                        g->tracker = new TrackerGrid(this,g);
                        g->tracker->scanGrid();                    
                    }
                    g->tracker->broadcastDiff(p, true );
                }
                // prims
                if(p->prim_drawer) {
                    if( !p->prim_drawer->tracker) p->prim_drawer->tracker = new TrackerPrimDrawer(this,p->prim_drawer);
                    p->prim_drawer->tracker->scanPrimDrawer();
                    p->prim_drawer->tracker->broadcastDiff(p, true );
                }
                
                // children
                for(int i=0;i<p->children_num;i++) {
                    Prop2D *chp = p->children[i];
                    if(!chp->tracker) {
                        chp->tracker = new Tracker2D(this,chp);
                        chp->tracker->scanProp2D(p);
                    }
                    chp->tracker->broadcastDiff(true);
                }
            }
            cur = cur->next;
        }
    }    
}
void RemoteHead::heartbeat() {
    if(enable_spritestream) track2D();
    if(enable_videostream) broadcastCapturedScreen();
    if( (!enable_videostream) && (!enable_spritestream) ) {
        print("RemoteHead::heartbeat: no streaming enabled, please call enableSpriteStream or enableVideoStream. ");
    }
#ifdef USE_UNTZ    
    if(audio_buf_ary){
        RScopedLock _l(&g_lock);
        for(;;) {
            size_t used = audio_buf_ary->getUsedNum();
            if(used==0)break;
            Buffer *b = audio_buf_ary->getTop();
            assert(b);
            //            print("heartbeat: audio used:%d next buf len:%d",audio_buf_ary->getUsedNum(), b->used );
            assert(b->used % (sizeof(float)*2) == 0 ); // L+R of float sample
            broadcastUS1UI1Bytes( PACKETTYPE_S2C_CAPTURED_AUDIO, b->used/sizeof(float)/2, b->buf, b->used );

            static int total_audio_samples_sent_bytes = 0;
            total_audio_samples_sent_bytes += b->used;
            //            print("sent audio: %f %d", now(), total_audio_samples_sent_bytes );
            audio_buf_ary->shift();
        }
    }
#endif
    
    uv_run_times(100);
}    
static void remotehead_on_close_callback( uv_handle_t *s ) {
    print("remotehead_on_close_callback");
    Client *cli = (Client*)s->data;
    cli->parent_rh->delClient(cli); // Call this before on_disconnect_cb to make it possible to delete prop in callback and it causes write to network
    if( cli->parent_rh->on_disconnect_cb ) {
        cli->parent_rh->on_disconnect_cb( cli->parent_rh, cli );
    }
    cli->onDelete();
    delete cli;
}
static void remotehead_on_packet_callback( uv_stream_t *s, uint16_t funcid, char *argdata, uint32_t argdatalen ) {
    Client *cli = (Client*)s->data;
    //    print("on_packet_callback. id:%d fid:%d len:%d", funcid, argdatalen );
    switch(funcid) {
    case PACKETTYPE_PING:
        {
            uint32_t sec = get_u32(argdata+0);
            uint32_t usec = get_u32(argdata+4);
            sendUS1UI2( s, PACKETTYPE_PING, sec, usec );
        }
        break;
    case PACKETTYPE_C2S_KEYBOARD:
        {
            uint32_t keycode = get_u32(argdata);
            uint32_t action = get_u32(argdata+4);
            uint32_t modbits = get_u32(argdata+8);
            bool mod_shift,mod_ctrl,mod_alt;
            getModkeyBits(modbits, &mod_shift, &mod_ctrl, &mod_alt);
            //                        print("kbd: %d %d %d %d %d", keycode, action, mod_shift, mod_ctrl, mod_alt );            
            if(cli->parent_rh->on_keyboard_cb) {
                cli->parent_rh->on_keyboard_cb(cli,keycode,action,mod_shift,mod_ctrl,mod_alt);
            }
        }
        break;
    case PACKETTYPE_C2S_MOUSE_BUTTON:
        {
            uint32_t button = get_u32(argdata);
            uint32_t action = get_u32(argdata+4);
            uint32_t modbits = get_u32(argdata+8);
            bool mod_shift,mod_ctrl,mod_alt;
            getModkeyBits(modbits, &mod_shift, &mod_ctrl, &mod_alt);
                        print("mou: %d %d %d %d %d", button, action, mod_shift, mod_ctrl, mod_alt );
            if(cli->parent_rh->on_mouse_button_cb) {
                cli->parent_rh->on_mouse_button_cb(cli,button,action,mod_shift,mod_ctrl,mod_alt);
            }
        }
        break;
    case PACKETTYPE_C2S_CURSOR_POS:
        {
            float x = get_f32(argdata);
            float y = get_f32(argdata+4);
            if(cli->parent_rh->on_mouse_cursor_cb) {
                cli->parent_rh->on_mouse_cursor_cb(cli,x,y);
            }            
        }
        break;
    default:
        print("unhandled funcid: %d",funcid);
        break;
    }
}

static void remotehead_on_read_callback( uv_stream_t *s, ssize_t nread, const uv_buf_t *inbuf ) {
    Client *cl = (Client*) s->data;
    if(nread>0) {
        bool res = parseRecord( s, &cl->recvbuf, inbuf->base, nread, remotehead_on_packet_callback );
        if(!res) {
            print("receiveData failed");
            uv_close( (uv_handle_t*)s, remotehead_on_close_callback );
            return;
        }        
    } else if( nread <= 0 ) {
        print("on_read_callback EOF. clid:%d", cl->id );
        uv_close( (uv_handle_t*)s, remotehead_on_close_callback );
    }
}

void moyai_libuv_alloc_buffer( uv_handle_t *handle, size_t suggested_size, uv_buf_t *outbuf ) {
    *outbuf = uv_buf_init( (char*) MALLOC(suggested_size), suggested_size );
}

static void remotehead_on_accept_callback( uv_stream_t *listener, int status ) {
    if( status != 0 ) {
        print("remotehead_on_accept_callback status:%d", status);
        return;
    }

    uv_tcp_t *newsock = (uv_tcp_t*) MALLOC( sizeof(uv_tcp_t) );
    uv_tcp_init( uv_default_loop(), newsock );
    if( uv_accept( listener, (uv_stream_t*) newsock ) == 0 ) {
        RemoteHead *rh = (RemoteHead*)listener->data;
        Client *cl = new Client(newsock, rh );        
        newsock->data = cl;
        cl->tcp = newsock;
        cl->parent_rh->addClient(cl);
        print("remotehead_on_accept_callback. ok status:%d client-id:%d", status, cl->id );

        int r = uv_read_start( (uv_stream_t*) newsock, moyai_libuv_alloc_buffer, remotehead_on_read_callback );
        if(r) {
            print("uv_read_start: fail ret:%d",r);
            return;
        }

        sendWindowSize((uv_stream_t*)newsock, cl->parent_rh->window_width, cl->parent_rh->window_height);

        if(rh->enable_spritestream) {
            cl->parent_rh->scanSendAllPrerequisites( (uv_stream_t*) newsock );
            cl->parent_rh->scanSendAllProp2DSnapshots( (uv_stream_t*) newsock);
        }
        if(rh->enable_videostream) {
            JPEGCoder *jc = cl->parent_rh->jc;
            assert(jc);
            sendUS1UI3( (uv_stream_t*)newsock, PACKETTYPE_S2C_JPEG_DECODER_CREATE, jc->capture_pixel_skip, jc->orig_w, jc->orig_h );
        }
        if( cl->parent_rh->on_connect_cb ) {
            cl->parent_rh->on_connect_cb( cl->parent_rh, cl );
        }
    }
}

bool init_tcp_listener( uv_tcp_t *l, void *data, int portnum, void (*cb)(uv_stream_t*l,int status) ) {
    
    int r = uv_tcp_init( uv_default_loop(), l );
    if(r) {
        print("uv_tcp_init failed");
        return false;
    }
    l->data = data;
    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", portnum, &addr );
    r = uv_tcp_bind( l, (const struct sockaddr*) &addr, SO_REUSEADDR );
    if(r) {
        print("uv_tcp_bind failed");
        return false;
    }
    r = uv_listen( (uv_stream_t*)l, 10, cb );
    if(r) {
        print("uv_listen failed");
        return false;
    }
    return true;
    
}

// return false if can't
// TODO: implement error handling
bool RemoteHead::startServer( int portnum ) {
    return init_tcp_listener( &listener, (void*)this, portnum, remotehead_on_accept_callback );
}

void RemoteHead::notifySoundPlay( Sound *snd, float vol ) {
    if(enable_spritestream) broadcastUS1UI1F1( PACKETTYPE_S2C_SOUND_PLAY, snd->id, vol );
}
void RemoteHead::notifySoundStop( Sound *snd ) {
    if(enable_spritestream) broadcastUS1UI1( PACKETTYPE_S2C_SOUND_STOP, snd->id );
}

// [numframes of float values for ch1][numframes of float values for ch2]
void RemoteHead::appendAudioSamples( uint32_t numChannels, float *interleavedSamples, uint32_t numSamples ) {
#ifdef USE_UNTZ
    if(!audio_buf_ary)return;

    RScopedLock _l(&g_lock);
    //    print("pushing samples. numSamples:%d numChannels:%d", numSamples, numChannels );
    bool ret = audio_buf_ary->push( (const char*)interleavedSamples, numSamples * numChannels * sizeof(float) );
    if(!ret) print("appendAudioSamples: audio_buffer full?");
    //        print("appendAudioSamples pushed %d bytes. ret:%d used:%d", numSamples*sizeof(float), ret, audio_buffer->used );
#else
    print("appendAudioSamples is't implemented");
#endif
}

////////////////

TrackerGrid::TrackerGrid( RemoteHead *rh, Grid *target ) : target_grid(target), cur_buffer_index(0), parent_rh(rh) {
    for(int i=0;i<2;i++) {
        index_table[i] = (int32_t*) MALLOC(target->getCellNum() * sizeof(int32_t) );
        flip_table[i] = (uint8_t*) MALLOC(target->getCellNum() * sizeof(uint8_t) );
        texofs_table[i] = (PacketVec2*) MALLOC(target->getCellNum() * sizeof(PacketVec2) );
        color_table[i] = (PacketColor*) MALLOC(target->getCellNum() * sizeof(PacketColor) );
    }
}
TrackerGrid::~TrackerGrid() {
    parent_rh->notifyGridDeleted(target_grid);
    for(int i=0;i<2;i++) {
        FREE( index_table[i] );
        FREE( flip_table[i] );
        FREE( texofs_table[i] );
        FREE( color_table[i] );
    }
}
void TrackerGrid::scanGrid() {
    for(int y=0;y<target_grid->height;y++){
        for(int x=0;x<target_grid->width;x++){
            int ind = target_grid->index(x,y);
            index_table[cur_buffer_index][ind] = target_grid->get(x,y);
            uint8_t bits = 0;
            if( target_grid->getXFlip(x,y) ) bits |= GTT_FLIP_BIT_X;
            if( target_grid->getYFlip(x,y) ) bits |= GTT_FLIP_BIT_Y;
            if( target_grid->getUVRot(x,y) ) bits |= GTT_FLIP_BIT_UVROT;
            flip_table[cur_buffer_index][ind] = bits;
            Vec2 texofs;
            target_grid->getTexOffset(x,y,&texofs);
            texofs_table[cur_buffer_index][ind].x = texofs.x;
            texofs_table[cur_buffer_index][ind].y = texofs.y;
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
    case GTT_FLIP:
        curtbl = (char*) flip_table[curind];
        prevtbl = (char*) flip_table[prevind];
        break;
    case GTT_TEXOFS:
        curtbl = (char*) texofs_table[curind];
        prevtbl = (char*) texofs_table[prevind];
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
    case GTT_FLIP:
        compsz = target_grid->getCellNum() * sizeof(uint8_t);
        break;
    case GTT_TEXOFS:
        compsz = target_grid->getCellNum() * sizeof(Vec2);
        break;
    case GTT_COLOR:
        compsz = target_grid->getCellNum() * sizeof(PacketColor);
        break;   
    }
    int cmp = memcmp( curtbl, prevtbl, compsz );
    return cmp;
}

void TrackerGrid::broadcastDiff( Prop2D *owner, bool force ) {
    if( checkDiff( GTT_INDEX ) || force ) {
        broadcastGridConfs(owner); // TODO: not necessary to send every time
        parent_rh->broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT, target_grid->id,
                                         (const char*) index_table[cur_buffer_index],
                                         target_grid->getCellNum() * sizeof(int32_t) );
    }
    if( checkDiff( GTT_FLIP) || force ) {
        broadcastGridConfs(owner); // TODO: not necessary to send every time
        parent_rh->broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_FLIP_SNAPSHOT, target_grid->id,
                                         (const char*) flip_table[cur_buffer_index],
                                         target_grid->getCellNum() * sizeof(uint8_t) );
    }
    if( checkDiff( GTT_TEXOFS ) || force ) {
        broadcastGridConfs(owner);
        parent_rh->broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_TEXOFS_SNAPSHOT, target_grid->id,
                                         (const char*) texofs_table[cur_buffer_index],
                                         target_grid->getCellNum() * sizeof(Vec2) );
    }
    if( checkDiff( GTT_COLOR ) || force ) {
        broadcastGridConfs(owner); // TODO: not necessary to send every time
        parent_rh->broadcastUS1UI1Bytes( PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT, target_grid->id,
                                         (const char*) color_table[cur_buffer_index],
                                         target_grid->getCellNum() * sizeof(PacketColor) );
    }
}

void TrackerGrid::broadcastGridConfs( Prop2D *owner ) {
    parent_rh->broadcastUS1UI3( PACKETTYPE_S2C_GRID_CREATE, target_grid->id, target_grid->width, target_grid->height );
    int dk_id = 0;
    if(target_grid->deck) dk_id = target_grid->deck->id; else if(owner->deck) dk_id = owner->deck->id;
    if(dk_id) parent_rh->broadcastUS1UI2( PACKETTYPE_S2C_GRID_DECK, target_grid->id, dk_id );
    parent_rh->broadcastUS1UI2( PACKETTYPE_S2C_GRID_PROP2D, target_grid->id, owner->id );    
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
    out->debug = target_tb->debug_id;
    out->rot = 0; // fixed
    out->xflip = 0; // fixed
    out->yflip = 0; // fixed
    out->color.r = target_tb->color.r;
    out->color.g = target_tb->color.g;
    out->color.b = target_tb->color.b;
    out->color.a = target_tb->color.a;
    out->priority = target_tb->priority;

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
void TrackerTextBox::broadcastDiff( bool force ) {
    if( checkDiff() || force ) {
        parent_rh->broadcastUS1UI1( PACKETTYPE_S2C_TEXTBOX_CREATE, target_tb->id );
        parent_rh->broadcastUS1UI2( PACKETTYPE_S2C_TEXTBOX_LAYER, target_tb->id, target_tb->getParentLayer()->id );        
        parent_rh->broadcastUS1UI2( PACKETTYPE_S2C_TEXTBOX_FONT, target_tb->id, target_tb->font->id );
        parent_rh->broadcastUS1UI1Wstr( PACKETTYPE_S2C_TEXTBOX_STRING, target_tb->id, target_tb->str, target_tb->len_str );
        parent_rh->broadcastUS1UI1F2( PACKETTYPE_S2C_TEXTBOX_LOC, target_tb->id, target_tb->loc.x, target_tb->loc.y );
        parent_rh->broadcastUS1UI1F2( PACKETTYPE_S2C_TEXTBOX_SCL, target_tb->id, target_tb->scl.x, target_tb->scl.y );        
        PacketColor pc;
        pc.r = target_tb->color.r;
        pc.g = target_tb->color.g;
        pc.b = target_tb->color.b;
        pc.a = target_tb->color.a;        
        parent_rh->broadcastUS1UI1Bytes( PACKETTYPE_S2C_TEXTBOX_COLOR, target_tb->id, (const char*)&pc, sizeof(pc) );            
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
void TrackerColorReplacerShader::broadcastDiff( bool force ) {
    if( checkDiff() || force ) {
        PacketColorReplacerShaderSnapshot pkt;
        setupPacketColorReplacerShaderSnapshot( &pkt, target_shader );
        parent_rh->broadcastUS1Bytes( PACKETTYPE_S2C_COLOR_REPLACER_SHADER_SNAPSHOT, (const char*)&pkt, sizeof(pkt) );
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
            FREE( pktbuf[cur_buffer_index] );
        }
        
        size_t sz = target_pd->prim_num * sizeof(PacketPrim);
        pktbuf[cur_buffer_index] = (PacketPrim*) MALLOC(sz);
        pktmax[cur_buffer_index] = target_pd->prim_num;
    }
    // 
    // scan
    pktnum[cur_buffer_index] = target_pd->prim_num;
    for(int i=0;i<pktnum[cur_buffer_index];i++){
        copyPrimToPacketPrim( &pktbuf[cur_buffer_index][i], target_pd->prims[i] );
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
void TrackerPrimDrawer::broadcastDiff( Prop2D *owner, bool force ) {
    if( checkDiff() || force ) {
        if( pktnum[cur_buffer_index] > 0 ) {
            //            print("sending %d prims for prop %d", pktnum[cur_buffer_index], owner->id );
            //            for(int i=0;i<pktnum[cur_buffer_index];i++) print("#### primid:%d", pktbuf[cur_buffer_index][i].prim_id );
            parent_rh->broadcastUS1UI1Bytes( PACKETTYPE_S2C_PRIM_BULK_SNAPSHOT,
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
void TrackerImage::broadcastDiff( Deck *owner_dk, bool force ) {
    if( checkDiff() || force ) {
        assertmsg( owner_dk->getUperCell()>0, "only tiledeck is supported now" );
        //        print("TrackerImage::broadcastDiff bufsz:%d", target_image->getBufferSize() );
        parent_rh->broadcastUS1UI3( PACKETTYPE_S2C_IMAGE_ENSURE_SIZE,
                                   target_image->id, target_image->width, target_image->height );
        parent_rh->broadcastUS1UI1Bytes( PACKETTYPE_S2C_IMAGE_RAW,
                                         target_image->id, (const char*) imgbuf[cur_buffer_index], target_image->getBufferSize() );
        parent_rh->broadcastUS1UI2( PACKETTYPE_S2C_TEXTURE_IMAGE, owner_dk->tex->id, target_image->id );
        parent_rh->broadcastUS1UI2( PACKETTYPE_S2C_TILEDECK_TEXTURE, owner_dk->id, owner_dk->tex->id ); // to update tileeck's image_width/height
    }
}
////////////////////
TrackerCamera::TrackerCamera( RemoteHead *rh, Camera *target ) : target_camera(target), cur_buffer_index(0), parent_rh(rh) {
}
TrackerCamera::~TrackerCamera() {
}
void TrackerCamera::scanCamera() {
    locbuf[cur_buffer_index] = Vec2( target_camera->loc.x, target_camera->loc.y );
}
void TrackerCamera::flipCurrentBuffer() {
    cur_buffer_index = ( cur_buffer_index == 0 ? 1 : 0 );
}
bool TrackerCamera::checkDiff() {
    Vec2 curloc, prevloc;
    if( cur_buffer_index == 0 ) {
        curloc = locbuf[0];
        prevloc = locbuf[1];
    } else {
        curloc = locbuf[1];
        prevloc = locbuf[0];
    }
    return curloc != prevloc;
}
void TrackerCamera::broadcastDiff( bool force ) {
    if( checkDiff() || force ) {
        parent_rh->broadcastUS1UI1F2( PACKETTYPE_S2C_CAMERA_LOC, target_camera->id, locbuf[cur_buffer_index].x, locbuf[cur_buffer_index].y );
    }
}
void TrackerCamera::unicastDiff( Client *dest, bool force ) {
    if( checkDiff() || force ) {
        if( dest->isReprecation()) {
            sendUS1UI2F2( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2R_CAMERA_LOC, dest->id,  target_camera->id, locbuf[cur_buffer_index].x, locbuf[cur_buffer_index].y );
        } else { 
            sendUS1UI1F2( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2C_CAMERA_LOC, target_camera->id, locbuf[cur_buffer_index].x, locbuf[cur_buffer_index].y );
        }
    }
}
void TrackerCamera::unicastCreate( Client *dest ) {
    print("TrackerCamera: unicastCreate. id:%d repr:%d",dest->id, dest->isReprecation() );
    if( dest->isReprecation() ) {
        print("sending s2r_cam_creat clid:%d camid:%d",dest->id, target_camera->id);
        sendUS1UI2( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2R_CAMERA_CREATE, dest->id, target_camera->id );
    } else {
        print("sending s2c_cam_creat camid:%d",target_camera->id);
        sendUS1UI1( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2C_CAMERA_CREATE, target_camera->id );
    }
    POOL_SCAN(target_camera->target_layers,Layer) {
        Layer *l = it->second;
        if(dest->isReprecation()) {
            print("sending s2r_cam_dyn_lay cl:%d cam:%d lay:%d", dest->id, target_camera->id, l->id);
            sendUS1UI3( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2R_CAMERA_DYNAMIC_LAYER, dest->id, target_camera->id, l->id );            
        } else {
            print("sending s2c_cam_dyN_lay cam:%d lay:%d ", target_camera->id, l->id);
            sendUS1UI2( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2C_CAMERA_DYNAMIC_LAYER, target_camera->id, l->id );
        }
    }
}

//////////////////////
TrackerViewport::TrackerViewport( RemoteHead *rh, Viewport *target ) : target_viewport(target), cur_buffer_index(0), parent_rh(rh) {
}
TrackerViewport::~TrackerViewport() {
}
void TrackerViewport::scanViewport() {
    sclbuf[cur_buffer_index] = Vec2( target_viewport->scl.x, target_viewport->scl.y );
}
void TrackerViewport::flipCurrentBuffer() {
    cur_buffer_index = ( cur_buffer_index == 0 ? 1 : 0 );        
}
bool TrackerViewport::checkDiff() {
    Vec2 curscl, prevscl;
    if( cur_buffer_index == 0 ) {
        curscl = sclbuf[0];
        prevscl = sclbuf[1];
    } else {
        curscl = sclbuf[1];
        prevscl = sclbuf[0];
    }
    return curscl != prevscl;
}
void TrackerViewport::broadcastDiff( bool force ) {
    if( checkDiff() | force ) {
        parent_rh->broadcastUS1UI1F2( PACKETTYPE_S2C_VIEWPORT_SCALE, target_viewport->id, sclbuf[cur_buffer_index].x, sclbuf[cur_buffer_index].y );
    }
}
void TrackerViewport::unicastDiff( Client *dest, bool force ) {
    if( checkDiff() || force ) {
        if(dest->isReprecation()) {
            print("XXXXXXXXXX:%f %f", sclbuf[cur_buffer_index].x, sclbuf[cur_buffer_index].y );
            sendUS1UI2F2( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2R_VIEWPORT_SCALE, dest->id, target_viewport->id, sclbuf[cur_buffer_index].x, sclbuf[cur_buffer_index].y );
        } else {
            sendUS1UI1F2( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2C_VIEWPORT_SCALE, target_viewport->id, sclbuf[cur_buffer_index].x, sclbuf[cur_buffer_index].y );
        }        
    }
}
void TrackerViewport::unicastCreate( Client *dest ) {
    print("TrackerViewport::unicastCreate. id:%d",dest->id);
    if(dest->isReprecation()) {
        sendUS1UI2( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2R_VIEWPORT_CREATE, dest->id, target_viewport->id );        
    } else {
        sendUS1UI1( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2C_VIEWPORT_CREATE, target_viewport->id );
    }
    
    for(std::unordered_map<unsigned int,Layer*>::iterator it = target_viewport->target_layers.idmap.begin();
        it != target_viewport->target_layers.idmap.end(); ++it ) {
        Layer *l = it->second;
        print("  TrackerViewport::unicastCreate: camera_dynamic_layer:%d", l->id );
        if(dest->isReprecation()) {
            sendUS1UI3( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2R_VIEWPORT_DYNAMIC_LAYER, dest->id, target_viewport->id, l->id );
        } else {
            sendUS1UI2( (uv_stream_t*) dest->getTCP(), PACKETTYPE_S2C_VIEWPORT_DYNAMIC_LAYER, target_viewport->id, l->id );
        }        
    }
}

/////////////////////
static void reprecator_on_packet_cb( uv_stream_t *s, uint16_t funcid, char *argdata, uint32_t argdatalen ) {
    //    print("reprecator_on_packet_cb. funcid:%d",funcid);
    Client *realcl = (Client*)s->data;
    Reprecator *rep = realcl->parent_reprecator;
    assert(rep);
    RemoteHead *rh = rep->parent_rh;
    assert(rh);
    
    switch(funcid) {
    case PACKETTYPE_R2S_CLIENT_LOGIN:
        {
            int reproxy_cl_id = get_u32(argdata+0);
            Client *newcl = Client::createLogicalClient((uv_tcp_t*)s,rh);
            rep->addLogicalClient(newcl);
            print("received r2s_login. giving a new newclid:%d, reproxy_cl_id:%d",newcl->id, reproxy_cl_id);
            sendUS1UI2(s,PACKETTYPE_S2R_NEW_CLIENT_ID, newcl->id, reproxy_cl_id );
            if(rh->on_connect_cb) {
                rh->on_connect_cb(rh,newcl);
            }
        }
        break;
    case PACKETTYPE_R2S_CLIENT_LOGOUT:
        {
            int gclid = get_u32(argdata+0);
            print("received r2s_client_logout gclid:%d",gclid);
            Client *logcl = rep->logical_cl_pool.get(gclid);
            if(logcl) {
                assert(logcl->id==gclid);
                print("found client, deleting");
                if(logcl->parent_rh->on_disconnect_cb) {
                    logcl->parent_rh->on_disconnect_cb( logcl->parent_rh,logcl);
                }
                logcl->onDelete();
                rep->logical_cl_pool.del(logcl->id);
                delete logcl;
            } else {
                print("can't find logical client id:%d",gclid);
            }
        }
        break;
    case PACKETTYPE_R2S_KEYBOARD:
        {
            uint32_t logclid = get_u32(argdata+0);
            uint32_t kc = get_u32(argdata+4);
            uint32_t act = get_u32(argdata+8);
            uint32_t modbits = get_u32(argdata+12);
            print("received r2s_kbd. logclid:%d kc:%d act:%d modbits:%d", logclid, kc, act, modbits );
            bool mod_shift,mod_ctrl,mod_alt;
            getModkeyBits(modbits, &mod_shift, &mod_ctrl, &mod_alt);
            /*
              if(cli->parent_rh->on_keyboard_cb) {
                cli->parent_rh->on_keyboard_cb(cli,keycode,action,mod_shift,mod_ctrl,mod_alt);
              }
              これをやるには、cliが必要。cliは、tcpとrecvbufがある。

              アプリとしてはcli->idは識別のために欲しい。cliのポインタも使っている(これは最悪IDからの検索にできる)
              
              案1: LOGINのときにnew Clientやって、parentが何もない論理的なクライアントとしてフラグをたてておいて、それを引数として使う。
              案2: on_keyboard_cbを2種類にして、論理的なクライアントからのと直接のクライアントのを分ける。

              案1が必要。論理的なクライアントは必要。なぜならカメラとかも必要だから。
              
             */
            Client *logcl = rep->getLogicalClient(logclid);
            if(logcl) {
                if(rh->on_keyboard_cb) {
                    rh->on_keyboard_cb(logcl,kc,act,mod_shift,mod_ctrl,mod_alt);
                }
            } else {
                print("logical cliend id:%d not found", logclid);
            }
        }
        
        break;
    default:
        break;
    }
}
static void reprecator_on_close_callback( uv_handle_t *s ) {
    print("reprecator_on_close_callback");
    Client *cl = (Client*)s->data;
    assert(cl->parent_reprecator);
    int ids_toclean[1024];
    int ids_toclean_num=0;
    POOL_SCAN(cl->parent_reprecator->logical_cl_pool,Client) {
        Client *logcl = it->second;
        if(logcl->reprecator_tcp == (uv_tcp_t*)s) {
            print("freeing logical client id:%d", logcl->id);
            if( logcl->parent_rh->on_disconnect_cb ) {
                logcl->parent_rh->on_disconnect_cb( logcl->parent_rh, logcl );
            }
            logcl->onDelete();
            if( ids_toclean_num==elementof(ids_toclean))break;
            ids_toclean[ids_toclean_num] = logcl->id;
            ids_toclean_num++;
            delete logcl;
            // yes we can use it=pool.erase(xx) but i dont like it..
        }
    }
    for(int i=0;i<ids_toclean_num;i++) {
        print("deleting from pool:%d",ids_toclean[i]);
        cl->parent_reprecator->logical_cl_pool.del(ids_toclean[i]);
    }
    
    cl->parent_reprecator->delRealClient(cl);
}
static void reprecator_on_read_callback( uv_stream_t *s, ssize_t nread, const uv_buf_t *inbuf ) {
    //    print("reprecator_on_read_callback nread:%d",nread);
    if(nread>0) {
        Client *cl = (Client*)s->data;
        bool res = parseRecord( s, &cl->recvbuf, inbuf->base, nread, reprecator_on_packet_cb );
        if(!res) {
            uv_close( (uv_handle_t*)s, reprecator_on_close_callback );
            return;
        }
    } else if( nread<=0) {
        print("reprecator_on_read_callback eof or error" );
        uv_close( (uv_handle_t*)s, reprecator_on_close_callback );
    }
}
static void reprecator_on_accept_callback( uv_stream_t *listener, int status ) {
    print("reprecator_on_accept_callback status:%d",status);
    if( status != 0) {
        print("reprecator_on_accept_callback status:%d",status);
        return;
    }

    uv_tcp_t *newsock = (uv_tcp_t*) MALLOC( sizeof(uv_tcp_t) );
    uv_tcp_init( uv_default_loop(), newsock );
    if( uv_accept( listener, (uv_stream_t*) newsock ) == 0 ) {
        Reprecator *rep = (Reprecator*)listener->data;
        Client *cl = new Client(newsock,rep);
        rep->addRealClient(cl);
        newsock->data=(void*)cl;
        
        int r = uv_read_start( (uv_stream_t*) newsock, moyai_libuv_alloc_buffer, reprecator_on_read_callback );
        if(r) {
            print("uv_read_start: fail ret:%d",r);
            return;
        }

        print("accepted new reprecator");

        sendWindowSize((uv_stream_t*)newsock, rep->parent_rh->window_width, rep->parent_rh->window_height);
        rep->parent_rh->scanSendAllPrerequisites( (uv_stream_t*) newsock );
        rep->parent_rh->scanSendAllProp2DSnapshots( (uv_stream_t*) newsock);
    }    
}

void Reprecator::addRealClient( Client *cl) {
    Client *stored = cl_pool.get(cl->id);
    if(!stored) {
        cl->parent_reprecator = this;
        cl_pool.set(cl->id,cl);
    }
}
void Reprecator::delRealClient(Client*cl) {
    cl_pool.del(cl->id);
}
void Reprecator::addLogicalClient( Client *cl) {
    Client *stored = logical_cl_pool.get(cl->id);
    if(!stored) {
        logical_cl_pool.set(cl->id,cl);
    }
}
void Reprecator::delLogicalClient(Client*cl) {
    logical_cl_pool.del(cl->id);
}
Client *Reprecator::getLogicalClient(uint32_t logclid) {
    return logical_cl_pool.get(logclid);
}
Reprecator::Reprecator(RemoteHead *rh, int portnum) : parent_rh(rh) {
    if( ! init_tcp_listener( &listener, (void*)this, portnum, reprecator_on_accept_callback ) ) {
        assertmsg(false, "can't initialize reprecator server");
    }
    print("Reprecator server started");
}

/////////////////////

void RemoteHead::enableVideoStream( int w, int h, int pixel_skip ) {
    enable_videostream = true;
    assertmsg(!jc, "can't call enableVideoStream again");    
    jc = new JPEGCoder(w,h,pixel_skip);
    audio_buf_ary = new BufferArray(256);
    print("enableVideoStream done");
}
void RemoteHead::enableReprecation(int portnum) {
    assertmsg(!reprecator, "can't enable reprecation twice");
    reprecator = new Reprecator(this,portnum);
}

// Note: don't support dynamic cameras
void RemoteHead::broadcastCapturedScreen() {
    assert(jc);
    Image *img = jc->getImage();
    double t0 = now();
    target_moyai->capture(img);
    double t1 = now();
    size_t sz = jc->encode();
    double t2 = now();
    if((t1-t0)>0.04) print("slow screen capture. %f", t1-t0);
    if((t2-t1)>0.02) print("slow encode. %f sz:%d",t2-t1, sz);
    
    //print("broadcastCapturedScreen time:%f,%f size:%d", t1-t0,t2-t1,sz );
#if 0
    writeFile("encoded.jpg", (char*)jc->compressed, jc->compressed_size);
#endif    
    broadcastUS1Bytes( PACKETTYPE_S2C_CAPTURED_FRAME, (const char*)jc->compressed, jc->compressed_size );
}

void RemoteHead::broadcastTimestamp() {
    double t = now();
    uint32_t sec = (uint32_t)t;
    uint32_t usec = (t - sec)*1000000;    
    broadcastUS1UI2( PACKETTYPE_TIMESTAMP, sec, usec );
}

const char *RemoteHead::funcidToString(PACKETTYPE pkt) {
    switch(pkt) {
    case PACKETTYPE_PING:  return "PACKETTYPE_PING";
    case PACKETTYPE_TIMESTAMP: return "PACKETTYPE_TIMESTAMP";
    
    // client to server 
    case PACKETTYPE_C2S_KEYBOARD: return "PACKETTYPE_C2S_KEYBOARD";
    case PACKETTYPE_C2S_MOUSE_BUTTON: return "PACKETTYPE_C2S_MOUSE_BUTTON";
    case PACKETTYPE_C2S_CURSOR_POS:  return "PACKETTYPE_C2S_CURSOR_POS";
    case PACKETTYPE_C2S_TOUCH_BEGIN: return "PACKETTYPE_C2S_TOUCH_BEGIN";
    case PACKETTYPE_C2S_TOUCH_MOVE: return "PACKETTYPE_C2S_TOUCH_MOVE";
    case PACKETTYPE_C2S_TOUCH_END: return "PACKETTYPE_C2S_TOUCH_END";
    case PACKETTYPE_C2S_TOUCH_CANCEL: return "PACKETTYPE_C2S_TOUCH_CANCEL";
        
    // reprecator to server
    case PACKETTYPE_R2S_CLIENT_LOGIN: return "PACKETTYPE_R2S_CLIENT_LOGIN";
    case PACKETTYPE_R2S_CLIENT_LOGOUT: return "PACKETTYPE_R2S_CLIENT_LOGOUT";        
    case PACKETTYPE_R2S_KEYBOARD: return "PACKETTYPE_R2S_KEYBOARD";
    case PACKETTYPE_R2S_MOUSE_BUTTON: return "PACKETTYPE_R2S_MOUSE_BUTTON";
    case PACKETTYPE_R2S_CURSOR_POS: return "PACKETTYPE_R2S_CURSOR_POS";
    case PACKETTYPE_S2R_NEW_CLIENT_ID: return "PACKETTYPE_S2R_NEW_CLIENT_ID";
    case PACKETTYPE_S2R_CAMERA_CREATE: return "PACKETTYPE_S2R_CAMERA_CREATE";
    case PACKETTYPE_S2R_CAMERA_DYNAMIC_LAYER: return "PACKETTYPE_S2R_CAMERA_DYNAMIC_LAYER";
    case PACKETTYPE_S2R_CAMERA_LOC: return "PACKETTYPE_S2R_CAMERA_LOC";        
    case PACKETTYPE_S2R_VIEWPORT_CREATE: return "PACKETTYPE_S2R_VIEWPORT_CREATE";
    case PACKETTYPE_S2R_VIEWPORT_DYNAMIC_LAYER: return "PACKETTYPE_S2R_VIEWPORT_DYNAMIC_LAYER";
    case PACKETTYPE_S2R_VIEWPORT_SCALE: return "PACKETTYPE_S2R_VIEWPORT_SCALE";        
        
    // server to client
    case PACKETTYPE_S2C_PROP2D_SNAPSHOT: return "PACKETTYPE_S2C_PROP2D_SNAPSHOT";
    case PACKETTYPE_S2C_PROP2D_LOC: return "PACKETTYPE_S2C_PROP2D_LOC";
    case PACKETTYPE_S2C_PROP2D_GRID: return "PACKETTYPE_S2C_PROP2D_GRID";
    case PACKETTYPE_S2C_PROP2D_INDEX: return "PACKETTYPE_S2C_PROP2D_INDEX";
    case PACKETTYPE_S2C_PROP2D_SCALE: return "PACKETTYPE_S2C_PROP2D_SCALE";
    case PACKETTYPE_S2C_PROP2D_ROT: return "PACKETTYPE_S2C_PROP2D_ROT";
    case PACKETTYPE_S2C_PROP2D_XFLIP: return "PACKETTYPE_S2C_PROP2D_XFLIP";
    case PACKETTYPE_S2C_PROP2D_YFLIP: return "PACKETTYPE_S2C_PROP2D_YFLIP";
    case PACKETTYPE_S2C_PROP2D_COLOR: return "PACKETTYPE_S2C_PROP2D_COLOR";
    case PACKETTYPE_S2C_PROP2D_OPTBITS: return "PACKETTYPE_S2C_PROP2D_OPTBITS";
    case PACKETTYPE_S2C_PROP2D_PRIORITY: return "PACKETTYPE_S2C_PROP2D_PRIORITY";
    case PACKETTYPE_S2C_PROP2D_DELETE: return "PACKETTYPE_S2C_PROP2D_DELETE";
    case PACKETTYPE_S2C_PROP2D_CLEAR_CHILD: return "PACKETTYPE_S2C_PROP2D_CLEAR_CHILD";
    case PACKETTYPE_S2C_PROP2D_LOC_VEL: return "PACKETTYPE_S2C_PROP2D_LOC_VEL";        
    
    case PACKETTYPE_S2C_LAYER_CREATE: return "PACKETTYPE_S2C_LAYER_CREATE";
    case PACKETTYPE_S2C_LAYER_VIEWPORT: return "PACKETTYPE_S2C_LAYER_VIEWPORT";
    case PACKETTYPE_S2C_LAYER_CAMERA: return "PACKETTYPE_S2C_LAYER_CAMERA";
    case PACKETTYPE_S2C_VIEWPORT_CREATE: return "PACKETTYPE_S2C_VIEWPORT_CREATE";
    //    case PACKETTYPE_S2C_VIEWPORT_SIZE: 331,  not used now
    case PACKETTYPE_S2C_VIEWPORT_SCALE: return "PACKETTYPE_S2C_VIEWPORT_SCALE";
    case PACKETTYPE_S2C_VIEWPORT_DYNAMIC_LAYER: return "PACKETTYPE_S2C_VIEWPORT_DYNAMIC_LAYER";        
    case PACKETTYPE_S2C_CAMERA_CREATE: return "PACKETTYPE_S2C_CAMERA_CREATE";
    case PACKETTYPE_S2C_CAMERA_LOC: return "PACKETTYPE_S2C_CAMERA_LOC";
    case PACKETTYPE_S2C_CAMERA_DYNAMIC_LAYER: return "PACKETTYPE_S2C_CAMERA_DYNAMIC_LAYER";
    
    case PACKETTYPE_S2C_TEXTURE_CREATE: return "PACKETTYPE_S2C_TEXTURE_CREATE";
    case PACKETTYPE_S2C_TEXTURE_IMAGE: return "PACKETTYPE_S2C_TEXTURE_IMAGE";
    case PACKETTYPE_S2C_IMAGE_CREATE: return "PACKETTYPE_S2C_IMAGE_CREATE";
    case PACKETTYPE_S2C_IMAGE_LOAD_PNG: return "PACKETTYPE_S2C_IMAGE_LOAD_PNG";
    case PACKETTYPE_S2C_IMAGE_ENSURE_SIZE: return "PACKETTYPE_S2C_IMAGE_ENSURE_SIZE";
    case PACKETTYPE_S2C_IMAGE_RAW: return "PACKETTYPE_S2C_IMAGE_RAW";
    
    case PACKETTYPE_S2C_TILEDECK_CREATE: return "PACKETTYPE_S2C_TILEDECK_CREATE";
    case PACKETTYPE_S2C_TILEDECK_TEXTURE: return "PACKETTYPE_S2C_TILEDECK_TEXTURE";
    case PACKETTYPE_S2C_TILEDECK_SIZE: return "PACKETTYPE_S2C_TILEDECK_SIZE";
    case PACKETTYPE_S2C_GRID_CREATE: return "PACKETTYPE_S2C_GRID_CREATE";
    case PACKETTYPE_S2C_GRID_DECK: return "PACKETTYPE_S2C_GRID_DECK";
    case PACKETTYPE_S2C_GRID_PROP2D: return "PACKETTYPE_S2C_GRID_PROP2D";
    case PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT: return "PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT";
    case PACKETTYPE_S2C_GRID_TABLE_FLIP_SNAPSHOT: return "PACKETTYPE_S2C_GRID_TABLE_FLIP_SNAPSHOT";
    case PACKETTYPE_S2C_GRID_TABLE_TEXOFS_SNAPSHOT: return "PACKETTYPE_S2C_GRID_TABLE_TEXOFS_SNAPSHOT";
    case PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT: return "PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT";
    case PACKETTYPE_S2C_GRID_DELETE: return "PACKETTYPE_S2C_GRID_DELETE";

    case PACKETTYPE_S2C_TEXTBOX_CREATE: return "PACKETTYPE_S2C_TEXTBOX_CREATE";
    case PACKETTYPE_S2C_TEXTBOX_FONT: return "PACKETTYPE_S2C_TEXTBOX_FONT";
    case PACKETTYPE_S2C_TEXTBOX_STRING: return "PACKETTYPE_S2C_TEXTBOX_STRING";
    case PACKETTYPE_S2C_TEXTBOX_LOC: return "PACKETTYPE_S2C_TEXTBOX_LOC";
    case PACKETTYPE_S2C_TEXTBOX_SCL: return "PACKETTYPE_S2C_TEXTBOX_SCL";
    case PACKETTYPE_S2C_TEXTBOX_COLOR: return "PACKETTYPE_S2C_TEXTBOX_COLOR";
    case PACKETTYPE_S2C_TEXTBOX_LAYER: return "PACKETTYPE_S2C_TEXTBOX_LAYER";
    case PACKETTYPE_S2C_FONT_CREATE: return "PACKETTYPE_S2C_FONT_CREATE";
    case PACKETTYPE_S2C_FONT_CHARCODES: return "PACKETTYPE_S2C_FONT_CHARCODES";
    case PACKETTYPE_S2C_FONT_LOADTTF: return "PACKETTYPE_S2C_FONT_LOADTTF";

    case PACKETTYPE_S2C_COLOR_REPLACER_SHADER_SNAPSHOT: return "PACKETTYPE_S2C_COLOR_REPLACER_SHADER_SNAPSHOT";
    case PACKETTYPE_S2C_PRIM_BULK_SNAPSHOT: return "PACKETTYPE_S2C_PRIM_BULK_SNAPSHOT";

    case PACKETTYPE_S2C_SOUND_CREATE_FROM_FILE: return "PACKETTYPE_S2C_SOUND_CREATE_FROM_FILE";
    case PACKETTYPE_S2C_SOUND_CREATE_FROM_SAMPLES: return "PACKETTYPE_S2C_SOUND_CREATE_FROM_SAMPLES";
    case PACKETTYPE_S2C_SOUND_DEFAULT_VOLUME: return "PACKETTYPE_S2C_SOUND_DEFAULT_VOLUME";
    case PACKETTYPE_S2C_SOUND_PLAY: return "PACKETTYPE_S2C_SOUND_PLAY";
    case PACKETTYPE_S2C_SOUND_STOP: return "PACKETTYPE_S2C_SOUND_STOP";
    case PACKETTYPE_S2C_SOUND_POSITION: return "PACKETTYPE_S2C_SOUND_POSITION";

    case PACKETTYPE_S2C_JPEG_DECODER_CREATE: return "PACKETTYPE_S2C_JPEG_DECODER_CREATE";
    case PACKETTYPE_S2C_CAPTURED_FRAME: return "PACKETTYPE_S2C_CAPTURED_FRAME";
    case PACKETTYPE_S2C_CAPTURED_AUDIO: return "PACKETTYPE_S2C_CAPTURED_AUDIO";
    
    case PACKETTYPE_S2C_FILE: return "PACKETTYPE_S2C_FILE";

    case PACKETTYPE_S2C_WINDOW_SIZE: return "PACKETTYPE_S2C_WINDOW_SIZE";

    case PACKETTYPE_ERROR: return "PACKETTYPE_ERROR";

    case PACKETTYPE_MAX: return "PACKETTYPE_MAX";
    }
    assertmsg(false,"invalid packet type:%d", pkt);
    return "invalid packet type";
}

    
#if 1
#define FUNCID_LOG(id,sendfuncname) print( "%s %s", funcidToString(id),sendfuncname)
#else
#define FUNCID_LOG(id,sendfuncname) ;
#endif

#define REPRECATOR_ITER_SEND  if(reprecator) POOL_SCAN(reprecator->cl_pool,Client)
#define CLIENT_ITER_SEND for( ClientIteratorType it = cl_pool.idmap.begin(); it != cl_pool.idmap.end(); ++it )

void RemoteHead::broadcastUS1Bytes( uint16_t usval, const char *data, size_t datalen ) {
    CLIENT_ITER_SEND sendUS1Bytes( (uv_stream_t*)it->second->tcp, usval, data, datalen );
    REPRECATOR_ITER_SEND sendUS1Bytes( (uv_stream_t*) it->second->tcp, usval, data, datalen );
}
void RemoteHead::broadcastUS1UI1Bytes( uint16_t usval, uint32_t uival, const char *data, size_t datalen ) {
    CLIENT_ITER_SEND sendUS1UI1Bytes( (uv_stream_t*)it->second->tcp, usval, uival, data, datalen );
    REPRECATOR_ITER_SEND sendUS1UI1Bytes( (uv_stream_t*)it->second->tcp, usval, uival, data, datalen );
}
void RemoteHead::broadcastUS1UI1( uint16_t usval, uint32_t uival ) {
    CLIENT_ITER_SEND sendUS1UI1(  (uv_stream_t*)it->second->tcp, usval, uival );
    REPRECATOR_ITER_SEND sendUS1UI1(  (uv_stream_t*)it->second->tcp, usval, uival );
}
void RemoteHead::broadcastUS1UI2( uint16_t usval, uint32_t ui0, uint32_t ui1 ) {
    CLIENT_ITER_SEND sendUS1UI2(  (uv_stream_t*)it->second->tcp, usval, ui0, ui1 );
    REPRECATOR_ITER_SEND sendUS1UI2(  (uv_stream_t*)it->second->tcp, usval, ui0, ui1 );
}
void RemoteHead::broadcastUS1UI3( uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2 ) {
    CLIENT_ITER_SEND sendUS1UI3( (uv_stream_t*)it->second->tcp, usval, ui0, ui1, ui2 );
    REPRECATOR_ITER_SEND sendUS1UI3( (uv_stream_t*)it->second->tcp, usval, ui0, ui1, ui2 );
}
void RemoteHead::broadcastUS1UI1Wstr( uint16_t usval, uint32_t uival, wchar_t *wstr, int wstr_num_letters ) {
    CLIENT_ITER_SEND sendUS1UI1Wstr( (uv_stream_t*)it->second->tcp, usval, uival, wstr, wstr_num_letters );
    REPRECATOR_ITER_SEND sendUS1UI1Wstr( (uv_stream_t*)it->second->tcp, usval, uival, wstr, wstr_num_letters );
}
void RemoteHead::broadcastUS1UI1F4( uint16_t usval, uint32_t uival, float f0, float f1, float f2, float f3 ) {
    CLIENT_ITER_SEND sendUS1UI1F4( (uv_stream_t*)it->second->tcp, usval, uival, f0, f1, f2, f3 );
    REPRECATOR_ITER_SEND sendUS1UI1F4( (uv_stream_t*)it->second->tcp, usval, uival, f0, f1, f2, f3 );
}
void RemoteHead::broadcastUS1UI1F2( uint16_t usval, uint32_t uival, float f0, float f1 ) {
    CLIENT_ITER_SEND sendUS1UI1F2( (uv_stream_t*)it->second->tcp, usval, uival, f0, f1 );
    REPRECATOR_ITER_SEND sendUS1UI1F2( (uv_stream_t*)it->second->tcp, usval, uival, f0, f1 );
}

void RemoteHead::nearcastUS1UI1F2( Prop2D *p, uint16_t usval, uint32_t uival, float f0, float f1 ) {
    POOL_SCAN(cl_pool,Client) {
        Client *cl = it->second;
        if(cl->canSee(p)==false) continue;
        sendUS1UI1F2( (uv_stream_t*)cl->tcp, usval, uival, f0, f1 );
    }
    REPRECATOR_ITER_SEND sendUS1UI1F2( (uv_stream_t*)it->second->tcp, usval, uival, f0, f1 );
}
void RemoteHead::broadcastUS1UI1F1( uint16_t usval, uint32_t uival, float f0 ) {
    CLIENT_ITER_SEND sendUS1UI1F1( (uv_stream_t*)it->second->tcp, usval, uival, f0 );
    REPRECATOR_ITER_SEND sendUS1UI1F1( (uv_stream_t*)it->second->tcp, usval, uival, f0 );
}


bool RemoteHead::appendChangelist(Prop2D *p, PacketProp2DSnapshot *pkt) {
    if( changelist_used == elementof(changelist) )return false;
    changelist[changelist_used] = ChangeEntry(p,pkt);
    changelist_used++;
    return true;
}
void RemoteHead::broadcastSortedChangelist() {
    static SorterEntry tosort[elementof(changelist)];
    for(int i=0;i<changelist_used;i++) {
        tosort[i].val = changelist[i].p->loc_sync_score;
        tosort[i].ptr = &changelist[i];
    }
    quickSortF( tosort, 0, changelist_used-1);
    //    print("sortChangelist:%d",changelist_used);

    int max_send_num = 200; // about 1.5Mbps
    if( max_send_num > changelist_used ) max_send_num = changelist_used;
    int sent_n=0;
    for(int i=changelist_used-1;i>=0;i--) { // reverse order: biggest first
        //        print("KKK:%d %f",i, changelist[i].p->loc_sync_score);
        ChangeEntry *e = (ChangeEntry*)tosort[i].ptr;
        nearcastUS1UI1F2( e->p, PACKETTYPE_S2C_PROP2D_LOC, e->pkt->prop_id, e->pkt->loc.x, e->pkt->loc.y );
        e->p->loc_sync_score=0;
        sent_n++;
        if( sent_n >= max_send_num )break;
    }
    //    print("broadcastChangelist: tot:%d sent:%d max:%d", changelist_used, sent_n, max_send_num);
}


///////////////////
int calcModkeyBits(bool shift, bool ctrl, bool alt ) {
    int out=0;
    if(shift) out |= MODKEY_BIT_SHIFT;
    if(ctrl) out |= MODKEY_BIT_CONTROL;
    if(alt) out |= MODKEY_BIT_ALT;
    return out;
}
void getModkeyBits(int val, bool *shift, bool *ctrl, bool *alt ) {
    *shift = val & MODKEY_BIT_SHIFT;
    *ctrl = val & MODKEY_BIT_CONTROL;
    *alt = val & MODKEY_BIT_ALT;
}

///////////////////
char sendbuf_work[1024*1024*8];

void on_write_end( uv_write_t *req, int status ) {
    if(status==-1) {
        print("on_write_end: status:%d",status);
    }
    FREE(req->data);
    FREE(req);
}

#define SET_RECORD_LEN_AND_US1 \
    assert(totalsize<=sizeof(sendbuf_work));\
    set_u32( sendbuf_work+0, totalsize - 4 ); \
    set_u16( sendbuf_work+4, usval );

#define SETUP_UV_WRITE \
    uv_write_t *write_req = (uv_write_t*)MALLOC(sizeof(uv_write_t));\
    char *_data = (char*) MALLOC(totalsize);\
    memcpy( _data, sendbuf_work, totalsize );\
    write_req->data = _data;        \
    uv_buf_t buf = uv_buf_init(_data,totalsize);\
    int r = uv_write( write_req, s, &buf, 1, on_write_end );\
    if(r) { print("uv_write fail. %d",r); return -1; }


int sendUS1RawArgs( uv_stream_t *s, uint16_t usval, const char *data, uint32_t datalen ) {
    size_t totalsize = 4 + 2 + datalen;
    SET_RECORD_LEN_AND_US1;
    memcpy( sendbuf_work+4+2,data, datalen);
    SETUP_UV_WRITE;
    return totalsize;
}
int sendUS1( uv_stream_t *s, uint16_t usval ) {
    size_t totalsize = 4 + 2;
    SET_RECORD_LEN_AND_US1;
    SETUP_UV_WRITE;
    return totalsize;    
}
int sendUS1Bytes( uv_stream_t *s, uint16_t usval, const char *bytes, uint16_t byteslen ) {
    size_t totalsize = 4 + 2 + (4+byteslen);
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, byteslen );
    memcpy( sendbuf_work+4+2+4, bytes, byteslen );
    SETUP_UV_WRITE;
    return totalsize;
}
int sendUS1UI1Bytes( uv_stream_t *s, uint16_t usval, uint32_t uival, const char *bytes, uint32_t byteslen ) {
    size_t totalsize = 4 + 2 + 4 + (4+byteslen);
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, uival );
    set_u32( sendbuf_work+4+2+4, byteslen );
    memcpy( sendbuf_work+4+2+4+4, bytes, byteslen );
    SETUP_UV_WRITE;
    return totalsize;
}
int sendUS1UI1( uv_stream_t *s, uint16_t usval, uint32_t uival ) {
    size_t totalsize = 4 + 2 + 4;
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, uival );
    SETUP_UV_WRITE;
    return totalsize;
}
int sendUS1UI2( uv_stream_t *s, uint16_t usval, uint32_t ui0, uint32_t ui1 ) {
    size_t totalsize = 4 + 2 + 4+4;
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, ui0 );
    set_u32( sendbuf_work+4+2+4, ui1 );
    SETUP_UV_WRITE;
    return totalsize;
}
int sendUS1UI3( uv_stream_t *s, uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2 ) {
    size_t totalsize = 4 + 2 + 4+4+4;
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, ui0 );
    set_u32( sendbuf_work+4+2+4, ui1 );
    set_u32( sendbuf_work+4+2+4+4, ui2 );    
    SETUP_UV_WRITE;
    return totalsize;
}
int sendUS1UI4( uv_stream_t *s, uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2, uint32_t ui3 ) {
    size_t totalsize = 4 + 2 + 4+4+4+4+4;
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, ui0 );
    set_u32( sendbuf_work+4+2+4, ui1 );
    set_u32( sendbuf_work+4+2+4+4, ui2 );
    set_u32( sendbuf_work+4+2+4+4+4, ui3 );
    SETUP_UV_WRITE;
    return totalsize;
}
int sendUS1UI5( uv_stream_t *s, uint16_t usval, uint32_t ui0, uint32_t ui1, uint32_t ui2, uint32_t ui3, uint32_t ui4 ) {
    size_t totalsize = 4 + 2 + 4+4+4+4+4;
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, ui0 );
    set_u32( sendbuf_work+4+2+4, ui1 );
    set_u32( sendbuf_work+4+2+4+4, ui2 );
    set_u32( sendbuf_work+4+2+4+4+4, ui3 );
    set_u32( sendbuf_work+4+2+4+4+4+4, ui4 );
    SETUP_UV_WRITE;
    return totalsize;
}
int sendUS1UI1F1( uv_stream_t *s, uint16_t usval, uint32_t uival, float f0 ) {
    size_t totalsize = 4 + 2 + 4+4;
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, uival );
    memcpy( sendbuf_work+4+2+4, &f0, 4 );
    SETUP_UV_WRITE;
    return totalsize;    
}
int sendUS1UI1F2( uv_stream_t *s, uint16_t usval, uint32_t uival, float f0, float f1 ) {
    size_t totalsize = 4 + 2 + 4+4+4;
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, uival );
    memcpy( sendbuf_work+4+2+4, &f0, 4 );
    memcpy( sendbuf_work+4+2+4+4, &f1, 4 );
    SETUP_UV_WRITE;
    return totalsize;    
}
int sendUS1UI2F2( uv_stream_t *s, uint16_t usval, uint32_t uival0, uint32_t uival1, float f0, float f1 ) {
    size_t totalsize = 4 + 2 + 4+4+4+4;
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, uival0 );
    set_u32( sendbuf_work+4+2+4, uival1 );    
    memcpy( sendbuf_work+4+2+4+4, &f0, 4 );
    memcpy( sendbuf_work+4+2+4+4+4, &f1, 4 );
    SETUP_UV_WRITE;
    return totalsize;    
}
int sendUS1UI1F4( uv_stream_t *s, uint16_t usval, uint32_t uival, float f0, float f1, float f2, float f3 ) {
    size_t totalsize = 4 + 2 + 4+4+4+4+4;
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, uival );
    memcpy( sendbuf_work+4+2+4, &f0, 4 );
    memcpy( sendbuf_work+4+2+4+4, &f1, 4 );
    memcpy( sendbuf_work+4+2+4+4+4, &f2, 4 );
    memcpy( sendbuf_work+4+2+4+4+4+4, &f3, 4 );    
    SETUP_UV_WRITE;
    return totalsize;    
}

int sendUS1F2( uv_stream_t *s, uint16_t usval, float f0, float f1 ) {
    size_t totalsize = 4 + 2 + 4+4;
    SET_RECORD_LEN_AND_US1;
    memcpy( sendbuf_work+4+2, &f0, 4 );
    memcpy( sendbuf_work+4+2+4, &f1, 4 );
    SETUP_UV_WRITE;
    return totalsize;
}
int sendUS1UI1Str( uv_stream_t *s, uint16_t usval, uint32_t uival, const char *cstr ) {
    int cstrlen = strlen(cstr);
    assert( cstrlen <= 255 );
    size_t totalsize = 4 + 2 + 4 + (1+cstrlen);
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, uival );
    set_u8( sendbuf_work+4+2+4, (unsigned char) cstrlen );
    memcpy( sendbuf_work+4+2+4+1, cstr, cstrlen );
    SETUP_UV_WRITE;
    return totalsize;
}
int sendUS1UI2Str( uv_stream_t *s, uint16_t usval, uint32_t ui0, uint32_t ui1, const char *cstr ) {
    int cstrlen = strlen(cstr);
    assert( cstrlen <= 255 );
    size_t totalsize = 4 + 2 + 4+4 + (1+cstrlen);
    SET_RECORD_LEN_AND_US1;
    set_u32( sendbuf_work+4+2, ui0 );
    set_u32( sendbuf_work+4+2+4, ui1 );    
    set_u8( sendbuf_work+4+2+4+4, (unsigned char) cstrlen );
    memcpy( sendbuf_work+4+2+4+4+1, cstr, cstrlen );
    SETUP_UV_WRITE;
    return totalsize;
}
// [record-len:16][usval:16][cstr-len:8][cstr-body][data-len:32][data-body]
int sendUS1StrBytes( uv_stream_t *s, uint16_t usval, const char *cstr, const char *data, uint32_t datalen ) {
    int cstrlen = strlen(cstr);
    assert( cstrlen <= 255 );
    size_t totalsize = 4 + 2 + (1+cstrlen) + (4+datalen);
    SET_RECORD_LEN_AND_US1;
    set_u8( sendbuf_work+4+2, (unsigned char) cstrlen );
    memcpy( sendbuf_work+4+2+1, cstr, cstrlen );
    set_u32( sendbuf_work+4+2+1+cstrlen, datalen );
    memcpy( sendbuf_work+4+2+1+cstrlen+4, data, datalen );
    SETUP_UV_WRITE;
    //    print("send_packet_str_bytes: cstrlen:%d datalen:%d totallen:%d", cstrlen, datalen, totalsize );
    return totalsize;
}
void parsePacketStrBytes( char *inptr, char *outcstr, char **outptr, size_t *outsize ) {
    uint8_t slen = get_u8(inptr);
    char *s = inptr + 1;
    uint32_t datalen = get_u32(inptr+1+slen);
    *outptr = inptr + 1 + slen + 4;
    memcpy( outcstr, s, slen );
    outcstr[slen]='\0';
    *outsize = (size_t) datalen;
}

// convert wchar_t to 
int sendUS1UI1Wstr( uv_stream_t *s, uint16_t usval, uint32_t uival, wchar_t *wstr, int wstr_num_letters ) {
    // lock is correctly handled by  sendUS1UI1Bytes later in this func
#if defined(__APPLE__) || defined(__linux__)
    assert( sizeof(wchar_t) == sizeof(int32_t) );
    size_t bufsz = wstr_num_letters * sizeof(int32_t);
    UTF8 *outbuf = (UTF8*) MALLOC( bufsz + 1);
    assert(outbuf);
    UTF8 *orig_outbuf = outbuf;    
    const UTF32 *inbuf = (UTF32*) wstr;
    ConversionResult r = ConvertUTF32toUTF8( &inbuf, inbuf+wstr_num_letters, &outbuf, outbuf+bufsz, strictConversion );
    assertmsg(r==conversionOK, "ConvertUTF32toUTF8 failed:%d bufsz:%d", r, bufsz );    
#else
    assert( sizeof(wchar_t) == sizeof(int16_t) );
    size_t bufsz = wstr_num_letters * sizeof(int16_t) * 2; // utf8 gets bigger than utf16
    UTF8 *outbuf = (UTF8*) MALLOC( bufsz + 1);
    assert(outbuf);
    UTF8 *orig_outbuf = outbuf;        
    const UTF16 *inbuf = (UTF16*) wstr;
    ConversionResult r = ConvertUTF16toUTF8( &inbuf, inbuf+wstr_num_letters, &outbuf, outbuf+bufsz, strictConversion );
    assertmsg(r==conversionOK, "ConvertUTF16toUTF8 failed:%d bufsz:%d", r, bufsz );    
#endif    
    size_t outlen = outbuf - orig_outbuf;
    //    print("ConvertUTF32toUTF8 result utf8 len:%d out:'%s'", outlen, orig_outbuf );
    int ret = sendUS1UI1Bytes( s, usval, uival, (const char*) orig_outbuf, outlen );
    FREE(orig_outbuf);
    return ret;    
}

void sendFile( uv_stream_t *s, const char *filename ) {
    const size_t MAXBUFSIZE = 1024*1024*4;
    char *buf = (char*) MALLOC(MAXBUFSIZE);
    assert(buf);
    size_t sz = MAXBUFSIZE;
    bool res = readFile( filename, buf, &sz );
    assertmsg(res, "sendFile: file '%s' read error", filename );
    int r = sendUS1StrBytes( s, PACKETTYPE_S2C_FILE, filename, buf, sz );
    assert(r>0);
    print("sendFile: path:%s len:%d data:%x %x %x %x sendres:%d", filename, sz, buf[0], buf[1], buf[2], buf[3], r );
    FREE(buf);
}
void sendPing( uv_stream_t *s ) {
    double t = now();
    uint32_t sec = (uint32_t)t;
    uint32_t usec = (t - sec)*1000000;
    sendUS1UI2( s, PACKETTYPE_PING, sec, usec );    
}
void sendWindowSize( uv_stream_t *outstream, int w, int h ) {
    sendUS1UI2( outstream, PACKETTYPE_S2C_WINDOW_SIZE, w,h );
}
void sendViewportCreateScale( uv_stream_t *outstream, Viewport *vp ) {
    sendUS1UI1( outstream, PACKETTYPE_S2C_VIEWPORT_CREATE, vp->id );
    sendUS1UI1F2( outstream, PACKETTYPE_S2C_VIEWPORT_SCALE, vp->id, vp->scl.x, vp->scl.y );
}
void sendCameraCreateLoc( uv_stream_t *outstream, Camera *cam ) {
    sendUS1UI1( outstream, PACKETTYPE_S2C_CAMERA_CREATE, cam->id );
    sendUS1UI1F2( outstream, PACKETTYPE_S2C_CAMERA_LOC, cam->id, cam->loc.x, cam->loc.y );
}
void sendLayerSetup( uv_stream_t *outstream, Layer *l ) {
    sendUS1UI2( outstream, PACKETTYPE_S2C_LAYER_CREATE, l->id, l->priority );
    if( l->viewport ) sendUS1UI2( outstream, PACKETTYPE_S2C_LAYER_VIEWPORT, l->id, l->viewport->id);
    if( l->camera ) sendUS1UI2( outstream, PACKETTYPE_S2C_LAYER_CAMERA, l->id, l->camera->id );
}
void sendImageSetup( uv_stream_t *outstream, Image *img ) {
    print("sending image_create id:%d", img->id );
    sendUS1UI1( outstream, PACKETTYPE_S2C_IMAGE_CREATE, img->id );
    if( img->last_load_file_path[0] ) {
        print("sending image_load_png: '%s'", img->last_load_file_path );
        sendUS1UI1Str( outstream, PACKETTYPE_S2C_IMAGE_LOAD_PNG, img->id, img->last_load_file_path );                
    }
    if( img->width>0 && img->buffer) {
        // this image is not from file, maybe generated.
        sendUS1UI3( outstream, PACKETTYPE_S2C_IMAGE_ENSURE_SIZE, img->id, img->width, img->height );
    }
    if( img->modified_pixel_num > 0 ) {
        // modified image (includes loadPNG case)
        sendUS1UI3( outstream, PACKETTYPE_S2C_IMAGE_ENSURE_SIZE, img->id, img->width, img->height );
        sendUS1UI1Bytes( outstream, PACKETTYPE_S2C_IMAGE_RAW, img->id, (const char*) img->buffer, img->getBufferSize() );                
    }
}
void sendTextureCreateWithImage( uv_stream_t *outstream, Texture *tex ) {
    sendUS1UI1( outstream, PACKETTYPE_S2C_TEXTURE_CREATE, tex->id );
    sendUS1UI2( outstream, PACKETTYPE_S2C_TEXTURE_IMAGE, tex->id, tex->image->id );
}
void sendDeckSetup( uv_stream_t *outstream, Deck *dk ) {
    assertmsg(dk->getUperCell()>0, "only tiledeck is supported" ); // TODO: Support PackDeck
    TileDeck *td = (TileDeck*) dk;
    //        print("sending tiledeck_create id:%d", td->id );
    sendUS1UI1( outstream, PACKETTYPE_S2C_TILEDECK_CREATE, dk->id );
    sendUS1UI2( outstream, PACKETTYPE_S2C_TILEDECK_TEXTURE, dk->id, td->tex->id );
    //        print("sendS2RTileDeckSize: id:%d %d,%d,%d,%d", td->id, sprw, sprh, cellw, cellh );        
    sendUS1UI5( outstream, PACKETTYPE_S2C_TILEDECK_SIZE, td->id, td->tile_width, td->tile_height, td->cell_width, td->cell_height );
}
void sendFontSetupWithFile( uv_stream_t *outstream, Font *f ) {
    print("sending font id:%d path:%s", f->id, f->last_load_file_path );
    sendUS1UI1( outstream, PACKETTYPE_S2C_FONT_CREATE, f->id );
    // utf32toutf8
    sendUS1UI1Wstr( outstream, PACKETTYPE_S2C_FONT_CHARCODES, f->id, f->charcode_table, f->charcode_table_used_num );
    sendFile( outstream, f->last_load_file_path );
    sendUS1UI2Str( outstream, PACKETTYPE_S2C_FONT_LOADTTF, f->id, f->pixel_size, f->last_load_file_path );
}
void sendColorReplacerShaderSetup( uv_stream_t *outstream, ColorReplacerShader *crs ) {
    print("sending col repl shader id:%d", crs->id );
    PacketColorReplacerShaderSnapshot ss;
    setupPacketColorReplacerShaderSnapshot(&ss,crs);
    sendUS1Bytes( outstream, PACKETTYPE_S2C_COLOR_REPLACER_SHADER_SNAPSHOT, (const char*)&ss, sizeof(ss) );        
}
void sendSoundSetup( uv_stream_t *outstream, Sound *snd ) {
    if( snd->last_load_file_path[0] ) {
        sendFile( outstream, snd->last_load_file_path );
        print("sending sound load file: %d, '%s'", snd->id, snd->last_load_file_path );
        sendUS1UI1Str( outstream, PACKETTYPE_S2C_SOUND_CREATE_FROM_FILE, snd->id, snd->last_load_file_path );
    } else if( snd->last_samples ){
        sendUS1UI1Bytes( outstream, PACKETTYPE_S2C_SOUND_CREATE_FROM_SAMPLES, snd->id,
                         (const char*) snd->last_samples,
                         snd->last_samples_num * sizeof(snd->last_samples[0]) );
    }
    sendUS1UI1F1( outstream, PACKETTYPE_S2C_SOUND_DEFAULT_VOLUME, snd->id, snd->default_volume );
    if(snd->isPlaying()) {
        sendUS1UI1F2( outstream, PACKETTYPE_S2C_SOUND_POSITION, snd->id, snd->getTimePositionSec(), snd->last_play_volume );
    }
}

////////////////////
Buffer::Buffer() : buf(0), size(0), used(0) {
}
void Buffer::ensureMemory( size_t sz ) {
    if(!buf) {
        buf = (char*) MALLOC(sz);
        assert(buf);
        size = sz;
        used = 0;
    }
}

Buffer::~Buffer() {
    if(buf) {
        FREE(buf);
    }
    size = used = 0;
}

bool Buffer::push( const char *data, size_t datasz ) {
    size_t left = size - used;
    if( left < datasz ) return false;
    memcpy( buf + used, data, datasz );
    used += datasz;
    //    fprintf(stderr, "buffer_push: pushed %d bytes, used: %d\n", (int)datasz, (int)b->used );
    return true;
}
bool Buffer::pushWithNum32( const char *data, size_t datasz ) {
    size_t left = size - used;
    if( left < 4 + datasz ) return false;
    set_u32( buf + used, datasz );
    used += 4;
    push( data, datasz );
    return true;
}
bool Buffer::pushU32( unsigned int val ) {
    size_t left = size - used;
    if( left < 4 ) return false;
    set_u32( buf + used, val );
    used += 4;
    //    fprintf(stderr, "buffer_push_u32: pushed 4 bytes. val:%u\n",val );
    return true;
}
bool Buffer::pushU16( unsigned short val ) {
    size_t left = size - used;
    if( left < 2 ) return false;
    set_u16( buf + used, val );
    used += 2;
    return true;
}
bool Buffer::pushU8( unsigned char val ) {
    size_t left = size - used;
    if( left < 1 ) return false;
    set_u8( buf + used, val );
    used += 1;
    return true;
}

// ALL or NOTHING. true when success
bool Buffer::shift( size_t toshift ) {
    if( used < toshift ) return false;
    if( toshift == used ) { // most cases
        used = 0;
        return true;
    }
    // 0000000000 size=10
    // uuuuu      used=5
    // ss         shift=2
    //   mmm      move=3
    memmove( buf, buf + toshift, used - toshift );
    used -= toshift;
    return true;
}

//////////////////
BufferArray::BufferArray( int maxnum ) {
    buffers = (Buffer**) MALLOC( maxnum * sizeof(Buffer*) );
    assert(buffers);
    buffer_num = maxnum;
    buffer_used = 0;
    for(int i=0;i<maxnum;i++) buffers[i] = NULL;
}
BufferArray::~BufferArray() {
    for(unsigned int i=0;i<buffer_num;i++) {
        delete buffers[i];
        FREE(buffers[i]);
    }
}
bool BufferArray::push(const char *data, size_t len) {
    if(buffer_used == buffer_num)return false;
    Buffer *b = new Buffer();
    b->ensureMemory(len);
    b->push(data,len);
    buffers[buffer_used] = b;
    buffer_used++;
    return true;
}
Buffer *BufferArray::getTop() {
    if(buffer_used==0)return NULL;
    return buffers[0];    
}
void BufferArray::shift() {
    if(buffer_used==0)return;
    Buffer *top = buffers[0];
    for(unsigned int i=0;i<buffer_used-1;i++) {
        buffers[i] = buffers[i+1];
    }
    buffers[buffer_used]=NULL;
    buffer_used--;
    delete top;
}


//////////////////
int Client::idgen = 1;
Client::Client( uv_tcp_t *sk, RemoteHead *rh ) {
    init(sk);
    parent_rh = rh;
}
Client::Client( uv_tcp_t *sk, ReprecationProxy *reproxy )  {
    init(sk);
    parent_reproxy = reproxy;
}
Client::Client( uv_tcp_t *sk, Reprecator *repr ) {
    init(sk);
    parent_reprecator = repr;
}
Client::Client( RemoteHead *rh ) {
    init(NULL);
    parent_rh = rh;
}
Client *Client::createLogicalClient( uv_tcp_t *reprecator_tcp, RemoteHead *rh ) {
    Client *cl = new Client(rh);
    cl->reprecator_tcp = reprecator_tcp;
    print("createLogicalClient called. newclid:%d",cl->id);
    return cl;
}

void Client::init(uv_tcp_t *sk) {
    id = ++idgen;
    tcp = sk;
    parent_rh = NULL;
    parent_reproxy = NULL;
    parent_reprecator = NULL;
    target_camera = NULL;
    target_viewport = NULL;
    recvbuf.ensureMemory(8*1024); // Only receiving keyboard and mouse input events!
    initialized_at = now();
    global_client_id = 0;
    reprecator_tcp=NULL;
}
Client::~Client() {
    print("~Client called for %d", id );
}

// return false when error(to close)
bool parseRecord( uv_stream_t *s, Buffer *recvbuf, const char *data, size_t datalen, void (*funcCallback)( uv_stream_t *s, uint16_t funcid, char *data, uint32_t datalen ) ) {
    bool pushed = recvbuf->push( data, datalen );
    //    print("parseRecord: datalen:%d bufd:%d pushed:%d", datalen, recvbuf->used, pushed );

    if(!pushed) {
        print("recv buf full? close.");
        return false;
    }

    // Parse RPC
    //        fprintf(stderr, "recvbuf used:%zu\n", c->recvbuf->used );
    while(true) { // process everything in one poll
        //            print("recvbuf:%d", c->recvbuf->used );
        if( recvbuf->used < (4+2) ) return true; // need more data from network
        //              <---RECORDLEN------>
        // [RECORDLEN32][FUNCID32][..DATA..]            
        size_t record_len = get_u32( recvbuf->buf );
        unsigned int func_id = get_u16( recvbuf->buf + 4 );

        if( recvbuf->used < (4+record_len) ) {
            //   print("need. used:%d reclen:%d", recvbuf->used, record_len );
            return true; // need more data from network
        }
        if( record_len < 2 ) {
            fprintf(stderr, "invalid packet format" );
            return false;
        }
        //        fprintf(stderr, "dispatching func_id:%d record_len:%lu\n", func_id, record_len );
        //        dump( recvbuf->buf, record_len-4);
        funcCallback( s, func_id, (char*) recvbuf->buf +4+2, record_len - 2 );
        recvbuf->shift( 4 + record_len );
        //            fprintf(stderr, "after dispatch recv func: buffer used: %zu\n", c->recvbuf->used );
        //            if( c->recvbuf->used > 0 ) dump( c->recvbuf->buf, c->recvbuf->used );
    }
}

void Client::saveStream( const char *data, size_t datalen ) {
    const size_t BUFFER_MAX = 128*1024;
    saved_stream.ensureMemory(BUFFER_MAX);
    size_t room = saved_stream.getRoom();
    if( room < datalen ) {
        flushStreamToFile();
        saved_stream.used = 0;
    }
    saved_stream.push(data,datalen);
    //    print("saveStream: used:%d", saved_stream.used);
}
void Client::flushStreamToFile() {
    uint32_t sec = (uint32_t)initialized_at;
    uint32_t usec = (uint32_t)(( initialized_at - sec ) * 1000000);
    Format outpath( "/tmp/moyaistream_%u_%u_%u", id, sec, usec );
    //    print("flushing recorded stream to %s size:%d", outpath.buf, saved_stream.used );
    // flush to file
    bool wret = appendFile( outpath.buf, saved_stream.buf, saved_stream.used );
    if(!wret) {
        print("saveStream: can't append data to '%s', discarding stream", outpath.buf );
    }
}
void Client::onDelete() {
    flushStreamToFile();
    saved_stream.used = 0;
}

bool Client::canSee(Prop2D*p) {
    if(!target_viewport) return true;
    Vec2 minv, maxv;
    target_viewport->getMinMax(&minv,&maxv);
    return p->isInView(&minv,&maxv,target_camera);    
}

//////////////////////
static void reproxy_on_close_callback( uv_handle_t *s ) {
    print("reproxy_on_close_callback");
    Client *cli = (Client*)s->data;
    if(cli->parent_reproxy->close_callback) cli->parent_reproxy->close_callback((uv_stream_t*)s);
    cli->parent_reproxy->delClient(cli);
    cli->onDelete();
    delete cli;
}
static void reproxy_on_read_callback( uv_stream_t *s, ssize_t nread, const uv_buf_t *inbuf ) {
    //    print("reproxy_on_read_callback: nread:%d",nread);
    Client *cl = (Client*)s->data;
    if(nread>0) {
        ReprecationProxy *rp = cl->parent_reproxy;
        assert(rp);
        assert(rp->func_callback);
        bool res = parseRecord(s, &cl->recvbuf, inbuf->base, nread, rp->func_callback );
        if(!res) {
            uv_close( (uv_handle_t*)s, reproxy_on_close_callback );
            return;
        }
    } else if( nread<=0 ) {
        print("reproxy_on_read_callback EOF. clid:%d", cl->id );
        uv_close( (uv_handle_t*)s, reproxy_on_close_callback );
    }
}
void reproxy_on_accept_callback(uv_stream_t *listener, int status) {
    print("reproxy_on_accept_callback: status:%d",status);
    if(status!=0) {
        print("reproxy_on_accept_callback error status:%d",status);
        return;
    }
    
    uv_tcp_t *newsock = (uv_tcp_t*)MALLOC( sizeof(uv_tcp_t));
    uv_tcp_init( uv_default_loop(), newsock );
    if( uv_accept( listener, (uv_stream_t*)newsock) == 0 ) {
        ReprecationProxy *rp = (ReprecationProxy*)listener->data;
        Client *cl = new Client(newsock,rp);
        newsock->data = cl;
        cl->parent_reproxy->addClient(cl);
        print("reproxy_on_accept_callback. accepted");

        int r = uv_read_start( (uv_stream_t*)newsock, moyai_libuv_alloc_buffer, reproxy_on_read_callback );
        if(r) {
            print("uv_read_start: failed. ret:%d",r);
            return;
        }
        assert(rp->accept_callback);
        rp->accept_callback((uv_stream_t*)newsock);
    }
}
ReprecationProxy::ReprecationProxy(int portnum) : func_callback(NULL) {
    bool res = init_tcp_listener( &listener, (void*)this, portnum, reproxy_on_accept_callback );
    assertmsg(res, "Reproxy: listen error");
    print("Reproxy: listening on %d", portnum);
}
void ReprecationProxy::addClient( Client *cl) {
    Client *stored = cl_pool.get(cl->id);
    if(!stored) {
        cl->parent_reproxy = this;
        cl_pool.set(cl->id,cl);
    }
}
void ReprecationProxy::delClient(Client*cl) {
    cl_pool.del(cl->id);
}
void ReprecationProxy::broadcastUS1RawArgs(uint16_t funcid, const char*data, size_t datalen ) {
    POOL_SCAN(cl_pool,Client) {
        sendUS1RawArgs( (uv_stream_t*)it->second->tcp, funcid, data, datalen );
    }
}
Client *ReprecationProxy::getClientByGlobalId(unsigned int gclid) {
    POOL_SCAN(cl_pool,Client) {
        if( it->second->global_client_id==gclid) {
            return it->second;
        }
    }
    return NULL;
}
