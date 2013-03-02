#ifndef _MOYAI_H_
#define _MOYAI_H_

#include <stdlib.h>
#include <strings.h>

#include <GL/glfw.h>

#include "soil/src/SOIL.h"

#include "cumino.h"
#include "freetype-gl/freetype-gl.h"
#include "freetype-gl/vertex-buffer.h"

#ifdef USE_FMOD
#include "fmod/api/inc/fmod.h"
#include "fmod/api/inc/fmod_errors.h"
#endif

typedef enum {
    DIMENSION_INVAL = 0,
    DIMENSION_2D = 2,    
    DIMENSION_3D = 3,
} DIMENSION;


class Vec3 {
public:
    float x,y,z;
    inline Vec3(float xx, float yy, float zz ) : x(xx),y(yy),z(zz){}
    inline Vec3(float xx, float yy ) : x(xx),y(yy),z(0){}
    inline Vec3() : x(0), y(0), z(0) {}
    inline Vec3 normalize(float l){
        float xx = x, yy = y, zz = z;
        ::normalize( &xx, &yy, &zz, l );
        return Vec3(xx,yy,zz);
    }
    inline float len(){
        return ::len( x,y,z,0,0,0 );
    }
    inline float len(Vec3 tgt){
        return ::len( x,y,z,tgt.x,tgt.y,tgt.z);
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
    inline Vec3 mul(float val){ return Vec3( x*val, y*val, z*val); }
    inline Vec3 add( Vec3 v){ return Vec3( x+v.x, y+v.y, z+v.z);}
    inline Vec3 add( float xx, float yy, float zz ){ return Vec3( x+xx, y+yy,z+zz);}
    inline Vec3 add( float xx, float yy ){ return Vec3( x+xx, y+yy,z);}    
    inline Vec3 to( Vec3 v){ return Vec3( v.x - x, v.y - y, v.z - z ); }
    inline Vec3 randomize(float r){ return Vec3( x + range(-r,r), y + range(-r,r), z + range(-r,r) ); }
    static inline Vec3 angle(float rad){ return Vec3( cos(rad), sin(rad), 0 ); }
    inline void toSign(int*xs,int*ys){ *xs = sign(x); *ys = sign(y); }
    inline void toSign(int*xs,int*ys,int*zs){ *xs = sign(x); *ys = sign(y); *zs = sign(z); }
    inline Vec3 operator+(Vec3 arg){ return Vec3(x+arg.x,y+arg.y,z+arg.z); }
    inline Vec3 operator-(Vec3 arg){ return Vec3(x-arg.x,y-arg.y,z-arg.z); }
    inline Vec3 operator*(float f){ return Vec3(x*f,y*f,z*f); }
    inline Vec3 operator/(float f){ return Vec3(x/f,y/f,z/f); }    
    inline Vec3 operator*=(float f){ x *= f; y *= f; z *= f; return Vec3(x,y,z); }
    inline Vec3 operator/=(float f){ x /= f; y /= f; z /= f; return Vec3(x,y,z); }                
    inline Vec3 operator+=(Vec3 arg){ x += arg.x; y += arg.y; z += arg.z; return Vec3(x,y,z); }
    inline Vec3 operator-=(Vec3 arg){ x -= arg.x; y -= arg.y; z -= arg.z; return Vec3(x,y,z); }
    inline bool operator==(Vec3 arg){ return (x==arg.x && y==arg.y && z==arg.z); }
    inline bool operator!=(Vec3 arg){ return (x!=arg.x || y!=arg.y || z!=arg.z); }
    inline bool operator>=(Vec3 arg){ return (x>=arg.x && y>=arg.y && z>=arg.z); }
    inline bool operator>(Vec3 arg){ return (x>arg.x && y>arg.y && z>arg.z); }    
    inline bool operator<=(Vec3 arg){ return (x<=arg.x && y<=arg.y && z <=arg.z); }
    inline bool operator<(Vec3 arg){ return (x<arg.x && y<arg.y && z<arg.z); }        
    inline bool isZero(){ return (x==0 && y==0 && z == 0 ); }
    inline Vec3 friction( float diff ){
        float l = len();
        l -= diff;
        Vec3 out = Vec3(x,y,z).normalize(l);
        if( l < 0 ){
            return Vec3(0,0,0);
        } else {
            return out;
        }
    }
    static inline Vec3 random(float v) { return Vec3(0,0,0).randomize(v); }
    static inline Vec3 random() { return random(1); }
    static inline Vec3 fromDir(DIR d){
        int dx,dy;
        dirToDXDY(d,&dx,&dy);
        return Vec3(dx,dy);
    }
    // clockwise, up=y+
    inline Vec3 rot(float v) {
        return Vec3( x * cos(v) - y * sin(v),
                     x * sin(v) + y * cos(v),
                     0 );
    }
};



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
    inline Vec2 operator+(Vec3 arg){ return Vec2(x+arg.x,y+arg.y); }    
    inline Vec2 operator-(Vec2 arg){ return Vec2(x-arg.x,y-arg.y); }
    inline Vec2 operator-(Vec3 arg){ return Vec2(x-arg.x,y-arg.y); }    
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


class VertexFormat {
public:
    // float only
    char types[4]; // 'v': {f,f,f} 'c':{f,f,f,f}  't':{f,f}, 'n':{f,f,f} normal
    int types_used;
    int num_float;
    int coord_offset, color_offset, texture_offset, normal_offset; // -1:not used
    VertexFormat() : types_used(0), num_float(0), coord_offset(-1), color_offset(-1), texture_offset(-1), normal_offset(-1) {
        for(int i=0;i<elementof(types);i++){
            types[i] = 0;
        }
    }
    void declareCoordVec3(int index ){ addType('v'); }
    void declareColor(int index ){ addType('c'); }
    void declareUV(int index){ addType('t'); }
    void declareNormal(int index){ addType('n'); }
    void addType(char t){
        assertmsg( types_used < elementof(types), "too many types");
        types[types_used++] = t;
        updateSize();
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
};

class VertexBuffer {
public:
    VertexFormat *fmt;
    float *buf;
    int array_len, total_num_float, unit_num_float;
    GLuint gl_name;
    VertexBuffer() : fmt(NULL), buf(NULL), array_len(0), total_num_float(0), unit_num_float(0), gl_name(0) {}
    void setFormat( VertexFormat *f ) { fmt = f; }
    void reserve(int cnt){
        assertmsg(fmt, "vertex format is not set" );
        array_len = cnt;
        unit_num_float = fmt->getNumFloat();
        total_num_float = array_len * unit_num_float;
        buf = (float*)malloc( total_num_float * sizeof(float));
        assert(buf);
    }
    void setCoord( int index, Vec3 v ) {
        assertmsg(fmt, "vertex format is not set" );
        assert( index < array_len );
        int ofs = fmt->coord_offset;
        assertmsg( ofs >= 0, "coord have not declared in vertex format" );
        int index_in_array = index * unit_num_float + ofs;
        buf[index_in_array] = v.x;
        buf[index_in_array+1] = v.y;
        buf[index_in_array+2] = v.z;
    }
    void setColor( int index, Color c ) {
        assertmsg(fmt, "vertex format is not set");
        assert( index < array_len );
        int ofs = fmt->color_offset;
        assertmsg( ofs >= 0, "color have not declared in vertex format");
        int index_in_array = index * unit_num_float + ofs;
        buf[index_in_array] = c.r;
        buf[index_in_array+1] = c.g;
        buf[index_in_array+2] = c.b;
        buf[index_in_array+3] = c.a;        
    }
    void setUV( int index, float u, float v ) {
        assertmsg(fmt, "vertex format is not set");
        assert( index < array_len );
        int ofs = fmt->texture_offset;
        assertmsg( ofs >= 0, "texcoord have not declared in vertex format");
        int index_in_array = index * unit_num_float + ofs;
        buf[index_in_array] = u;
        buf[index_in_array+1] = v;
    }
    void setNormal( int index, float x, float y, float z ) { 
        assertmsg(fmt, "vertex format is not set");
        assert( index < array_len );
        int ofs = fmt->normal_offset;
        assertmsg( ofs >= 0, "normal have not declared in vertex format" );
        int index_in_array = index * unit_num_float + ofs;
        buf[index_in_array] = x;
        buf[index_in_array+1] = y;
        buf[index_in_array+2] = z;        
    }
    
    void bless(){
        assert(fmt);
        if( gl_name == 0 ){
            glGenBuffers(1, &gl_name);
            glBindBuffer( GL_ARRAY_BUFFER, gl_name );
            glBufferData( GL_ARRAY_BUFFER, total_num_float * sizeof(float), buf, GL_STATIC_DRAW );
            glBindBuffer( GL_ARRAY_BUFFER, 0 );
            print("VB genbuffer ret name:%d", gl_name );
        }
    }

};
class IndexBuffer {
public:
    int *buf;
    int array_len;
    GLuint gl_name;
    IndexBuffer() : buf(0), array_len(0), gl_name(0) {}
    void set( int *in, int l ) {
        if(buf)free(buf);
        buf = (int*) malloc( sizeof(int) * l );
        assert(buf);
        for(int i=0;i<l;i++){
            buf[i] = in[i];
        }
        array_len = l;
    }
    void bless(){
        if( gl_name == 0 ){
            glGenBuffers(1, &gl_name);
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gl_name );
            // データがよく変わるときは GL_DYNAMIC_DRAWらしいけど、それはコンセプトから外れた使い方だからデフォルトはSTATICにしておく。
            glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * array_len, buf, GL_STATIC_DRAW );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        }
    }
    
};
class Mesh {
public:
    VertexBuffer * vb;
    IndexBuffer *ib;
    GLuint prim_type;
    Mesh() : vb(0), ib(0) {
    }
    void setVertexBuffer(VertexBuffer *b) { vb = b; }
    void setIndexBuffer(IndexBuffer *b ){ ib = b; }
    void setPrimType( GLuint t) { prim_type = t; }
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
    void getPixelRaw( int x, int y, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a );
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
    static int idgen;    
    int id;
    Prop *next;
    Prop *prev;

    DIMENSION dimension;

    Layer *parent_layer;
    bool to_clean;
    double accum_time;
    unsigned int poll_count;
    bool visible;
    TileDeck *deck;
    
    inline Prop() : id(++idgen), next(NULL), prev(NULL), dimension(DIMENSION_INVAL), parent_layer(NULL), to_clean(false), accum_time(0),  poll_count(0), visible(true), deck(NULL) {
    }
    ~Prop() {

    }

    bool basePoll(double dt);
    virtual bool propPoll(double dt){
        return true;
    }
    virtual void onDelete(){}
    inline void setVisible(bool flg){ visible = flg; }
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
    
    virtual void render(Camera *cam){};
    
};

class Prop2D : public Prop {
 public:
    
    Vec2 loc;
    Vec2 draw_offset;
    Vec2 scl;
    
    static const int MAX_GRID = 8;
    Grid *grids[MAX_GRID];  // 頭から入れていってnullだったら終了
    int grid_used_num;
    

    int index;
    Color color;

    static const int MAX_CHILDREN = 8;
    Prop2D *children[MAX_CHILDREN];
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
    

    inline Prop2D() : Prop() {

        dimension = DIMENSION_2D;
        
        //        print("newprop: id:%d", id );
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

class Prop3D : public Prop {
public:
    Vec3 loc;
    Vec3 scl;
    Vec3 rot;
    Mesh *mesh;
    bool billboard;
    Prop3D() : Prop(), loc(0,0,0), scl(1,1,1), rot(0,0,0), mesh(NULL), billboard(false) {
        dimension = DIMENSION_3D;
    }
    inline void setLoc(Vec3 l) { loc = l; }        
    inline void setScl(Vec3 s) { scl = s; }
    inline void setRot(Vec3 r) { rot = r; }
    inline void setMesh( Mesh *m) { mesh = m; }
    virtual bool prop3DPoll(double dt) { return true; }
    virtual bool propPoll(double dt) {
        if( prop3DPoll(dt) == false ) return false;
        return true;
    }

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
        assertmsg( !p->parent_layer, "inserting prop twice");
        if(prop_top){
            p->next = prop_top;
            prop_top->prev = p;
        }
        prop_top = p;
        p->parent_layer = this;
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

    void capture( Image *img );
    
};

#ifdef USE_FMOD

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
#endif


class Pad {
public:
    bool up, down, left, right;
    
    Pad() : up(false), down(false), left(false), right(false) {
    }
    void readGLFW();
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
