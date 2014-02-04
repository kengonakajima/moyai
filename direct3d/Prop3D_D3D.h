#pragma once

#include "../common.h"
#include "../common/Mesh.h"
#include "../common/Renderable.h"
#include "../common/Material.h"
#include "../common/FragmentShader.h"

class Prop3D_D3D : public Prop, public Renderable 
{

public:

	Vec3 loc;
	Vec3 scl;
	Vec3 rot;
	Mesh *mesh;

	Prop3D_D3D **children;
	int children_num, children_max;

	Material *material;
	Vec3 sort_center;
	bool skip_rot;
	int billboard_index; // enable by >=0
	FragmentShader_D3D *fragment_shader;
	bool depth_mask;
	bool alpha_test;
	bool cull_back_face;
	Vec3 draw_offset;

	Prop3D_D3D();
	virtual ~Prop3D_D3D();
	
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
	void addChild( Prop3D_D3D *p );
	void deleteChild( Prop3D_D3D *p );

	void setMaterial( Material *mat ) { material = mat; }
	void setMaterialChildren( Material *mat ); 

	void setBillboardIndex( int ind ) { billboard_index = ind;  }

	inline void setFragmentShader( FragmentShader_D3D *fs )
	{
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

inline void Prop3D_D3D::cleanRenderOptions() 
{
	/*
	if( fragment_shader ) glUseProgram( 0 );
	glDepthMask(true);
	*/
}

inline void Prop3D_D3D::performRenderOptions() 
{
	/*
	glDepthMask( depth_mask );
	if( alpha_test ) {
		glEnable( GL_ALPHA_TEST );
		glAlphaFunc( GL_GREATER, 0.01 );
	} else {
		glDisable( GL_ALPHA_TEST );
	}
	if( cull_back_face ) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);                    
	} else  {
		glDisable(GL_CULL_FACE);
	}
	if( fragment_shader ){
		glUseProgram( fragment_shader->program );
		fragment_shader->updateUniforms();
	}
	*/
}