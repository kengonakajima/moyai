#pragma once

#include "Viewport.h"
#include "Light.h"
#include "Mesh.h"
#include "DrawBatch.h"
#include "Pool.h"

#ifdef WIN32
#include "GL/glew.h"
#endif



class Camera;
class Deck;
class Material;
class MoyaiClient;

class Layer : public Group {
public:
	Camera *camera;
	Viewport *viewport;
	Light *light;
    int debug_id;
    MoyaiClient *parent_client;

    static const int PRIORITY_MAX = 2147483647;
	// working area to avoid allocation in inner loops
	SorterEntry sorter_opaque[Prop::CHILDREN_ABS_MAX];
	SorterEntry sorter_transparent[Prop::CHILDREN_ABS_MAX];

    int priority; // decided when inserting layer into moyaiclient

    ObjectPool<Camera> dynamic_cameras;

    void (*callback_func)(Layer *l,DrawBatchList *bl);
    
	Layer() : Group(), camera(NULL), viewport(NULL), light(NULL), debug_id(0), priority(0), callback_func(NULL) {
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
    void addDynamicCamera( Camera *cam );
    void delDynamicCamera( Camera *cam );
    void onTrackDynamicCameras();
    bool hasDynamicCamera() {
        return dynamic_cameras.size();
    }
    int render( DrawBatchList *bl );
	int renderAllProps( DrawBatchList *bl );

	void selectCenterInside( Vec2 minloc, Vec2 maxloc, Prop*out[], int *outlen );
	inline void selectCenterInside( Vec2 center, float dia, Prop *out[], int *outlen){
		selectCenterInside( center - Vec2(dia,dia),
			center + Vec2(dia,dia),
			out, outlen );
	}
	inline void drawMesh( int dbg, Mesh *mesh, Deck *deck, Vec3 *loc, Vec3 *locofs, Vec3 *scl, Vec3 *rot, Vec3 *localloc, Vec3 *localscl, Vec3 *localrot, Material *material  );

	void setupProjectionMatrix3D();
	Vec2 getScreenPos( Vec3 at );
	Vec3 getWorldPos( Vec2 scrpos );

	int getHighestPriority();
    void setCallbackFunc( void (*cb)(Layer *l,DrawBatchList*) ) { callback_func = cb; }
};
