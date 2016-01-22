#pragma once

#include "Grid.h"

class CharGrid : public Grid {
public:
	int ascii_offset;
	CharGrid(int w, int h ) : Grid(w,h), ascii_offset(0) {}
	void printf( int x, int y, Color c, const char *fmt, ...);
	void setAsciiOffset( int ofs ){ ascii_offset = ofs; }
};
