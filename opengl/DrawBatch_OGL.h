#pragma once


#ifdef WIN32
#include "GL/glew.h"
#endif

#include "../common/VertexBuffer.h"
#include "../common/IndexBuffer.h"
#include "../common/Mesh.h"
#include "../common/FragmentShader.h"

typedef enum {
    BLENDTYPE_INVAL = 0,
    BLENDTYPE_SRC_ALPHA = 1,
    BLENDTYPE_ADD = 2,
} BLENDTYPE;

typedef enum {
    VFTYPE_INVAL = 0,
    VFTYPE_COORD_COLOR = 1,
    VFTYPE_COORD_COLOR_UV = 2,
} VFTYPE;

class DrawBatch {
public:
    VFTYPE vf_type;
    GLuint tex; // 0 for non-text
    GLuint prim_type; 
    FragmentShader *f_shader; // 0 for default
    BLENDTYPE blend_type;
    int line_width; // used only when GL_LINES
    VertexBuffer *vb; // contains all verts in this batch
    IndexBuffer *ib;
    int vert_used; // where to put next vertex
    int index_used;
    
    Mesh *mesh; // overrides ib,vb,prim_type,vf_type
    Vec2 translate, scale; // used only when drawing mesh
    float radrot;
    
    static const int MAXQUAD = 256;
    static const int MAXVERTEX = MAXQUAD*4;
    static const int MAXINDEX = MAXQUAD*6;
    DrawBatch( VFTYPE vft, GLuint tx, GLuint primtype, FragmentShader *fs, BLENDTYPE bt, int linew = 1 );
    DrawBatch( FragmentShader *fs, BLENDTYPE bt, GLuint tx, Vec2 translate, Vec2 scale, float r, Mesh *m );
    ~DrawBatch() {
        delete vb;
        delete ib;
    };
    bool shouldContinue( VFTYPE vft, GLuint texid, GLuint primtype, FragmentShader *fs, BLENDTYPE bt, int linew = 1);
    void draw();
    int hasVertexRoom( int n ) {
        return MAXVERTEX - vert_used;
    }
    void pushVertices( int vnum, Color *colors, Vec3 *coords, int inum, int *inds);    
    void pushVertices( int vnum, Color *colors, Vec3 *coords, Vec2 *uvs, int inum, int *inds);
    static VertexFormat *getVertexFormat(VFTYPE t);
};


class DrawBatchList_OGL {
public:
    static const int MAXBATCH = 1024;
    int used;
    DrawBatch *batches[MAXBATCH];
    DrawBatchList_OGL();
    void clear();
    DrawBatch *getCurrentBatch();
    DrawBatch *startNextBatch( VFTYPE vft, GLuint tex, GLuint primtype, FragmentShader *fs, BLENDTYPE bt, int linew = 1 );
    DrawBatch *startNextMeshBatch( FragmentShader *fs, BLENDTYPE bt, GLuint tex, Vec2 tr, Vec2 scl, float radrot, Mesh *mesh );
    int drawAll();

    bool appendSprite1( FragmentShader *fs, BLENDTYPE bt, GLuint tex, Color c, Vec2 tr, Vec2 scl, float radrot, Vec2 uv0, Vec2 uv1 );

    bool appendMesh( FragmentShader *fs, BLENDTYPE bt, GLuint tex, Vec2 tr, Vec2 scl, float radrot, Mesh *mesh );
    bool appendLine( Vec2 a, Vec2 b, Color c, Vec2 tr, Vec2 scl, float radrot, int linew );
    bool appendRect( Vec2 a, Vec2 b, Color c, Vec2 tr, Vec2 scl, float radrot );
};
