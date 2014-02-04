#ifdef USE_D3D

#include "Font_D3D.h"

bool Font_D3D::loadFromTTF(const char *path, const wchar_t *codes, int pixelsz ) 
{        
	/*
	pixel_size = pixelsz;
	font = texture_font_new( atlas, path, pixelsz );
	if(!font){
		return false;
	}
	texture_font_load_glyphs( font, codes);
	*/

	return true;
}

#endif