#pragma once
#ifdef WIN32
#include "GL/glew.h"
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

class IndexBuffer {
public:
	int *buf;
	int array_len;
    int render_len;
	GLuint gl_name;
	IndexBuffer() : buf(0), array_len(0), render_len(0), gl_name(0) {}
	~IndexBuffer();
	void reserve(int l );
	void setIndex( int index, int i );
	int getIndex( int index );
	void set( int *in, int l );
    void setRenderLen(int l); 
	void bless();
    void unbless();
	void dump();
};
