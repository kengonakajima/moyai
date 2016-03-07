#include "client.h"

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

Prim *PrimDrawer::getPrimById( int id ) {
    for(int i=0;i<prim_num;i++) {
        Prim *p = prims[i];
        if(p->id == id) return p;
    }
    return NULL;
}
void PrimDrawer::ensurePrims(){
    if(!prims){
        prim_max = 64;
        if(!prims) prims = (Prim**) MALLOC( sizeof(Prim*) * prim_max );
        assert(prims);
        prim_num = 0;
    }
}
Prim *PrimDrawer::addLine(Vec2 from, Vec2 to, Color c, int width){
    ensurePrims();
    assertmsg( prim_num <= prim_max, "too many prims" );
    Prim *p = new Prim( PRIMTYPE_LINE, from, to, c, width );
    prims[prim_num++] = p;
    return p;
}
Prim *PrimDrawer::addRect(Vec2 from, Vec2 to, Color c, int width ){
    ensurePrims();
    assertmsg( prim_num <= prim_max, "too many prims" );
    Prim *p = new Prim( PRIMTYPE_RECTANGLE, from, to, c, width );
    prims[prim_num++] = p;
    return p;
}

void PrimDrawer::drawAll( DrawBatchList *bl, Viewport *vp, Vec2 tr, Vec2 scl, float radrot ){
    for(int i=0;i<prim_num;i++){
        prims[i]->draw(vp,bl,tr,scl,radrot);
    }
}
void PrimDrawer::clear(){
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
void PrimDrawer::deletePrim( int prim_id ) {
    Prim *prim = getPrimById(prim_id);
    if(!prim) return;
    int found_index = -1;
    for(int i=0;i<prim_num;i++) {
        //        prt("PT:%d:ID:%d ", prims[i]->type, prims[i]->id );
        if( prims[i]->id == prim_id ) {
            delete prims[i];
            found_index = i;
            break;
        }
    }
    //    print("found_index:%d prim_num:%d", found_index, prim_num );
    assert( found_index >= 0 );
    for(int i=found_index;i<prim_num-1;i++) {
        prims[i] = prims[i+1];
    }
    prim_num--;
    prims[prim_num] = NULL;
}
void PrimDrawer::onTrack( Prop2D *owner, RemoteHead *rh ) {
    if(!tracker) {
        tracker = new TrackerPrimDrawer(rh,this);
    }
    tracker->scanPrimDrawer();
    tracker->broadcastDiff(owner,rh->listener, false );
    tracker->flipCurrentBuffer();        
}

// 
void PrimDrawer::ensurePrim( Prim *p ) {
    Prim *existing = getPrimById(p->id);
    if(existing) return;
    addPrim(p);
}
void PrimDrawer::addPrim( Prim *p ) {
    ensurePrims();
    assertmsg( prim_num <= prim_max, "too many prims" );
    prims[prim_num++] = p;        
}
