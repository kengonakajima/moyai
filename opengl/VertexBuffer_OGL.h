#pragma once

#include "../common/VertexFormat.h"
#include "../common.h"
#ifdef WIN32
#include "GL/glew.h"
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif

class VertexBuffer_OGL {
public:
	VertexFormat *fmt;
	float *buf;
	int array_len, total_num_float, unit_num_float;
	GLuint gl_name;

	VertexBuffer_OGL() : fmt(NULL), buf(NULL), array_len(0), total_num_float(0), unit_num_float(0), gl_name(0) {}
	~VertexBuffer_OGL() {
		if(gl_name>0) {
			glDeleteBuffers(1,&gl_name);
		}
		assert(buf);
		FREE(buf);
	}
	void setFormat( VertexFormat *f ) { fmt = f; }
	void reserve(int cnt);
	void copyFromBuffer( float *v, int vert_cnt );
	void setCoord( int index, Vec3 v );
	Vec3 getCoord( int index );
	void setCoordBulk( Vec3 *v, int num );
	void setColor( int index, Color c );
	Color getColor( int index );    
	void setUV( int index, Vec2 uv );
	Vec2 getUV( int index );
	void setUVBulk( Vec2 *uv, int num );
	void setNormal( int index, Vec3 v );
	Vec3 getNormal( int index );
	void setNormalBulk( Vec3 *v, int num );
	void bless();
	Vec3 calcCenterOfCoords();
	void dump();
};
