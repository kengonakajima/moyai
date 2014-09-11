#pragma once

#include "../common.h"
#include "../common/Grid.h"
#include "../common/Renderable.h"
#include "../common/PrimDrawer.h"
#include "../common/Layer.h"
#include "ShaderManager_D3D.h"

class AnimCurve;
class FragmentShader_D3D;
class DefaultShader_D3D;
class VertexBuffer_D3D;

class Prop2D_D3D : public Prop, public Renderable 
{

public:

	Vec2 loc;
	Vec2 draw_offset;
	Vec2 scl;

	static const int MAX_GRID = 8;
	Grid *grids[MAX_GRID];  // “ª‚©‚ç“ü‚ê‚Ä‚¢‚Á‚Änull‚¾‚Á‚½‚çI—¹
	int grid_used_num;

	Color color;

	Prop2D_D3D *children[Prop::CHILDREN_ABS_MAX];
	int children_num;

	AnimCurve *anim_curve;
	double anim_start_at; // from accum_time

	bool xflip, yflip, uvrot;

	FragmentShader_D3D *fragment_shader;

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

	Prop2D_D3D(); 
	virtual ~Prop2D_D3D();

	virtual bool prop2DPoll(double dt){ return true;}
	virtual bool propPoll(double dt);
	virtual void onAnimFinished(){}
	virtual void setParentGroup(Group *group);

	inline void setIndex( int ind)
	{
		index = ind;
	}

	inline void setScl(Vec2 s)
	{
		scl = s;
	}

	inline void setScl(float s) { scl.x = scl.y = s; }

	inline void setScl(float x, float y )
	{
		scl.x = x;
		scl.y = y;
	}

	inline void seekScl(float x, float y, double time_sec )
	{
		seek_scl_orig = scl;
		seek_scl_started_at = accum_time;        
		seek_scl_time = time_sec;
		seek_scl_target = Vec2(x,y);
	}

	inline bool isSeekingScl(){ return seek_scl_time != 0 && ( seek_scl_time + seek_scl_started_at  > accum_time ); }

	inline void setLoc( Vec2 p)
	{
		loc = p;
	}

	inline void setLoc( float x, float y )
	{
		loc.x = x;
		loc.y = y;
	}

	inline void setRot( float r ){ rot = r; }

	inline void seekRot( float r, double time_sec )
	{
		seek_rot_orig = rot;
		seek_rot_started_at = accum_time;
		seek_rot_time = time_sec;
		seek_rot_target = r;
	}

	inline bool isSeekingRot() { return seek_rot_time != 0 && (seek_rot_time + seek_rot_started_at > accum_time); }

	inline bool addGrid( Grid *g )
	{
		assert(g);
		if( grid_used_num >= elementof(grids) )
		{
			print("WARNING: too many grid in a prop");
			return false;
		}

		grids[grid_used_num++] = g;
		updateMinMaxSizeCache();

		if (!g->fragment_shader)
		{
			g->fragment_shader = g_context.m_pShaderManager->GetShader(ShaderManager_D3D::SHADER_INSTANCING);
		}

		return true;
	}

	inline Grid* getGrid(int index) 
	{
		assert(index>=0 && index < elementof(grids) ) ;
		return grids[index];
	}

	inline void clearGrid() 
	{
		grid_used_num = 0; // object have to be freed by app
		updateMinMaxSizeCache();        
	}

	inline bool addChild( Prop2D_D3D *p )
	{
		assert(p);
		if( children_num >= elementof(children) ) {
			assertmsg(false,"WARNING: too many children in a prop");
			return false;
		}
		children[children_num++] = p;
		p->parent_group = parent_group;
		updateMinMaxSizeCache();

		return true;
	}

	inline void clearChildren() 
	{
		children_num=0;
	}

	inline bool clearChild( Prop2D_D3D *p ) 
	{
		for(int i=0;i<elementof(children);i++) 
		{
			if( children[i] ==p ) 
			{
				for(int j=i;j<elementof(children)-1;j++)
				{
					children[j] = children[j+1];
				}

				children_num --;

				return true;
			}
		}

		return false;
	}

	inline void setColor( Color c )
	{
		color = c;
	}

	inline Color getColor() { return color; }

	inline void setColor(float r, float g, float b, float a )
	{
		color = Color(r,g,b,a);
	}

	inline void seekColor( Color c, double time_sec ) 
	{
		seek_color_orig = color;
		seek_color_started_at = accum_time;
		seek_color_time = time_sec;
		seek_color_target = c;        
	}

	inline bool isSeekingColor(){ return seek_color_time != 0 && (seek_color_time + seek_color_started_at > accum_time); }    

	inline void setAnim(AnimCurve *ac )
	{
		assert(ac);
		anim_curve = ac;
		anim_start_at = accum_time;
	}

	inline void clearAnim() 
	{
		anim_curve = NULL;
	}

	inline void ensureAnim( AnimCurve *ac ) 
	{
		if( anim_curve != ac ) setAnim(ac);
	}

	inline void setUVRot( bool flg){ uvrot = flg; }
	inline void setXFlip( bool flg){ xflip = flg; }
	inline void setYFlip( bool flg){ yflip = flg; }

	void drawIndex(TileDeck *dk, int ind, float minx, float miny, float maxx, float maxy, bool hrev, bool vrev, float uofs, float vofs, bool uvrot, float radrot, Layer_D3D::InstanceData &instanceData);

	virtual void onIndexChanged(int previndex ) { }

	inline void setFragmentShader( FragmentShader_D3D *fs )
	{
		assert(fs);
		fragment_shader = fs;
	}

	Prop *getNearestProp();

	inline void ensurePrimDrawer()
	{
		if(!prim_drawer ) prim_drawer = new PrimDrawer();
	}

	inline void addLine(Vec2 from, Vec2 to, Color c, int width=1 )
	{
		ensurePrimDrawer();
		prim_drawer->addLine( from, to, c, width );
		updateMinMaxSizeCache();
	}

	inline void addRect( Vec2 from, Vec2 to, Color c )
	{
		ensurePrimDrawer();
		prim_drawer->addRect( from, to, c );
		updateMinMaxSizeCache();
	}

	inline void clearPrims()
	{
		if(prim_drawer)prim_drawer->clear();
	}

	inline int getPrimNum() 
	{
		if(prim_drawer) return prim_drawer->prim_num; else return 0;
	}

	inline Prim* getPrim(int index) { return prim_drawer->prims[index]; }

	inline bool isCenterInside(Vec2 minloc, Vec2 maxloc)
	{
		return ( loc.x >= minloc.x && loc.x <= maxloc.x && loc.y >= minloc.y && loc.y <= maxloc.y );
	}

	void updateMinMaxSizeCache();

	inline bool hit( Vec2 at, float margin = 0 )
	{
		return ( at.x >= loc.x - scl.x/2 - margin ) && ( at.x <= loc.x + scl.x/2 + margin) &&
			( at.y >= loc.y - scl.y/2 - margin) && ( at.y <= loc.y + scl.y/2 + margin );
	}    

	virtual void render(Camera *cam);

	inline void getRect( Vec2 *min_out, Vec2 *max_out ) 
	{
		min_out->x = loc.x - scl.x/2;
		min_out->y = loc.y - scl.y/2;
		max_out->x = loc.x + scl.x/2;
		max_out->y = loc.y + scl.y/2;
	}

	inline Vec2 getBottomLeft() 
	{
		if( grid_used_num > 0 ) return loc; else return loc - scl/2;
	}

	inline Layer *getParentLayer() 
	{
		return (Layer*) parent_group;
	}

private:

	void init();
	Layer_D3D::RenderData& getNewRenderData();
};