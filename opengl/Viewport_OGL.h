#pragma once

#include "../common/Enums.h"
#include "../common.h"

class Viewport_OGL {
public:
	int screen_width, screen_height;
	DIMENSION dimension;
	Vec3 scl;
	float near_clip, far_clip;
	Viewport_OGL() : screen_width(0), screen_height(0), dimension(DIMENSION_2D), scl(0,0,0), near_clip(0.01), far_clip(100) { }
	void setSize(int scrw, int scrh );
	void setScale2D( float sx, float sy );
	void setClip3D( float near, float far ); 
	void getMinMax( Vec2 *minv, Vec2 *maxv );
};