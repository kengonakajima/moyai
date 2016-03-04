#pragma once

#include "client.h"

class FragmentShader {
public:
    static int idgen;
    int id;
	GLuint shader;
	GLuint program;
	FragmentShader() : id(idgen++), shader(0), program(0) {};
	bool load( const char *src);
	virtual void updateUniforms(){};
};
