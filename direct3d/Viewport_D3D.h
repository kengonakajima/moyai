#pragma once

#include "../common/Enums.h"
#include "../common.h"

class Viewport_D3D 
{

public:

	int screen_width, screen_height;
	DIMENSION dimension;
	Vec3 scl;
	float near_clip, far_clip;

	Viewport_D3D();
	~Viewport_D3D();

	void setSize(int scrw, int scrh );
	void setScale2D( float sx, float sy );
	void setClip3D( float near, float far ); 
	void getMinMax( Vec2 *minv, Vec2 *maxv );
};