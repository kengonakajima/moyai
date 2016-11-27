#include "Viewport.h"
#include "Remote.h"
#include "client.h"
#include "Layer.h"

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
void Viewport::addTargetLayer(Layer *to_add) {
    Layer *l = target_layers.get(to_add->id);
    if(l) {
        print("Viewport::addTargetLayer:warning: layer %d already added", to_add->id );
        return;
    }
    target_layers.set(to_add->id,to_add);
    
}
void Viewport::onTrackDynamic() {
    if(!remote_client) return;
    //    print("Viewport::onTrackDynamic: clid:%d layersize:%d",remote_client->id, target_layers.size() );
    if(!tracker) {
        tracker = new TrackerViewport(NULL,this);
        tracker->unicastCreate(remote_client);
    }
    tracker->scanViewport();
    tracker->unicastDiff(remote_client,false);
    tracker->flipCurrentBuffer();
}
