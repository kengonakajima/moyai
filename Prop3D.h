#pragma once

#include "common.h"
#include "Mesh.h"
#include "Renderable.h"
#include "Material.h"
#include "FragmentShader.h"

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
	FragmentShader *fragment_shader;
	bool depth_mask;
	bool alpha_test;
	bool cull_back_face;
	Vec3 draw_offset;
    bool use_additive_blend;

	Prop3D() : Prop(), loc(0,0,0), scl(1,1,1), rot(0,0,0), mesh(NULL), children(NULL), children_num(0), children_max(0), material(NULL), sort_center(0,0,0), skip_rot(false), fragment_shader(NULL), depth_mask(true), alpha_test(false), cull_back_face(true), draw_offset(0,0,0) , use_additive_blend(false) {
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

inline void Prop3D::cleanRenderOptions() {
#if !(TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE || defined(__linux__) )
	if( fragment_shader ) glUseProgram( 0 );
#endif    
	glDepthMask(true);
}

inline void Prop3D::performRenderOptions() {
	glDepthMask( depth_mask );
	if( alpha_test ) {
		glEnable( GL_ALPHA_TEST );
		glAlphaFunc( GL_GREATER, 0.01f );
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
#if !(TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE || defined(__linux__) )
		glUseProgram( fragment_shader->program );
#endif        
		fragment_shader->updateUniforms();
	}
}
