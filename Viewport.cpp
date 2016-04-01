#include "Viewport.h"
#include "Remote.h"

int Viewport::id_gen = 1;

void Viewport::setSize(int scrw, int scrh ) {
	screen_width = scrw;
	screen_height = scrh;
}
void Viewport::setScale2D( float sx, float sy ){
	dimension = DIMENSION_2D;
	scl = Vec3(sx,sy,1);

}
void Viewport::setClip3D( float neardist, float fardist ) {        
	near_clip = neardist;
	far_clip = fardist;
	dimension = DIMENSION_3D;
}

void Viewport::getMinMax( Vec2 *minv, Vec2 *maxv ){
	minv->x = -scl.x/2;
	maxv->x = scl.x/2;
	minv->y = -scl.y/2;
	maxv->y = scl.y/2;
}

void Viewport::onTrack( RemoteHead *rh ) {
    if(!tracker) {
        tracker = new TrackerViewport(rh,this);
    }
    tracker->scanViewport();
    tracker->broadcastDiff(false);
    tracker->flipCurrentBuffer();
}
