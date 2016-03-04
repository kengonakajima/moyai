#pragma once

#include "Viewport.h"
#include "Prim.h"
#include "DrawBatch.h"

class PrimDrawer {
public:
	Prim **prims;
	int prim_max;
	int prim_num;
	PrimDrawer() : prims(NULL), prim_max(0), prim_num(0) {}
	~PrimDrawer(){
		clear();
	}
	void ensurePrims();
	Prim *addLine(Vec2 from, Vec2 to, Color c, int width=1);
	Prim *addRect(Vec2 from, Vec2 to, Color c, int width=1 );
	void drawAll( DrawBatchList *bl, Viewport *vp, Vec2 tr, Vec2 scl, float radrot );
	void clear();
	void getMinMax( Vec2 *minv, Vec2 *maxv );
    Prim *getPrimById( int id );
};
