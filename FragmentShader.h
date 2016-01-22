#pragma once
#ifdef WIN32
#include "GL/glew.h"
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

class FragmentShader {
public:
	GLuint shader;
	GLuint program;
	FragmentShader() : shader(0), program(0) {};
	bool load( const char *src);
	virtual void updateUniforms(){};
};
