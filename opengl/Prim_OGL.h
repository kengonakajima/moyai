#pragma once

#include "../common.h"
#include "../common/Enums.h"
#ifdef WIN32
#include "GL/glew.h"
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

#include "../common/DrawBatch.h"

class Prim_OGL {
public:
	PRIMTYPE type;
	Vec2 a,b;
	Color color;
	int line_width;

	Prim_OGL( PRIMTYPE t, Vec2 a, Vec2 b, Color c, int line_width = 1 );
    ~Prim_OGL() {
    }
	void draw( DrawBatchList *bl, Vec2 tr, Vec2 scl, float radrot );
};

