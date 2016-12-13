#pragma once

#include "Enums.h"
#include "common.h"
#include "Pool.h"

class TrackerViewport;
class Client;
class Layer;

class Viewport {
public:
    static int id_gen;
    int id;
	int screen_width, screen_height;
	DIMENSION dimension;
	Vec3 scl;
	float near_clip, far_clip;
    TrackerViewport *tracker;
    Client *remote_client;
    ObjectPool<Layer> target_layers;
	Viewport(Client *cl=NULL);
	void setSize(int scrw, int scrh );
	void setScale2D( float sx, float sy );
	void setClip3D( float near, float far ); 
	void getMinMax( Vec2 *minv, Vec2 *maxv );
    void onTrack( RemoteHead *rh );
    void onTrackDynamic(); // for dynamic_viewport.
    void addTargetLayer(Layer *l);
};
