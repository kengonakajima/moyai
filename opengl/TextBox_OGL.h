#pragma once

#include "Prop2D_OGL.h"
#include "Font_OGL.h"
#include "../freetype-gl/vertex-buffer.h"

class TextBox_OGL : public Prop2D_OGL {
public:
	vertex_buffer_t *vb;
	wchar_t *str;
	Font_OGL *font;
	//    Color color;
    int len_str;

	TextBox_OGL() : len_str(0) {
		vb = vertex_buffer_new( "v3f:t2f:c4f" );
		str = NULL;
		setScl(1,1);
	}

	inline void setFont( Font_OGL *f ){
		assert(f);
		font = f;
	}

	void render(Camera *cam );

	inline void setString( const char *s ){
		setString( (char*) s );
	}
	inline void setString( char *u8s ){
		int l = strlen(u8s);
		wchar_t *out = (wchar_t*)MALLOC((l+1)*sizeof(wchar_t));
		mbstowcs(out, u8s, l+1 );
		setString(out);
		FREE(out);
	}

	inline void setString( const wchar_t *s ){
		setString( (wchar_t*)s );
	}
	inline void setString( wchar_t *s ){
		size_t l = wcslen(s);
		if(str){
			FREE(str);
		}
		str = (wchar_t*)MALLOC( (l+1) * sizeof(wchar_t) );
		wcscpy( str, s );
		assert( wcslen(str) == wcslen(s) );
        len_str = l;
	}
    bool compareString( const char *u8s ) {
        int l = strlen(u8s);
		wchar_t *out = (wchar_t*)MALLOC((l+1)*sizeof(wchar_t));
		mbstowcs(out, u8s, l+1 );
        int ret = wcscmp( str, out );
        FREE(out);
        //        print("compareString: %d '%S' '%S'",ret, str, out );
        return ret == 0;
    }
    int getStringLength() { return len_str; }
};
