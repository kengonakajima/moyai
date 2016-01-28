#pragma once

#include "common.h"
#include "client.h"

class Texture {
public:
    static int idgen;
    int id;
	GLuint tex;
	Image *image;
	Texture() : tex(0), image(NULL) {
	}

	void setImage( Image *img );
	bool load( const char *path );
	void setLinearMagFilter();
	void setLinearMinFilter();    
	void getSize( int *w, int *h){
		assertmsg(tex!=0,"getSize: not init?");
		glBindTexture( GL_TEXTURE_2D, tex );
		GLint  ww,hh;
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ww );
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &hh );
		*w = ww;
		*h = hh;
	}
	inline Vec2 getSize() { // use this vector direct to Prop2D::setScl(v)
		int w,h;
		getSize( &w, &h );
		return Vec2(w,h);
	}
};
