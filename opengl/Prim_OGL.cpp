
#include "Prim_OGL.h"

Prim_OGL::Prim_OGL( PRIMTYPE t, Vec2 a, Vec2 b, Color c, int line_width ) : type(t), a(a),b(b), color(c), line_width(line_width) {
}
void Prim_OGL::draw( DrawBatchList *bl, Vec2 tr, Vec2 scl, float radrot ) {
    switch(type) {
    case PRIMTYPE_LINE:
        bl->appendLine(a,b,color,tr,scl,radrot);
        break;
    case PRIMTYPE_RECTANGLE:
        bl->appendRect(a,b,color,tr,scl,radrot);
        break;
    default:
        break;
    }
};
