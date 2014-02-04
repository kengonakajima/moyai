#pragma once

#include "../common.h"
#include "../common/Enums.h"
#include "GL/glew.h"

class Prim_OGL {
public:
	PRIMTYPE type;
	Vec2 a,b;
	Color color;
	int line_width;
	inline Prim_OGL( PRIMTYPE t, Vec2 a, Vec2 b, Color c, int line_width = 1 ) : type(t), a(a),b(b), color(c), line_width(line_width) {
	}
	inline void draw(Vec2 ofs){
		glDisable( GL_TEXTURE_2D );
		glLineWidth(line_width);
		switch(type){
		case PRIMTYPE_LINE:
			glBegin( GL_LINES );
			glColor4f( color.r, color.g, color.b, color.a );
			glVertex2f( ofs.x + a.x, ofs.y + a.y );
			glVertex2f( ofs.x + b.x, ofs.y + b.y );
			glEnd();
			break;
		case PRIMTYPE_RECTANGLE:
			glBegin( GL_QUADS );
			glColor4f( color.r, color.g, color.b, color.a );
			glVertex2f( ofs.x + a.x, ofs.y + a.y ); // top left
			glVertex2f( ofs.x + b.x, ofs.y + a.y ); // top right
			glVertex2f( ofs.x + b.x, ofs.y + b.y ); // bottom right
			glVertex2f( ofs.x + a.x, ofs.y + b.y ); // bottom left
			glEnd();
			break;
		default:
			break;
		}
	}
};