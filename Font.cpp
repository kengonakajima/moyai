#include "cumino.h"

#include "Font.h"

int Font::idgen = 1;

bool Font::loadFromTTF(const char *path, const wchar_t *codes, int pixelsz ) {
    if(codes == NULL ) codes = charcode_table;
    
	pixel_size = pixelsz;
    if(!skip_actual_font_load) {
        const char *cpath = platformCStringPath(path);
        font = texture_font_new( atlas, cpath, pixelsz );
        if(!font){
            return false;
        }
        texture_font_load_glyphs( font, codes);
    }
    // for headless
    strncpy( last_load_file_path, path, sizeof(last_load_file_path) );
    if(codes) setCharCodes(codes);
    return true;
}
void Font::setCharCodes( const wchar_t *codes ) {
    charcode_table_used_num = wcslen(codes);
    size_t cpsz = charcode_table_used_num * sizeof(wchar_t);
    memcpy( charcode_table, codes, cpsz );
}

