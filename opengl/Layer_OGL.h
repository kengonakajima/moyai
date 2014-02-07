#pragma once

#include "Viewport_OGL.h"
#include "../common/Light.h"
#include "../common/Mesh.h"
#ifdef WIN32
#include "GL/glew.h"
#endif

class Camera;
class TileDeck;
class Material;

class Layer_OGL : public Group {
public:
	Camera *camera;
	Viewport_OGL *viewport;
	GLuint last_tex_gl_id;
	Light *light;

	// working area to avoid allocation in inner loops
	SorterEntry sorter_opaque[Prop::CHILDREN_ABS_MAX];
	SorterEntry sorter_transparent[Prop::CHILDREN_ABS_MAX];


	Layer_OGL() : Group(), camera(NULL), viewport(NULL), last_tex_gl_id(0), light(NULL) {
		to_render = true;
	}
	inline void setViewport( Viewport_OGL *vp ){
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

	int getHighestPriority();

};
