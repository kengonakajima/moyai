#include "PrimDrawer.h"

void PrimDrawer::getMinMax( Vec2 *minv, Vec2 *maxv ) {
	*minv = Vec2(0,0);
	*maxv = Vec2(0,0);

	for(int i=0;i<prim_num;i++){
		Prim *prm = prims[i];
		if( prm->a.x < minv->x ) minv->x = prm->a.x;
		if( prm->a.y < minv->y ) minv->y = prm->a.y;
		if( prm->a.x > maxv->x ) maxv->x = prm->a.x;
		if( prm->a.y > maxv->y ) maxv->y = prm->a.y;

		if( prm->b.x < minv->x ) minv->x = prm->b.x;
		if( prm->b.y < minv->y ) minv->y = prm->b.y;
		if( prm->b.x > maxv->x ) maxv->x = prm->b.x;
		if( prm->b.y > maxv->y ) maxv->y = prm->b.y;
	}
}