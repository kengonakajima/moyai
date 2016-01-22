#ifdef USE_OPENGL

#include "../common/Viewport.h"
#include "Prim_OGL.h"

Prim_OGL::Prim_OGL( PRIMTYPE t, Vec2 a, Vec2 b, Color c, int line_width ) : type(t), a(a),b(b), color(c), line_width(line_width) {
}
void Prim_OGL::draw( Viewport *vp, DrawBatchList *bl, Vec2 tr, Vec2 scl, float radrot ) {
    switch(type) {
    case PRIMTYPE_LINE:
        bl->appendLine(vp,a,b,color,tr,scl,radrot,line_width);
        break;
    case PRIMTYPE_RECTANGLE:
        bl->appendRect(vp,a,b,color,tr,scl,radrot);
        break;
    default:
        break;
    }
};


#endif
