#pragma once

#include "client.h"

class FragmentShader {
public:
	GLuint shader;
	GLuint program;
	FragmentShader() : shader(0), program(0) {};
	bool load( const char *src);
	virtual void updateUniforms(){};
};
