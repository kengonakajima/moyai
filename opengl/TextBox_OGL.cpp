#ifdef USE_OPENGL

#include "TextBox_OGL.h"

void TextBox_OGL::render(Camera *cam ) {
    if(!str) return;
    
	glBindTexture( GL_TEXTURE_2D, font->atlas->id );

	int line_num = 0;
	for(unsigned int i=0; i < wcslen(str); ++i ){
		if( str[i] == L"\n"[0] ){
			line_num++;
		}
	}

	size_t i;
	float x = loc.x;
	float y = loc.y + line_num * font->pixel_size;
	if(cam){
		x -= cam->loc.x;
		y -= cam->loc.y;
	}
	float basex = x;

	glBegin(GL_QUADS);
	glColor4f( color.r, color.g, color.b, color.a );

	float xscl = (float)scl.x / (float) font->pixel_size;
	float yscl = (float)scl.y / (float) font->pixel_size;
	int y_margin = (int)(font->pixel_size / 7.0f ); // suitable for ascii/japanese(CJK) mixed text
	for( i=0; i<wcslen(str); ++i ){
		if( str[i] == L"\n"[0] ){
			x = basex;
			y -= font->pixel_size * yscl;
			continue;
		}
		texture_glyph_t *glyph = texture_font_get_glyph( font->font, str[i] );
		if( glyph == NULL ) continue;

		int kerning = 0;
		if( i > 0){
			kerning = texture_glyph_get_kerning( glyph, str[i-1] );
		}
		//        print("ofsy: %d h:%d", glyph->offset_y, glyph->height );        
		x += kerning * xscl;
		int x0  = (int)( x + glyph->offset_x *yscl);
		int y0  = (int)( y + glyph->offset_y * xscl) + y_margin;
		int x1  = (int)( x0 + glyph->width * xscl );
		int y1  = (int)( y0 - glyph->height * yscl ) + y_margin;

		float s0 = glyph->s0;
		float t0 = glyph->t0;
		float s1 = glyph->s1;
		float t1 = glyph->t1;
		float depth = 10;
		glTexCoord2f(s0,t0); glVertex3i( x0,y0, depth );
		glTexCoord2f(s0,t1); glVertex3i( x0,y1, depth );
		glTexCoord2f(s1,t1); glVertex3i( x1,y1, depth );
		glTexCoord2f(s1,t0); glVertex3i( x1,y0, depth );

		x += glyph->advance_x * xscl;

	}
	glEnd();
}

#endif
