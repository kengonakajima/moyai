#ifndef _MOYAI_H_
#define _MOYAI_H_

#include <stdlib.h>
#include <strings.h>

#include <GLUT/glut.h>

#include "soil/src/SOIL.h"

#include "cumino.h"
#include "freetype-gl/freetype-gl.h"
#include "freetype-gl/vertex-buffer.h"

#ifdef USE_FMOD
#include "fmod/api/inc/fmod.h"
#include "fmod/api/inc/fmod_errors.h"
#endif


class Vec2 {
public:
    float x,y;
    inline Vec2(float xx, float yy) : x(xx),y(yy){}
    inline Vec2() : x(0), y(0) {}
    inline Vec2 normalize(float l){
        float xx = x, yy = y;
        ::normalize( &xx, &yy, l );
        return Vec2(xx,yy);
    }
    inline float len(){
        return ::len( x,y,0,0 );
    }
    inline float len(Vec2 tgt){
        return ::len( x,y,tgt.x,tgt.y);
    }
    inline DIR toDir() {
        const float pi4 = M_PI / 4.0f;
        float at = atan2( x,y );
        if( at >= -pi4  && at <= pi4 ){
            return DIR_UP;
        } else if( at >= pi4 && at <= pi4*3){
            return DIR_RIGHT;
        } else if( at >= pi4*3 || at <= -pi4*3 ){
            return DIR_DOWN;
        } else if( at <= -pi4 && at >= -pi4*3 ){
            return DIR_LEFT;
        } else {
            return DIR_NONE;
        }
    }
    inline Vec2 mul(float val){ return Vec2( x*val, y*val); }
    inline Vec2 add( Vec2 v){ return Vec2( x+v.x, y+v.y);}
    inline Vec2 add( float xx, float yy ){ return Vec2( x+xx, y+yy);}
    inline Vec2 to( Vec2 v){ return Vec2( v.x - x, v.y - y); }
    inline Vec2 randomize(float r){ return Vec2( x + range(-r,r), y + range(-r,r) ); }
    static inline Vec2 angle(float rad){ return Vec2( cos(rad), sin(rad) ); }
    inline void toSign(int*xs,int*ys){ *xs = sign(x); *ys = sign(y); }
    inline Vec2 operator+(Vec2 arg){ return Vec2(x+arg.x,y+arg.y); }
    inline Vec2 operator-(Vec2 arg){ return Vec2(x-arg.x,y-arg.y); }
    inline Vec2 operator*(float f){ return Vec2(x*f,y*f); }
    inline Vec2 operator/(float f){ return Vec2(x/f,y/f); }    
    inline Vec2 operator*=(float f){ x *= f; y *= f; return Vec2(x,y); }
    inline Vec2 operator/=(float f){ x /= f; y /= f; return Vec2(x,y); }                
    inline Vec2 operator+=(Vec2 arg){ x += arg.x; y += arg.y; return Vec2(x,y); }
    inline Vec2 operator-=(Vec2 arg){ x -= arg.x; y -= arg.y; return Vec2(x,y); }
    inline bool operator==(Vec2 arg){ return (x==arg.x && y==arg.y); }
    inline bool operator!=(Vec2 arg){ return (x!=arg.x || y!=arg.y); }
    inline bool operator>=(Vec2 arg){ return (x>=arg.x && y>=arg.y); }
    inline bool operator>(Vec2 arg){ return (x>arg.x && y>arg.y); }    
    inline bool operator<=(Vec2 arg){ return (x<=arg.x && y<=arg.y); }
    inline bool operator<(Vec2 arg){ return (x<arg.x && y<arg.y); }        
    inline bool isZero(){ return (x==0 && y==0 ); }
    inline Vec2 friction( float diff ){
        float l = len();
        l -= diff;
        Vec2 out = Vec2(x,y).normalize(l);
        if( l < 0 ){
            return Vec2(0,0);
        } else {
            return out;
        }
    }
    static inline Vec2 random(float v) { return Vec2(0,0).randomize(v); }
    static inline Vec2 random() { return random(1); }
    static inline Vec2 fromDir(DIR d){
        int dx,dy;
        dirToDXDY(d,&dx,&dy);
        return Vec2(dx,dy);
    }
    // clockwise, up=y+
    inline Vec2 rot(float v) {
        return Vec2( x * cos(v) - y * sin(v),
                     x * sin(v) + y * cos(v) );
    }
};
class Vec3 {
    float x,y,z;
    inline Vec3(float xx, float yy, float zz) : x(xx),y(yy),z(zz){}
    inline Vec3() : x(0), y(0), z(0) {   }
};


class Viewport {
public:
    int screen_width, screen_height;
    Vec2 scl;
    Viewport() : screen_width(0), screen_height(0), scl(0,0) {}
    void setSize(int scrw, int scrh );
    void setScale( float sx, float sy );
    void takeEffect();
    void getMinMax( Vec2 *minv, Vec2 *maxv );
};

class Color {
public:
    float r,g,b,a;
    inline Color( unsigned int v ) {
        r = (float)((v & 0xff0000)>>16)/255;
        g = (float)((v & 0xff00)>>8)/255;
        b = (float)(v & 0xff)/255;
        a = 1.0;        
    }
    inline Color(float _r, float _g, float _b, float _a ) : r(_r), g(_g), b(_b), a(_a) {}
    inline Color(){ r=g=b=a=0; }
    inline Color add(Color v){
        return Color( r * (1-v.a) + v.r * v.a,
                      g * (1-v.a) + v.g * v.a,
                      b * (1-v.a) + v.b * v.a,
                      a
                      );
    }
    inline Color adjust(float v){
        float rr=r*v,gg=g*v,bb=b*v;
        if(rr>1)rr=1;
        if(gg>1)gg=1;
        if(bb>1)bb=1;
        return Color(rr,gg,bb,a);
    }
    
};

class Image {
public:
    unsigned char *buffer;
    int width, height;
    Image() : buffer(NULL), width(0), height(0) {}
    ~Image() { if(buffer)free(buffer); }
    void setSize(int w, int h ) { width = w; height = h; }
    void setPixel( int x, int y, Color c );
    Color getPixel( int x, int y );
    bool writePNG(const char *path);    
};

class Texture {
public:
    GLuint tex;
    Image *image;
    Texture() : tex(0), image(NULL) {
    }

    void setImage( Image *img );
    bool load( const char *path );
    void setLinearFilter();
    void getSize( int *w, int *h){
        assertmsg(tex!=0,"getSize: not init?");
        glBindTexture( GL_TEXTURE_2D, tex );
        GLint  ww,hh;
        glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ww );
        glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &hh );
        *w = ww;
        *h = hh;
    }
    
};

class TileDeck {
 public:
    int cell_width, cell_height;
    int tile_width, tile_height;
    int image_width, image_height;

    // 得られる値のキャッシュ


    
    Texture *tex;
    TileDeck(){
        tex = NULL;
    }
    void setTexture( Texture *t ){
        assert(t->tex!=0);
        tex = t;
    }

    // tw,th : in tiles
    // cw,ch : cell size
    inline void setSize( int tw, int th, int cw, int ch, int iw, int ih  ){
        tile_width = tw;
        tile_height = th;
        cell_width = cw;
        cell_height = ch;
        image_width = iw;
        image_height = ih;
    }
};

class FragmentShader {
public:
    GLuint shader;
    GLuint program;
    FragmentShader() : shader(0), program(0) {};
    bool load( const char *src);
    virtual void updateUniforms(){};
};
class ColorReplacerShader : public FragmentShader {
public:
    float epsilon;
    Color from_color;
    Color to_color;
    ColorReplacerShader() : epsilon(0), from_color(0,0,0,0), to_color(0,0,0,0){};
    bool init();
    void setColor( Color from, Color to, float eps ) {
        epsilon = eps;
        to_color = to;
        from_color = from;
    }
    virtual void updateUniforms();
};
    

class Grid {
 public:
    int width, height;
    int *index_table;
    bool *xflip_table;
    bool *yflip_table;
    Vec2 *texofs_table;
    bool *rot_table;
    Color *color_table;
    TileDeck *deck;
    FragmentShader *fragment_shader;
    Color color;
    bool visible;
    
    static const int GRID_FLAG_XFLIP = 1;
    static const int GRID_FLAG_YFLIP = 2;
    static const int GRID_NOT_USED = -1;
    Grid(int w, int h ) : color(1,1,1,1) {
        width = w;
        height = h;
        // 各テーブルは、実際に使う時にmallocする。
        index_table = NULL;
        xflip_table = NULL;
        yflip_table = NULL;
        texofs_table = NULL;
        rot_table = NULL;
        color_table = NULL;

        fragment_shader = NULL;
        visible = true;
        
        // Propのdeckを上書きする。
        deck = NULL;
    }
    ~Grid(){
        if(index_table) free(index_table);
        if(xflip_table) free(xflip_table);
        if(yflip_table) free(yflip_table);
    }
    void setDeck( TileDeck *d ){
        deck = d;
    }
    inline int index(int x, int y){
        assertmsg(x>=0 && x<width,"invalid x:%d",x);
        assertmsg(y>=0 && y<height,"invalid y:%d",y);
        return ( x + y * width );
    }
#define ENSURE_GRID_TABLE( membname, T, inival)  if( !membname ){ membname = (T*) malloc(width*height*sizeof(T)); int i=0; for(int y=0;y<height;y++){ for(int x=0;x<width;x++){ membname[i++] = inival; }}}
    inline void set(int x, int y, int ind ){
        ENSURE_GRID_TABLE( index_table, int, GRID_NOT_USED );
        index_table[ index(x,y) ] = ind;
    }
    inline int get(int x, int y){
        if(!index_table){
            return GRID_NOT_USED;
        }
        return index_table[ index(x,y) ];
    }
    inline void setXFlip( int x, int y, bool flag ){
        ENSURE_GRID_TABLE( xflip_table, bool, false );
        xflip_table[ index(x,y) ] = flag;
    }
    inline void setYFlip( int x, int y, bool flag ){
        ENSURE_GRID_TABLE( yflip_table, bool, false );
        yflip_table[ index(x,y) ] = flag;
    }
    // 0~1. 1でちょうど1セル分ずれる.tex全体ではない。
    inline void setTexOffset( int x, int y, Vec2 *v ) {
        ENSURE_GRID_TABLE( texofs_table, Vec2, Vec2(0,0) );
        int i = index(x,y);
        texofs_table[i].x = v->x;
        texofs_table[i].y = v->y;        
    }
    inline void setUVRot( int x, int y, bool flag ){
        ENSURE_GRID_TABLE( rot_table, bool, false );
        rot_table[ index(x,y) ] = flag;
    }
    inline void setColor( int x, int y, Color col ) {
        ENSURE_GRID_TABLE( color_table, Color, Color(1,1,1,1) );
        color_table[ index(x,y) ] = col;
    }
    inline void setFragmentShader( FragmentShader *fs ){
        fragment_shader = fs;
    }
    inline void setColor( Color c){ color = c; }
    inline void setVisible( bool flg ){ visible = flg; }
    void clear();
};

class CharGrid : public Grid {
public:
    int ascii_offset;
    CharGrid(int w, int h ) : Grid(w,h), ascii_offset(0) {}
    void printf( int x, int y, Color c, const char *fmt, ...);
    void setAsciiOffset( int ofs ){ ascii_offset = ofs; }
};



class AnimCurve {
public:
    int index_table[32];
    int index_used;
    float step_time;
    bool loop;
    AnimCurve( float st, bool do_loop, int *inds, int indslen ){
        assertmsg( indslen <= elementof(index_table), "animcurve length %d is onger than %d : ", indslen, elementof(index_table) );
        for(int i=0;i<indslen;i++){
            index_table[i] = inds[i];
        }
        index_used = indslen;
        step_time = st;
        loop = do_loop;
    }

    // t: アニメーション開始からの時間
    inline int getIndex( double t ){
        if(t<0) t=0; 
        int ind = (int)(t / step_time);
        assert(ind>=0);
        if(loop){
            return index_table[ ind % index_used ];
        } else {
            if( ind >= index_used ){
                return index_table[ index_used - 1];
            } else {
                return index_table[ ind ];
            }
        }
    }
};

typedef enum{
    PRIMTYPE_NONE = 0,
    PRIMTYPE_LINE = 1,
    PRIMTYPE_RECTANGLE = 2,
} PRIMTYPE;

class Prim {
public:
    PRIMTYPE type;
    Vec2 a,b;
    Color color;
    inline Prim( PRIMTYPE t, Vec2 a, Vec2 b, Color c ) : type(t), a(a),b(b), color(c) {
    }
    inline void draw(Vec2 ofs){
        glDisable( GL_TEXTURE_2D );
        switch(type){
        case PRIMTYPE_LINE:
            glBegin( GL_LINES );
            glColor4f( color.r, color.g, color.b, color.a );
            glVertex2f( ofs.x + a.x, ofs.y + a.y );
            glVertex2f( ofs.x + b.x, ofs.y + b.y );
            glEnd();
            break;
        case PRIMTYPE_RECTANGLE:
            glBegin( GL_QUADS );
            glColor4f( color.r, color.g, color.b, color.a );
            glVertex2f( ofs.x + a.x, ofs.y + a.y ); // top left
            glVertex2f( ofs.x + b.x, ofs.y + a.y ); // top right
            glVertex2f( ofs.x + b.x, ofs.y + b.y ); // bottom right
            glVertex2f( ofs.x + a.x, ofs.y + b.y ); // bottom left
            glEnd();
            break;
        default:
            break;
        }
    }
};

class PrimDrawer {
public:
    Prim **prims;
    int prim_max;
    int prim_num;
    PrimDrawer() : prims(NULL), prim_max(0), prim_num(0) {}
    ~PrimDrawer(){
        clear();

    }
    inline void ensurePrims(){
        if(!prims){
            prim_max = 64;
            if(!prims) prims = (Prim**) malloc( sizeof(Prim*) * prim_max );
            assert(prims);
            prim_num = 0;
        }
    }
    inline void addLine(Vec2 from, Vec2 to, Color c ){
        ensurePrims();
        assertmsg( prim_num <= prim_max, "too many prims" );
        prims[prim_num++] = new Prim( PRIMTYPE_LINE, from, to, c );
    }
    inline void addRect(Vec2 from, Vec2 to, Color c ){
        ensurePrims();
        assertmsg( prim_num <= prim_max, "too many prims" );        
        prims[prim_num++] = new Prim( PRIMTYPE_RECTANGLE, from, to, c );
    }
    
    inline void drawAll(Vec2 ofs ){
        for(int i=0;i<prim_num;i++){
            prims[i]->draw(ofs);
        }
    }
    inline void clear(){
        if(prims){
            for(int i=0;i<prim_num;i++){
                delete prims[i];
            }
            delete prims;
        }
        prims = NULL;
        prim_num = 0;
        prim_max = 0;
    }
    void getMinMax( Vec2 *minv, Vec2 *maxv );
};

class Layer;
class Camera;

class Prop {
 public:
    int id;
    Prop *next;
    Prop *prev;

    
    Vec2 loc;
    Vec2 draw_offset;
    
    static const int MAX_GRID = 8;
    Grid *grids[MAX_GRID];  // 頭から入れていってnullだったら終了
    int grid_used_num;
    
    static const int MAX_CHILDREN = 8;
    Prop *children[MAX_CHILDREN];
    int children_num;
    
    TileDeck *deck;
    int index;
    Color color;
    Vec2 scl;
    Layer *parentLayer;
    bool to_clean;
    AnimCurve *anim_curve;
    double anim_start_at; // from accum_time
    double accum_time;
    bool xflip, yflip, uvrot;
    unsigned int poll_count;
    FragmentShader *fragment_shader;
    bool visible;
    // scale anim
    double seek_scl_time; // 0:not seeking
    double seek_scl_started_at; 
    Vec2 seek_scl_target;
    Vec2 seek_scl_orig;

    float rot;
    // rot anim
    double seek_rot_time;
    double seek_rot_started_at;
    float seek_rot_target;
    float seek_rot_orig;

    // color anim
    double seek_color_time;
    double seek_color_started_at;
    Color seek_color_target;
    Color seek_color_orig;
    
    
    PrimDrawer *prim_drawer;

    // prop-size cache for fast culling
    Vec2 max_rt_cache, min_lb_cache;
    
    static int idgen;
    inline Prop(){
        id = idgen++;
        next = prev = NULL;
        
        //        print("newprop: id:%d", id );
        deck = NULL;
        color = Color(1,1,1,1);
        parentLayer = NULL;
        to_clean = false;
        loc.x = loc.y = 0;
        draw_offset.x = draw_offset.y = 0;
        scl.x = scl.y = 32;
        anim_curve = NULL;
        accum_time = 0;
        anim_start_at = 0;
        grid_used_num = 0;
        children_num = 0;
        xflip = yflip = uvrot = false;
        poll_count = 0;
        visible = true;
        rot = 0;
        seek_scl_time = seek_scl_started_at = 0;
        seek_rot_time = seek_rot_started_at = 0;
        seek_color_time = seek_color_started_at = 0;

        fragment_shader = NULL;
        prim_drawer = NULL;
        max_rt_cache = Vec2(0,0);
        min_lb_cache = Vec2(0,0);
    }
    ~Prop(){
        for(int i=0;i<grid_used_num;i++){
            if(grids[i]) delete grids[i];
        }
        for(int i=0;i<children_num;i++){
            if(children[i]) delete children[i];
        }
        if(prim_drawer) delete prim_drawer;
    }
    inline void setDeck( TileDeck *d ){
        deck = d;
        assert( d->cell_width > 0 );
        assert( d->cell_height > 0 );        
    }
    inline void setTexture( Texture *t ){
        assert(t->tex!=0);        
        TileDeck *d = new TileDeck(); // TODO: d leaks
        d->setTexture(t);
        int w,h;
        t->getSize(&w,&h);
        d->setSize( 1,1, w, h, w,h );
        deck = d;
        
    }
    inline void setIndex( int ind){
        index = ind;
    }
    inline void setScl(Vec2 s){
        scl = s;
    }
    inline void setScl(float x, float y ){
        scl.x = x;
        scl.y = y;
    }
    inline void seekScl(float x, float y, double time_sec ){
        seek_scl_orig = scl;
        seek_scl_started_at = accum_time;        
        seek_scl_time = time_sec;
        seek_scl_target = Vec2(x,y);
    }
    inline bool isSeekingScl(){ return seek_scl_time != 0 && ( seek_scl_time + seek_scl_started_at  > accum_time ); }
    inline void setLoc( Vec2 p){
        loc = p;
    }
    inline void setLoc( float x, float y ){
        loc.x = x;
        loc.y = y;
    }
    inline void setRot( float r ){ rot = r; }
    inline void seekRot( float r, double time_sec ){
        seek_rot_orig = rot;
        seek_rot_started_at = accum_time;
        seek_rot_time = time_sec;
        seek_rot_target = r;
    }
    inline bool isSeekingRot(){ return seek_rot_time != 0 && (seek_rot_time + seek_rot_started_at > accum_time); }
    
    inline bool addGrid( Grid *g ){
        assert(g);
        if( grid_used_num >= elementof(grids) ){
            print("WARNING: too many grid in a prop");
            return false;
        }
        grids[grid_used_num++] = g;
        updateMinMaxSizeCache();
        return true;
    }
    inline bool addChild( Prop *p ){
        assert(p);
        if( children_num >= elementof(children) ) {
            assertmsg(false,"WARNING: too many children in a prop");
            return false;
        }
        children[children_num++] = p;
        updateMinMaxSizeCache();
        return true;
    }
    inline void clearChildren() {
        children_num=0;
    }    
    inline void setColor( Color c ){
        color = c;
    }
    inline void setColor(float r, float g, float b, float a ){
        color = Color(r,g,b,a);
    }
    inline void seekColor( Color c, double time_sec ) {
        seek_color_orig = color;
        seek_color_started_at = accum_time;
        seek_color_time = time_sec;
        seek_color_target = c;        
    }
    
    inline void setAnim(AnimCurve *ac ){
        assert(ac);
        anim_curve = ac;
        anim_start_at = accum_time;
    }

    inline void setUVRot( bool flg){ uvrot = flg; }
    inline void setXFlip( bool flg){ xflip = flg; }
    inline void setYFlip( bool flg){ yflip = flg; }

    void drawIndex( TileDeck *dk, int ind, float minx, float miny, float maxx, float maxy, bool hrev, bool vrev, float uofs, float vofs, bool uvrot, float radrot );
    
    virtual void render(Camera *cam);
    bool basePoll(double dt);
    virtual bool propPoll(double dt){
        return true;
    }

    virtual void onIndexChanged(int previndex ){}

    inline void setFragmentShader( FragmentShader *fs ){
        assert(fs);
        fragment_shader = fs;
    }
    Prop *getNearestProp();
    inline void setVisible(bool flg){ visible = flg; }

    inline void ensurePrimDrawer(){
        if(!prim_drawer ) prim_drawer = new PrimDrawer();
    }
    inline void addLine(Vec2 from, Vec2 to, Color c ){
        ensurePrimDrawer();
        prim_drawer->addLine( from, to, c );
        updateMinMaxSizeCache();
    }
    inline void addRect( Vec2 from, Vec2 to, Color c ){
        ensurePrimDrawer();
        prim_drawer->addRect( from, to, c );
        updateMinMaxSizeCache();
    }
    inline void clearPrims(){
        if(prim_drawer)prim_drawer->clear();
    }
    inline bool isCenterInside(Vec2 minloc, Vec2 maxloc){
        return ( loc.x >= minloc.x && loc.x <= maxloc.x && loc.y >= minloc.y && loc.y <= maxloc.y );
    }
    void updateMinMaxSizeCache();

    inline bool hit( Vec2 at ){
        return ( at.x >= loc.x - scl.x/2 ) && ( at.x <= loc.x + scl.x/2 ) && ( at.y >= loc.y - scl.y/2 ) && ( at.y <= loc.y + scl.y/2 );
    }
    virtual void onDelete(){}
};

class Camera {
public:
    Vec2 loc;
    Camera(){}
    inline void setLoc(float x,float y){
        loc.x = x;
        loc.y = y;
    }
    Vec2 screenToWorld( int scr_x, int scr_y, int scr_w, int scr_h );

};

class Layer {
 public:
    Camera *camera;
    Prop *prop_top;
    Viewport *viewport;
    
    int id;
    
    static int idgen;
    
    Layer() : camera(NULL), prop_top(NULL), viewport(NULL) {
        id = idgen++;
    }
    inline void setViewport( Viewport *vp ){
        viewport = vp;
    }
    inline void setCamera(Camera *cam){
        camera = cam;
    }

    inline void insertProp(Prop*p){
        //        assert(p->deck);
        assertmsg( !p->parentLayer, "inserting prop twice");
        if(prop_top){
            p->next = prop_top;
            prop_top->prev = p;
        }
        prop_top = p;
        p->parentLayer = this;
        p->prev = NULL;
    }

    int renderAllProps();
    int pollAllProps(double dt);
    void selectCenterInside( Vec2 minloc, Vec2 maxloc, Prop*out[], int *outlen );
    inline void selectCenterInside( Vec2 center, float dia, Prop *out[], int *outlen){
        selectCenterInside( center - Vec2(dia,dia),
                            center + Vec2(dia,dia),
                            out, outlen );
    }
    void dumpProps();
};

class Font {
public:
    texture_font_t *font;
    texture_atlas_t *atlas;
    int pixel_size;
    Font( ){
        font = NULL;
        atlas = texture_atlas_new( 1024, 1024, 1 );
    }
    bool loadFromTTF(const char *path, wchar_t *codes, int pixelsz ){
        pixel_size = pixelsz;
        font = texture_font_new( atlas, path, pixelsz );
        if(!font){
            return false;
        }
        
        texture_font_load_glyphs( font, codes);

        
        return true;
        
    }
};
class TextBox : public Prop {
public:
    vertex_buffer_t *vb;
    wchar_t *str;
    Font *font;
    Color color;
    
    TextBox() : color(0.5,0.5,0.5,1) {
        vb = vertex_buffer_new( "v3f:t2f:c4f" );
        str = NULL;
    }
    inline void setFont( Font *f ){
        assert(f);
        font = f;
    }

    void render(Camera *cam );

    inline void setString( const char *s ){
        setString( (char*) s );
    }
    inline void setString( char *s ){
        int l = strlen(s);
        wchar_t *out = (wchar_t*)malloc((l+1)*sizeof(wchar_t));
        mbstowcs(out, s, l+1 );
        setString(out);
        free(out);
    }
        
    inline void setString( const wchar_t *s ){
        setString( (wchar_t*)s );
    }
    inline void setString( wchar_t *s ){
        size_t l = wcslen(s);
        if(str){
            free(str);
        }
        str = (wchar_t*)malloc( (l+1) * sizeof(wchar_t) );
        wcscpy( str, s );
    }
};



class Moyai {
public:
    static const int MAXLAYERS = 16;
    Layer *layers[MAXLAYERS];
    
    inline int findFreeLayerIndex(){
        for(int i=0;i<MAXLAYERS;i++){
            if( layers[i] == NULL ){
                return i;
            }
        }
        return -1;
    }
    
    Moyai(){
        for(int i=0;i<MAXLAYERS;i++){
            layers[i] = NULL;
        }
    }
    
    void insertLayer( Layer *l ) {
        int freei = findFreeLayerIndex(); // 後から追加したレイヤの描画順が後ろ
        assert(freei>=0);
        layers[freei] = l;
    }

    int renderAll();
    int pollAll(double dt );


    static inline void screenToGL( int scr_x, int scr_y, int scrw, int scrh, Vec2 *out ){
        out->x = scr_x - scrw/2;
        out->y = scr_y - scrh/2;
        out->y *= -1;
    }
    
    
};

class SoundSystem;

class Sound {
public:
    FMOD_SOUND *sound;
    SoundSystem *parent;
    FMOD_CHANNEL *ch;
    
    float default_volume;
    Sound( SoundSystem *s);
    void play();
    void play(float vol);
};


class SoundSystem {
public:
    FMOD_SYSTEM *sys;
    SoundSystem();
    Sound *newSound( const char *path, float vol );
    Sound *newSound( const char *path );
};



class Pad {
public:
    bool up, down, left, right;
    
    Pad() : up(false), down(false), left(false), right(false) {
    }
    void parseKey( bool is_keydown, unsigned char key, int x, int y ) {
        //    printf("keydown %d\n", key );
        switch(key){
        case 32: // space
            break;
        case 119: // w
            up = is_keydown;
            break;
        case 97: // a
            left = is_keydown;
            break;
        case 115: // s
            down = is_keydown;
            break;
        case 100: // d
            right = is_keydown;
            break;
        default:
            break;
        }    
    }
    void getVec(float *dx, float *dy ){
        *dx = *dy = 0;
        if( up ) *dy=1.0;
        if( down ) *dy=-1.0;
        if( right ) *dx=1.0;
        if( left ) *dx=-1.0;
        if(dx!=0 || dy!=0){
            normalize( dx, dy, 1 );
        }
    }
};



#endif
