#include "Camera.h"
#include "Remote.h"
#include "client.h"
#include "Layer.h"

int Camera::id_gen = 1;

void Camera::screenToGL( int scr_x, int scr_y, int scrw, int scrh, Vec2 *out ) {    
	out->x = scr_x - scrw/2;
	out->y = scr_y - scrh/2;
	out->y *= -1;
}

Vec2 Camera::screenToWorld( int scr_x, int scr_y, int scr_w, int scr_h ) {
	Vec2 glpos;
	screenToGL( scr_x, scr_y, scr_w, scr_h, & glpos );
	return glpos + Vec2(loc.x,loc.y);
}
void Camera::onTrack( RemoteHead *rh ) {
    if(!tracker) {
        tracker = new TrackerCamera(rh,this);
    }
    tracker->scanCamera();
    tracker->broadcastDiff(false);
    tracker->flipCurrentBuffer();
}
void Camera::onTrackDynamic() {
    if(!remote_client) return;
    //    print("onTrackDynamic: clid:%d layersize:%d",remote_client->id, target_layers.size() );
    if(!tracker) {
        tracker = new TrackerCamera(NULL,this);
        tracker->unicastCreate(remote_client);
    }
    tracker->scanCamera();
    tracker->unicastDiff(remote_client,false);
    tracker->flipCurrentBuffer();
}
void Camera::addTargetLayer(Layer *to_add) {
    Layer *l = target_layers.get(to_add->id);
    if(l) {
        print("addTargetLayer:warning: layer %d already added", to_add->id );
        return;
    }
    target_layers.set(to_add->id,to_add);
}
void Camera::delTargetLayer(Layer *to_del) {
    Layer *l = target_layers.get(to_del->id);
    if(!l) {
        print("delTargetLayer:warning: layer %d not found", to_del->id );
        return;
    }
    target_layers.del(to_del->id);
}
