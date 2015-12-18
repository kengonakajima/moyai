
#include "Prim_OGL.h"

Prim_OGL::Prim_OGL( PRIMTYPE t, Vec2 a, Vec2 b, Color c, int line_width ) : type(t), a(a),b(b), color(c), line_width(line_width), mesh(NULL) {
}
VertexFormat *Prim_OGL::vf_prim_common = NULL;
VertexFormat *Prim_OGL::getVertexFormat() {
    if(!vf_prim_common) {
        vf_prim_common = new VertexFormat();
        vf_prim_common->declareCoordVec3();
        vf_prim_common->declareColor();
    }
    return vf_prim_common;
}
void Prim_OGL::draw() {
    glLineWidth(line_width);
    if(!mesh) {
        mesh = new Mesh();
        VertexFormat *vf = getVertexFormat();
        VertexBuffer *vb = new VertexBuffer();
        vb->setFormat(vf);
        IndexBuffer *ib = new IndexBuffer();
        mesh->setVertexBuffer(vb);
        mesh->setIndexBuffer(ib);
        switch(type){
        case PRIMTYPE_LINE:
            vb->reserve(2);
            ib->reserve(2);
            vb->setCoord(0, Vec3(a.x,a.y,0) );
            vb->setCoord(1, Vec3(b.x,b.y,0) );
            vb->setColor(0, color );
            vb->setColor(1, color );
            ib->setIndex(0,0);
            ib->setIndex(1,1);
            mesh->setPrimType(GL_LINES);
        break;
        case PRIMTYPE_RECTANGLE:
            vb->reserve(4);
            ib->reserve(4);
            vb->setCoord(0, Vec3(a.x,a.y,0) ); // top left
            vb->setCoord(1, Vec3(b.x,a.y,0) ); // top right
            vb->setCoord(2, Vec3(b.x,b.y,0) ); // bottom right
            vb->setCoord(3, Vec3(a.x,b.y,0) ); // bottom left
            vb->setColor(0, color );
            vb->setColor(1, color );
            vb->setColor(2, color );
            vb->setColor(3, color );
            ib->setIndex(0,0);
            ib->setIndex(1,1);
            ib->setIndex(2,2);
            ib->setIndex(3,3);
            mesh->setPrimType(GL_QUADS);
            break;
        default:
            break;
        }
    }
    mesh->vb->bless();
    assert( mesh->vb->gl_name > 0 );        
    mesh->ib->bless();
    assert( mesh->ib->gl_name > 0 );

    int vert_sz = mesh->vb->fmt->getNumFloat() * sizeof(float);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->ib->gl_name );
    glBindBuffer( GL_ARRAY_BUFFER, mesh->vb->gl_name );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );

    if( mesh->vb->fmt->coord_offset >= 0 ){
        glEnableClientState( GL_VERTEX_ARRAY );        
        glVertexPointer( 3, GL_FLOAT, vert_sz, (char*)0 + mesh->vb->fmt->coord_offset * sizeof(float) );
    }
    if( mesh->vb->fmt->color_offset >= 0 ){
        glEnableClientState( GL_COLOR_ARRAY );
        glColorPointer( 4, GL_FLOAT, vert_sz, (char*)0 + mesh->vb->fmt->color_offset * sizeof(float));
    }

    glDrawElements( mesh->prim_type, mesh->ib->array_len, GL_UNSIGNED_INT, 0);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
};
