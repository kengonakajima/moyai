#pragma once
#ifdef WIN32
#include "GL/glew.h"
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

class FragmentShader_OGL {
public:
	GLuint shader;
	GLuint program;
	FragmentShader_OGL() : shader(0), program(0) {};
	bool load( const char *src);
	virtual void updateUniforms(){};
};
