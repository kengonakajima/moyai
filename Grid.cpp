#include "client.h"

#include "Grid.h"

void (*g_moyai_grid_change_callback)( void *grid, GRID_CHANGE_TYPE t, int x, int y, void *data, size_t datasize );

int Grid::idgen = 1;

void Grid::clear(int ind){
	for(int y=0;y<height;y++){
		for(int x=0;x<width;x++){
			set(x,y,ind);
		}
	}
    uv_changed = true;
}

void Grid::fillColor( Color c ) {
	for(int y=0;y<height;y++) {
		for(int x=0;x<width;x++) {
			setColor(x,y,c);
		}
	}
    color_changed = true;    
}
