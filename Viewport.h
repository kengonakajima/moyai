#pragma once

#include "Enums.h"
#include "common.h"

class Viewport {
public:
    static int id_gen;
    int id;
	int screen_width, screen_height;
	DIMENSION dimension;
	Vec3 scl;
	float near_clip, far_clip;
	Viewport() : screen_width(0), screen_height(0), dimension(DIMENSION_2D), scl(0,0,0), near_clip(0.01), far_clip(100) {
        id = id_gen++;
    }
	void setSize(int scrw, int scrh );
	void setScale2D( float sx, float sy );
	void setClip3D( float near, float far ); 
	void getMinMax( Vec2 *minv, Vec2 *maxv );
};
