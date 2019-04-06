#pragma once

#include "Grid.h"

class CharGrid : public Grid {
public:
	int ascii_offset;
    int hiragana_unicode_offset;
    int katakana_unicode_offset;
    int kigou_unicode_offset;
	CharGrid(int w, int h ) : Grid(w,h), ascii_offset(0), hiragana_unicode_offset(128), katakana_unicode_offset(224), kigou_unicode_offset(320) {}
	void printf( int x, int y, Color c, const char *fmt, ...);
};
