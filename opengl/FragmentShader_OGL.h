#pragma once

#include "GL/glew.h"

class FragmentShader_OGL {
public:
	GLuint shader;
	GLuint program;
	FragmentShader_OGL() : shader(0), program(0) {};
	bool load( const char *src);
	virtual void updateUniforms(){};
};