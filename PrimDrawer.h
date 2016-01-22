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
	inline void ensurePrims(){
		if(!prims){
			prim_max = 64;
			if(!prims) prims = (Prim**) MALLOC( sizeof(Prim*) * prim_max );
			assert(prims);
			prim_num = 0;
		}
	}
	inline void addLine(Vec2 from, Vec2 to, Color c, int width=1){
		ensurePrims();
		assertmsg( prim_num <= prim_max, "too many prims" );
		prims[prim_num++] = new Prim( PRIMTYPE_LINE, from, to, c, width );
	}
	inline void addRect(Vec2 from, Vec2 to, Color c, int width=1 ){
		ensurePrims();
		assertmsg( prim_num <= prim_max, "too many prims" );        
		prims[prim_num++] = new Prim( PRIMTYPE_RECTANGLE, from, to, c, width );
	}

	inline void drawAll( DrawBatchList *bl, Viewport *vp, Vec2 tr, Vec2 scl, float radrot ){
		for(int i=0;i<prim_num;i++){
			prims[i]->draw(vp,bl,tr,scl,radrot);
		}
	}
	inline void clear(){
		if(prims){
			for(int i=0;i<prim_num;i++){
				delete prims[i];
			}
			FREE(prims);
		}
		prims = NULL;
		prim_num = 0;
		prim_max = 0;
	}
	void getMinMax( Vec2 *minv, Vec2 *maxv );
};
