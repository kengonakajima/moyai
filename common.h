#ifndef _MOYAI_COMMON_H_
#define _MOYAI_COMMON_H_

#include <stdlib.h>

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <assert.h>

#ifndef WIN32
#include <strings.h>
#endif

#include "cumino.h"
#include "zlib.h"
#include "png.h"

#ifdef WIN32
#undef min
#undef max
#endif

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
        const float pi4 = (float) M_PI / 4.0f;
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
    inline Vec3 randomize(float r){ return Vec3( x + (float)range(-r,r), y + (float)range(-r,r), z + (float)range(-r,r) ); }
    static inline Vec3 angle(float rad){ return Vec3( cos(rad), sin(rad), 0 ); }
    inline void toSign(int*xs,int*ys){ *xs = sign(x); *ys = sign(y); }
    inline void toSign(int*xs,int*ys,int*zs){ *xs = sign(x); *ys = sign(y); *zs = sign(z); }
    inline Vec3 operator+(Vec3 arg){ return Vec3(x+arg.x,y+arg.y,z+arg.z); }
    inline Vec3 operator-(Vec3 arg){ return Vec3(x-arg.x,y-arg.y,z-arg.z); }
    inline Vec3 operator-() { return Vec3(x*-1,y*-1,z*-1); }
    inline Vec3 operator*(float f){ return Vec3(x*f,y*f,z*f); }
    inline Vec3 operator*(Vec3 v){ return Vec3(x*v.x,y*v.y,z*v.z); }    
    inline Vec3 operator/(float f){ return Vec3(x/f,y/f,z/f); }    
    inline Vec3 operator*=(float f){ x *= f; y *= f; z *= f; return Vec3(x,y,z); }
    inline Vec3 operator*=(Vec3 v){ x *= v.x; y *= v.y; z *= v.z; return Vec3(x,y,z); }
    inline Vec3 operator/=(float f){ x /= f; y /= f; z /= f; return Vec3(x,y,z); }
    inline Vec3 operator+=(Vec3 arg){ x += arg.x; y += arg.y; z += arg.z; return Vec3(x,y,z); }
    inline Vec3 operator-=(Vec3 arg){ x -= arg.x; y -= arg.y; z -= arg.z; return Vec3(x,y,z); }
    inline bool operator==(Vec3 arg){ return (x==arg.x && y==arg.y && z==arg.z); }
    inline bool operator!=(Vec3 arg){ return (x!=arg.x || y!=arg.y || z!=arg.z); }
    inline bool operator>=(Vec3 arg){ return (x>=arg.x && y>=arg.y && z>=arg.z); }
    inline bool operator>(Vec3 arg){ return (x>arg.x && y>arg.y && z>arg.z); }    
    inline bool operator<=(Vec3 arg){ return (x<=arg.x && y<=arg.y && z <=arg.z); }
    inline bool operator<(Vec3 arg){ return (x<arg.x && y<arg.y && z<arg.z); }
    inline Vec3 cross(Vec3 v) { return Vec3( y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x ); }
    inline float dot(Vec3 v) { return x*v.x + y*v.y + z*v.z; }
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
        return Vec3( (float)dx, (float)dy);
    }
    // clockwise, up=y+
    inline Vec3 rot(float v) {
        return Vec3( x * cos(v) - y * sin(v),
                     x * sin(v) + y * cos(v),
                     0 );
    }
    inline Vec3 interpolate(Vec3 v, float zero_to_one) {
        return Vec3( ::interpolate(x,v.x,zero_to_one),
                     ::interpolate(y,v.y,zero_to_one),
                     ::interpolate(z,v.z,zero_to_one) );
    }
    inline Vec3 floor() {
        return Vec3( ::floor(x), ::floor(y), ::floor(z) );
    }
    inline Vec3 toInt() {
        return Vec3( (float)((int)(x)), (float)((int)(y)), (float)((int)(z)) );
    }
    inline bool hitCube( Vec3 to, float dia ) {
        return ( (x>to.x-dia) && (x<to.x+dia) && (y>to.y-dia) && (y<to.y+dia) && (z>to.z-dia) && (z<to.z+dia) );
    }
};

class AABB {
public:
    Vec3 min,max;
    inline AABB( Vec3 min, Vec3 max ) : min(min),max(max) {}
    inline AABB( Vec3 bottom_center, float width, float height ) {
        min.x = bottom_center.x - width;
        min.y = bottom_center.y;
        min.z = bottom_center.z - width;

        max.x = bottom_center.x + width;
        max.y = bottom_center.y + height;
        max.z = bottom_center.z + width;        
    }
    inline Vec3 get( int x, int y, int z ) {
        Vec3 out;
        if(x>0) out.x = max.x; else out.x = min.x;
        if(y>0) out.y = max.y; else out.y = min.y;
        if(z>0) out.z = max.z; else out.z = min.z;
        return out;
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
        const float pi4 = (float)(M_PI / 4.0f);
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
    inline Vec2 randomize(float r){ return Vec2( x + (float)range(-r,r), y + (float)range(-r,r) ); }
    static inline Vec2 angle(float rad){ return Vec2( cos(rad), sin(rad) ); }
    inline void toSign(int*xs,int*ys){ *xs = sign(x); *ys = sign(y); }
    inline Vec2 operator+(Vec2 arg){ return Vec2(x+arg.x,y+arg.y); }
    inline Vec2 operator+(Vec3 arg){ return Vec2(x+arg.x,y+arg.y); }    
    inline Vec2 operator-(Vec2 arg){ return Vec2(x-arg.x,y-arg.y); }
    inline Vec2 operator-(Vec3 arg){ return Vec2(x-arg.x,y-arg.y); }
    inline Vec2 operator-(){ return Vec2(x*-1,y*-1); }
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
        return Vec2( (float)dx, (float)dy);
    }
    // clockwise, up=y+
    inline Vec2 rot(float v) {
        return Vec2( x * cos(v) - y * sin(v),
                     x * sin(v) + y * cos(v) );
    }
    inline Vec2 toInt() {
        return Vec2( (float)((int)(x)), (float)((int)(y)) );
    }
    inline bool isNearRect(Vec2 to, float dia) {
        return( x >= to.x - dia &&
                x <= to.x + dia &&
                y >= to.y - dia &&
                y <= to.y + dia );
    }
    inline float dot( Vec2 v ) {
        return x * v.x + y * v.y;
    }
    
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
    inline Color mul(Color v){
        return Color( r * v.r, g * v.g, b * v.b, a );
    }    
    inline Color adjust(float v){
        float rr=r*v,gg=g*v,bb=b*v;
        if(rr>1)rr=1;
        if(gg>1)gg=1;
        if(bb>1)bb=1;
        return Color(rr,gg,bb,a);
    }
    inline Color interpolate( Color c, float rate ) {
        return Color( ::interpolate( r, c.r, rate ),
                      ::interpolate( g, c.g, rate ),
                      ::interpolate( b, c.b, rate ),
                      ::interpolate( a, c.a, rate ) );
    }
    inline unsigned int toCode() {
        return  ( (int)(r * 255) << 16 ) + ( (int)(g * 255) << 8 ) + (int)( b * 255);
    }
    
};


class Group;


class Prop {
public:
    static int idgen;    
    int id;
    int debug_id;
    Prop *next;
    Prop *prev;
    Group *parent_group;
    
    bool to_clean;
    double accum_time;
    unsigned int poll_count;

    static const int CHILDREN_ABS_MAX = 64;


    inline Prop() : id(++idgen), debug_id(0), next(NULL), prev(NULL), parent_group(NULL), to_clean(false), accum_time(0),  poll_count(0) {
    }
    ~Prop() {

    }

    bool basePoll(double dt);
    virtual bool propPoll(double dt){
        return true;
    }
    virtual void onDelete(){}

    
};



class Group {
public:
    Prop *prop_top;
    int id;
    static int idgen;
    int last_poll_num;
    bool to_render;
    bool skip_poll;
    Group() : prop_top(NULL), last_poll_num(0), to_render(false), skip_poll(false) {
        id = idgen++;        
    }

    inline void insertProp(Prop*p){
        //        assert(p->deck);
        assertmsg( !p->parent_group, "inserting prop twice");
        if(prop_top){
            p->next = prop_top;
            prop_top->prev = p;
        }
        prop_top = p;
        p->parent_group = this;
        p->prev = NULL;
    }
    int pollAllProps(double dt);
    Prop *findPropById( int id );
    
};


class Image {
public:
    unsigned char *buffer;
    int width, height;
    Image() : buffer(NULL), width(0), height(0) {}
    ~Image() { if(buffer)FREE(buffer); }
    void setSize(int w, int h ); 
    void setPixel( int x, int y, Color c );
    Color getPixel( int x, int y );
    void getPixelRaw( int x, int y, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a );
    void setPixelRaw( int x, int y, unsigned char r,  unsigned char g,  unsigned char b,  unsigned char a );
    void loadPNG( const char *path );    
    bool writePNG(const char *path);
    void ensureBuffer();
    void copyAlpha( int fromx0, int fromy0, int fromx1, int fromy1, int tox0, int toy0 );
};




class Moyai {
public:
    static const int MAXGROUPS = 32;
    Group *groups[MAXGROUPS];
    
    inline int findFreeGroupIndex(){
        for(int i=0;i<MAXGROUPS;i++){
            if( groups[i] == NULL ){
                return i;
            }
        }
        return -1;
    }
    
    Moyai(){
        for(int i=0;i<MAXGROUPS;i++) groups[i] = NULL;
    }
    
    void insertGroup( Group *g ) {
        int freei = findFreeGroupIndex(); // 後から追加したレイヤの描画順が後ろ
        assert(freei>=0);
        groups[freei] = g;
    }
    int poll(double dt );
};



#endif
