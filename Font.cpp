#include "cumino.h"

#include "Font.h"

bool Font::loadFromTTF(const char *path, const wchar_t *codes, int pixelsz ) {        
	pixel_size = pixelsz;
    const char *cpath = platformCStringPath(path);
	font = texture_font_new( atlas, cpath, pixelsz );
	if(!font){
		return false;
	}
	texture_font_load_glyphs( font, codes);
	return true;
}

