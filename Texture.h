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
	bool load( const char *path, bool multiply_color_by_alpha = true );
	void setLinearMagFilter();
	void setLinearMinFilter();    
	void getSize( int *w, int *h){
        assertmsg(image,"Texture::getSize: no image");
        *w = image->width;
        *h = image->height;
	}
	inline Vec2 getSize() { // use this vector direct to Prop2D::setScl(v)
		int w,h;
		getSize( &w, &h );
		return Vec2(w,h);
	}
};
