#pragma once

#include "common.h"

#include "Mesh.h"

class Deck;
#include "FragmentShader.h"


class TrackerGrid;
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
	Deck *deck;
	FragmentShader *fragment_shader;
	bool visible;
    void *parent_prop; // Used only when headless server
    Mesh *mesh;
    bool color_changed;
    bool uv_changed;
    int debug;
    Vec2 rel_scl, rel_loc;
    TrackerGrid *tracker;
    float uv_margin;    
	static const int GRID_FLAG_XFLIP = 1;
	static const int GRID_FLAG_YFLIP = 2;
	static const int GRID_NOT_USED = -1;
	Grid(int w, int h ) : width(w), height(h), index_table(NULL), xflip_table(NULL), yflip_table(NULL), texofs_table(NULL), rot_table(NULL), color_table(NULL), deck(NULL), fragment_shader(NULL), visible(true), parent_prop(NULL), mesh(NULL), color_changed(false), uv_changed(false), debug(0), rel_scl(1,1), rel_loc(0,0), tracker(NULL), uv_margin(0) {
        id = idgen++;
	}
	~Grid();
	void setDeck( Deck *d ){
		deck = d;
	}
	inline int index(int x, int y){
		assertmsg(x>=0 && x<width,"invalid x:%d",x);
		assertmsg(y>=0 && y<height,"invalid y:%d",y);
		return ( x + y * width );
	}
    inline int getCellNum() { return width * height; }
#define ENSURE_GRID_TABLE( membname, T, inival)  if( !membname ){ membname = (T*) MALLOC(width*height*sizeof(T)); int i=0; for(int y=0;y<height;y++){ for(int x=0;x<width;x++){ membname[i++] = inival; }}}
	inline void set(int x, int y, int ind ){
		ENSURE_GRID_TABLE( index_table, int, GRID_NOT_USED );
        int orig = index_table[ index(x,y) ];
		index_table[ index(x,y) ] = ind;
        if(orig!=ind) uv_changed = true;
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
        uv_changed = true;
    }
	inline void setXFlip( int x, int y, bool flag ){
		ENSURE_GRID_TABLE( xflip_table, bool, false );
		xflip_table[ index(x,y) ] = flag;
        uv_changed = true;
	}
    inline void setXFlipIndex( int i, bool flag ) {
        setXFlip( i % width, i / width, flag );
    }
    inline bool getXFlip( int x, int y ) {
        if( !xflip_table ) return false;
        return xflip_table[ index(x,y) ];
    }
	inline void setYFlip( int x, int y, bool flag ){
		ENSURE_GRID_TABLE( yflip_table, bool, false );
		yflip_table[ index(x,y) ] = flag;
        uv_changed = true;        
	}
    inline void setYFlipIndex( int i, bool flag ) {
        setYFlip( i % width, i / width, flag );
    }    
    inline bool getYFlip( int x, int y ) {
        if( !yflip_table ) return false;
        return yflip_table[ index(x,y) ];
    }
	// 0~1. 1 for just a cell. not by whole texture
	inline void setTexOffset( int x, int y, Vec2 *v ) {
		ENSURE_GRID_TABLE( texofs_table, Vec2, Vec2(0,0) );
		int i = index(x,y);
		texofs_table[i].x = v->x;
		texofs_table[i].y = v->y;
        uv_changed = true;        
	}
    inline void setTexOffsetIndex( int ind, Vec2 *v ) {
        setTexOffset( ind % width, ind / width, v );
    }
    inline void getTexOffset( int x, int y, Vec2 *outv ) {
        if( !texofs_table ) {
            outv->x = outv->y = 0;
        } else {
            int i = index(x,y);
            outv->x = texofs_table[i].x;
            outv->y = texofs_table[i].y;
        }
    }
	inline void setUVRot( int x, int y, bool flag ){
		ENSURE_GRID_TABLE( rot_table, bool, false );
		rot_table[ index(x,y) ] = flag;
        uv_changed = true;        
	}
    inline void setUVRotIndex( int i, bool flag ) {
        setUVRot( i % width, i / width, flag );
    }
    inline bool getUVRot( int x, int y ) {
		ENSURE_GRID_TABLE( rot_table, bool, false );
		return rot_table[ index(x,y) ];
    }
	inline void setColor( int x, int y, Color col ) {
		ENSURE_GRID_TABLE( color_table, Color, Color(1,1,1,1) );
		color_table[ index(x,y) ] = col;
        color_changed = true;        
	}
    inline void setColorIndex( int ind, Color col ) {
        setColor( ind % width, ind / width, col );
    }
	inline Color getColor( int x, int y ) {
        if(!color_table) return Color(1,1,1,1);
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
