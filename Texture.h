#pragma once

#include "common.h"
#include "client.h"

class Texture {
public:
    static int idgen;
    int id;
	GLuint tex;
	Image *image;
	Texture() : id(idgen++), tex(0), image(NULL) {
	}

	void setImage( Image *img );
    void setGLTexture( GLuint gl_tex_name, int w, int h ) {
        image = new Image();
        image->setSize(w,h); // empty, for getSize()
        tex = gl_tex_name;
    }
	bool load( const char *path, bool multiply_color_by_alpha = true );
	void setLinearMagFilter();
	void setLinearMinFilter();
    int getWidth() { return image->width; }
    int getHeight() { return image->height; }
	void getSize( int *w, int *h){
        assertmsg(image,"Texture::getSize: no image");
        *w = image->width;
        *h = image->height;
	}
	inline Vec2 getSize() { // use this vector direct to Prop2D::setScl(v)
		int w,h;
		getSize( &w, &h );
		return Vec2( (float)w, (float)h);
	}
};
