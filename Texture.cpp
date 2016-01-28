#include "client.h"

#include "Texture.h"


int Texture::idgen = 1;

bool Texture::load( const char *path ){
    if(image) {
        print("Texture::load: Warning: image is already set. possible leak");
    }
    Image *img = new Image();
    img->loadPNG(path);
    setImage(img);
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


