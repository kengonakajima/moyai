#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"

class Mesh 
{
public:
	VertexBuffer * vb;
	IndexBuffer *ib;
	unsigned int prim_type;

	bool transparent;
	float line_width;
	Mesh() : vb(0), ib(0), prim_type(0), transparent(false), line_width(1) {
	}
	void setVertexBuffer(VertexBuffer *b) { vb = b; }
	void setIndexBuffer(IndexBuffer *b ){ ib = b; }
	void setPrimType( unsigned int t) { prim_type = t; }
	Vec3 getCenter() { return vb->calcCenterOfCoords(); }
	void dump();
    void deleteBuffers() {
        if(vb) delete vb;
        if(ib) delete ib;
    }
    bool hasIndexesToRender() { return ib->render_len > 0; }
};
