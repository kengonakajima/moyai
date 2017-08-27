#pragma once

#include "common.h"
#include "VertexFormat.h"

#ifdef WIN32
#include "GL/glew.h"
#endif
#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
#include "OpenGLES/ES1/glext.h"
#elif TARGET_OS_IPHONE
#include "OpenGLES/ES1/glext.h"
#elif TARGET_OS_MAC
#include <OpenGL/gl.h>
#else
#   error "Unknown Apple platform"
#endif
#endif
#ifdef __linux__
#include <GLemu.h>
#endif

class VertexBuffer {
public:
	VertexFormat *fmt;
	float *buf;
	int array_len, total_num_float, unit_num_float, render_len;
	GLuint gl_name;
	VertexBuffer() : fmt(NULL), buf(NULL), array_len(0), total_num_float(0), unit_num_float(0), render_len(0), gl_name(0) {}
	~VertexBuffer() {
		if(gl_name>0) {
            unbless();
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
    void unbless();
    void rebless() { unbless(); bless(); }
	Vec3 calcCenterOfCoords();
	void dump(int lim);
};
