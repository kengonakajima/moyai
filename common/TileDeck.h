#pragma once

#include "Texture.h"

class TileDeck {
public:
    static int idgen;
    int id;
	int cell_width, cell_height; // Size in pixels of a single sprite
	int tile_width, tile_height; // Number of sprites in the atlas
	int image_width, image_height; // Size of the image in pixels
	Texture *tex;
	TileDeck() : cell_width(0), cell_height(0), tile_width(0), tile_height(0), image_width(0), image_height(0),tex(NULL) {}
	void setTexture( Texture *t ){
		assertmsg(t->tex!=0, "invalid texture" );
		tex = t;
		tex->getSize( &image_width, &image_height );
	}
	void setImage( Image *img ) {
		tex = new Texture();
		tex->setImage(img);
		image_width = img->width;
		image_height = img->height;
	}

	// sprw,sprh : number of sprites in the atlas
	// cellw,cellh : size in pixels of a single sprite 
	inline void setSize( int sprw, int sprh, int cellw, int cellh ){
		tile_width = sprw;
		tile_height = sprh;
		cell_width = cellw;
		cell_height = cellh;
	}

	inline void getUVFromIndex( int ind, float *u0, float *v0, float *u1, float *v1, float uofs, float vofs, float eps ) {
		float uunit = (float) cell_width / (float) image_width;
		float vunit = (float) cell_height / (float) image_height;
		int start_x = cell_width * (int)( ind % tile_width );
		int start_y = cell_height * (int)( ind / tile_width );

		*u0 = (float) start_x / (float) image_width + eps + uofs * uunit; 
		*v0 = (float) start_y / (float) image_height + eps + vofs * vunit; 
		*u1 = *u0 + uunit - eps*2;  // *2 because adding eps once for u0 and v0
		*v1 = *v0 + vunit - eps*2;
	}
	// (x0,y0)-(x1,y1) : (0,0)-(16,16) for 16x16 sprite
	inline void getPixelPosition( int ind, int *x0, int *y0, int *x1, int *y1 ) {
		int start_x = cell_width * (int)( ind % tile_width );
		int start_y = cell_height * (int)( ind / tile_width );
		*x0 = start_x;
		*y0 = start_y;
		*x1 = start_x + cell_width;
		*y1 = start_y + cell_height;
	}
};
