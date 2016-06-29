#pragma once
#include "client.h"

#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
typedef unsigned short IndexBufferType;
#define INDEX_BUFFER_GL_TYPE GL_UNSIGNED_SHORT
#else
typedef unsigned int IndexBufferType;
#define INDEX_BUFFER_GL_TYPE GL_UNSIGNED_INT
#endif
     

class IndexBuffer {
public:
	IndexBufferType *buf; // 16bit for ES, 32bit for normal
	int array_len;
    int render_len;
	GLuint gl_name;
	IndexBuffer() : buf(0), array_len(0), render_len(0), gl_name(0) {}
	~IndexBuffer();
	void reserve(int l );
	void setIndex( int index_at, IndexBufferType ind );
	IndexBufferType getIndex( int index_at );
	void set( IndexBufferType *in, int l );
    void setRenderLen(int l); 
	void bless();
    void unbless();
	void dump(int lim);
};
