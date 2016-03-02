#pragma once

#include "freetype-gl/freetype-gl.h"

class Font {
public:
    static int idgen;
    int id;
	texture_font_t *font;
	texture_atlas_t *atlas;
	int pixel_size;
    char last_load_file_path[256];
    wchar_t charcode_table[4096];
    int charcode_table_used_num;
	Font( ){
        id = idgen++;
		font = NULL;
		atlas = texture_atlas_new( 1024, 1024, 1 );
        last_load_file_path[0] = '\0';
        memset( charcode_table, 0, sizeof(charcode_table) );
        charcode_table_used_num = 0;
	}
	bool loadFromTTF(const char *path, const wchar_t *codes, int pixelsz );
};
