#pragma once

#include "common.h"
#include "Enums.h"
#ifdef WIN32
#include "GL/glew.h"
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif
#include "Viewport.h"
#include "DrawBatch.h"

class Prim {
public:
	PRIMTYPE type;
	Vec2 a,b;
	Color color;
	int line_width;

	Prim( PRIMTYPE t, Vec2 a, Vec2 b, Color c, int line_width = 1 );
    ~Prim() {
    }
	void draw( Viewport *vp, DrawBatchList *bl, Vec2 tr, Vec2 scl, float radrot );
};

