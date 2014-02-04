#include "Grid.h"

void Grid::clear(){
	for(int y=0;y<height;y++){
		for(int x=0;x<width;x++){
			set(x,y,GRID_NOT_USED);
		}
	}
}

void Grid::fillColor( Color c ) {
	for(int y=0;y<height;y++) {
		for(int x=0;x<width;x++) {
			setColor(x,y,c);
		}
	}
}