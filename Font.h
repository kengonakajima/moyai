#pragma once

#include "freetype-gl/freetype-gl.h"

class Font {
public:
	texture_font_t *font;
	texture_atlas_t *atlas;
	int pixel_size;
	Font( ){
		font = NULL;
		atlas = texture_atlas_new( 1024, 1024, 1 );
	}
	bool loadFromTTF(const char *path, const wchar_t *codes, int pixelsz );
};
