// HMP (Headless Moyai Protocol viewer client)

#include <locale.h>

#include "client.h"
#include "ConvertUTF.h"

#include "vw.h"

static const int SCRW=966, SCRH=544;

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
uv_stream_t *g_stream;
Buffer g_recvbuf;
GLFWwindow *g_window;

Viewport *g_debug_viewport;
Layer *g_debug_layer;
Font *g_debug_font;
TextBox *g_debug_tb;

SoundSystem *g_soundsystem;

uint64_t g_total_read;

///////////////

// debug funcs
Prop2D *findProp2DByTexture( uint32_t tex_id ) {
    for( std::unordered_map<unsigned int,Prop2D*>::iterator it = g_prop2d_pool.idmap.begin(); it != g_prop2d_pool.idmap.end(); ++it ) {
        Prop2D *p =  it->second;
        if( p->deck && p->deck->tex && p->deck->tex->id == tex_id ) {
            return p;
        }
    }
    return NULL;
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
#if 0
#endif


void setupDebugStat() {
    int retina = 1;
#if defined(__APPLE__)
    retina = 2;
#endif    
    g_debug_viewport = new Viewport();
    g_debug_viewport->setSize(SCRW*retina,SCRH*retina);
    g_debug_viewport->setScale2D(SCRW,SCRH);
    g_debug_layer = new Layer();
    g_debug_layer->setViewport(g_debug_viewport);
    g_moyai_client->insertLayer(g_debug_layer);
    g_debug_font = new Font();
    wchar_t charcodes[] = L" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    g_debug_font->loadFromTTF("./assets/cinecaption227.ttf", charcodes, 12 );
    g_debug_tb = new TextBox();
    g_debug_tb->setFont(g_debug_font);
    g_debug_tb->setScl(1);
    g_debug_tb->setLoc(-SCRW/2+10,SCRH/2-15);
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
    if(g_stream) sendUS1UI5( g_stream, PACKETTYPE_C2S_KEYBOARD, keycode, action, mod_shift, mod_ctrl, mod_alt );
}
void mouseButtonCallback( GLFWwindow *window, int button, int action, int mods ) {
    int mod_shift = mods & GLFW_MOD_SHIFT;
    int mod_ctrl = mods & GLFW_MOD_CONTROL;
    int mod_alt = mods & GLFW_MOD_ALT;
    if(g_stream) sendUS1UI5( g_stream, PACKETTYPE_C2S_MOUSE_BUTTON, button, action, mod_shift, mod_ctrl, mod_alt );
}
void cursorPosCallback( GLFWwindow *window, double x, double y ) {
    if(g_stream) sendUS1F2( g_stream, PACKETTYPE_C2S_CURSOR_POS, x, y );
}

void on_packet_callback( uv_stream_t *s, uint16_t funcid, char *argdata, uint32_t argdatalen ) {
    switch(funcid) {
    case PACKETTYPE_S2C_PROP2D_SNAPSHOT:
        {
            uint32_t pktsize = get_u32(argdata+0);
            //        print("PACKETTYPE_S2C_PROP2D_SNAPSHOT len:%d", argdatalen );            
            PacketProp2DSnapshot pkt;
            assertmsg( pktsize == sizeof(pkt), "invalid argdatalen:%d pktsize:%d",argdatalen, pktsize );
            memcpy(&pkt,argdata+4,sizeof(pkt));
            //            prt("s%d ", pkt.prop_id );


            if( pkt.debug) print("packettype_prop2d_create! id:%d layer_id:%d loc:%f,%f scl:%f,%f index:%d tdid:%d", pkt.prop_id, pkt.layer_id, pkt.loc.x, pkt.loc.y, pkt.scl.x, pkt.scl.y, pkt.index, pkt.tiledeck_id );

            Layer *layer = NULL;
            Prop2D *parent_prop = NULL;
            
            if( pkt.layer_id > 0 ) {
                layer = g_layer_pool.get( pkt.layer_id );
            } else if( pkt.parent_prop_id > 0 ) {
                //                print("Child prop.id:%d par:%d", pkt.prop_id, pkt.parent_prop_id );                
                parent_prop = g_prop2d_pool.get(pkt.parent_prop_id);
            }
            if( !(layer || parent_prop ) ) {
                // no parent!
                print("prop %d no parent? layerid:%d parentpropid:%d", pkt.prop_id, pkt.layer_id, pkt.parent_prop_id );
                break;
            }

            TileDeck *dk = g_tiledeck_pool.get( pkt.tiledeck_id ); // deck can be null (may have grid,textbox)
            Prop2D *prop = g_prop2d_pool.get(pkt.prop_id);
            if(!prop) {
                prop = g_prop2d_pool.ensure(pkt.prop_id);
                if(layer) {
                    //                    print("  inserting prop %d to layer %d", pkt.prop_id, pkt.layer_id );
                    layer->insertProp(prop);
                } else if(parent_prop) {
                    Prop2D *found_prop = prop->getChild( pkt.prop_id );
                    if(!found_prop) {
                        //                        print("  adding child prop %d to a prop %d", pkt.prop_id, pkt.parent_prop_id );
                        parent_prop->addChild(prop);
                    }
                }
            }
            if(dk) prop->setDeck(dk);
            prop->setIndex(pkt.index);
            prop->setScl( Vec2(pkt.scl.x,pkt.scl.y) );
            prop->setLoc( Vec2(pkt.loc.x, pkt.loc.y) );
            prop->setRot( pkt.rot );
            prop->setXFlip( pkt.xflip );
            prop->setYFlip( pkt.yflip );
            prop->setUVRot( pkt.uvrot );
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
        }

        break;
    case PACKETTYPE_S2C_LAYER_CREATE:
        {
            uint32_t id = get_u32( argdata+0 );
            print("PACKETTYPE_S2C_LAYER_CREATE layer_id:%d", id );

            Layer *l = g_layer_pool.get(id);
            if(!l) {
                l = g_layer_pool.ensure(id);
                g_moyai_client->insertLayer(l);
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

    case PACKETTYPE_S2C_VIEWPORT_SIZE:
        {
            unsigned int viewport_id = get_u32(argdata);
            unsigned int w = get_u32(argdata+4);
            unsigned int h = get_u32(argdata+8);
            
            print("received viewport_size id:%d w:%d h:%d", viewport_id, w,h );
            Viewport *vp = g_viewport_pool.ensure(viewport_id);
            assert(vp);
            vp->setSize(w,h);
        }
        break;
    case PACKETTYPE_S2C_VIEWPORT_SCALE:
        {
            unsigned int viewport_id = get_u32(argdata);
            float sclx = get_f32(argdata+4);
            float scly = get_f32(argdata+8);
            print("received viewport_scale id:%d x:%f y:%f", viewport_id, sclx, scly );            
                        
            Viewport *vp = g_viewport_pool.ensure(viewport_id);
            assert(vp);
            vp->setScale2D(sclx,scly);
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
            //            print("received camera_loc. id:%d (%f,%f)", camera_id, x,y );            
            Camera *cam = g_camera_pool.get(camera_id);
            assert(cam);
            cam->setLoc(x,y);
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
            //            print("received texture_image. tex:%d img:%d", tex_id, img_id );
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
            tex->setImage(img);
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
            img->setOptionalLoadPath( cstrpath );
        }
        break;
    case PACKETTYPE_S2C_TILEDECK_CREATE:
        {
            unsigned int dk_id = get_u32(argdata);
            //            print("received tiledeck_create. id:%d", dk_id);
            TileDeck *dk = g_tiledeck_pool.ensure(dk_id);
            assert(dk);
        }
        break;        
    case PACKETTYPE_S2C_TILEDECK_TEXTURE:
        {
            unsigned int dk_id = get_u32(argdata);
            unsigned int tex_id = get_u32(argdata+4);
            //            print("received tiledeck_texture. dk:%d tex:%d", dk_id, tex_id );
            
            TileDeck *dk = g_tiledeck_pool.get(dk_id);
            assert(dk);
            Texture *tex = g_texture_pool.get(tex_id);
            assert(tex);
            dk->setTexture(tex);
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
                prt("D[%d]", prop_id);
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
                    outcol.r = cols[i].r;
                    outcol.g = cols[i].g;
                    outcol.b = cols[i].b;
                    outcol.a = cols[i].a;
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
            //                        static int hoge = 0;
            //                        hoge++;
            //                        if(hoge>8) exit(0);


            
            uint32_t tb_id = get_u32(argdata);
            //            TextBox *hogetb = g_textbox_pool.get(tb_id);
            //            if(hogetb) {
            //                print("ALREADY tb.%d par:%p", tb_id, hogetb->getParentLayer() );
            //                break;
            //            }
            g_textbox_pool.ensure(tb_id);
            //            print("tb_creat id:%d ptr:%p par:%p", tb_id, tb, tb->getParentLayer() );
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
            g_font_pool.ensure(font_id);            
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
                print("  crs new..");
                s = g_crshader_pool.ensure(pkt.shader_id);
            }
            if( !s->init() ) {
                assertmsg(false, "shader %d init failed, fatal", pkt.shader_id);
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
            snd = g_soundsystem->newSoundFromMemory( samples, samples_num );
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
                snd->play(vol);
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
                if(snd->isPlaying() == false ) snd->play(last_play_vol);
                snd->setTimePositionSec( pos_sec );
            } else {
                print("sound_position: %d not found", snd_id );
            }
        }
        break;
    default:
        print("unhandled packet type:%d", funcid );
        break;
    }
}

void on_data( uv_stream_t *s, ssize_t nread, const uv_buf_t *buf) {
    g_total_read += nread;
    parseRecord( s, &g_recvbuf, buf->base, nread, on_packet_callback );
    
}
void on_connect( uv_connect_t *connect, int status ) {
    print("on_connect status:%d",status);
    
    int r = uv_read_start( (uv_stream_t*)connect->handle, moyai_libuv_alloc_buffer, on_data );
    if(r) {
        print("uv_read_start: fail:%d",r);
    }
    g_stream = connect->handle;
    g_recvbuf.ensureMemory(1024*1024*16);
}

int main( int argc, char **argv ) {

#ifdef __APPLE__    
    setlocale( LC_ALL, "ja_JP");
#endif
#ifdef WIN32    
    setlocale( LC_ALL, "jpn");
#endif    
    
    const char *host = "127.0.0.1";
    if( argc > 1 && argv[1] ) host = argv[1];
    int port = 22222;
    if( argc > 2 && argv[2] ) port = atoi(argv[2]);
    print("viewer config: host:'%s' port:%d", host, port );
    
    Moyai::globalInitNetwork();

    uv_tcp_t *client = (uv_tcp_t*)MALLOC( sizeof(uv_tcp_t) );
    uv_tcp_init( uv_default_loop(), client );
    struct sockaddr_in svaddr;
    uv_ip4_addr( host, port, &svaddr );

    uv_connect_t *connect = (uv_connect_t*)MALLOC( sizeof(uv_connect_t));
    
    int r = uv_tcp_connect( connect, client, (struct sockaddr*) &svaddr, on_connect );
    if(r) {
        print("uv_tcp_connect failed");
        return 1;
    }

    g_soundsystem = new SoundSystem();
    
    //

    if( !glfwInit() ) {
        print("can't init glfw");
        exit(1);        
    }

    glfwSetErrorCallback( glfw_error_cb );
    g_window =  glfwCreateWindow( SCRW, SCRH, "headless moyai viewer", NULL, NULL );
    if(g_window == NULL ) {
        print("can't open glfw window");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(g_window);    
    glfwSetWindowCloseCallback( g_window, winclose_callback );
    glfwSetInputMode( g_window, GLFW_STICKY_KEYS, GL_TRUE );
    glfwSwapInterval(1); // vsync
#ifdef WIN32
	glewInit();
#endif
    glClearColor(0.2,0.2,0.2,1);

    glfwSetKeyCallback( g_window, keyboardCallback );
    glfwSetMouseButtonCallback( g_window, mouseButtonCallback );
    glfwSetCursorPosCallback( g_window, cursorPosCallback );
    
    g_moyai_client = new MoyaiClient( g_window );

    g_filedepo = new FileDepo();

    print("start viewer loop");

    // Client side debug status
    setupDebugStat();

    uint64_t last_total_read;
    double last_total_read_at;
    
    while( !glfwWindowShouldClose(g_window) ){
        static double last_poll_at = now();

        double t = now();
        double dt = t - last_poll_at;

        glfwPollEvents();
        uv_run_times(10);
        int polled = g_moyai_client->poll(dt);
        int rendered = g_moyai_client->render();

        if(t>last_total_read_at+1) {
            float kbps = (float)((g_total_read-last_total_read)*8)/1000.0f;
            Format fmt( "polled:%d rendered:%d %.1fKbps", polled, rendered, kbps);
            last_total_read = g_total_read;
            last_total_read_at = t;
            updateDebugStat( fmt.buf );
        }

        if( glfwGetKey( g_window, 'Q') ) break;
        
        last_poll_at = t;
    }

    glfwTerminate();    
    return 0;
}
