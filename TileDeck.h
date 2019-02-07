#pragma once

#include "Texture.h"

class Deck {
public:
    static int idgen;
    int id;    
	int image_width, image_height; // Size of the image in pixels
    int ofs_x, ofs_y;
	Texture *tex;
    
    Deck() : id(idgen++), image_width(0), image_height(0), ofs_x(0), ofs_y(0), tex(NULL) {
    }
	void setTexture( Texture *t ){
		assertmsg(t->tex!=0, "invalid texture" );
		tex = t;
		tex->getSize( &image_width, &image_height );
	}
	void setImage( Image *img, int offset_x=0, int offset_y=0 ) {
		tex = new Texture();
		tex->setImage(img);
		image_width = img->width;
		image_height = img->height;
        ofs_x=offset_x;
        ofs_y=offset_y;
	}
	virtual void getUVFromIndex( int ind, float *u0, float *v0, float *u1, float *v1, float uofs, float vofs, float eps ) {
        print("getUVFromIndex:should never called");
    }
    virtual float getUperCell() { return 0; }
    virtual float getVperCell() { return 0;}
};

class TileDeck : public Deck {
public:

	int cell_width, cell_height; // Size in pixels of a single sprite
	int tile_width, tile_height; // Number of sprites in the atlas


	TileDeck() : Deck(), cell_width(0), cell_height(0), tile_width(0), tile_height(0) {}

	// sprw,sprh : number of sprites in the atlas
	// cellw,cellh : size in pixels of a single sprite 
	void setSize( int sprw, int sprh, int cellw, int cellh ){
		tile_width = sprw;
		tile_height = sprh;
		cell_width = cellw;
		cell_height = cellh;
	}
    virtual float getUperCell() { return (float) cell_width / (float) image_width; }
    virtual float getVperCell() { return (float) cell_height / (float) image_height; }    
	virtual void getUVFromIndex( int ind, float *u0, float *v0, float *u1, float *v1, float uofs, float vofs, float eps ) {
        assert( image_width > 0 && image_height > 0 );
		float uunit = getUperCell();
		float vunit = getVperCell();
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


class UVRect {
public:
    float u0,v0,u1,v1;
};
class PackDeck : public Deck {
public:
    UVRect *rects;
    size_t rect_num;
    PackDeck() : Deck(), rects(NULL), rect_num(0) {
    }
    void setRects( UVRect *in_rects, size_t n ) {
        assert(rects==NULL);
        size_t sz = n*sizeof(UVRect);
        rects = (UVRect*)MALLOC(sz);
        memcpy(rects,in_rects,n*sizeof(UVRect));
        rect_num = n;
    }
	virtual void getUVFromIndex( int ind, float *u0, float *v0, float *u1, float *v1, float uofs, float vofs, float eps ) {
        assert(ind>=0 && ind<(int)rect_num);
        *u0 = rects[ind].u0;
        *v0 = rects[ind].v0;
        *u1 = rects[ind].u1;
        *v1 = rects[ind].v1;
    }
    ~PackDeck() {
        if(rects)FREE(rects);
    }
};
