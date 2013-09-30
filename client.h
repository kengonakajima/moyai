#ifndef _MOYAI_CLIENT_H_
#define _MOYAI_CLIENT_H_


#include "common.h"

#include "soil/src/SOIL.h"
#include "freetype-gl/freetype-gl.h"
#include "freetype-gl/vertex-buffer.h"
#include "fmod/api/inc/fmod.h"
#include "fmod/api/inc/fmod_errors.h"

#include <GL/glfw.h>




typedef enum {
    DIMENSION_INVAL = 0,
    DIMENSION_2D = 2,    
    DIMENSION_3D = 3,
} DIMENSION;


class Viewport {
public:
    int screen_width, screen_height;
    DIMENSION dimension;
    Vec3 scl;
    float near_clip, far_clip;
    Viewport() : screen_width(0), screen_height(0), dimension(DIMENSION_2D), scl(0,0,0), near_clip(0.01), far_clip(100) { }
    void setSize(int scrw, int scrh );
    void setScale2D( float sx, float sy );
    void setClip3D( float near, float far ); 
    void getMinMax( Vec2 *minv, Vec2 *maxv );
};



class VertexFormat {
public:
    // float only
    char types[4]; // 'v': {f,f,f} 'c':{f,f,f,f}  't':{f,f}, 'n':{f,f,f} normal
    int types_used;
    int num_float;
    int coord_offset, color_offset, texture_offset, normal_offset; // -1:not used
    VertexFormat() : types_used(0), num_float(0), coord_offset(-1), color_offset(-1), texture_offset(-1), normal_offset(-1) {
        for(unsigned int i=0;i<elementof(types);i++){
            types[i] = 0;
        }
    }
    void declareCoordVec3(){ addType('v'); }
    void declareColor(){ addType('c'); }
    void declareUV(){ addType('t'); }
    void declareNormal(){ addType('n'); }
    void addType(char t){
        assertmsg( types_used < elementof(types), "too many types");
        types[types_used++] = t;
        updateSize();
    }
    bool isCoordVec3Declared( int index ) {
        assert(index>=0 && index<types_used);
        return types[index] == 'v';
    }
    bool isColorDeclared( int index ) {
        assert(index>=0 && index<types_used);
        return types[index] == 'c';
    }
    bool isUVDeclared( int index ) {
        assert(index>=0 && index<types_used);
        return types[index] == 't';
    }
    bool isNormalDeclared( int index ) {
        assert(index>=0 && index<types_used);
        return types[index] == 'n';
    }
    void updateSize(){
        num_float = 0;
        for(int i=0;i<types_used;i++){
            switch(types[i]){
            case 'v':
                coord_offset = num_float;
                num_float += 3;
                break;
            case 'n':
                normal_offset = num_float;
                num_float += 3;
                break;
            case 'c':
                color_offset = num_float;
                num_float += 4;
                break;
            case 't':
                texture_offset = num_float;
                num_float += 2;
                break;
            default:
                assertmsg( false, "vertexformat: updateSize: invalid type name: '%c'", types[i]);
            }
        }        
    };
    inline size_t getNumFloat() {
        return num_float;
    }
    void dump();
};

class VertexBuffer {
public:
    VertexFormat *fmt;
    float *buf;
    int array_len, total_num_float, unit_num_float;
    GLuint gl_name;

    VertexBuffer() : fmt(NULL), buf(NULL), array_len(0), total_num_float(0), unit_num_float(0), gl_name(0) {}
    ~VertexBuffer() {
        if(gl_name>0) {
            glDeleteBuffers(1,&gl_name);
        }
        assert(buf);
        FREE(buf);
    }
    void setFormat( VertexFormat *f ) { fmt = f; }
    void reserve(int cnt);
    void copyFromBuffer( float *v, int vert_cnt );
    void setCoord( int index, Vec3 v );
    Vec3 getCoord( int index );
    void setCoordBulk( Vec3 *v, int num );
    void setColor( int index, Color c );
    Color getColor( int index );    
    void setUV( int index, Vec2 uv );
    Vec2 getUV( int index );
    void setUVBulk( Vec2 *uv, int num );
    void setNormal( int index, Vec3 v );
    Vec3 getNormal( int index );
    void setNormalBulk( Vec3 *v, int num );
    void bless();
    Vec3 calcCenterOfCoords();
    void dump();
};
class IndexBuffer {
public:
    int *buf;
    int array_len;
    GLuint gl_name;
    IndexBuffer() : buf(0), array_len(0), gl_name(0) {}
    ~IndexBuffer();
    void reserve(int l );
    void setIndex( int index, int i );
    int getIndex( int index );
    void set( int *in, int l );
    void bless();
    void dump();
};

class Light {
public:
    Vec3 pos;
    Color diffuse;
    Color ambient;
    Color specular;
    Light() : pos(0,0,0), diffuse(1,1,1,1), ambient(0,0,0,1), specular(0,0,0,0) {
    }
};

class TileDeck;
class Mesh {
public:
    VertexBuffer * vb;
    IndexBuffer *ib;
    GLuint prim_type;
    bool transparent;
    float line_width;
    Mesh() : vb(0), ib(0), prim_type(0), transparent(false), line_width(1) {
    }
    void setVertexBuffer(VertexBuffer *b) { vb = b; }
    void setIndexBuffer(IndexBuffer *b ){ ib = b; }
    void setPrimType( GLuint t) { prim_type = t; }
    Vec3 getCenter() { return vb->calcCenterOfCoords(); }
    void dump();
    
};




class Texture {
public:
    GLuint tex;
    Image *image;
    Texture() : tex(0), image(NULL) {
    }

    void setImage( Image *img );
    bool load( const char *path );
    void setLinearMagFilter();
    void setLinearMinFilter();    
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
    Texture *tex;
    TileDeck() : cell_width(0), cell_height(0), tile_width(0), tile_height(0), image_width(0), image_height(0),tex(NULL) {}
    void setTexture( Texture *t ){
        assert(t->tex!=0);
        tex = t;
        tex->getSize( &image_width, &image_height );
    }
    void setImage( Image *img ) {
        tex = new Texture();
        tex->setImage(img);
        image_width = img->width;
        image_height = img->height;
    }

    // sprw,sprh : sprite size
    // cellw,cellh : cell nums
    // imgw,imgh : image size
    inline void setSize( int sprw, int sprh, int cellw, int cellh ){
        tile_width = sprw;
        tile_height = sprh;
        cell_width = cellw;
        cell_height = cellh;
    }

    inline void getUVFromIndex( int ind, float *u0, float *v0, float *u1, float *v1, float uofs, float vofs, float eps ) {
        float uunit = (float) cell_width / (float) image_width;
        float vunit = (float) cell_height / (float) image_height;
        int start_x = cell_width * (int)( ind % tile_width );
        int start_y = cell_height * (int)( ind / tile_width );
    
        *u0 = (float) start_x / (float) image_width + eps + uofs * uunit; 
        *v0 = (float) start_y / (float) image_height + eps + vofs * vunit; 
        *u1 = *u0 + uunit - eps; 
        *v1 = *v0 + vunit - eps;
    }
    // (x0,y0)-(x1,y1) : (0,0)-(16,16) for 16x16 sprite
    inline void getPixelPosition( int ind, int *x0, int *y0, int *x1, int *y1 ) {
        int start_x = cell_width * (int)( ind % tile_width );
        int start_y = cell_height * (int)( ind / tile_width );
        *x0 = start_x;
        *y0 = start_y;
        *x1 = start_x + cell_width;
        *y1 = start_y + cell_height;
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
    float enfat_epsilon;
    
    static const int GRID_FLAG_XFLIP = 1;
    static const int GRID_FLAG_YFLIP = 2;
    static const int GRID_NOT_USED = -1;
    Grid(int w, int h ) : width(w), height(h), index_table(NULL), xflip_table(NULL), yflip_table(NULL), texofs_table(NULL), rot_table(NULL), color_table(NULL), deck(NULL), fragment_shader(NULL), color(1,1,1,1), visible(true), enfat_epsilon(0) {
    }
    ~Grid(){
        if(index_table) FREE(index_table);
        if(xflip_table) FREE(xflip_table);
        if(yflip_table) FREE(yflip_table);
        if(texofs_table) FREE(texofs_table);
        if(rot_table) FREE(rot_table);
        if(color_table) FREE(color_table);
    }
    void setDeck( TileDeck *d ){
        deck = d;
    }
    inline int index(int x, int y){
        assertmsg(x>=0 && x<width,"invalid x:%d",x);
        assertmsg(y>=0 && y<height,"invalid y:%d",y);
        return ( x + y * width );
    }
#define ENSURE_GRID_TABLE( membname, T, inival)  if( !membname ){ membname = (T*) MALLOC(width*height*sizeof(T)); int i=0; for(int y=0;y<height;y++){ for(int x=0;x<width;x++){ membname[i++] = inival; }}}
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
    inline Color getColor( int x, int y ) {
        ENSURE_GRID_TABLE( color_table, Color, Color(1,1,1,1) );
        return color_table[ index(x,y) ];
    }
    inline void setFragmentShader( FragmentShader *fs ){
        fragment_shader = fs;
    }
    inline void setColor( Color c){ color = c; }
    inline void setVisible( bool flg ){ visible = flg; }
    inline bool getVisible() { return visible; }
    inline void clear(int x, int y) { set(x,y,GRID_NOT_USED); }    
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
    inline int getIndex( double t , bool *finished = NULL ){
        if(t<0) t=0;
        if( step_time == 0 ) {
            assert( index_used >= 0 );
            return index_table[0];
        }
        
        int ind = (int)(t / step_time);
        assert(ind>=0);
        if(finished) *finished = false;
        if(loop){
            return index_table[ ind % index_used ];
        } else {
            if( ind >= index_used ){
                if(finished) *finished = true;
                return index_table[ index_used - 1];
            } else {
                return index_table[ ind ];
            }
        }
    }
};

class Animation {
public:
    AnimCurve *curves[32];
    Animation() {
        memset( curves, 0, sizeof(curves));
    }
    void reg( int index, AnimCurve *cv ) {
        assertmsg( curves[index] == NULL, "can't register anim curve twice" ); 
        curves[index] = cv;
    }
    
    int getIndex( int curve_ind, double start_at, double t, bool *finished  ) {
        assert( curve_ind >= 0 && curve_ind < elementof(curves) ); 
        AnimCurve *ac = curves[curve_ind];
        if(!ac) return 0;
        return ac->getIndex( t - start_at, finished );
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
            if(!prims) prims = (Prim**) MALLOC( sizeof(Prim*) * prim_max );
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

class Camera;

class Renderable {
public:
    DIMENSION dimension;
    int priority;        

    bool visible;
    TileDeck *deck;
    float enfat_epsilon;
    Renderable() : dimension(DIMENSION_INVAL), priority(0), visible(true), deck(NULL), enfat_epsilon(0) {
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
        d->setSize( 1,1, w, h );
        deck = d;        
    }

    inline void setVisible(bool flg){ visible = flg; }
    inline bool getVisible() { return visible; }        

    inline void swapPriority( Renderable *target ) {
        int p = priority;
        priority = target->priority;
        target->priority = p;
    }
    inline void ensureFront( Renderable *target ) {
        if( target->priority > priority ) {
            swapPriority(target);
        }
    }
    
    virtual void render(Camera *cam){};
    
};

class Prop2D : public Prop, public Renderable {
 public:
    
    Vec2 loc;
    Vec2 draw_offset;
    Vec2 scl;
    
    static const int MAX_GRID = 8;
    Grid *grids[MAX_GRID];  // 頭から入れていってnullだったら終了
    int grid_used_num;
    

    int index;
    Color color;

    Prop2D *children[Prop::CHILDREN_ABS_MAX];
    int children_num;

    AnimCurve *anim_curve;
    double anim_start_at; // from accum_time

    bool xflip, yflip, uvrot;

    FragmentShader *fragment_shader;

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

    float tex_epsilon;

    inline Prop2D() : Prop(), Renderable() {
        priority = id;
        dimension = DIMENSION_2D;

        index = 0;

        color = Color(1,1,1,1);

        children_num = 0;

        loc.x = loc.y = 0;
        draw_offset.x = draw_offset.y = 0;
        scl.x = scl.y = 32;
        anim_curve = NULL;

        anim_start_at = 0;
        grid_used_num = 0;

        xflip = yflip = uvrot = false;

        rot = 0;
        seek_scl_time = seek_scl_started_at = 0;
        seek_rot_time = seek_rot_started_at = 0;
        seek_color_time = seek_color_started_at = 0;

        fragment_shader = NULL;
        prim_drawer = NULL;
        max_rt_cache = Vec2(0,0);
        min_lb_cache = Vec2(0,0);

        tex_epsilon = 0;
    }
    ~Prop2D(){
        for(int i=0;i<grid_used_num;i++){
            if(grids[i]) delete grids[i];
        }
        for(int i=0;i<children_num;i++){
            if(children[i]) delete children[i];
        }        
        if(prim_drawer) delete prim_drawer;
    }

    
    virtual bool prop2DPoll(double dt){ return true;}
    virtual bool propPoll(double dt);        

    inline void setIndex( int ind){
        index = ind;
    }
    inline void setScl(Vec2 s){
        scl = s;
    }
    inline void setScl(float s) { scl.x = scl.y = s; }
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
    inline Grid* getGrid(int index) {
        assert(index>=0 && index < elementof(grids) ) ;
        return grids[index];
    }
    
    inline void clearGrid() {
        grid_used_num = 0; // object have to be freed by app
        updateMinMaxSizeCache();        
    }
    inline bool addChild( Prop2D *p ){
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
    inline bool clearChild( Prop2D *p ) {
        for(int i=0;i<elementof(children);i++) {
            if( children[i] ==p ) {
                for(int j=i;j<elementof(children)-1;j++){
                    children[j] = children[j+1];
                }
                children_num --;
                return true;
            }
        }
        return false;
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
    inline bool isSeekingColor(){ return seek_color_time != 0 && (seek_color_time + seek_color_started_at > accum_time); }    
    inline void setAnim(AnimCurve *ac ){
        assert(ac);
        anim_curve = ac;
        anim_start_at = accum_time;
    }
    inline void clearAnim() {
        anim_curve = NULL;
    }
    inline void ensureAnim( AnimCurve *ac ) {
        if( anim_curve != ac ) setAnim(ac);
    }
    

    inline void setUVRot( bool flg){ uvrot = flg; }
    inline void setXFlip( bool flg){ xflip = flg; }
    inline void setYFlip( bool flg){ yflip = flg; }

    void drawIndex( TileDeck *dk, int ind, float minx, float miny, float maxx, float maxy, bool hrev, bool vrev, float uofs, float vofs, bool uvrot, float radrot );
    

    
    virtual void onIndexChanged(int previndex ){}

    inline void setFragmentShader( FragmentShader *fs ){
        assert(fs);
        fragment_shader = fs;
    }
    Prop *getNearestProp();


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
    virtual void render(Camera *cam);

};

class Material {
public:
    Color diffuse;
    Color ambient;
    Color specular;
    Material() : diffuse(1,1,1,1), ambient(0,0,0,0), specular(0,0,0,0) {}
};

class Prop3D : public Prop, public Renderable {
public:
    Vec3 loc;
    Vec3 scl;
    Vec3 rot;
    Mesh *mesh;

    Prop3D **children;
    int children_num, children_max;

    Material *material;
    Vec3 sort_center;
    bool skip_rot;
    int billboard_index; // enable by >=0
    FragmentShader *fragment_shader;
    bool depth_mask;
    bool alpha_test;
    bool cull_back_face;
    Vec3 draw_offset;

    
    Prop3D() : Prop(), loc(0,0,0), scl(1,1,1), rot(0,0,0), mesh(NULL), children(NULL), children_num(0), children_max(0), material(NULL), sort_center(0,0,0), skip_rot(false), billboard_index(-1), fragment_shader(NULL), depth_mask(true), alpha_test(false), cull_back_face(true), draw_offset(0,0,0) {
        priority = id;        
        dimension = DIMENSION_3D;
    }
    ~Prop3D() {
        //       if(children) FREE(children);
    }
    inline void setLoc(Vec3 l) { loc = l; }
    inline void setLoc(float x, float y, float z) { loc.x = x; loc.y = y; loc.z = z; }            
    inline void setScl(Vec3 s) { scl = s; }
    inline void setScl(float x, float y, float z) { scl.x = x; scl.y = y; scl.z = z; }
    inline void setScl(float s){ setScl(s,s,s); }
    inline void setRot(Vec3 r) { rot = r; }
    inline void setRot(float x, float y, float z) { rot.x = x; rot.y = y; rot.z = z; }
    inline void setMesh( Mesh *m) { mesh = m; }
    void reserveChildren( int n );
    int countSpareChildren();
    void addChild( Prop3D *p );
    void deleteChild( Prop3D *p );
    void setMaterial( Material *mat ) { material = mat; }
    void setMaterialChildren( Material *mat ); 
    void setBillboardIndex( int ind ) { billboard_index = ind;  }
    inline void setFragmentShader( FragmentShader *fs ){
        assert(fs);
        fragment_shader = fs;
    }
    inline void setDepthMask(bool flg) { depth_mask = flg; }
    inline void setAlphaTest(bool flg) { alpha_test = flg; }
    inline void setCullBackFace(bool flg) { cull_back_face = flg; }
    inline void cleanRenderOptions();
    inline void performRenderOptions();
    Vec2 getScreenPos();
    
    virtual bool prop3DPoll(double dt) { return true; }
    virtual bool propPoll(double dt);
    
};

class Camera {
public:
    Vec3 loc;
    Vec3 look_at, look_up;
    Camera(){}
    inline void setLoc(Vec2 lc) {
        loc.x = lc.x;
        loc.y = lc.y;
        loc.z = 0;
    }
    inline void setLoc(Vec3 lc) {
        loc = lc;
    }
    inline void setLoc(float x,float y){
        loc.x = x;
        loc.y = y;
    }
    inline void setLoc( float x, float y, float z ) {
        loc.x = x; loc.y = y; loc.z = z;
    }
    inline void setLookAt( Vec3 at, Vec3 up ) {
        look_at = at;
        look_up = up;
    }
    inline Vec3 getLookAt() { return look_at; }
    static void screenToGL( int scr_x, int scr_y, int scrw, int scrh, Vec2 *out );
        
    Vec2 screenToWorld( int scr_x, int scr_y, int scr_w, int scr_h );
    Vec3 getDirection() { return look_at - loc; }

    inline void adjustInsideDisplay( Vec2 scrsz, Vec2 area_min, Vec2 area_max, float zoom_rate ) {
        float xsz = scrsz.x / 2 / zoom_rate;
        float ysz = scrsz.y / 2 / zoom_rate;
        float left = area_min.x + xsz;
        if( loc.x < left ) {
            loc.x = left;
        }
        float right = area_max.x - xsz;
        if( loc.x > right ) {
            loc.x = right;
        }

        float bottom = area_min.y + ysz;
        if( loc.y < bottom ) {
            loc.y = bottom;
        }
        float top = area_max.y - ysz;
        if( loc.y > top ) {
            loc.y = top;
        }
    }
    
};



class Layer : public Group {
 public:
    Camera *camera;
    Viewport *viewport;
    GLuint last_tex_gl_id;
    Light *light;
    
    // working area to avoid allocation in inner loops
    SorterEntry sorter_opaque[Prop::CHILDREN_ABS_MAX];
    SorterEntry sorter_transparent[Prop::CHILDREN_ABS_MAX];
    

    Layer() : Group(), camera(NULL), viewport(NULL), last_tex_gl_id(0), light(NULL) {
        to_render = true;
    }
    inline void setViewport( Viewport *vp ){
        viewport = vp;
    }
    inline void setCamera(Camera *cam){
        camera = cam;
    }
    inline void setLight(Light *l){
        light = l;
    }

    int renderAllProps();

    void selectCenterInside( Vec2 minloc, Vec2 maxloc, Prop*out[], int *outlen );
    inline void selectCenterInside( Vec2 center, float dia, Prop *out[], int *outlen){
        selectCenterInside( center - Vec2(dia,dia),
                            center + Vec2(dia,dia),
                            out, outlen );
    }
    inline void drawMesh( int dbg, Mesh *mesh, TileDeck *deck, Vec3 *loc, Vec3 *locofs, Vec3 *scl, Vec3 *rot, Vec3 *localloc, Vec3 *localscl, Vec3 *localrot, Material *material  );
    inline void drawBillboard(int billboard_index, TileDeck *deck, Vec3 *loc, Vec3 *scl  );

    void setupProjectionMatrix3D();
    Vec2 getScreenPos( Vec3 at );
    Vec3 getWorldPos( Vec2 scrpos );


    
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
    bool loadFromTTF(const char *path, const wchar_t *codes, int pixelsz ){
        pixel_size = pixelsz;
        font = texture_font_new( atlas, path, pixelsz );
        if(!font){
            return false;
        }
        
        texture_font_load_glyphs( font, codes);

        
        return true;
        
    }
};
class TextBox : public Prop2D {
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
        wchar_t *out = (wchar_t*)MALLOC((l+1)*sizeof(wchar_t));
        mbstowcs(out, s, l+1 );
        setString(out);
        FREE(out);
    }
        
    inline void setString( const wchar_t *s ){
        setString( (wchar_t*)s );
    }
    inline void setString( wchar_t *s ){
        size_t l = wcslen(s);
        if(str){
            FREE(str);
        }
        str = (wchar_t*)MALLOC( (l+1) * sizeof(wchar_t) );
        wcscpy( str, s );
    }
};

class MoyaiClient : public Moyai {
public:
    MoyaiClient() : Moyai() {
    }
    int render();
    void capture( Image *img );
    void insertLayer( Layer *l ) {
        insertGroup( l );
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
    void playDistance(float mindist, float maxdist, float dist, float relvol );
    
};


class SoundSystem {
public:
    FMOD_SYSTEM *sys;
    SoundSystem();
    
    Sound * newSE( const char *path ) { return newSE(path,1.0f); }
    Sound * newSE( const char *path, float vol ) { return newSound(path,vol,false); };
    Sound * newBGM( const char *path ) { return newBGM(path,1.0f); }
    Sound * newBGM( const char *path, float vol ) { return newSound(path,vol,true); };
    
    Sound *newSound( const char *path, float vol, bool use_stream_currently_ignored );
    Sound *newSound( const char *path );
};



class Pad {
public:
    bool up, down, left, right;
    
    Pad() : up(false), down(false), left(false), right(false) {
    }
    void readGLFW();
    void getVec( Vec2 *v ){
        float dx=0,dy=0;
        if( up ) dy=1.0;
        if( down ) dy=-1.0;
        if( right ) dx=1.0;
        if( left ) dx=-1.0;
        if(dx!=0 || dy!=0){
            normalize( &dx, &dy, 1 );
        }
        v->x = dx;
        v->y = dy;
    }
};


#endif
