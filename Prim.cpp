#include "client.h"

#include "Viewport.h"
#include "Prim.h"

int Prim::idgen = 1;

Prim::Prim( PRIMTYPE t, Vec2 a, Vec2 b, Color c, int line_width ) : id(idgen++), type(t), a(a),b(b), color(c), line_width(line_width) {
}
void Prim::draw( Viewport *vp, DrawBatchList *bl, Vec2 tr, Vec2 scl, float radrot ) {
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



