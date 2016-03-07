#pragma once

#include "Viewport.h"
#include "Prim.h"
#include "DrawBatch.h"

class TrackerPrimDrawer;
class Prop2D;

class PrimDrawer {
public:
	Prim **prims;
	int prim_max;
	int prim_num;
    TrackerPrimDrawer *tracker;
	PrimDrawer() : prims(NULL), prim_max(0), prim_num(0), tracker(0) {}
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
    void onTrack( Prop2D *owner, RemoteHead *rh );
    void ensurePrim( Prim *p );
    void addPrim( Prim *p );
    void deletePrim( int prim_id );
};
