#pragma once

#include "../common.h"
#include "Mesh.h"

class TileDeck;
#if defined(USE_OPENGL) || defined(USE_D3D)
#include "FragmentShader.h"
#else
class FragmentShader;
#endif


typedef enum {
    GRID_CHANGE_INDEX = 1, // int
    GRID_CHANGE_XFLIP = 2, // int
    GRID_CHANGE_YFLIP = 3, // int
    GRID_CHANGE_TEXOFS = 4, // float, float
    GRID_CHANGE_UVROT = 5, // int
    GRID_CHANGE_COLOR = 6, // float, float, float, float
    GRID_CHANGE_BULK_INDEX = 7, // int * w * h
} GRID_CHANGE_TYPE;

extern void (*g_moyai_grid_change_callback)( void *grid, GRID_CHANGE_TYPE t, int x, int y, void *data, size_t datasize );

class Grid {
public:
    static int idgen;
    int id;
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
    void *parent_prop; // Used only when headless server
    Mesh *mesh;
    bool color_changed;
    bool uv_changed;
    
	static const int GRID_FLAG_XFLIP = 1;
	static const int GRID_FLAG_YFLIP = 2;
	static const int GRID_NOT_USED = -1;
	Grid(int w, int h ) : width(w), height(h), index_table(NULL), xflip_table(NULL), yflip_table(NULL), texofs_table(NULL), rot_table(NULL), color_table(NULL), deck(NULL), fragment_shader(NULL), visible(true), enfat_epsilon(0), parent_prop(NULL), mesh(NULL), color_changed(false), uv_changed(false) {
        id = idgen++;
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
        if( g_moyai_grid_change_callback ) g_moyai_grid_change_callback( this, GRID_CHANGE_INDEX, x, y, &ind, 4 );
        uv_changed = true;
	}
	inline int get(int x, int y){
		if(!index_table){
			return GRID_NOT_USED;
		}
		return index_table[ index(x,y) ];
	}
    void bulkSetIndex( int *inds ) {
        ENSURE_GRID_TABLE( index_table, int, GRID_NOT_USED );
        memcpy( index_table, inds, width*height*sizeof(int) );
        if( g_moyai_grid_change_callback ) g_moyai_grid_change_callback( this, GRID_CHANGE_BULK_INDEX, 0,0, inds, 4 * width * height );
        uv_changed = true;
    }
	inline void setXFlip( int x, int y, bool flag ){
		ENSURE_GRID_TABLE( xflip_table, bool, false );
		xflip_table[ index(x,y) ] = flag;
        if( g_moyai_grid_change_callback ) {
            int f = flag;
            g_moyai_grid_change_callback( this, GRID_CHANGE_XFLIP, x, y, &f, 4 );
        }
        uv_changed = true;
	}
	inline void setYFlip( int x, int y, bool flag ){
		ENSURE_GRID_TABLE( yflip_table, bool, false );
		yflip_table[ index(x,y) ] = flag;
        if( g_moyai_grid_change_callback ) {
            int f = flag;
            g_moyai_grid_change_callback( this, GRID_CHANGE_YFLIP, x, y, &f, 4 );
        }
        uv_changed = true;        
	}
	// 0~1. 1 for just a cell. not by whole texture
	inline void setTexOffset( int x, int y, Vec2 *v ) {
		ENSURE_GRID_TABLE( texofs_table, Vec2, Vec2(0,0) );
		int i = index(x,y);
		texofs_table[i].x = v->x;
		texofs_table[i].y = v->y;
        if( g_moyai_grid_change_callback) {
            float val[2] = { v->x, v->y };
            g_moyai_grid_change_callback( this, GRID_CHANGE_TEXOFS, x, y, val, 4*2 );
        }
        uv_changed = true;        
	}
	inline void setUVRot( int x, int y, bool flag ){
		ENSURE_GRID_TABLE( rot_table, bool, false );
		rot_table[ index(x,y) ] = flag;
        if( g_moyai_grid_change_callback ) {
            int f = flag;
            g_moyai_grid_change_callback( this, GRID_CHANGE_UVROT, x, y, &f, 4 );
        }
        uv_changed = true;        
	}
	inline void setColor( int x, int y, Color col ) {
		ENSURE_GRID_TABLE( color_table, Color, Color(1,1,1,1) );
		color_table[ index(x,y) ] = col;
        if( g_moyai_grid_change_callback ) {
            float val[4] = { col.r, col.g, col.b, col.a };
            g_moyai_grid_change_callback( this, GRID_CHANGE_COLOR, x, y, val, 4*4 );
        }
        color_changed = true;        
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
	void clear( int ind = GRID_NOT_USED );

	void fillColor( Color c );
};
