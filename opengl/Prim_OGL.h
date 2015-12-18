#pragma once

#include "../common.h"
#include "../common/Enums.h"
#ifdef WIN32
#include "GL/glew.h"
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

#include "../common/Mesh.h"

class Prim_OGL {
public:
	PRIMTYPE type;
	Vec2 a,b;
	Color color;
	int line_width;
    Mesh *mesh;
	Prim_OGL( PRIMTYPE t, Vec2 a, Vec2 b, Color c, int line_width = 1 );
	void draw();
    static VertexFormat *vf_prim_common;
    static VertexFormat *getVertexFormat();
};

