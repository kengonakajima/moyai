// HMP (Headless Moyai Protocol viewer client)

#include <locale.h>

#include "client.h"
#include "ConvertUTF.h"

#include "JPEGCoder.h"
#include "vw.h"

#ifdef USE_UNTZ
#include "threading/Threading.h"
UNTZ::Sound *g_audio_stream;
#endif

ObjectPool<Layer> g_layer_pool;
ObjectPool<Viewport> g_viewport_pool;
ObjectPool<Camera> g_camera_pool;
ObjectPool<Texture> g_texture_pool;
ObjectPool<Image> g_image_pool;
ObjectPool<TileDeck> g_tiledeck_pool;
ObjectPool<Prop2D> g_prop2d_pool;
ObjectPool<Grid> g_grid_pool;
ObjectPool<TextBox> g_textbox_pool;
ObjectPool<Font> g_font_pool;
ObjectPool<ColorReplacerShader> g_crshader_pool;
ObjectPool<Prim> g_prim_pool;
ObjectPool<Sound> g_sound_pool;

MoyaiClient *g_moyai_client;        
FileDepo *g_filedepo;
Stream *g_stream;
GLFWwindow *g_window;

Viewport *g_local_viewport;
Layer *g_video_layer; // for videostream
Layer *g_debug_layer;
Font *g_debug_font;
TextBox *g_debug_tb;

SoundSystem *g_soundsystem;

uint64_t g_total_read;
uint64_t g_total_read_count;
uint64_t g_total_unzipped_bytes;
uint64_t g_packet_count;

char *g_server_ip_addr = (char*)"127.0.0.1";
int g_port = HEADLESS_SERVER_PORT;
int g_window_width = 0;
int g_window_height = 0;
int g_timestamp_count = 0;
double g_last_ping_at=0;
double g_last_ping_rtt=0;

bool g_enable_print_stats = false;
bool g_enable_reprecation = false;

#if defined(__APPLE__)
#define RETINA 2
#else
#define RETINA 1
#endif    

JPEGCoder *g_jc;

Prop2D *g_video_prop;
Texture *g_video_tex;

unsigned int g_recv_counts[PACKETTYPE_MAX];
unsigned int g_recv_totalcounts[PACKETTYPE_MAX];
unsigned int g_recv_sizes[PACKETTYPE_MAX];
unsigned int g_recv_totalsizes[PACKETTYPE_MAX];

ReprecationProxy *g_reproxy;

bool g_done=false;

Buffer g_savebuf;
char g_savepath[512];

///////////////

// debug funcs
Prop2D *findProp2DByTexture( uint32_t tex_id ) {
    POOL_SCAN(g_prop2d_pool,Prop2D) {
        Prop2D *p =  it->second;
        if( p->deck && p->deck->tex && p->deck->tex->id == tex_id ) {
            return p;
        }
    }
    return NULL;
}

///////////////

void setupVideoViewer( int imgw, int imgh ) {
    if(!g_video_layer) {
        g_video_layer = new Layer();
        g_video_layer->priority = 0;
        g_video_layer->setViewport(g_local_viewport);
        g_moyai_client->insertLayer(g_video_layer);
    }
    print("setupVideoViewer: img:%d,%d",imgw,imgh);
    if(!g_video_tex) {
        g_video_tex = new Texture();
        g_video_tex->setImage(g_jc->capture_img);
    }
    if(!g_video_prop) {
        g_video_prop = new Prop2D();
        g_video_prop->setTexture(g_video_tex);
        g_video_prop->setLoc(0,0);
        g_video_prop->setScl(imgw*RETINA,imgh*RETINA);
        g_video_layer->insertProp(g_video_prop);
    }
}
void updateVideoViewer() {
    g_video_tex->setImage(g_jc->capture_img);
#if 0
    g_jc->capture_img->writePNG("decoded.png");
#endif    
    //    g_video_prop->setTexture(g_hogetex);
    g_video_prop->setTexture(g_video_tex);
}

///////////////

File::File( const char *inpath, const char *indata, size_t indata_len ) {
    strncpy( path, inpath, sizeof(path) );
    data = (char*) MALLOC( indata_len );
    memcpy( data, indata, indata_len );
    data_len = indata_len;
    print("File: init. path:'%s' size:%d data:%x %x %x %x", path, indata_len, indata[0], indata[1], indata[2], indata[3] );
}
// save a file in directory 
bool File::saveInTmpDir( const char *dir_prefix, char *outpath, size_t outpathsize ) {
    snprintf( outpath, outpathsize, "%s/tmp_%s", dir_prefix, path );
    gsubString( outpath+strlen(dir_prefix)+1, '/', '_' );           
    bool wr = writeFile( outpath, data, data_len );
    if(!wr) {
        print("saveInTmpDir: can't write file: '%s'", outpath );
        return false;
    }
    return true;
}
File *FileDepo::get( char *path ) {
    for(int i=0;i<elementof(files);i++) {
        if( files[i] && files[i]->comparePath(path) ) {
            return files[i];
        }
    }
    return NULL;
}

File *FileDepo::ensure( char *path, char *data, size_t datalen ) {
    File *f = get(path);
    if(f) return f;    
    for(int i=0;i<elementof(files);i++) {
        if( files[i] == NULL ) {
            print("ensure: alloc ind:%d", i );
            files[i] = new File( path, data, datalen );
            return files[i];
        }
    }
    assertmsg( false, "file full for: '%s'", path );
    return NULL;
}

File *FileDepo::getByIndex(int ind) {
    assert(ind>=0 && ind< MAX_FILES);
    return files[ind];
}


///////////////////

void flushBufferToFile( Buffer *buf, const char *path ) {
    bool wret = appendFile( path, buf->buf, buf->used );
    if(!wret) {
        print("flushBufferToFile: can't append data to '%s', discarding stream", path );
    }
}
void saveStreamToBuffer( Buffer *outbuf, const char *data, size_t datalen, const char *path ) {
    const size_t BUFFER_MAX = 128*1024;
    outbuf->ensureMemory(BUFFER_MAX);
    size_t room = outbuf->getRoom();
    if( room < datalen ) {
        flushBufferToFile(outbuf,path);
        outbuf->used = 0;
    }
    outbuf->push(data,datalen);
    //    print("saveStream: used:%d", outbuf->used);
}

//////////////////

void winclose_callback( GLFWwindow *w ){
    exit(0);
}

void glfw_error_cb( int code, const char *desc ) {
    print("glfw_error_cb. code:%d desc:'%s'", code, desc );
}

wchar_t *allocateWCharStringFromUTF8String( const uint8_t *in_uint8ary, size_t sz ) {
#if defined(__APPLE__)
    const UTF8 *in_u8 = (UTF8*) in_uint8ary;
    UTF32 *out_u32 = (UTF32*) MALLOC(sz * sizeof(UTF32) );
    memset(out_u32,0, sz*sizeof(UTF32));
    UTF32 *orig_out_u32 = out_u32;
    ConversionResult r = ConvertUTF8toUTF32(&in_u8,in_u8+sz,&out_u32,out_u32+sz, strictConversion );
    assertmsg(r == conversionOK, "ConvertUTF8toUTF32 failed. result:%d",r );
    wchar_t *charcodes = (wchar_t*) orig_out_u32;
    return charcodes;
#else
            assertmsg(false, "not implemented");
#endif
}


///////////////////
#ifdef USE_UNTZ

RCriticalSection g_audio_lock;
BufferArray *g_audio_buf_ary;

// interleavedSamples: [ch1 num_samples][ch2 num_samples] ch1:left ch2:right
void appendAudioSampleStereo( float *interleavedSamples, uint32_t num_samples) {
    RScopedLock l(&g_audio_lock);
    if(!g_audio_buf_ary) {
        g_audio_buf_ary = new BufferArray(256);
    }
    g_audio_buf_ary->push( (const char*)interleavedSamples, num_samples * 2 * sizeof(float));
    //    print("appendAudioSampleStereo. pushed. used:%d", g_audio_buf_ary->getUsedNum() );
}

UInt32 stream_callback(float* buffers, UInt32 numChannels, UInt32 length, void* userdata) {
    if(!g_audio_buf_ary) {
        for(int i=0;i<length;i++) buffers[i]=0.0f;
        return length;
    }
#if 1
    assert(numChannels==2);
    {
        RScopedLock _l(&g_audio_lock);
        int used = g_audio_buf_ary->getUsedNum();
        if(used==0)return 0;
        Buffer *topbuf = g_audio_buf_ary->getTop();
        
        
        size_t to_copy_samples = length;
        size_t samples_in_buf = topbuf->used / sizeof(float) / 2; // samples per channel
        assert(samples_in_buf>0);
        
        if( to_copy_samples > samples_in_buf ) to_copy_samples = samples_in_buf;

        //        print("stream_callback. used:%d toplen:%d cblen:%d to_copy_smpl:%d", used, topbuf->used, length, to_copy_samples );
        
        for(int i=0;i<to_copy_samples*numChannels;i++) {
            buffers[i] = ((float*)(topbuf->buf))[i];
        }
        g_audio_buf_ary->shift();
        return to_copy_samples;
    }
#endif    
}

#endif    
///////////////////



void setupDebugStat() {
    g_debug_layer = new Layer();
    g_debug_layer->setViewport(g_local_viewport);
    g_moyai_client->insertLayer(g_debug_layer);
    g_debug_layer->priority = Layer::PRIORITY_MAX;
    g_debug_font = new Font();
    wchar_t charcodes[] = L" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    g_debug_font->loadFromTTF("./assets/cinecaption227.ttf", charcodes, 12 );
    g_debug_tb = new TextBox();
    g_debug_tb->setFont(g_debug_font);
    g_debug_tb->setScl(1);
    g_debug_tb->setLoc(-g_window_width/2+10,g_window_height/2-15);
    g_debug_tb->setString("not init");
    g_debug_layer->insertProp(g_debug_tb);    
}
void updateDebugStat( const char *s ) {
    g_debug_tb->setString(s);
}
void keyboardCallback( GLFWwindow *window, int keycode, int scancode, int action, int mods ) {
    int mod_shift = mods & GLFW_MOD_SHIFT;
    int mod_ctrl = mods & GLFW_MOD_CONTROL;
    int mod_alt = mods & GLFW_MOD_ALT;
    if(g_stream) sendUS1UI3( g_stream, PACKETTYPE_C2S_KEYBOARD, keycode, action, calcModkeyBits(mod_shift, mod_ctrl, mod_alt ));
}
void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods ) {
    int mod_shift = mods & GLFW_MOD_SHIFT;
    int mod_ctrl = mods & GLFW_MOD_CONTROL;
    int mod_alt = mods & GLFW_MOD_ALT;
    if(g_stream) sendUS1UI3( g_stream, PACKETTYPE_C2S_MOUSE_BUTTON, button, action, calcModkeyBits(mod_shift, mod_ctrl, mod_alt ));
}
void cursorPosCallback( GLFWwindow *window, double x, double y ) {
    if(g_stream) sendUS1F2( g_stream, PACKETTYPE_C2S_CURSOR_POS, x, y );
}


void on_packet_callback( Stream *s, uint16_t funcid, char *argdata, uint32_t argdatalen ) {
    g_packet_count++;
    
    // if(g_enable_reprecation) print("funcid:%d l:%d",funcid, argdatalen);

    if(funcid>=0 && funcid<PACKETTYPE_MAX) {
        g_recv_counts[funcid]++;
        g_recv_totalcounts[funcid]++;
        g_recv_sizes[funcid]+=argdatalen+2+4;        
        g_recv_totalsizes[funcid]+=argdatalen+2+4;
    }


    // first step switch
    if(g_reproxy) {
        switch(funcid) {
        case PACKETTYPE_S2C_VIEWPORT_DYNAMIC_LAYER:
            print("ignoring vp_dyn_layer");
            return;
        case PACKETTYPE_S2C_CAMERA_DYNAMIC_LAYER:
            print("ignoring cam_dyn_layer");
            return;
        case PACKETTYPE_S2C_PROP2D_LOC:
            {
                uint32_t propid = get_u32(argdata+0);
                float x = get_f32(argdata+4);
                float y = get_f32(argdata+8);
                //                print("step1sw: p2dloc: id:%d loc:%f,%f",propid, x,y);
                Prop2D *p = g_prop2d_pool.get(propid);
                if(p) {
                    POOL_SCAN(g_reproxy->cl_pool,Client) {
                        Client *cl = it->second;
                        if(cl->canSee(p)==false) {
                            continue;
                        } else {
                            sendUS1UI1F2( cl, PACKETTYPE_S2C_PROP2D_LOC, propid,x,y);
                        }                    
                    }
                }
            }
            break; // must go to second step switch.
        case PACKETTYPE_S2C_PROP2D_INDEX_LOC:
            {
                uint32_t propid = get_u32(argdata+0);
                uint32_t index = get_u32(argdata+4);
                float x = get_f32(argdata+8);
                float y = get_f32(argdata+12);
                Prop2D *p = g_prop2d_pool.get(propid);
                if(p) {
                    POOL_SCAN(g_reproxy->cl_pool,Client) {
                        Client *cl = it->second;
                        if(cl->canSee(p)==false) {
                            continue;
                        } else {
                            sendUS1UI2F2( cl, PACKETTYPE_S2C_PROP2D_INDEX_LOC, propid,index,x,y);
                        }                    
                    }
                }                
            }
            break;// must go to second step switch.
        case PACKETTYPE_S2R_CAMERA_CREATE:
            {
                uint32_t gclid = get_u32(argdata+0);
                uint32_t camid = get_u32(argdata+4);
                print("received s2r_cam_create. gclid:%d camid:%d",gclid,camid);
                Client *cl = g_reproxy->getClientByGlobalId(gclid);
                if(cl) {
                    Camera *cam = g_camera_pool.ensure(camid);
                    cl->target_camera = cam;
                    print(" sending as s2c_camera_create id:%d",camid);
                    sendUS1UI1( cl, PACKETTYPE_S2C_CAMERA_CREATE, camid );
                } else {
                    print("  client gcl:%d is not found",gclid );
                }
            }            
            return;
        case PACKETTYPE_S2R_CAMERA_DYNAMIC_LAYER:
            {
                uint32_t gclid=get_u32(argdata+0);
                uint32_t camid=get_u32(argdata+4);
                uint32_t layid=get_u32(argdata+8);
                print("received s2r_cam_dyn_layer gcl:%d cam:%d l:%d", gclid, camid, layid );
                Client *cl = g_reproxy->getClientByGlobalId(gclid);
                if(cl) {
                    print(" sending as s2c_camera_dynamic_layer");
                    sendUS1UI2( cl, PACKETTYPE_S2C_CAMERA_DYNAMIC_LAYER, camid, layid );
                } else {
                    print(" client gclid:%d is not found", gclid);
                }
            }
            return;
        case PACKETTYPE_S2R_CAMERA_LOC:
            {
                uint32_t gclid=get_u32(argdata+0);
                uint32_t camid=get_u32(argdata+4);
                float x = get_f32(argdata+4+4);
                float y = get_f32(argdata+4+4+4);
                //                print("received s2r_camera_loc. gclid:%d camid:%d (%f,%f)", gclid, camid, x,y );
                Client *cl = g_reproxy->getClientByGlobalId(gclid);
                if(cl) {
                    Camera *cam = g_camera_pool.get(camid);
                    if(cam) cam->setLoc(x,y);
                    sendUS1UI1F2( cl, PACKETTYPE_S2C_CAMERA_LOC, camid, x,y );
                } else {
                    print("client gclid:%d not found", gclid);
                }
            }
            return;
        case PACKETTYPE_S2R_VIEWPORT_CREATE:
            {
                uint32_t gclid = get_u32(argdata+0);
                uint32_t viewport_id = get_u32(argdata+4);
                print("received s2r_viewport_create. gclid:%d vpid:%d", gclid, viewport_id );
                Client *cl = g_reproxy->getClientByGlobalId(gclid);
                if(cl) {
                    Viewport *vp = g_viewport_pool.ensure(viewport_id);
                    cl->target_viewport = vp;
                    print("  sending as s2c_viewport_create");
                    sendUS1UI1( cl, PACKETTYPE_S2C_VIEWPORT_CREATE, viewport_id);
                    //                    Viewport *vp = g_viewport_pool.ensure(viewport_id);                
                    //                    cl->target_viewport = vp;
                } else {
                    print("  client gclid:%d not found", gclid);
                }
            }
            return;
        case PACKETTYPE_S2R_VIEWPORT_DYNAMIC_LAYER:
            {
                uint32_t gclid = get_u32(argdata+0);
                uint32_t vp_id = get_u32(argdata+4);
                uint32_t layid = get_u32(argdata+8);
                print("received s2r_vp_dyn_layer. gclid:%d vpid:%d lid:%d", gclid, vp_id, layid );
                Client *cl = g_reproxy->getClientByGlobalId(gclid);
                if(cl) {
                    print("  sending as s2c_viewport_dyn_layer");
                    sendUS1UI2( cl, PACKETTYPE_S2C_VIEWPORT_DYNAMIC_LAYER, vp_id, layid);
                } else {
                    print("  client gclid:%d not found", gclid);
                }
            }
            return;
        case PACKETTYPE_S2R_VIEWPORT_SCALE:
            {
                uint32_t gclid=get_u32(argdata+0);
                uint32_t vpid=get_u32(argdata+4);
                float sx=get_f32(argdata+8);
                float sy=get_f32(argdata+12);
                print("received s2r_vp_scl. gclid:%d vpid:%d s:%f,%f",gclid,vpid,sx,sy);
                Client *cl = g_reproxy->getClientByGlobalId(gclid);
                if(cl){
                    Viewport *vp = g_viewport_pool.get(vpid);
                    if(vp) vp->setScale2D(sx,sy);
                    print("sending as s2c_vp_scale");
                    sendUS1UI1F2( cl, PACKETTYPE_S2C_VIEWPORT_SCALE, vpid, sx,sy);
                } else {
                    print("client gclid:%d not found",gclid);
                }
            }
            return;
        default:
            //        print("forwarding pkt: f:%d sz:%d",funcid, argdatalen);
            g_reproxy->broadcastUS1RawArgs(funcid,argdata,argdatalen);
            break;
        }
    }

    // second step switch
    switch(funcid) {
    case PACKETTYPE_PING:
        {
            uint32_t sec = get_u32(argdata+0);
            uint32_t usec = get_u32(argdata+4);
            
            double t = (double)(sec) + (double)(usec)/1000000.0f;
            double dt = now() - t;
            g_last_ping_rtt = dt;
            //            print("received ping: %u %u dt:%f", sec, usec, dt );
        }
        break;
    case PACKETTYPE_TIMESTAMP:
        {
            g_timestamp_count++;
        }
        break;
    case PACKETTYPE_ZIPPED_RECORDS:
        {
            //            print("received zipped_records argdatalen:%d", argdatalen );
            static char unzipout[8*1024*1024];
            int unzipped_size = memDecompressSnappy( unzipout, sizeof(unzipout), argdata, argdatalen );
            bool pushed = s->unzipped_recvbuf.push(unzipout,unzipped_size);
            if(!pushed) {
                print("unzipped_recvbuf full?");
                break;
            }
            //            print("pushed to unzipped_recvbuf. used:%d", s->unzipped_recvbuf.used);
            g_total_unzipped_bytes += unzipped_size;
        }
        break;

    case PACKETTYPE_S2R_NEW_CLIENT_ID:
        {
            if(g_reproxy) {
                uint32_t server_cl_id = get_u32(argdata+0);
                uint32_t reproxy_cl_id = get_u32(argdata+4);
                print("received new_client_id: server_cl_id:%d reproxy_cl_id:%d",server_cl_id,reproxy_cl_id);
                Client *cl = g_reproxy->getClient(reproxy_cl_id);
                if(cl) {
                    cl->global_client_id = server_cl_id;
                }                
            }
        }
        break;
    case PACKETTYPE_S2C_PROP2D_SNAPSHOT:
        {
            uint32_t pktsize = get_u32(argdata+0);
            //        print("PACKETTYPE_S2C_PROP2D_SNAPSHOT len:%d", argdatalen );            
            PacketProp2DSnapshot pkt;
            assertmsg( pktsize == sizeof(pkt), "invalid argdatalen:%d pktsize:%d",argdatalen, pktsize );
            memcpy(&pkt,argdata+4,sizeof(pkt));
            //            prt("s%d ", pkt.prop_id );


            if( pkt.debug ||pkt.prop_id==29) print("packettype_prop2d_create! id:%d layer_id:%d parentprop:%d loc:%f,%f scl:%f,%f index:%d tdid:%d col:%.1f,%.1f,%.1f,%.1f", pkt.prop_id, pkt.layer_id, pkt.parent_prop_id, pkt.loc.x, pkt.loc.y, pkt.scl.x, pkt.scl.y, pkt.index, pkt.tiledeck_id , pkt.color.r,pkt.color.g,pkt.color.b,pkt.color.a);

            Layer *layer = NULL;
            Prop2D *parent_prop = NULL;
            
            if( pkt.layer_id > 0 ) {
                layer = g_layer_pool.get( pkt.layer_id );
            } else if( pkt.parent_prop_id > 0 ) {
                //                print("Child prop.id:%d par:%d", pkt.prop_id, pkt.parent_prop_id );                
                parent_prop = g_prop2d_pool.get(pkt.parent_prop_id);
                if(!parent_prop) {
                    print("Warning: prop:%d can't find parent prop:%d", pkt.prop_id, pkt.parent_prop_id );
                } else {
                    //                    print("PARENT:%d FOUND, chld[0]:%p chn:%d", pkt.parent_prop_id, parent_prop->children[0], parent_prop->children_num);
                }
            }
            if( !(layer || parent_prop ) ) {
                // no parent!
                print("prop %d no parent? layerid:%d parentpropid:%d", pkt.prop_id, pkt.layer_id, pkt.parent_prop_id );
                break;
            }

            TileDeck *dk = g_tiledeck_pool.get( pkt.tiledeck_id ); // deck can be null (may have grid,textbox)
            if(!dk && pkt.tiledeck_id!=0) {
                print("TileDeck is not initialized yet! id:%d",pkt.tiledeck_id);
                break;
            }
            Prop2D *prop = g_prop2d_pool.get(pkt.prop_id);
            if(!prop) {
                prop = g_prop2d_pool.ensure(pkt.prop_id);
                if(layer) {
                    //                    print("  inserting prop %d to layer %d", pkt.prop_id, pkt.layer_id );
                    layer->insertProp(prop);
                } else if(parent_prop) {
                    Prop2D *found_prop = prop->getChild( pkt.prop_id );
                    if(!found_prop) {
                        print("  adding child prop %d to a prop %d", pkt.prop_id, pkt.parent_prop_id );
                        parent_prop->addChild(prop);
                    }
                } else {
                    print("Warning: this prop has no parent? id:%d layer:%d",pkt.prop_id, pkt.layer_id );
                }
            }
            if(dk) prop->setDeck(dk);
            prop->setIndex(pkt.index);
            prop->setScl( Vec2(pkt.scl.x,pkt.scl.y) );
            prop->setLoc( Vec2(pkt.loc.x, pkt.loc.y) );
            prop->setRot( pkt.rot );
            prop->setXFlip( getXFlipFromFlipRotBits(pkt.fliprotbits) );
            prop->setYFlip( getYFlipFromFlipRotBits(pkt.fliprotbits) );
            prop->setUVRot( getUVRotFromFlipRotBits(pkt.fliprotbits) );
            Color col( pkt.color.r, pkt.color.g, pkt.color.b, pkt.color.a );
            prop->setColor(col);
            prop->use_additive_blend = pkt.optbits & PROP2D_OPTBIT_ADDITIVE_BLEND;
            if(pkt.shader_id != 0 ) {
                // TODO: now moyai only has color replacer shader.
                ColorReplacerShader *crs = g_crshader_pool.get(pkt.shader_id);
                if(crs) {
                    prop->setFragmentShader(crs);
                } else {
                    print("  colorreplacershader %d not found", pkt.shader_id);
                }
            }
            prop->priority = pkt.priority;
            //            print("KKKKKKKKKK: ind:%d dk:%p vis:%d scl:%f,%f", prop->index, dk, prop->visible, prop->scl.x, prop->scl.y);
        }
        break;
    case PACKETTYPE_S2C_PROP2D_LOC:
        {
            uint32_t id = get_u32(argdata+0);
            Prop2D *prop = g_prop2d_pool.get(id);
            if(prop) {
                float x = get_f32(argdata+4);
                float y = get_f32(argdata+8);
                prop->setLoc(x,y);
            }
        }
        break;
    case PACKETTYPE_S2C_PROP2D_INDEX_LOC:
        {
            uint32_t id = get_u32(argdata+0);
            uint32_t ind = get_u32(argdata+4);
            Prop2D *prop = g_prop2d_pool.get(id);
            if(prop) {
                float x = get_f32(argdata+8);
                float y = get_f32(argdata+12);
                prop->setLoc(x,y);
                prop->setIndex(ind);
            }
        }
        break;
    case PACKETTYPE_S2C_PROP2D_LOC_VEL:
        {
            uint32_t id = get_u32(argdata+0);
            Prop2D *prop = g_prop2d_pool.get(id);
            if(prop) {
                float lx = get_f32(argdata+4);
                float ly = get_f32(argdata+8);
                float vx = get_f32(argdata+12);
                float vy = get_f32(argdata+16);                
                print("LLLLLBLBBBBBVEL:%d %f %f",id, vx,vy);
                
                prop->setLoc(lx,ly);
                prop->remote_vel = Vec2(vx,vy);
            }
        }
        break;        
    case PACKETTYPE_S2C_PROP2D_SCALE:
        {
            uint32_t id = get_u32(argdata+0);
            Prop2D *prop = g_prop2d_pool.get(id);
            if(prop) {
                float sx = get_f32(argdata+4);
                float sy = get_f32(argdata+8);
                prop->setScl(sx,sy);
            }
        }
        break;
    case PACKETTYPE_S2C_PROP2D_COLOR:
        {
            uint32_t id = get_u32(argdata+0);
            Prop2D *prop = g_prop2d_pool.get(id);
            if(prop) {
                uint32_t pktsz = get_u32(argdata+4);
                assert(pktsz==sizeof(PacketColor));
                PacketColor col;
                memcpy( &col, argdata+8, sizeof(col) );
                prop->setColor( Color(col.r,col.g,col.b,col.a) );
            }            
        }
        break;
    case PACKETTYPE_S2C_PROP2D_ROT:
        {
            uint32_t id = get_u32(argdata+0);
            Prop2D *prop = g_prop2d_pool.get(id);
            if(prop) {
                float r = get_f32(argdata+4);
                prop->setRot(r);
            }            
        }
        break;
    case PACKETTYPE_S2C_PROP2D_INDEX:
        {
            uint32_t id = get_u32(argdata+0);
            Prop2D *prop = g_prop2d_pool.get(id);
            if(prop) {
                int index = get_u32(argdata+4);
                prop->setIndex(index);
            }
        }
        break;
    case PACKETTYPE_S2C_PROP2D_FLIPROTBITS:
        {
            uint32_t id = get_u32(argdata+0);
            uint8_t bits = argdata[4];
            Prop2D *prop = g_prop2d_pool.get(id);
            if(prop) {
                bool xfl,yfl,uvr;
                fromFlipRotBits(bits,&xfl,&yfl,&uvr);
                prop->setXFlip(xfl);
                prop->setYFlip(yfl);
                prop->setUVRot(uvr);
            }
        }
        break;
    case PACKETTYPE_S2C_PROP2D_OPTBITS:
        {
            uint32_t id = get_u32(argdata+0);
            Prop2D *prop = g_prop2d_pool.get(id);
            if(prop) {
                uint32_t bits = get_u32(argdata+4);
                prop->use_additive_blend = bits & PROP2D_OPTBIT_ADDITIVE_BLEND;
                prt("OPT %d", bits );
            }
        }
        break;
    case PACKETTYPE_S2C_PROP2D_PRIORITY:
        {
            uint32_t id = get_u32(argdata+0);
            Prop2D *prop = g_prop2d_pool.get(id);
            if(prop) {
                uint32_t prio = get_u32(argdata+4);
                prop->priority = prio;
                prt("PRIO %d", prio);
            }
        }
        break;
        
    case PACKETTYPE_S2C_LAYER_CREATE:
        {
            uint32_t id = get_u32( argdata+0 );
            uint32_t prio = get_u32( argdata+4 );
            print("PACKETTYPE_S2C_LAYER_CREATE layer_id:%d prio:%d", id, prio );

            Layer *l = g_layer_pool.get(id);
            if(!l) {
                l = g_layer_pool.ensure(id);
                g_moyai_client->insertLayer(l);
                l->priority = prio;
                print("created a layer" );
            } else {
                print("layer found");
            }
        }
        break;

    case PACKETTYPE_S2C_VIEWPORT_CREATE:
        {
            unsigned int viewport_id = get_u32(argdata);
            print("received viewport_create. id:%d", viewport_id );
            Viewport *vp = g_viewport_pool.ensure(viewport_id);
            assert(vp);            
        }
        break;

    case PACKETTYPE_S2C_VIEWPORT_SCALE:
        {
            unsigned int viewport_id = get_u32(argdata);
            float sclx = get_f32(argdata+4);
            float scly = get_f32(argdata+8);
            print("received viewport_scale id:%d scl.x:%f scl.y:%f", viewport_id, sclx, scly);

            Viewport *vp = g_viewport_pool.ensure(viewport_id);
            assert(vp);
            vp->setScale2D(sclx,scly);
        }
        break;
    case PACKETTYPE_S2C_VIEWPORT_DYNAMIC_LAYER:
        {
            unsigned int viewport_id = get_u32(argdata);
            unsigned int layer_id = get_u32(argdata+4);
            Viewport *vp = g_viewport_pool.get(viewport_id);
            Layer *l = g_layer_pool.get(layer_id);
            print("received vp dyn layer. vp:%d l:%d",viewport_id, layer_id);
            if(vp&&l) {
                l->setViewport(vp);
            }
        }
        break;

    case PACKETTYPE_S2C_CAMERA_CREATE:
        {
            unsigned int camera_id = get_u32(argdata);
            print("received camera_create. id:%d", camera_id );
            Camera *cam = g_camera_pool.ensure(camera_id);
            assert(cam);
        }
        break;
    case PACKETTYPE_S2C_CAMERA_LOC:
        {
            unsigned int camera_id = get_u32(argdata);
            float x = get_f32(argdata+4);
            float y = get_f32(argdata+4+4);
            //            print("received s2c_camera_loc. id:%d (%f,%f)", camera_id, x,y );            
            Camera *cam = g_camera_pool.get(camera_id);
            assert(cam);
            cam->setLoc(x,y);
        }
        break;
    case PACKETTYPE_S2C_CAMERA_DYNAMIC_LAYER:
        {
            unsigned int camera_id = get_u32(argdata);
            unsigned int layer_id = get_u32(argdata+4);
            Camera *cam = g_camera_pool.get(camera_id);
            Layer *l = g_layer_pool.get(layer_id);
            print("received s2c_camera_dynamic_layer. cam:%d l:%d",camera_id, layer_id);
            if(cam && l) {
                l->setCamera(cam);
            }
        }
        break;

    case PACKETTYPE_S2C_LAYER_CAMERA:
        {
            unsigned int layer_id = get_u32(argdata);
            unsigned int camera_id = get_u32(argdata+4);
            print("received layer_camera. l:%d cam:%d", layer_id, camera_id );                        
            Layer *l = g_layer_pool.get(layer_id);
            assert(l);
            Camera *cam = g_camera_pool.ensure(camera_id);
            assert(cam);
            l->setCamera(cam);
        }
        break;
    case PACKETTYPE_S2C_LAYER_VIEWPORT:
        {
            unsigned int layer_id = get_u32(argdata);
            unsigned int viewport_id = get_u32(argdata+4);
            print("received layer_viewport. l:%d vp:%d", layer_id, viewport_id );
            Layer *l = g_layer_pool.get(layer_id);
            assert(l);
            Viewport *vp = g_viewport_pool.ensure(viewport_id);
            assert(vp);
            l->setViewport(vp);
        }
        break;


    case PACKETTYPE_S2C_TEXTURE_CREATE:
        {
            unsigned int tex_id = get_u32(argdata);
            print("received texture_create. id:%d", tex_id );
            Texture *tex = g_texture_pool.ensure(tex_id);
            assert(tex);
        }
        break;
    case PACKETTYPE_S2C_TEXTURE_IMAGE:
        {
            unsigned int tex_id = get_u32(argdata);
            unsigned int img_id = get_u32(argdata+4);
            print("received texture_image. tex:%d img:%d", tex_id, img_id );
#if 0

            {
                Prop2D *p = findProp2DByTexture(tex_id);
                if(p) {
                    p->debug_id = 111;
                    print("  deck info: %d,%d,%d,%d  ind:%d scl:%f,%f",
                          p->deck->cell_width, p->deck->cell_height, p->deck->tile_width, p->deck->tile_height,
                          p->index, p->scl.x, p->scl.y
                          );
                } else {
                    print("  deck not found for prop, tex_id:%d ", tex_id );
                }
            }
              
            
#endif            
            Texture *tex = g_texture_pool.get(tex_id);
            assert(tex);
            Image *img = g_image_pool.get(img_id);
            assert(img);
            if(g_enable_reprecation) {
                tex->image = img; // skip generating OpenGL texture
            } else {
                tex->setImage(img);
            }
        }
        break;
    case PACKETTYPE_S2C_IMAGE_CREATE:
        {
            unsigned int img_id = get_u32(argdata);
            Image *img = g_image_pool.ensure(img_id);
            assert(img);
            print("received image_create. id:%d", img_id );
        }
        break;
    case PACKETTYPE_S2C_IMAGE_LOAD_PNG:        
        {
            unsigned int img_id = get_u32(argdata);
            unsigned char pathlen = get_u8(argdata+4);
            char *path = argdata+4+1;
            char cstrpath[256];
            memcpy( cstrpath, path, pathlen );
            cstrpath[pathlen]='\0';
            print("received image loadpng. id:%d path:'%s' ", img_id, cstrpath );
            Image *img = g_image_pool.ensure(img_id);
            File *fe = g_filedepo->get(cstrpath);
            assert(fe);
            bool ret = img->loadPNGMem( (unsigned char*) fe->data, fe->data_len );
            assert(ret);
            strncpy( img->last_load_file_path, cstrpath, sizeof(img->last_load_file_path) );
        }
        break;
    case PACKETTYPE_S2C_TILEDECK_CREATE:
        {
            unsigned int dk_id = get_u32(argdata);
            print("received tiledeck_create. id:%d", dk_id);
            TileDeck *dk = g_tiledeck_pool.ensure(dk_id);
            assert(dk);
        }
        break;        
    case PACKETTYPE_S2C_TILEDECK_TEXTURE:
        {
            unsigned int dk_id = get_u32(argdata);
            unsigned int tex_id = get_u32(argdata+4);
            print("received tiledeck_texture. dk:%d tex:%d", dk_id, tex_id );
            
            TileDeck *dk = g_tiledeck_pool.get(dk_id);
            assert(dk);
            Texture *tex = g_texture_pool.get(tex_id);
            assert(tex);
            if(g_enable_reprecation) {
                dk->tex = tex;
            } else {
                dk->setTexture(tex);
            }
        }
        break;
    case PACKETTYPE_S2C_TILEDECK_SIZE: 
        {
            unsigned int dk_id = get_u32(argdata);
            int sprw = get_u32(argdata+4);
            int sprh = get_u32(argdata+8);
            int cellw = get_u32(argdata+12);
            int cellh = get_u32(argdata+16);
            print("received tiledeck_size. dk:%d %d,%d,%d,%d", dk_id, sprw, sprh, cellw, cellh );            
            TileDeck *dk = g_tiledeck_pool.get(dk_id);
            assert(dk);
            dk->setSize( sprw, sprh, cellw, cellh );

        }
        break;
    case PACKETTYPE_S2C_FILE:
        {
            // pathと内容を同時に受け取る。
            char *dataptr;
            size_t datasize;
            char cstrpath[256];
            parsePacketStrBytes( argdata, cstrpath, &dataptr, &datasize );
            
            print("received file. path:'%s' datalen:%d data:%x %x %x %x", cstrpath, datasize, dataptr[0], dataptr[1], dataptr[2], dataptr[3] );
            g_filedepo->ensure( cstrpath, dataptr, datasize );
        }
        break;
    case PACKETTYPE_S2C_PROP2D_DELETE:
        {
            uint32_t prop_id = get_u32(argdata);
            Prop2D *prop = g_prop2d_pool.get(prop_id);
            if(prop) {
                //                prt("D[%d]", prop_id);
                prop->to_clean = true;
                g_prop2d_pool.del(prop_id);
            }
        }
        break;
    case PACKETTYPE_S2C_PROP2D_CLEAR_CHILD:
        {
            uint32_t owner_prop_id = get_u32(argdata);
            uint32_t child_prop_id = get_u32(argdata+4);
            
            Prop2D *owner_prop = g_prop2d_pool.get(owner_prop_id);
            if(owner_prop) {
                Prop2D *child_prop = g_prop2d_pool.get(child_prop_id);
                owner_prop->clearChild(child_prop);
                //                print("prop2d_clear_child remove %d from owner %d", child_prop_id, owner_prop_id );
            }
        }
        break;
    case PACKETTYPE_S2C_GRID_CREATE:
        {
            uint32_t grid_id = get_u32(argdata);
            uint32_t w = get_u32(argdata+4);
            uint32_t h = get_u32(argdata+8);
            //            print("grid_create. id:%d w:%d h:%d", grid_id, w, h );
            Grid *g = g_grid_pool.ensure( grid_id, w, h );
            assert(g);
        }
        break;
    case PACKETTYPE_S2C_GRID_DECK:
        {
            uint32_t grid_id = get_u32(argdata);
            uint32_t deck_id = get_u32(argdata+4);
            //            print("grid_deck. id:%d tid:%d", grid_id, deck_id );
            Grid *g = g_grid_pool.get(grid_id);
            if(g) {
                TileDeck *td = g_tiledeck_pool.get(deck_id);
                if(td) {
                    //                    print("grid_deck: td:%d found. %d,%d,%d,%d",deck_id, td->cell_width, td->cell_height, td->tile_width, td->tile_height );
                    g->setDeck(td);
                } else {
                    print("grid_deck: can't find td:%d", deck_id);
                }                
            } else {
                print("can't find grid id:%d",grid_id);
            }
        }
        break;
    case PACKETTYPE_S2C_GRID_PROP2D:
        {
            uint32_t grid_id = get_u32(argdata);
            uint32_t prop_id = get_u32(argdata+4);
            //            print("grid_prop2d: id:%d pid:%d", grid_id, prop_id );
            Grid *g = g_grid_pool.get(grid_id);
            Prop2D *p = g_prop2d_pool.get(prop_id);
            if( g && p ) {
                p->setGrid(g);                
            } else {
                print("grid_prop2d: grid:%p or prop:%p not found", g,p);
            }
        }
        break;
    case PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT:
        {
            uint32_t grid_id = get_u32(argdata);            
            Grid *g = g_grid_pool.get(grid_id);
            if(g) {
                int32_t *inds = (int32_t*)(argdata+8);
                g->bulkSetIndex(inds);
            }
        }
        break;
    case PACKETTYPE_S2C_GRID_TABLE_FLIP_SNAPSHOT:
        {
            uint32_t grid_id = get_u32(argdata);
            uint32_t bytes_num = get_u32(argdata+4);            
            Grid *g = g_grid_pool.get(grid_id);
            if(g) {
                uint8_t *flips = (uint8_t*)(argdata+8);
                for(int i=0;i<bytes_num;i++) {
                    uint8_t flipbits = flips[i];
                    g->setXFlipIndex( i, flipbits & GTT_FLIP_BIT_X );
                    g->setYFlipIndex( i, flipbits & GTT_FLIP_BIT_Y );
                    g->setUVRotIndex( i, flipbits & GTT_FLIP_BIT_UVROT );
                }
            }
        }
        break;
    case PACKETTYPE_S2C_GRID_TABLE_TEXOFS_SNAPSHOT:
        {
            uint32_t grid_id = get_u32(argdata);
            uint32_t bytes_num = get_u32(argdata+4);            
            Grid *g = g_grid_pool.get(grid_id);
            if(g) {
                assert( (bytes_num % sizeof(PacketVec2) ) == 0 );                
                PacketVec2 *ofstbl = (PacketVec2*)(argdata+8);
                int n = bytes_num / sizeof(PacketVec2);
                for(int i=0; i < n; i++ ) {
                    Vec2 ofs(ofstbl[i].x, ofstbl[i].y);
                    g->setTexOffsetIndex( i, &ofs );
                }
            }
        }
        break;
    case PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT:
        {
            uint32_t grid_id = get_u32(argdata);
            uint32_t bytes_num = get_u32(argdata+4);
            Grid *g = g_grid_pool.get(grid_id);
            if(g) {
                PacketColor *cols = (PacketColor*)(argdata+8);
                int n = bytes_num/sizeof(PacketColor);
                assert( (bytes_num % sizeof(PacketColor)) == 0 );
                for(int i=0;i<n;i++) {
                    Color outcol;
                    copyPacketColorToColor( &outcol, &cols[i] );
                    g->setColorIndex(i,outcol);
                }
            }
        }
        break;
    case PACKETTYPE_S2C_GRID_DELETE:
        {
            uint32_t grid_id = get_u32(argdata);
            //            print("grid_del: id:%d", grid_id);
            g_grid_pool.del(grid_id);
        }
        break;
    case PACKETTYPE_S2C_TEXTBOX_CREATE:
        {
            uint32_t tb_id = get_u32(argdata);
            TextBox *tb = g_textbox_pool.ensure(tb_id);
            if(g_enable_reprecation) tb->skip_meshing = true;
            //            print("received tb_create. id:%d ptr:%p par:%p", tb_id, tb, tb->getParentLayer() );
        }
        break;
    case PACKETTYPE_S2C_TEXTBOX_FONT:
        {
            uint32_t tb_id = get_u32(argdata);
            uint32_t font_id = get_u32(argdata+4);
            //            print("tb_font id:%d fid:%d", tb_id, font_id );
            TextBox *tb = g_textbox_pool.get(tb_id);
            if(!tb) {
                print("tb_font: tb %d not found", tb_id);
                break;
            }
            Font *f = g_font_pool.get(font_id);
            if(!f) {
                print("tb_font: font %d not found", font_id);
                break;
            }
            tb->setFont(f);
        }
        break;
    case PACKETTYPE_S2C_TEXTBOX_STRING:
        {
            uint32_t tb_id = get_u32(argdata);
            uint32_t bufsz = get_u32(argdata+4);
            uint8_t *str_utf8 = (uint8_t*)(argdata+8);
            TextBox *tb = g_textbox_pool.get(tb_id);
            if(!tb) {
                print("tb %d not found",tb_id);
                break;
            }
            char *tmpbuf = (char*)MALLOC( bufsz+1);
            assert(tmpbuf);
            memcpy(tmpbuf, str_utf8, bufsz );
            tmpbuf[bufsz] = '\0';
            //            print("tb_str. tb:%d bufsz:%d s:'%s'", tb_id, bufsz, tmpbuf );
            tb->setString(tmpbuf);
        }
        break;
    case PACKETTYPE_S2C_TEXTBOX_LOC:
        {
            uint32_t tb_id = get_u32(argdata);
            float x = get_f32(argdata+4);
            float y = get_f32(argdata+8);
            //            print("tb %d loc:%f,%f",tb_id, x,y);
            TextBox *tb = g_textbox_pool.get(tb_id);
            if(!tb) {
                print("tb %d not found", tb_id);
                break;
            }
            tb->setLoc(x,y);
        }
        break;
    case PACKETTYPE_S2C_TEXTBOX_SCL:
        {
            uint32_t tb_id = get_u32(argdata);
            float x = get_f32(argdata+4);
            float y = get_f32(argdata+8);
            //            print("tb %d scl:%f,%f",tb_id, x,y);
            TextBox *tb = g_textbox_pool.get(tb_id);
            if(!tb) {
                print("tb %d not found", tb_id);
                break;
            }
            tb->setScl(x,y);
        }
        break;                
    case PACKETTYPE_S2C_TEXTBOX_COLOR:
        {
            uint32_t tb_id = get_u32(argdata);
            uint32_t pktsz = get_u32(argdata+4);
            char *pktdata = (char*)(argdata+4+4);
            assert(pktsz == sizeof(PacketColor));
            TextBox *tb = g_textbox_pool.get(tb_id);
            if(!tb) {
                print("tb %d not found", tb_id);
                break;
            }
            PacketColor *pc = (PacketColor*) pktdata;
            Color col( pc->r, pc->g, pc->b, pc->a );
            //            print("tb %d color: %f %f %f %f par:%p", tb_id, col.r, col.g, col.b, col.a, tb->getParentLayer() );
            tb->setColor(col);
        }
        break;
    case PACKETTYPE_S2C_TEXTBOX_LAYER:
        {
            uint32_t tb_id = get_u32(argdata);
            uint32_t layer_id = get_u32(argdata+4);            
            TextBox *tb = g_textbox_pool.get(tb_id);
            Layer *l = g_layer_pool.get(layer_id);
            if(!tb) {
                print("tb_layer tb:%d not found", tb_id);
                break;
            }
            //            print("tb_layer tb:%d l:%d par:%p", tb_id, layer_id, tb->getParentLayer() );
            Layer *pl = tb->getParentLayer();
            if( pl ) {
                //                print("  tb %d already has parent layer id:%d, argument:%d", tb_id, pl->id, layer_id );
                break;
            } else {
                l->insertProp(tb);
            }
        }
        break;


    case PACKETTYPE_S2C_FONT_CREATE:
        {
            uint32_t font_id = get_u32(argdata);
            print("font_create id:%d", font_id );
            Font *f = g_font_pool.ensure(font_id);
            if(g_enable_reprecation) f->skip_actual_font_load=true; // don't generate opengl texture when repreproxy
        }
        break;        
    case PACKETTYPE_S2C_FONT_CHARCODES: // fontid, utf8str
        {
            uint32_t font_id = get_u32(argdata+0);
            uint32_t bufsz = get_u32(argdata+4);
            const uint8_t *in_u8 = (UTF8*)(argdata+8); // bin array. dont have null terminator
            print("charcodes: bufsz:%d", bufsz );

            Font *f = g_font_pool.get(font_id);
            if(!f) {
                print("font %d is not found", font_id);
                break;
            }
            wchar_t *charcodes = allocateWCharStringFromUTF8String(in_u8,bufsz);
            f->setCharCodes(charcodes);
            FREE(charcodes);
        }
        break;        
    case PACKETTYPE_S2C_FONT_LOADTTF:
        {
            uint32_t font_id = get_u32(argdata+0);
            uint32_t pixel_size = get_u32(argdata+4);
            uint8_t cstrlen = get_u8(argdata+4+4);
            char *path = argdata+4+4+1;
            char pathbuf[256+1];
            memcpy( pathbuf, path, cstrlen );
            pathbuf[cstrlen] = '\0';
            print("font_loadttf id:%d ps:%d path:'%s'", font_id, pixel_size, pathbuf );
            Font *f = g_font_pool.get(font_id);
            if(!f) {
                print("font %d not found", font_id);
                break;
            }
            // freetype-gl can only read TTF from file, not from memory
            File *file = g_filedepo->get(pathbuf);
            if(!file) {
                print("  can't find file in filedepo:'%s'", pathbuf );
                break;
            }
            char tmppath[1024];
            file->saveInTmpDir( "/tmp", tmppath, sizeof(tmppath) );
            bool res = f->loadFromTTF(tmppath,NULL,pixel_size);
            assert(res);            
        }
        break;
    case PACKETTYPE_S2C_COLOR_REPLACER_SHADER_SNAPSHOT:
        {
            uint32_t pktsize = get_u32(argdata+0);
            assertmsg(pktsize == sizeof(PacketColorReplacerShaderSnapshot), "invalid packet size:%d", pktsize );
            PacketColorReplacerShaderSnapshot pkt;
            memcpy(&pkt, argdata+4, sizeof(pkt) );
            //            print("crs ss id:%d", pkt.shader_id );
            ColorReplacerShader *s = g_crshader_pool.get(pkt.shader_id);
            if(!s) {
                print("received colorreplacershader_snapshot creating new one. id:", pkt.shader_id);
                s = g_crshader_pool.ensure(pkt.shader_id);
            }
            if(g_enable_reprecation==false) {
                if( !s->init() ) {
                    assertmsg(false, "shader %d init failed, fatal", pkt.shader_id);
                }
            }
            Color fromcol, tocol;
            copyPacketColorToColor( &fromcol, &pkt.from_color);
            copyPacketColorToColor( &tocol, &pkt.to_color);
            s->setColor( fromcol, tocol, pkt.epsilon);
        }
        break;

    case PACKETTYPE_S2C_PRIM_BULK_SNAPSHOT:
        {
            uint32_t prop_id = get_u32(argdata+0);
            uint32_t pktsize = get_u32(argdata+4);
            int pktnum = pktsize / sizeof(PacketPrim);
            assert( pktsize % sizeof(PacketPrim) == 0 );

            Prop2D *prop = g_prop2d_pool.get(prop_id);
            if(!prop) {
                print("primbulk: can't find prop %d", prop_id );
                break;
            }
            
            prop->ensurePrimDrawer();
                
            //            print("prim bulk. pktsize:%d num:%d",pktsize, pktnum );
            for(int i=0;i<pktnum;i++){
                PacketPrim *pkt = & ((PacketPrim*)(argdata+8))[i];
                Prim *prim = g_prim_pool.ensure( pkt->prim_id );
                prim->type = (PRIMTYPE) pkt->prim_type;
                prim->a.x = pkt->a.x;
                prim->a.y = pkt->a.y;
                prim->b.x = pkt->b.x;
                prim->b.y = pkt->b.y;
                copyPacketColorToColor( &prim->color, &pkt->color );
                prim->line_width = pkt->line_width;
                prop->prim_drawer->ensurePrim(prim);                
            }

            // check for deleted prims
            if( prop->prim_drawer->prim_num > pktnum ) {
                //                print( "primitive deleted? pkt:%d local:%d", pktnum, prop->prim_drawer->prim_num );
                for(int i=0;i<prop->prim_drawer->prim_num;i++) {
                    Prim *prim = prop->prim_drawer->prims[i];
                    bool found = false;
                    for(int j=0;j<pktnum;j++) {                        
                        PacketPrim *pkt = & ((PacketPrim*)(argdata+8))[i];
                        if( pkt->prim_id == prim->id ) {
                            found = true;
                        }
                    }
                    if(!found) {
                        //                        print("  local prim id:%d not found in bulk packet. deleting.", prim->id );
                        prop->prim_drawer->deletePrim(prim->id);
                    }
                }
            }
        }            
        break;
    case PACKETTYPE_S2C_IMAGE_ENSURE_SIZE: // for dynamic images
        {
            uint32_t img_id = get_u32(argdata+0);
            uint32_t w = get_u32(argdata+4);
            uint32_t h = get_u32(argdata+8);
            //            print("image_ensure_size. id:%d w:%d h:%d", img_id, w, h);
            Image *img = g_image_pool.get(img_id);
            if(!img) {
                print("  image not found. id:%d", img_id);
                break;
            }
            if( img->buffer ) {
                assertmsg( img->width == w && img->height == h, "dynamic image size is not implemented" );
            } else {
                print("  ensuring image buffer %d %d %d", img_id, w, h );
                img->setSize(w,h);
                img->ensureBuffer();
            }
        }
        break;
    case PACKETTYPE_S2C_IMAGE_RAW: // for dynamic images
        {
            uint32_t img_id = get_u32(argdata+0);
            uint32_t data_size = get_u32(argdata+4);
            
            //            print("image_raw. id:%d datasz:%d", img_id, data_size );
            Image *img = g_image_pool.get(img_id);
            if(!img) {
                print("  image not found:%d",img_id);
                break;
            }
            assertmsg( img->getBufferSize() == data_size, "dynamic image size is not implemented. local:%d pkt:%d", img->getBufferSize(), data_size);

            uint8_t *data = (uint8_t*)( argdata+8);
            //            ::dump( (const char*) data, 256 );            
            img->setAreaRaw( 0, 0, img->width, img->height, data, img->getBufferSize() );
            //            img->writePNG("./ppo.png");            
        }
        break;
    case PACKETTYPE_S2C_SOUND_CREATE_FROM_FILE:
        {
            uint32_t snd_id = get_u32(argdata+0);
            Sound *snd = g_sound_pool.get(snd_id);
            if(snd) {
                print("sound_create_from_file: snd %d found, ignore", snd_id );
                break;
            }
            uint8_t path_cstr_len = *(argdata+4);
            const char *path_cstr_head = argdata+4+1;
            assert( path_cstr_len <= 255 );
            char path[256];
            memcpy( path, path_cstr_head, path_cstr_len );
            path[path_cstr_len] = '\0';
            print("sound_create_from_file. id:%d path:%s", snd_id, path );
            File *file = g_filedepo->get(path);
            if(!file) {
                print("  can't find file in filedepo:'%s", path );
                break;
            }
            char tmppath[1024];
            file->saveInTmpDir( "/tmp", tmppath, sizeof(tmppath) );
            snd = g_soundsystem->newSound( tmppath );
            g_sound_pool.set( snd_id, snd );                
        }
        break;
    case PACKETTYPE_S2C_SOUND_CREATE_FROM_SAMPLES:
        {
            uint32_t snd_id = get_u32(argdata+0);
            Sound *snd = g_sound_pool.get(snd_id);
            if(snd) {
                print("sound_create_from_samples: snd %d found, ignore", snd_id );
                break;
            }
            uint32_t bytenum = get_u32(argdata+4);
            assertmsg( bytenum % sizeof(float) == 0, "invalid sample format" );
            size_t samples_num = bytenum / sizeof(float);
            float* samples = (float*)(argdata+4+4);
            if(g_enable_reprecation) {
                snd = g_soundsystem->newSoundFromMemoryVirtual( samples, samples_num );
            } else {
                snd = g_soundsystem->newSoundFromMemory( samples, samples_num );
            }            
            g_sound_pool.set( snd_id, snd );
            print("sound_create_from_samples: id:%d samples_num:%d", snd_id, samples_num );
        }
        break;
    case PACKETTYPE_S2C_SOUND_DEFAULT_VOLUME:
        {
            uint32_t snd_id = get_u32(argdata+0);
            float vol = get_f32(argdata+4);
            Sound *snd = g_sound_pool.get(snd_id);
            if(snd) {
                snd->default_volume = vol;
            } else {
                print("sound_default_volume: id:%d vol:%f", snd_id, vol );
                break;
            }            
        }
        break;
    case PACKETTYPE_S2C_SOUND_PLAY:
        {
            uint32_t snd_id = get_u32(argdata+0);
            float vol = get_f32(argdata+4);
            Sound *snd = g_sound_pool.get(snd_id);
            if(snd) {
                if(g_enable_reprecation) {
                } else {
                    snd->play(vol);
                }                
            } else {
                print("sound_play: %d not found", snd_id );
            }
                
        }
        break;
    case PACKETTYPE_S2C_SOUND_STOP:
        {
            uint32_t snd_id = get_u32(argdata+0);
            Sound *snd = g_sound_pool.get(snd_id);
            if(snd) {
                snd->stop();
            } else {
                print( "sound_stop: snd %d not found", snd_id );
            }
        }
        break;
    case PACKETTYPE_S2C_SOUND_POSITION:
        {
            uint32_t snd_id = get_u32(argdata+0);
            float pos_sec = get_f32(argdata+4);
            float last_play_vol = get_f32(argdata+8);
            
            Sound *snd = g_sound_pool.get(snd_id);
            if(snd) {
                print("sound_position: id:%d pos:%f last:%f", snd_id, pos_sec, last_play_vol );
                if(!g_enable_reprecation) {
                    if(snd->isPlaying() == false ) snd->play(last_play_vol);
                    snd->setTimePositionSec( pos_sec );
                }
            } else {
                print("sound_position: %d not found", snd_id );
            }
        }
        break;
    case PACKETTYPE_S2C_WINDOW_SIZE:
        {
            void setupClient( int win_w, int win_h );
            uint32_t w = get_u32(argdata+0);
            uint32_t h = get_u32(argdata+4);
            print("received window_size. %d,%d",w,h);
            if(g_enable_reprecation) {
                g_window_width = w;
                g_window_height = h;
                assert(!g_moyai_client);
                g_moyai_client = new MoyaiClient( NULL, w, h );
                assert(!g_soundsystem);
                g_soundsystem = new SoundSystem();                
            } else {
                setupClient(w,h);
            }            
        }
        break;
    case PACKETTYPE_S2C_JPEG_DECODER_CREATE:
        {
            uint32_t pixel_skip = get_u32(argdata+0);
            uint32_t orig_w = get_u32(argdata+4);
            uint32_t orig_h = get_u32(argdata+8);
            print("received jpeg_dec_creat: skip:%d %d,%d", pixel_skip, orig_w, orig_h );
            int imgw = orig_w/(pixel_skip+1), imgh = orig_h/(pixel_skip+1);
            g_jc = new JPEGCoder(imgw, imgh, 0);
            setupVideoViewer(imgw,imgh);
        }
        break;
    case PACKETTYPE_S2C_CAPTURED_FRAME:
        {
            assert(g_jc);
            uint32_t compressed_size = get_u32(argdata+0);
            g_jc->setCompressedData( (const unsigned char*)(argdata+4), compressed_size );
            double t0 = now();
            g_jc->decode();
            double t1 = now();
            if((t1-t0)>0.02) print("slow decode:%f",t1-t0);
            //            print("capt_frame: time:%f size:%d",t1-t0, compressed_size);
#if 0
            writeFile( "received.jpg", (const char*)g_jc->compressed, g_jc->compressed_size );
#endif            
            updateVideoViewer();
        }
        break;
    case PACKETTYPE_S2C_CAPTURED_AUDIO:
        {
            uint32_t num_samples = get_u32(argdata+0);
            uint32_t samples_bytes = get_u32(argdata+4);
            //            print("audio! ns:%d sb:%d", num_samples, samples_bytes );
            assert( num_samples * sizeof(float) * 2 == samples_bytes );
            
#ifdef USE_UNTZ
            float *interleavedSamples = (float*)(argdata+8);
            appendAudioSampleStereo(interleavedSamples, num_samples*2);
#else            
            print("CAPTURED_AUDIO: only implemented with UNTZ");
#endif            
        }
        break;
    default:
        print("unhandled packet type:%d", funcid );
        break;
    }
}

void on_data( uv_stream_t *s, ssize_t nread, const uv_buf_t *buf) {
    //    print("on_data. nread:%d",nread);
    if(nread<0) {
        print("read error! st:%d closing..", nread);
        g_done=true;
        return;
    } 
    g_total_read_count ++;
    g_total_read += nread;

    if(g_savepath[0]) {
        saveStreamToBuffer(&g_savebuf, buf->base, nread, g_savepath );
    }
    Stream *stream = (Stream*)s->data;
    parseRecord( stream, &stream->recvbuf, buf->base, nread, on_packet_callback );
    parseRecord( stream, &stream->unzipped_recvbuf, NULL, 0, on_packet_callback );
    
}
Stream *createStream(uv_tcp_t *tcp, bool compression ) {
    Stream *out = new Stream( (uv_tcp_t*) tcp, 8*1024, 16*1024*1024,compression);
    tcp->data=out;
    return out;
}

void on_connect( uv_connect_t *connect, int status ) {
    print("on_connect status:%d",status);
    
    int r = uv_read_start( (uv_stream_t*)connect->handle, moyai_libuv_alloc_buffer, on_data );
    if(r) {
        print("uv_read_start: fail:%d",r);
    }
    g_stream = createStream((uv_tcp_t*)connect->handle, false);
}


        
void printStats() {
    SorterEntry se[PACKETTYPE_MAX];
    int pktinds[PACKETTYPE_MAX];
    memset( pktinds,0,sizeof(pktinds));
    int se_ind=0;
    for(int i=0;i<PACKETTYPE_MAX;i++) {
        if( g_recv_totalcounts[i]==0)continue;
        se[se_ind].val = g_recv_sizes[i];
        pktinds[i] = i;
        se[se_ind].ptr = (void*) (&pktinds[i]);
        se_ind++;
    }
    quickSortF(se,0,se_ind-1);
    for(int i=0;i<se_ind;i++){
        int pkttype = *((int*)se[i].ptr);
        print("%s cnt: %d(%d) sz: %d(%d)",
              RemoteHead::funcidToString((PACKETTYPE)pkttype),
              g_recv_counts[pkttype], g_recv_totalcounts[pkttype], g_recv_sizes[pkttype], g_recv_totalsizes[pkttype] );
    }
           
    for(int i=0;i<elementof(g_recv_counts);i++) g_recv_counts[i]=g_recv_sizes[i]=0;
}

bool parseProgramArgs( int argc, char **argv ) {
    const char *port_prefix = "--port=";
    const char *save_prefix = "--savepath=";
    for(int i=1;i<argc;i++) {
        if( strncmp( argv[i], port_prefix, strlen(port_prefix) ) == 0 ){
            g_port = atoi( argv[i] + strlen(port_prefix) );
        } else {
            g_server_ip_addr = argv[i];
        }
        if( strcmp( argv[i], "--print_stats" ) == 0 ) {
            g_enable_print_stats = true;
        }
        if( strcmp( argv[i], "--reprecation" ) == 0 ) {
            g_enable_reprecation = true;
        }
        if( strncmp( argv[i], save_prefix, strlen(save_prefix)) == 0 ) {
            snprintf( g_savepath, sizeof(g_savepath), "%s", argv[i] + strlen(save_prefix));
        }
    }
    print("viewer config: serverip:'%s' port:%d window:%d,%d", g_server_ip_addr, g_port, g_window_width, g_window_height );
    return true;
}


void setupClient( int win_w, int win_h ) {
    if(g_moyai_client) {
        assertmsg( false, "setupclient: can't setup client twice");
    }
    g_window_width = win_w;
    g_window_height = win_h;
    
    g_soundsystem = new SoundSystem();

#ifdef USE_UNTZ
    g_audio_stream = UNTZ::Sound::create(44100,2,stream_callback, NULL );
    g_audio_stream->play();
#endif
    
    //

    if( !glfwInit() ) {
        print("can't init glfw");
        exit(1);        
    }

    glfwSetErrorCallback( glfw_error_cb );
    g_window =  glfwCreateWindow( g_window_width, g_window_height, "Moyai sprite stream viewer", NULL, NULL );
    if(g_window == NULL ) {
        print("can't open glfw window");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(g_window);    
    glfwSetWindowCloseCallback( g_window, winclose_callback );
    glfwSetInputMode( g_window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSwapInterval(1); // 1 for use vsync
#ifdef WIN32
	glewInit();
#endif
    glClearColor(0.2,0.2,0.2,1);

    glfwSetKeyCallback( g_window, keyboardCallback );
    glfwSetMouseButtonCallback( g_window, mouseButtonCallback );
    glfwSetCursorPosCallback( g_window, cursorPosCallback );
    
    g_moyai_client = new MoyaiClient( g_window, win_w, win_h );

    g_local_viewport = new Viewport();
    g_local_viewport->setSize(g_window_width*RETINA,g_window_height*RETINA);
    g_local_viewport->setScale2D(g_window_width,g_window_height);
    
    // Client side debug status
    setupDebugStat();    
}

void reproxy_on_packet_cb( Stream *stream, uint16_t funcid, char *argdata, uint32_t argdatalen ) {
    if(!g_reproxy)return;
    Client *cl = (Client*)stream;

    switch(funcid) {
    case PACKETTYPE_PING:
        break;
    case PACKETTYPE_C2S_KEYBOARD:
        {
            uint32_t keycode = get_u32(argdata);
            uint32_t action = get_u32(argdata+4);
            uint32_t modbits = get_u32(argdata+8);        
            print("reproxy_on_packet_cb: kbd. cl_g_id:%d",cl->global_client_id);
            sendUS1UI4(g_stream,PACKETTYPE_R2S_KEYBOARD, cl->global_client_id, keycode, action, modbits );
        }
        break;
    case PACKETTYPE_C2S_MOUSE_BUTTON:
        {
            uint32_t btn = get_u32(argdata);
            uint32_t act = get_u32(argdata+4);
            uint32_t modbits = get_u32(argdata+8);
            //            print("reproxy_on_packet_cb: mousebtn. cl_g_id:%d",cl->global_client_id);
            sendUS1UI4(g_stream,PACKETTYPE_R2S_MOUSE_BUTTON, cl->global_client_id, btn,act,modbits);
        }
        break;
    case PACKETTYPE_C2S_CURSOR_POS:
        {
            float x = get_f32(argdata);
            float y = get_f32(argdata+4);
            //            print("reproxy_on_packet_cb: cursor. %f,%f",x,y);
            sendUS1UI1F2(g_stream,PACKETTYPE_R2S_CURSOR_POS, cl->global_client_id,x,y);
        }        
        break;
    case PACKETTYPE_C2S_TOUCH_BEGIN:
    case PACKETTYPE_C2S_TOUCH_MOVE:
    case PACKETTYPE_C2S_TOUCH_END:
    case PACKETTYPE_C2S_TOUCH_CANCEL:
        break;
    default:
        print("Warning: reproxy_on_packet_cb: funcid:%d is not handled",funcid);
        break;
    }
}


void reproxy_on_close_cb( Stream *s ) {
    Client *cl = (Client*)s;
    print("reproxy_on_close_cb: sending logout to master.. clid:%d gclid:%d",cl->id, cl->global_client_id);
    sendUS1UI1(g_stream, PACKETTYPE_R2S_CLIENT_LOGOUT, cl->global_client_id);
}

void reproxy_on_accept_cb( Stream *newstream ) {
    print("reproxy_on_accept_cb");
    sendWindowSize(newstream,g_window_width,g_window_height);
    // scansendallprereqs
    POOL_SCAN(g_viewport_pool,Viewport) {
        print("sending vp id:%d scl:%f,%f", it->second->id, it->second->scl.x, it->second->scl.y );
        sendViewportCreateScale(newstream,it->second);
    }
    POOL_SCAN(g_camera_pool,Camera) {
        print("sending cam id:%d pos:%f,%f",it->second->id, it->second->loc.x, it->second->loc.y );
        sendCameraCreateLoc(newstream,it->second);
    }
    POOL_SCAN(g_layer_pool,Layer) {
        sendLayerSetup(newstream,it->second);
    }
    for(int i=0;i<FileDepo::MAX_FILES;i++) {
        File *f = g_filedepo->getByIndex(i);
        if(f) {
            int r = sendUS1StrBytes(newstream, PACKETTYPE_S2C_FILE, f->path, f->data, f->data_len );
            assert(r>0);
        }
    }
    POOL_SCAN(g_image_pool,Image) {
        sendImageSetup(newstream,it->second);
    }
    POOL_SCAN(g_texture_pool,Texture) {
        sendTextureCreateWithImage(newstream,it->second);
    }
    POOL_SCAN(g_tiledeck_pool,TileDeck) {
        sendDeckSetup(newstream,it->second);
    }
    POOL_SCAN(g_font_pool,Font) {
        sendFontSetupWithFile(newstream,it->second);
    }
    POOL_SCAN(g_crshader_pool,ColorReplacerShader) {
        sendColorReplacerShaderSetup(newstream,it->second);
    }
    POOL_SCAN(g_sound_pool,Sound) {
        sendSoundSetup(newstream,it->second);
    }
    
    // scansendallprop2dsnapshots ..

    // prop body, grid, prims, childrenを送るが、既存コードがremoteheadに依存しているので再利用できない。
    // scanProp2D, scanTextBox, scanGrid, scanPrimDrawer, をtrackerの外に出して再利用でよいかな。
    POOL_SCAN(g_prop2d_pool,Prop2D) {
        Prop2D *p = it->second;
        PacketProp2DSnapshot out;
        if(p->getParentLayer()) {
            makePacketProp2DSnapshot(&out,p,NULL);
            //            print("sending prop2d_snapshot id:%d", out.prop_id);
            sendUS1Bytes( newstream, PACKETTYPE_S2C_PROP2D_SNAPSHOT, (const char*)&out, sizeof(out));            
        } 

        // grids, mostly copied from TrackerGrid. TODO:refactor...
        static const int N=4096;
        static int32_t index_table[N]; // avoid malloc
        static uint8_t flip_table[N];
        static PacketVec2 texofs_table[N];
        static PacketColor color_table[N];
        for(int i=0;i<p->grid_used_num;i++) {
            Grid *g = p->grids[i];
            assertmsg(g->getCellNum()<=N, "too many cells in a grid? max:%d",N);
            for(int y=0;y<g->height;y++){
                for(int x=0;x<g->width;x++){
                    int ind = g->index(x,y);
                    index_table[ind] = g->get(x,y);
                    uint8_t bits = 0;
                    if( g->getXFlip(x,y) ) bits |= GTT_FLIP_BIT_X;
                    if( g->getYFlip(x,y) ) bits |= GTT_FLIP_BIT_Y;
                    if( g->getUVRot(x,y) ) bits |= GTT_FLIP_BIT_UVROT;
                    flip_table[ind] = bits;
                    Vec2 texofs;
                    g->getTexOffset(x,y,&texofs);
                    texofs_table[ind].x = texofs.x;
                    texofs_table[ind].y = texofs.y;
                    Color col = g->getColor(x,y);
                    copyColorToPacketColor( &color_table[ind], &col );
                }
            }
            sendUS1UI3( newstream, PACKETTYPE_S2C_GRID_CREATE, g->id, g->width, g->height );
            int dk_id = 0;
            if(g->deck) dk_id = g->deck->id; else if(p->deck) dk_id = p->deck->id;
            if(dk_id) sendUS1UI2( newstream, PACKETTYPE_S2C_GRID_DECK, g->id, dk_id );
            sendUS1UI2( newstream, PACKETTYPE_S2C_GRID_PROP2D, g->id, p->id );    
            sendUS1UI1Bytes( newstream, PACKETTYPE_S2C_GRID_TABLE_INDEX_SNAPSHOT, g->id,
                                         (const char*) index_table, g->getCellNum() * sizeof(int32_t) );
            sendUS1UI1Bytes( newstream, PACKETTYPE_S2C_GRID_TABLE_FLIP_SNAPSHOT, g->id,
                                         (const char*) flip_table, g->getCellNum() * sizeof(uint8_t) );
            sendUS1UI1Bytes( newstream, PACKETTYPE_S2C_GRID_TABLE_TEXOFS_SNAPSHOT, g->id,
                                         (const char*) texofs_table, g->getCellNum() * sizeof(Vec2) );
            sendUS1UI1Bytes( newstream, PACKETTYPE_S2C_GRID_TABLE_COLOR_SNAPSHOT, g->id,
                                         (const char*) color_table, g->getCellNum() * sizeof(PacketColor) );
        }

        // prims
        if(p->prim_drawer) {
            static PacketPrim prims[256]; // avoid malloc
            assertmsg( elementof(prims) >= p->prim_drawer->prim_num, "too many prims? max:%d got:%d", elementof(prims), p->prim_drawer->prim_num);
            for(int i=0;i<p->prim_drawer->prim_num;i++){
                copyPrimToPacketPrim( &prims[i], p->prim_drawer->prims[i]);
            }
            sendUS1UI1Bytes( newstream, PACKETTYPE_S2C_PRIM_BULK_SNAPSHOT, p->id,
                             (const char*) prims, p->prim_drawer->prim_num * sizeof(PacketPrim) );
            
        }

        // children
        for(int i=0;i<p->children_num;i++) {
            Prop2D *chp = p->children[i];
            makePacketProp2DSnapshot(&out,chp,p);
            sendUS1Bytes( newstream, PACKETTYPE_S2C_PROP2D_SNAPSHOT, (const char*)&out, sizeof(out));
        }
    }
    POOL_SCAN(g_textbox_pool,TextBox) {
        // copied from trackertextbox::broadcastdiff.. TODO:refactor.
        TextBox *target_tb = it->second;
        sendUS1UI1( newstream, PACKETTYPE_S2C_TEXTBOX_CREATE, target_tb->id );
        sendUS1UI2( newstream, PACKETTYPE_S2C_TEXTBOX_LAYER, target_tb->id, target_tb->getParentLayer()->id );        
        sendUS1UI2( newstream, PACKETTYPE_S2C_TEXTBOX_FONT, target_tb->id, target_tb->font->id );
        sendUS1UI1Wstr( newstream, PACKETTYPE_S2C_TEXTBOX_STRING, target_tb->id, target_tb->str, target_tb->len_str );
        sendUS1UI1F2( newstream, PACKETTYPE_S2C_TEXTBOX_LOC, target_tb->id, target_tb->loc.x, target_tb->loc.y );
        sendUS1UI1F2( newstream, PACKETTYPE_S2C_TEXTBOX_SCL, target_tb->id, target_tb->scl.x, target_tb->scl.y );        
        PacketColor pc;
        copyColorToPacketColor(&pc, &target_tb->color );
        sendUS1UI1Bytes( newstream, PACKETTYPE_S2C_TEXTBOX_COLOR, target_tb->id, (const char*)&pc, sizeof(pc) );            
    }

    print("sending client_login to master server");
    sendUS1UI1(g_stream, PACKETTYPE_R2S_CLIENT_LOGIN, newstream->id );
}

int main( int argc, char **argv ) {

#ifdef __APPLE__    
    setlocale( LC_ALL, "ja_JP");
#endif
#ifdef WIN32    
    setlocale( LC_ALL, "jpn");
#endif    

    bool argret = parseProgramArgs(argc, argv );
    if(!argret) {
        return 1;
    }
    
    Moyai::globalInitNetwork();

    uv_tcp_t *client = (uv_tcp_t*)MALLOC( sizeof(uv_tcp_t) );
    uv_tcp_init( uv_default_loop(), client );
    struct sockaddr_in svaddr;
    if(g_port==HEADLESS_SERVER_PORT && g_enable_reprecation) g_port = REPRECATOR_SERVER_PORT;
    uv_ip4_addr( g_server_ip_addr, g_port, &svaddr );

    uv_connect_t *connect = (uv_connect_t*)MALLOC( sizeof(uv_connect_t));
    
    int r = uv_tcp_connect( connect, client, (struct sockaddr*) &svaddr, on_connect );
    if(r) {
        print("uv_tcp_connect failed");
        return 1;
    }

    if(g_enable_reprecation) {
        g_reproxy = new ReprecationProxy(REPRECATOR_PROXY_PORT);
        g_reproxy->setFuncCallback( reproxy_on_packet_cb );
        g_reproxy->setAcceptCallback( reproxy_on_accept_cb );
        g_reproxy->setCloseCallback( reproxy_on_close_cb );
    }
    
    g_filedepo = new FileDepo();

    print("start viewer loop");



    uint64_t last_total_read=0;
    double last_total_read_at=0;
    double last_print_stats_at=0;
    float kbps = 0;
    
    g_done = false;
    while( !g_done ) {
        static double last_poll_at = now();
        double t = now();
        double dt = t - last_poll_at;

        uv_run_times(10);
        if( g_reproxy ) g_reproxy->heartbeat();
        if( g_moyai_client && g_enable_reprecation==false ) {
            if( glfwWindowShouldClose(g_window) ) {
                g_done = true;
            }
            
            glfwPollEvents();
            if( glfwGetKey( g_window, 'Q') ) break;
            if( glfwGetKey( g_window, 'M' ) ) {
                Vec2 s = g_video_prop->scl;
                s+= Vec2(1,1);
                g_video_prop->setScl(s);
                print("Scl:%f,%f",s.x,s.y);
            }

            int polled = g_moyai_client->poll(dt);
            int rendered = g_moyai_client->render();

            if(t>last_total_read_at+1) {
                kbps = (float)((g_total_read-last_total_read)*8)/1000.0f;
                last_total_read = g_total_read;
                last_total_read_at = t;
            }
            Format fmt( "polled:%d rendered:%d %.1fKbps Ping:%.1fms TS:%d rc:%d B/p:%.1f B/r:%.1f sbused:%d comp:%.3f",
                        polled, rendered, kbps, g_last_ping_rtt*1000,g_timestamp_count, g_total_read_count,
                        (float)g_total_read/(float)g_packet_count, (float)g_total_read/(float)g_total_read_count,
                        g_stream->sendbuf.used, (float)g_total_read/(float)g_total_unzipped_bytes );
            updateDebugStat( fmt.buf );
            if(t>g_last_ping_at+1) {
                g_last_ping_at = t;
                sendPing( g_stream );                
            }
        }

        if( g_enable_print_stats && last_print_stats_at < t-5) {
            last_print_stats_at = t;
            printStats();
        }

        if(g_stream) g_stream->flushSendbuf(1024);
        last_poll_at = t;
    }
    if(g_moyai_client) glfwTerminate();
    if(g_savepath[0]) {
        flushBufferToFile(&g_savebuf, g_savepath);
    }
    return 0;
}
