#pragma once

#include "Prop2D.h"
#include "Font.h"
#include "../freetype-gl/vertex-buffer.h"

class TextBox : public Prop2D {
public:
	wchar_t *str;
	Font *font;
    int len_str;
    Mesh *mesh;
    
	TextBox() : str(NULL), len_str(0), mesh(NULL) {
		setScl(1,1);
	}
    ~TextBox() {
        if(mesh) {
            mesh->deleteBuffers();
            delete mesh;
        }
    }

	inline void setFont( Font *f ){
		assert(f);
		font = f;
	}

	void render(Camera *cam, DrawBatchList *bl );

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
        clearMesh();
        updateMesh();
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
    void clearMesh() {
        if(mesh) {
            delete mesh;
            mesh = NULL;
        }
    }
    void updateMesh();
};
