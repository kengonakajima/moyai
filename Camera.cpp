#include "Camera.h"
#include "Remote.h"
#include "client.h"
#include "Layer.h"

int Camera::id_gen = 1;

Camera::Camera(Client *cl) : tracker(0), remote_client(cl) {
    if(cl) {
        if( cl->target_camera) {
            assertmsg(false,"client already have target_camera set");
        }
        cl->target_camera = this;
    }
    id = id_gen++;
}

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
        print("Camera::addTargetLayer:warning: layer %d already added", to_add->id );
        return;
    }
    target_layers.set(to_add->id,to_add);
}
void Camera::delTargetLayer(Layer *to_del) {
    Layer *l = target_layers.get(to_del->id);
    if(!l) {
        print("Camera::delTargetLayer:warning: layer %d not found", to_del->id );
        return;
    }
    target_layers.del(to_del->id);
}
void Camera::adjustInsideDisplay( Vec2 scrsz, Vec2 area_min, Vec2 area_max, float zoom_rate ) {        
    float xsz = scrsz.x / 2 / zoom_rate;
    float ysz = scrsz.y / 2 / zoom_rate;
    float left = area_min.x + xsz;
    if( loc.x < left ) {
        loc.x = left;
    }
    float right = area_max.x - xsz;
    if( loc.x > right ) {
        loc.x = right;
    }

    float bottom = area_min.y + ysz;
    if( loc.y < bottom ) {
        loc.y = bottom;
    }
    float top = area_max.y - ysz;
    if( loc.y > top ) {
        loc.y = top;
    }
}
