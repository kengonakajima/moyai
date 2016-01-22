#include "Texture.h"
#include "soil/src/SOIL.h"

int Texture::idgen = 1;

bool Texture::load( const char *path ){
	tex = SOIL_load_OGL_texture( path,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MULTIPLY_ALPHA );
	if(tex==0) {
        print("SOIL error '%s' on '%s'", SOIL_last_result(), path );
        return false;
    }
	glBindTexture( GL_TEXTURE_2D, tex );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );     
	print("soil_load_ogl_texture: new texid:%d", tex );

	// generate image from gl texture
	if(image) delete image;
	image = new Image();
	int w,h;
	getSize(&w,&h);
	image->setSize(w,h); 
	glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->buffer );

	return true;
}

void Texture::setLinearMagFilter(){
	glBindTexture( GL_TEXTURE_2D, tex );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}

void Texture::setLinearMinFilter(){
	glBindTexture( GL_TEXTURE_2D, tex );    
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );    
}

void Texture::setImage( Image *img ) {
	if(tex==0){
		glGenTextures( 1, &tex );
		assertmsg(tex!=0,"glGenTexture failed");
		glBindTexture( GL_TEXTURE_2D, tex );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); 

	}

	glBindTexture(GL_TEXTURE_2D, tex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->buffer );

	image = img;
}


