#include "client.h"

#include "Viewport.h"
#include "DrawBatch.h"

bool DrawBatch::debug_batch_cond=false;

DrawBatch::DrawBatch( Viewport *vp, VFTYPE vft, GLuint tx, GLuint primtype, FragmentShader *fs, BLENDTYPE bt, int linew ) : vf_type(vft), tex(tx), prim_type(primtype), f_shader(fs), blend_type(bt), line_width(linew), vert_used(0), index_used(0), mesh(NULL), translate(0,0), scale(8,8), radrot(0), viewport(vp), perform_transform(false) {
    setupVBIB(NULL,vft);
}
void DrawBatch::setupVBIB( Mesh *copy_from,VFTYPE vft ) {
    vb = new VertexBuffer();
    ib = new IndexBuffer();        
    if(copy_from) {
        vb->setFormat(copy_from->vb->fmt);
        vb->reserve(copy_from->vb->array_len);
        vb->copyFromBuffer(copy_from->vb->buf,copy_from->vb->array_len);
        ib->reserve(copy_from->ib->array_len);
        ib->copyFromBuffer(copy_from->ib->buf,copy_from->ib->array_len);
        prim_type = copy_from->prim_type;
        index_used = copy_from->ib->render_len;
        vert_used = copy_from->vb->array_len;
    } else {
        VertexFormat *vf = getVertexFormat(vft);
        vb->setFormat(vf);        
        vb->reserve(MAXVERTEX);
        ib->reserve(MAXINDEX);
    }
}
DrawBatch::DrawBatch( Viewport *vp, FragmentShader *fs, BLENDTYPE bt, GLuint tx, Vec2 tr, Vec2 scl, float r, Mesh *m, bool copy_mesh ) : vf_type(VFTYPE_INVAL), tex(tx), prim_type(0), f_shader(fs), blend_type(bt), line_width(0), vb(NULL), ib(NULL), vert_used(0), index_used(0), translate(tr), scale(scl), radrot(r), viewport(vp), perform_transform(true) {
    if(copy_mesh) {
        setupVBIB(m);
        mesh = NULL;
    } else {
        mesh = m;
    }
}
bool DrawBatch::shouldContinue( Viewport *vp, VFTYPE vft, GLuint texid, GLuint primtype, FragmentShader *fs, BLENDTYPE bt, int linew  ) {
    bool can_continue=(viewport==vp && vft==vf_type && texid == tex && primtype == prim_type && f_shader == fs && blend_type == bt && line_width == linew );
    if(debug_batch_cond && !can_continue) print("shouldcont: v%d t%d(%d,%d) p%d f%d b%d", vft==vf_type,texid==tex,texid,tex,primtype==prim_type,f_shader==fs,blend_type==bt);
    return can_continue;
}
void DrawBatch::pushVertices( int vnum, Color *colors, Vec3 *coords, int inum, int *inds) {
    for(int i=0;i<vnum;i++) {
        vb->setCoord(vert_used+i,coords[i]);
        vb->setColor(vert_used+i,colors[i]);
    }
    for(int i=0;i<inum;i++) {
        ib->setIndex(index_used+i,vert_used + inds[i]);
    }
    vert_used += vnum;
    index_used += inum;    
}
void DrawBatch::pushVertices( int vnum, Color *colors, Vec3 *coords, Vec2 *uvs, int inum, int *inds) {
    for(int i=0;i<vnum;i++) {
        vb->setCoord(vert_used+i,coords[i]);
        vb->setColor(vert_used+i,colors[i]);
        vb->setUV(vert_used+i,uvs[i]);
    }
    for(int i=0;i<inum;i++) {
        ib->setIndex(index_used+i,vert_used + inds[i]);
    }
    vert_used += vnum;
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
    if(tex==0) {
        glDisable(GL_TEXTURE_2D);
    } else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture( GL_TEXTURE_2D, tex );        
    }

    if( f_shader) {
#if !(TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE || defined(__linux__) )
        glUseProgram(f_shader->program);
#endif        
        f_shader->updateUniforms();
    }

    if( blend_type == BLENDTYPE_ADD ) {
        glBlendFunc(GL_ONE, GL_ONE );
    } else if( blend_type == BLENDTYPE_SRC_ALPHA ){
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    } else {
        assertmsg(false, "invalid blend type:%d", blend_type );
    }
    if( prim_type == GL_LINES ) glLineWidth(line_width);

    VertexBuffer *vb_use = NULL;
    IndexBuffer *ib_use = NULL;
    
    if(vb) {
        vb_use = vb;
        ib_use = ib;
    } else if(mesh) {
        vb_use = mesh->vb;
        ib_use = mesh->ib;
    } else {
        assertmsg(false, "invalid draw batch" );
    }
    
    // normal 2D sprites
    vb_use->bless();
    ib_use->bless();

#if 0
    if( prim_type == GL_LINES ) {
        print("GL_LINES. vbdump:");
        vb_use->dump(2);
    }
#endif    
    
    int vert_sz = vb_use->fmt->getNumFloat() * sizeof(float);
#if !defined(__linux__)    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ib_use->gl_name );
    glBindBuffer( GL_ARRAY_BUFFER, vb_use->gl_name );    
#endif    
    
    // 以下prop3dからのコピペ、動いたら共通化する
    if( vb_use->fmt->coord_offset >= 0 ){
        glEnableClientState( GL_VERTEX_ARRAY );        
        glVertexPointer( 3, GL_FLOAT, vert_sz, (char*)0 + vb_use->fmt->coord_offset * sizeof(float) );
    } else {
        glDisableClientState( GL_VERTEX_ARRAY );        
    }
    if( vb_use->fmt->color_offset >= 0 ){
        glEnableClientState( GL_COLOR_ARRAY );
        glColorPointer( 4, GL_FLOAT, vert_sz, (char*)0 + vb_use->fmt->color_offset * sizeof(float));
    } else {
        glDisableClientState( GL_COLOR_ARRAY );        
    }
    if( vb_use->fmt->texture_offset >= 0 ){
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );                    
        glTexCoordPointer( 2, GL_FLOAT, vert_sz, (char*)0 + vb_use->fmt->texture_offset * sizeof(float) );
    } else {
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );        
    }
    if( vb_use->fmt->normal_offset >= 0 ) {
        glEnableClientState( GL_NORMAL_ARRAY );
        glNormalPointer( GL_FLOAT, vert_sz, (char*)0 + vb_use->fmt->normal_offset * sizeof(float) );
    } else {
        glDisableClientState( GL_NORMAL_ARRAY );
    }

    glDisable(GL_LIGHTING); // TODO: may be outside of this function
    glDisable(GL_LIGHT0);

    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);    

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //    print("vp:%p xy:%f,%f", viewport, viewport->scl.x, viewport->scl.y );
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#define glOrtho_platform glOrthof
#else
#define glOrtho_platform glOrtho
#endif
    glOrtho_platform( -viewport->scl.x/2, viewport->scl.x/2, -viewport->scl.y/2, viewport->scl.y/2,-100,100);  // center is always (0,0)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();        

        
    if(perform_transform) {
        // grid, textbox, prims, and 3d meshes        
        glTranslatef( translate.x, translate.y, 0 );
        // TODO: apply camera
        glRotatef( radrot * (180.0f/M_PI), 0,0,1);
        // TODO: apply viewport scaling
        glScalef( scale.x, scale.y, 1 );
    }
    if(mesh) {
        assert( mesh->ib );
        glDrawElements( mesh->prim_type, mesh->ib->render_len, INDEX_BUFFER_GL_TYPE, 0);
    } else {
        glDrawElements( prim_type, index_used, INDEX_BUFFER_GL_TYPE, 0);        
    }

    // clean
#if !defined(__linux__)    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
#endif
    
#if !(TARGET_IPHONE_SIMULATOR ||TARGET_OS_IPHONE || defined(__linux__))
    if(f_shader) glUseProgram(0);
#endif    
}

void DrawBatch::dump() {
    print("DrawBatch vft:%d tex:%d prim_type:%d shader:%p blend_type:%d line_w:%d vert_used:%d index_used:%d tr:%f,%f scl:%f,%f rot:%f",
          vf_type, tex, prim_type, f_shader, blend_type, line_width, vert_used, index_used, translate.x, translate.y, scale.x, scale.y, radrot );
    vb->dump(vert_used);
    ib->dump(index_used);        
}

//////////////////////

// One sprite : includes 2 triangles
// returns true if success
bool DrawBatchList::appendSprite1( Viewport *vp, FragmentShader *fs, BLENDTYPE bt, GLuint tex, Color c, Vec2 tr, Vec2 scl, float radrot, Vec2 lb, Vec2 lt, Vec2 rt, Vec2 rb ) {
    //    print("appendspr: tr:%f,%f scl:%f,%f rot:%f", tr.x,tr.y,scl.x,scl.y,radrot);
    DrawBatch *b = getCurrentBatch();
    bool to_continue = false;
    if( b && b->shouldContinue( vp, VFTYPE_COORD_COLOR_UV, tex, GL_TRIANGLES, fs, bt ) && b->hasVertexRoom(4) ) {
        to_continue = true;
    }
    if( !to_continue ) {
        b = startNextBatch( vp, VFTYPE_COORD_COLOR_UV, tex, GL_TRIANGLES, fs, bt );
        if(!b) {
            print("DrawBatchList_OGL::appendSprite1: can't start sprite batch");
            return false;
        }
    }
    
    // 4 vertices for 2 trigles. requires 6 indexes
    // C --- D
    // |     |
    // A --- B
    
    Color cols[4] = { c,c,c,c };
    float basesize = 0.5;
    float ax = (-basesize)*scl.x, ay = (-basesize)*scl.y;
    float bx = (basesize)*scl.x, by = (-basesize)*scl.y;
    float cx = (-basesize)*scl.x, cy = (basesize)*scl.y;
    float dx = (basesize)*scl.x, dy = (basesize)*scl.y;

    if(radrot!=0) {
        float rotcos= ::cos(radrot), rotsin = ::sin(radrot);
#define ZROTX(x,y) ( rotcos * (x) - rotsin * (y) )
#define ZROTY(x,y) ( rotsin * (x) + rotcos * (y) )
        float rotax = ZROTX(ax,ay), rotay = ZROTY(ax,ay);
        float rotbx = ZROTX(bx,by), rotby = ZROTY(bx,by);
        float rotcx = ZROTX(cx,cy), rotcy = ZROTY(cx,cy);
        float rotdx = ZROTX(dx,dy), rotdy = ZROTY(dx,dy);
        ax = rotax; ay = rotay;
        bx = rotbx; by = rotby;
        cx = rotcx; cy = rotcy;
        dx = rotdx; dy = rotdy;
    }
    
    Vec3 coords[4] = {
        Vec3(tr.x+ax,tr.y+ay,0),
        Vec3(tr.x+bx,tr.y+by,0),
        Vec3(tr.x+cx,tr.y+cy,0),
        Vec3(tr.x+dx,tr.y+dy,0) };
    Vec2 uvs[4] = {
        lb,
        rb,
        lt,
        rt
    };
    int inds[6] = {
        0,2,1, // ACB
        1,2,3, // BCD
    };
    
    b->pushVertices( 4, cols, coords,uvs, 6, inds );
    return true;
}
bool DrawBatchList::appendLine( Viewport *vp, Vec2 p0, Vec2 p1, Color col, Vec2 trans, Vec2 scl, float radrot, int linew ) {
    DrawBatch *b = getCurrentBatch();
    bool to_continue = false;
    if( b && b->shouldContinue( vp, VFTYPE_COORD_COLOR, 0, GL_LINES, NULL, BLENDTYPE_SRC_ALPHA, linew ) && b->hasVertexRoom(2) ) {
        to_continue = true;
    }
    if(!to_continue) {
        b = startNextBatch( vp, VFTYPE_COORD_COLOR, 0, GL_LINES, NULL, BLENDTYPE_SRC_ALPHA, linew );
        if(!b) {
            print("appendline: can't start new batch");
            return false;
        }
    }
    Vec2 p0r( p0.x*scl.x, p0.y*scl.y );
    Vec2 p1r( p1.x*scl.x, p1.y*scl.y );
    float rotcos= ::cos(radrot), rotsin = ::sin(radrot);    
    Vec3 coords[2] = {
        Vec3(trans.x+ZROTX(p0r.x,p0r.y), trans.y+ZROTY(p0r.x,p0r.y), 0),
        Vec3(trans.x+ZROTX(p1r.x,p1r.y), trans.y+ZROTY(p1r.x,p1r.y), 0)
    };
    int inds[2] = { 0,1 };
    Color cols[2] = { col, col };
    b->pushVertices( 2, cols, coords, 2, inds );
    return true;
}
bool DrawBatchList::appendRect( Viewport *vp, Vec2 p0, Vec2 p1, Color c, Vec2 trans, Vec2 scl, float radrot ) {
    DrawBatch *b = getCurrentBatch();
    bool to_continue = false;
    if( b && b->shouldContinue( vp, VFTYPE_COORD_COLOR, 0, GL_TRIANGLES, NULL, BLENDTYPE_SRC_ALPHA ) && b->hasVertexRoom(4) ) {
        to_continue = true;
    }
    if(!to_continue) {
        b = startNextBatch( vp, VFTYPE_COORD_COLOR, 0, GL_TRIANGLES, NULL, BLENDTYPE_SRC_ALPHA );
        if(!b) {
            print("appendline: can't start new batch");
            return false;
        }
    }
    float rotcos= ::cos(radrot), rotsin = ::sin(radrot);
    Vec2 tl(p0.x*scl.x,p0.y*scl.y); // top left
    Vec2 tr(p1.x*scl.x,p0.y*scl.y); // top right
    Vec2 br(p1.x*scl.x,p1.y*scl.y); // bottom right
    Vec2 bl(p0.x*scl.x,p1.y*scl.y);  // bottom left

    Vec3 coords[4] = {
        Vec3( trans.x + ZROTX(tl.x,tl.y), trans.y + ZROTY(tl.x,tl.y), 0 ),
        Vec3( trans.x + ZROTX(tr.x,tr.y), trans.y + ZROTY(tr.x,tr.y), 0 ),
        Vec3( trans.x + ZROTX(br.x,br.y), trans.y + ZROTY(br.x,br.y), 0 ),
        Vec3( trans.x + ZROTX(bl.x,bl.y), trans.y + ZROTY(bl.x,bl.y), 0 )
    };
    int inds[6] = { 0,1,2,0,2,3 };
    Color cols[4] = { c, c, c, c };
    b->pushVertices( 4, cols, coords, 6, inds );
    return true;
}
bool DrawBatchList::appendMesh( Viewport *vp, FragmentShader *fs, BLENDTYPE bt, GLuint tex, Vec2 tr, Vec2 scl, float radrot, Mesh *mesh, bool copy_mesh ) {
    DrawBatch *b = startNextMeshBatch( vp, fs, bt, tex, tr,scl,radrot,mesh,copy_mesh );
    if(!b) {
        print("appendMesh: can't start mesh batch!" );
        return false;
    }
    return true;
}
DrawBatchList::DrawBatchList() : used(0) {
    for(int i=0;i<MAXBATCH;i++) {
        batches[i] = NULL;
    }
};
void DrawBatchList::clear() {
    for(int i=0;i<used;i++) {
        delete batches[i];
        batches[i] = NULL;
    }
    used=0;
}
void DrawBatchList::dump() {
    for(int i=0;i<used;i++) {
        DrawBatch *b = batches[i];
        print("[%d] VF:%d Tex:%3d prim:%d blt:%d lw:%d vn:%d in:%d vp:%p sh:%p lc:%.1f,%.1f scl:%.1f,%.1f",
              i, b->vf_type, b->tex, b->prim_type, b->blend_type, b->line_width,
              b->vert_used, b->index_used,
              b->viewport, b->f_shader,
              b->translate.x, b->translate.y, b->scale.x, b->scale.y );
    }
}
DrawBatch *DrawBatchList::getCurrentBatch() {
    if(used==0) return NULL;
    return batches[used-1];
}
DrawBatch *DrawBatchList::startNextBatch( Viewport *vp, VFTYPE vft, GLuint tex, GLuint primtype, FragmentShader *fs, BLENDTYPE bt, int linew ) {
    assertmsg( used<MAXBATCH, "max draw batch (vftype). need tune" );
    DrawBatch *b = new DrawBatch( vp, vft, tex, primtype, fs, bt, linew );
    batches[used] = b;
    used++;
    return b;
}
DrawBatch *DrawBatchList::startNextMeshBatch( Viewport *vp, FragmentShader *fs, BLENDTYPE bt, GLuint tex, Vec2 tr, Vec2 scl, float radrot, Mesh *mesh, bool copy_mesh ) {
    assertmsg( used<MAXBATCH, "max draw batch (withshader). need tune" );
    DrawBatch *b = new DrawBatch( vp, fs, bt, tex, tr, scl, radrot, mesh, copy_mesh );
    batches[used] = b;
    used++;
    return b;
}
int DrawBatchList::drawAll() {
    for(int i=0;i<used;i++) {
        batches[i]->draw();
    }
    return used;
}



