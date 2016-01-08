#pragma once
#ifdef WIN32
#include "GL/glew.h"
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

class IndexBuffer_OGL {
public:
	int *buf;
	int array_len;
	GLuint gl_name;
	IndexBuffer_OGL() : buf(0), array_len(0), gl_name(0) {}
	~IndexBuffer_OGL();
	void reserve(int l );
	void setIndex( int index, int i );
	int getIndex( int index );
	void set( int *in, int l );
	void bless();
    void unbless();
	void dump();
};
