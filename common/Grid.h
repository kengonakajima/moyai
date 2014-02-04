#pragma once

#include "../common.h"
#include "FragmentShader.h"

class TileDeck;

class Grid {
public:
	int width, height;
	int *index_table;
	bool *xflip_table;
	bool *yflip_table;
	Vec2 *texofs_table;
	bool *rot_table;
	Color *color_table;
	TileDeck *deck;
	FragmentShader *fragment_shader;
	bool visible;
	float enfat_epsilon;

	static const int GRID_FLAG_XFLIP = 1;
	static const int GRID_FLAG_YFLIP = 2;
	static const int GRID_NOT_USED = -1;
	Grid(int w, int h ) : width(w), height(h), index_table(NULL), xflip_table(NULL), yflip_table(NULL), texofs_table(NULL), rot_table(NULL), color_table(NULL), deck(NULL), fragment_shader(NULL), visible(true), enfat_epsilon(0) {
	}
	~Grid(){
		if(index_table) FREE(index_table);
		if(xflip_table) FREE(xflip_table);
		if(yflip_table) FREE(yflip_table);
		if(texofs_table) FREE(texofs_table);
		if(rot_table) FREE(rot_table);
		if(color_table) FREE(color_table);
	}
	void setDeck( TileDeck *d ){
		deck = d;
	}
	inline int index(int x, int y){
		assertmsg(x>=0 && x<width,"invalid x:%d",x);
		assertmsg(y>=0 && y<height,"invalid y:%d",y);
		return ( x + y * width );
	}
#define ENSURE_GRID_TABLE( membname, T, inival)  if( !membname ){ membname = (T*) MALLOC(width*height*sizeof(T)); int i=0; for(int y=0;y<height;y++){ for(int x=0;x<width;x++){ membname[i++] = inival; }}}
	inline void set(int x, int y, int ind ){
		ENSURE_GRID_TABLE( index_table, int, GRID_NOT_USED );
		index_table[ index(x,y) ] = ind;
	}
	inline int get(int x, int y){
		if(!index_table){
			return GRID_NOT_USED;
		}
		return index_table[ index(x,y) ];
	}
	inline void setXFlip( int x, int y, bool flag ){
		ENSURE_GRID_TABLE( xflip_table, bool, false );
		xflip_table[ index(x,y) ] = flag;
	}
	inline void setYFlip( int x, int y, bool flag ){
		ENSURE_GRID_TABLE( yflip_table, bool, false );
		yflip_table[ index(x,y) ] = flag;
	}
	// 0~1. 1‚Å‚¿‚å‚¤‚Ç1ƒZƒ‹•ª‚¸‚ê‚é.tex‘S‘Ì‚Å‚Í‚È‚¢B
	inline void setTexOffset( int x, int y, Vec2 *v ) {
		ENSURE_GRID_TABLE( texofs_table, Vec2, Vec2(0,0) );
		int i = index(x,y);
		texofs_table[i].x = v->x;
		texofs_table[i].y = v->y;        
	}
	inline void setUVRot( int x, int y, bool flag ){
		ENSURE_GRID_TABLE( rot_table, bool, false );
		rot_table[ index(x,y) ] = flag;
	}
	inline void setColor( int x, int y, Color col ) {
		ENSURE_GRID_TABLE( color_table, Color, Color(1,1,1,1) );
		color_table[ index(x,y) ] = col;
	}
	inline Color getColor( int x, int y ) {
		ENSURE_GRID_TABLE( color_table, Color, Color(1,1,1,1) );
		return color_table[ index(x,y) ];
	}
	inline void setFragmentShader( FragmentShader *fs ){
		fragment_shader = fs;
	}
	inline void setVisible( bool flg ){ visible = flg; }
	inline bool getVisible() { return visible; }
	inline void clear(int x, int y) { set(x,y,GRID_NOT_USED); }    
	void clear();

	void fillColor( Color c );
};