#pragma once


#ifdef WIN32
#include "GL/glew.h"
#endif

#include "../common/VertexBuffer.h"
#include "../common/IndexBuffer.h"

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
    GLuint program; // 0 for default
    VertexBuffer *vb; // contains all verts in this batch
    IndexBuffer *ib;
    int vert_used; // where to put next vertex
    int index_used;
    static const int MAXQUAD = 256;
    static const int MAXVERTEX = MAXQUAD*4;
    static const int MAXINDEX = MAXQUAD*6;
    DrawBatch( VFTYPE vft, GLuint tx, GLuint primtype, GLuint prog );
    ~DrawBatch() {
        delete vb;
        delete ib;
    };
    bool shouldContinue( VFTYPE vft, GLuint texid, GLuint primtype, GLuint prog );
    void draw();
    int hasVertexRoom( int n ) {
        return MAXVERTEX - vert_used;
    }
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
    DrawBatch *startNextBatch( VFTYPE vft, GLuint tex, GLuint primtype, GLuint prog );
    void drawAll();

    bool appendSprite1( GLuint program, GLuint tex, Color c, Vec2 tr, Vec2 scl, float radrot, Vec2 uv0, Vec2 uv1 );
};
