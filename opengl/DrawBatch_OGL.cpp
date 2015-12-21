#ifdef USE_OPENGL
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#endif


#include "../common/DrawBatch.h"


DrawBatch::DrawBatch( VFTYPE vft, GLuint tx, GLuint primtype, GLuint prog ) : tex(tx), prim_type(primtype), program(prog), vert_used(0), index_used(0) {
    VertexFormat *vf = getVertexFormat(vft);
    vb = new VertexBuffer();
    vb->setFormat(vf);
    vb->reserve(MAXVERTEX);
    ib = new IndexBuffer();
    ib->reserve(MAXINDEX);
}
bool DrawBatch::shouldContinue( VFTYPE vft, GLuint texid, GLuint primtype, GLuint prog ) {
    return (vft==vf_type && texid == tex && primtype == prim_type && prog == program );
}
void DrawBatch::pushVertices( int vnum, Color *colors, Vec3 *coords, Vec2 *uvs, int inum, int *inds) {
    for(int i=0;i<vnum;i++) {
        vb->setCoord(vert_used+i,coords[i]);
        vb->setColor(vert_used+i,colors[i]);
        vb->setUV(vert_used+i,uvs[i]);
    }
    vert_used += vnum;
    for(int i=0;i<inum;i++) {
        ib->setIndex(index_used+i,inds[i]);
    }
    index_used += inum;
}

VertexFormat *g_vf_coord_color = NULL, *g_vf_coord_color_tex = NULL;

VertexFormat *DrawBatch::getVertexFormat(VFTYPE t) {
    switch(t) {
    case VFTYPE_COORD_COLOR:
        if(!g_vf_coord_color) {
            g_vf_coord_color = new VertexFormat();
            g_vf_coord_color->declareCoordVec3();
            g_vf_coord_color->declareColor();
        }
        return g_vf_coord_color;
    case VFTYPE_COORD_COLOR_UV:
        if(!g_vf_coord_color_tex) {
            g_vf_coord_color_tex = new VertexFormat();
            g_vf_coord_color_tex->declareCoordVec3();
            g_vf_coord_color_tex->declareColor();
            g_vf_coord_color_tex->declareUV();
        }
        return g_vf_coord_color_tex;
    default:
        assertmsg(false, "invalid vftype");
    }
    return NULL;
}

void DrawBatch::draw() {
    print("DrawBatch::draw tex:%d ");    
    if(tex==0) {
        glDisable(GL_TEXTURE_2D);
    } else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture( GL_TEXTURE_2D, tex );        
    }

    glUseProgram(program);
    
    vb->bless();
    ib->bless();

    int vert_sz = vb->fmt->getNumFloat() * sizeof(float);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ib->gl_name );
    glBindBuffer( GL_ARRAY_BUFFER, vb->gl_name );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );

    // 以下prop3dからのコピペ、動いたら共通化する
    if( vb->fmt->coord_offset >= 0 ){
        glEnableClientState( GL_VERTEX_ARRAY );        
        glVertexPointer( 3, GL_FLOAT, vert_sz, (char*)0 + vb->fmt->coord_offset * sizeof(float) );
    }
    if( vb->fmt->color_offset >= 0 ){
        glEnableClientState( GL_COLOR_ARRAY );
        glColorPointer( 4, GL_FLOAT, vert_sz, (char*)0 + vb->fmt->color_offset * sizeof(float));
    }
    if( vb->fmt->texture_offset >= 0 ){
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );                    
        glTexCoordPointer( 2, GL_FLOAT, vert_sz, (char*)0 + vb->fmt->texture_offset * sizeof(float) );
    }
    if( vb->fmt->normal_offset >= 0 ) {
        glEnableClientState( GL_NORMAL_ARRAY );
        glNormalPointer( GL_FLOAT, vert_sz, (char*)0 + vb->fmt->normal_offset * sizeof(float) );
    }

    glDisable(GL_LIGHTING); // TODO: may be outside of this function
    glDisable(GL_LIGHT0);
        
    glLoadIdentity();    

    // TODO: apply camera    glTranslatef( loc.x, loc.y, 0 );
    //   glRotatef( rot * (180.0f/M_PI), 0,0,1);
    // TODO: apply viewport scaling   glScalef( scl.x, scl.y, 1 );

    glDrawElements( prim_type, index_used, GL_UNSIGNED_INT, 0);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
        
    if(program) {
        glUseProgram(0);
    }
    

}


//////////////////////

// One sprite : includes 2 triangles
// returns true if success
bool DrawBatchList_OGL::appendSprite1( GLuint program, GLuint tex, Color c, Vec2 tr, Vec2 scl, float radrot, Vec2 uv0, Vec2 uv1 ) {
    print("append: tr:%f,%f scl:%f,%f rot:%f", tr.x,tr.y,scl.x,scl.y,radrot);
    DrawBatch *b = getCurrentBatch();
    bool to_continue;
    if(!b) {
        to_continue = false;
    } else if( b->shouldContinue( VFTYPE_COORD_COLOR_UV, tex, GL_TRIANGLES, program ) ) {
        to_continue = true;
    } else if( b->hasVertexRoom(4) ) {
        to_continue = true;
    } else {
        to_continue = false;
    }
    if( !to_continue ) {
        print("starting new batch");
        b = startNextBatch(VFTYPE_COORD_COLOR_UV, tex, GL_TRIANGLES, program );
        if(!b) {
            print("DrawBatchList_OGL::appendSprite1: batch list size not enough!" );
            return false;
        }
    }
    
    // 頂点は4、三角形は2個なのでindexは6個
    // C --- D
    // |     |
    // A --- B
    
    Color cols[4] = { c,c,c,c };
    float basesize = 0.5;
    float ax = (-basesize)*scl.x, ay = (-basesize)*scl.y;
    float bx = (basesize)*scl.x, by = (-basesize)*scl.y;
    float cx = (-basesize)*scl.x, cy = (basesize)*scl.y;
    float dx = (basesize)*scl.x, dy = (basesize)*scl.y;
    Vec3 coords[4] = {
        Vec3(tr.x+ax,tr.y+ay,0),
        Vec3(tr.x+bx,tr.y+by,0),
        Vec3(tr.x+cx,tr.y+cy,0),
        Vec3(tr.x+dx,tr.y+dy,0) };
    Vec2 uvs[4] = {
        uv0,
        Vec2(uv1.x,uv0.y),
        Vec2(uv0.x,uv1.y),
        uv1
    };
    int inds[6] = {
        0,2,1, // ACB
        1,2,3, // BCD
    };
    
    b->pushVertices( 4, cols, coords,uvs, 6, inds );
    return true;
}

DrawBatchList_OGL::DrawBatchList_OGL() : used(0) {
    for(int i=0;i<MAXBATCH;i++) {
        batches[i] = NULL;
    }
};
void DrawBatchList_OGL::clear() {
    for(int i=0;i<used;i++) {
        delete batches[i];
        batches[i] = NULL;
    }
    used=0;
}
DrawBatch *DrawBatchList_OGL::getCurrentBatch() {
    return batches[used];
}
DrawBatch *DrawBatchList_OGL::startNextBatch( VFTYPE vft, GLuint tex, GLuint primtype, GLuint prog ) {
    assertmsg( used<MAXBATCH, "max draw batch. need tune" );
    DrawBatch *b = new DrawBatch(vft, tex, primtype, prog );
    batches[used] = b;
    used++;
    return b;
}
void DrawBatchList_OGL::drawAll() {
    for(int i=0;i<used;i++) {
        batches[i]->draw();
    }
}


#endif // USE_OPENGL
