#ifndef _MOYAI_COMMON_H_
#define _MOYAI_COMMON_H_

///////

#include <stdlib.h>

#ifdef __linux__
#include <map>
#else
#include <unordered_map>
#endif

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <assert.h>

#ifndef WIN32
#include <strings.h>
#endif

#if defined(WIN32)
#include <stdint.h>
#endif

#include <uv.h>

#include "cumino.h"

#ifdef WIN32
#undef min
#undef max
#endif

#pragma pack(push)
#pragma pack(1)
class Vec3 {
public:
    float x,y,z;
    inline Vec3(float xx, float yy, float zz ) : x(xx),y(yy),z(zz){}
    inline Vec3(float xx, float yy ) : x(xx),y(yy),z(0){}
    inline Vec3() : x(0), y(0), z(0) {}
    inline void set(float xx,float yy,float zz) { x=xx; y=yy; z=zz; }
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
    inline DIR4 toDir() {
        const float pi4 = (float) M_PI / 4.0f;
        float at = atan2( x,y );
        if( at >= -pi4  && at <= pi4 ){
            return DIR4_UP;
        } else if( at >= pi4 && at <= pi4*3){
            return DIR4_RIGHT;
        } else if( at >= pi4*3 || at <= -pi4*3 ){
            return DIR4_DOWN;
        } else if( at <= -pi4 && at >= -pi4*3 ){
            return DIR4_LEFT;
        } else {
            return DIR4_NONE;
        }
    }
    inline Vec3 mul(float val){ return Vec3( x*val, y*val, z*val); }
    inline Vec3 add( Vec3 v){ return Vec3( x+v.x, y+v.y, z+v.z);}
    inline Vec3 add( float xx, float yy, float zz ){ return Vec3( x+xx, y+yy,z+zz);}
    inline Vec3 add( float xx, float yy ){ return Vec3( x+xx, y+yy,z);}    
    inline Vec3 to( Vec3 v){ return Vec3( v.x - x, v.y - y, v.z - z ); }
    inline Vec3 randomize(float r){
        float dx = (float)range(-r,r); // to avoid arg eval order problem
        float dy = (float)range(-r,r);
        float dz = (float)range(-r,r);
        return Vec3(x+dx,y+dy,z+dz);
    }
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
    static inline Vec3 fromDir(DIR4 d){
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
    inline DIR4 toDir() {
        if( x==0.0f && y==0.0f) return DIR4_NONE;
        if( y > 0 ) {
            if( absolute(x) < absolute(y) ) return DIR4_UP; else if( x > 0 ) return DIR4_RIGHT; else return DIR4_LEFT;
        } else {
            if( absolute(x) < absolute(y) ) return DIR4_DOWN; else if( x > 0 ) return DIR4_RIGHT; else return DIR4_LEFT;
        }
    }
    inline Vec2 mul(float val){ return Vec2( x*val, y*val); }
    inline Vec2 add( Vec2 v){ return Vec2( x+v.x, y+v.y);}
    inline Vec2 add( float xx, float yy ){ return Vec2( x+xx, y+yy);}
    inline Vec2 to( Vec2 v){ return Vec2( v.x - x, v.y - y); }
    inline Vec2 randomize(float r){
        float dx=(float)range(-r,r); // to avoid arg eval order problem
        float dy=(float)range(-r,r);
        return Vec2(x+dx,y+dy);
    }
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
    static inline Vec2 fromDir(DIR4 d){
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
    // we can't decide alpha bits in fixed way: upper bits or lower bits..?
    inline unsigned int toCode() {
        return  ( (int)(r * 255) << 16 ) + ( (int)(g * 255) << 8 ) + (int)( b * 255);
    }
    inline void toRGB( unsigned char *r, unsigned char *g, unsigned char *b ) {
        unsigned int code = toCode();
        *r = (code >> 16) & 0xff;
        *g = (code >> 8) & 0xff;
        *b = (code) & 0xff;
    }
    inline void toRGBA(unsigned char *outr, unsigned char *outg, unsigned char *outb,unsigned char *outa ) {
        *outr = (int)(r*255);
        *outg = (int)(g*255);
        *outb = (int)(b*255);
        *outa = (int)(a*255);
    }
    inline void fromRGBA(unsigned char inr, unsigned char ing, unsigned char inb,unsigned char ina ) {
        r = ((float)inr)/255.0f;
        g = ((float)ing)/255.0f;
        b = ((float)inb)/255.0f;
        a = ((float)ina)/255.0f;
    }
    inline Color operator*(float f) { return Color( r*f, g*f, b*f, a*f ); }
    inline Color operator+( Color c ) { return Color(r+c.r, g+c.g, b+c.b, a+c.a); }
    inline Color operator==( Color c ) { return (r==c.r && g==c.g && b==c.b && a==c.a); }
    inline Color operator!=( Color c ) { return (r!=c.r || g!=c.g || b!=c.b || a!=c.a); }    
};
#pragma pack(pop)

class Group;

class Prop {
public:
    static int idgen;    
    int id;
    int debug_id;
    Prop *next;
    Prop *prev;
    
    bool to_clean;
    double accum_time;
	double poll_accum_time;
    unsigned int poll_count;

    static const int MAXINTERVAL = 8;
    double last_interval_at[MAXINTERVAL];
    
    static const int CHILDREN_ABS_MAX = 64;    

	static double frame_step_time;

    inline Prop() : id(++idgen), debug_id(0), next(NULL), prev(NULL), to_clean(false), accum_time(0), poll_accum_time(0.0), poll_count(0), parent_group(NULL) {
        for(int i=0;i<MAXINTERVAL;i++) last_interval_at[i]=0;
    }
    virtual ~Prop() {

    }

	bool pollCount(unsigned int value, double &timestamp);

    bool basePoll(double dt);
    virtual bool propPoll(double dt){
        return true;
    }
    virtual void onDelete(){}

	virtual void setParentGroup(Group *group) { parent_group = group; }
	Group* getParentGroup() const { return parent_group; }

    bool updateInterval( int timer_ind, double t );
    
protected:

	Group *parent_group;
};



class Group {
public:
    Prop *prop_top;
    int id;
    static int idgen;
    int last_poll_num;
    bool to_render;
    bool skip_poll;
#ifdef __linux__
    std::map<int,Prop*> idmap;
#else    
    std::unordered_map<int,Prop*> idmap;
#endif    

    double last_dt;
    Group() : prop_top(NULL), last_poll_num(0), to_render(false), skip_poll(false), last_dt(0) {
        id = idgen++;        
    }

	virtual ~Group() {}

    inline void insertProp(Prop*p){
        //        assert(p->deck);
        assertmsg( !p->getParentGroup(), "inserting prop twice");
        if(prop_top){
            p->next = prop_top;
            prop_top->prev = p;
        }
        prop_top = p;
        p->setParentGroup(this);
        p->prev = NULL;

        idmap[p->id] = p;
    }
    int pollAllProps(double dt);
    Prop *findPropById( int id );

    inline Prop *getProp(int id) {
        Prop *cur = prop_top;
        while(cur) {
            if(cur->id == id ) {
                return cur;
            }
            cur = cur->next;
        }
        return NULL;
    }
    int countProps();
    void setCleanFlagAll() {
        Prop *cur = prop_top;
        while(cur) {
            cur->to_clean=true;
            cur=cur->next;
        }
    }
};

class TrackerImage;
class Deck;
class RemoteHead;

class Image {
public:
    static int idgen;
    int id;
    unsigned char *buffer; // rgbargbargba..
    int width, height;
    char last_load_file_path[256];
    TrackerImage *tracker;
    int modified_pixel_num;
    Image() : buffer(NULL), width(0), height(0), tracker(0), modified_pixel_num(0) { id = idgen++;  last_load_file_path[0] = '\0'; }
    ~Image() { if(buffer)FREE(buffer); }
    void setSize(int w, int h ); 
    void setPixel( int x, int y, Color c );
    Color getPixel( int x, int y );
    void getPixelRaw( int x, int y, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a );
    void setPixelRaw( int x, int y, unsigned char r,  unsigned char g,  unsigned char b,  unsigned char a );
    void getAreaRaw( int x, int y, int w, int h, unsigned char *out, size_t outsz );
    void setAreaRaw( int x, int y, int w, int h, unsigned char *in, size_t insz );
    bool loadPNG( const char *path, bool multiply_color_by_alpha = true );
    bool loadPNGMem( unsigned char *ptr, size_t sz, bool multiply_color_by_alpha = true );
    bool loadRaw( const char *path );
    bool writePNG(const char *path);    
    bool writePNGMem( unsigned char **out, size_t *outsize );
    bool writeJPEG(const char *path);
    bool writeRaw( const char *path );
    void ensureBuffer();
    void copyAlpha( int fromx0, int fromy0, int fromx1, int fromy1, int tox0, int toy0 );
    void fill( Color c );
    void fillBoxLeftBottom( Color c, int draw_width, int draw_height );
    size_t getBufferSize() { return width * height * 4; }
    void onTrack( Deck *owner_dk, RemoteHead *rh );
};



class DeferredEvent {
public:
    double accum_time;
    double duration;
    void (*cb)( void* argptr );
    void *argptr;
    DeferredEvent() : accum_time(0), duration(0), cb(NULL), argptr(NULL) {}
    bool isUsed() { return duration > 0; }
};



class Moyai {
public:
    static const int MAXGROUPS = 32;
    Group *groups[MAXGROUPS];
    static const int MAXEVENTS = 4;
    DeferredEvent events[MAXEVENTS];    
    
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

	virtual ~Moyai() {}
    
    void insertGroup( Group *g ) {
        int freei = findFreeGroupIndex(); // The last group will be rendered lastly
        assert(freei>=0);
        groups[freei] = g;
    }
    virtual int poll(double dt );
    bool insertEvent( double delay, void (*cb)(void*argptr),void *argptr);
    void clearEvents();
    void pollEvents( double dt );
    Group *findGroupById(int id) {
        for(int i=0;i<MAXGROUPS;i++) {
            if( groups[i] && groups[i]->id ==id ) return groups[i];
        }
        return  NULL;
    }
    int countGroups() {
        int cnt=0;
        for(int i=0;i<MAXGROUPS;i++) {
            if( groups[i] ) cnt++;
        }
        return cnt;
    }
    Prop *getPropInAllGroups(int id) {
        for(int i=0;i<MAXGROUPS;i++) {
            if( groups[i] ) {
                return groups[i]->getProp(id);
            }
        }
        return NULL;
    }
    Group *getGroupByIndex(int ind) {
        return groups[ind];
    }
    static void globalInitNetwork();
};



#endif
