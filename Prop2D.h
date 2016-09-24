#pragma once

#include "common.h"
#include "Grid.h"
#include "Renderable.h"
#include "PrimDrawer.h"
#include "Layer.h"
#include "DrawBatch.h"
#include "Remote.h"

class AnimCurve;
class FragmentShader;

class Prop2D : public Prop, public Renderable {
public:

	Vec2 loc;
	Vec2 draw_offset;
	Vec2 scl;

	static const int MAX_GRID = 8;
	Grid *grids[MAX_GRID];  // Set from the top and quit when null found
	int grid_used_num;

	Color color;
	Prop2D *children[CHILDREN_ABS_MAX];
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

	bool render_children_first;
    bool use_additive_blend;

    static VertexFormat *vf_single_sprite; 

    Tracker2D *tracker;
    
	Prop2D() : Prop(), Renderable() {
        init();
    }
    Prop2D(Deck *dk, int index,float scl,Vec2 loc) : Prop(), Renderable() {
        init();
        setDeck(dk);
        setIndex(index);
        setScl(scl);
        setLoc(loc);
    }    
    void init() {
		priority = id;
		dimension = DIMENSION_2D;

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

		render_children_first = false;
        use_additive_blend = false;

        tracker = NULL;
	}
	virtual ~Prop2D(){
		for(int i=0;i<grid_used_num;i++){
			if(grids[i]) delete grids[i];
		}
		for(int i=0;i<children_num;i++){
			if(children[i]) delete children[i];
		}        
		if(prim_drawer) delete prim_drawer;
        if(tracker) delete tracker;
	}

	virtual bool prop2DPoll(double dt){ return true;}
	virtual bool propPoll(double dt);
	virtual void onAnimFinished(){}

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
    // Make sure set only one grid even when called many times
    inline bool setGrid( Grid *g ) {
        for(int i=0;i<grid_used_num;i++) {
            if( grids[i] == g ) return false;
        }
        return addGrid(g);
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
    inline Prop2D *getChild( int child_prop_id ) {
        for(int i=0;i<children_num;i++) {
            Prop2D *chp = (Prop2D*)children[i];
            if( chp->id == child_prop_id ) {
                return chp;
            }
        }
        return NULL;
    }
	void clearChildren();
	bool clearChild( Prop2D *p );
	inline void setColor( Color c ){
		color = c;
        onColorChanged();
	}
	inline Color getColor() { return color; }
	inline void setColor(float r, float g, float b, float a ){
        setColor(Color(r,g,b,a));
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

	virtual void onIndexChanged(int previndex ){}
    virtual void onColorChanged(){}
	inline void setFragmentShader( FragmentShader *fs ){
		fragment_shader = fs;
	}
	Prop *getNearestProp();


	inline void ensurePrimDrawer(){
		if(!prim_drawer ) prim_drawer = new PrimDrawer();
	}
	inline Prim *addLine(Vec2 from, Vec2 to, Color c, int width=1 ){
		ensurePrimDrawer();
        Prim *p = prim_drawer->addLine( from, to, c, width );
		updateMinMaxSizeCache();
        return p;
	}
	inline Prim *addRect( Vec2 from, Vec2 to, Color c ){
		ensurePrimDrawer();
        Prim *p = prim_drawer->addRect( from, to, c );
		updateMinMaxSizeCache();
        return p;
	}
	inline void clearPrims(){
		if(prim_drawer)prim_drawer->clear();
	}
	inline int getPrimNum() {
		if(prim_drawer) return prim_drawer->prim_num; else return 0;
	}
	inline Prim* getPrim(int index) { return prim_drawer->prims[index]; }

	inline bool isCenterInside(Vec2 minloc, Vec2 maxloc){
		return ( loc.x >= minloc.x && loc.x <= maxloc.x && loc.y >= minloc.y && loc.y <= maxloc.y );
	}
	void updateMinMaxSizeCache();
    BLENDTYPE getBlendType() {
        if( use_additive_blend ) return BLENDTYPE_ADD; else return BLENDTYPE_SRC_ALPHA;
    }

	inline bool hit( Vec2 at, float margin = 0 ){
		return ( at.x >= loc.x - scl.x/2 - margin ) && ( at.x <= loc.x + scl.x/2 + margin) &&
			( at.y >= loc.y - scl.y/2 - margin) && ( at.y <= loc.y + scl.y/2 + margin );
	}    

	virtual void render(Camera *cam, DrawBatchList *bl);

	inline void getRect( Vec2 *min_out, Vec2 *max_out ) {
		min_out->x = loc.x - scl.x/2;
		min_out->y = loc.y - scl.y/2;
		max_out->x = loc.x + scl.x/2;
		max_out->y = loc.y + scl.y/2;
	}
	inline Vec2 getBottomLeft() {
		if( grid_used_num > 0 ) return loc; else return loc - scl/2;
	}
	inline Layer *getParentLayer() {
		return (Layer*) parent_group;
	}
    inline Viewport *getViewport() {
        return getParentLayer()->viewport;
    }

    virtual void onTrack( RemoteHead *rh, Prop2D *parentprop );

    Prim *getPrimById( int prim_id ) {
        if( prim_drawer ) {
            return prim_drawer->getPrimById(prim_id);
        } else {
            return NULL;
        }
    }
    void deletePrim( int prim_id ) {
        if( prim_drawer ) prim_drawer->deletePrim(prim_id);
    }
};
