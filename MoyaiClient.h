#pragma once

#include "common.h"
#include "Layer.h"
#include "DrawBatch.h"

#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#include "GLFWiosemu.h"
#else
#include "GLFW/glfw3.h"
#endif

class MoyaiClient : public Moyai {
public:
    GLFWwindow *window;
    DrawBatchList batch_list;
    int last_draw_call_count;
    RemoteHead *remote_head;
	MoyaiClient(GLFWwindow *win) : Moyai(), window(win), last_draw_call_count(0), remote_head(0) {
	}
	int render();
	void capture( Image *img );
	void insertLayer( Layer *l ) {
        int hprio = getHighestPriority();
        l->priority = hprio;
        l->parent_client = this;
		insertGroup( l );
	}
    int getHighestPriority();
    void setRemoteHead( RemoteHead *rh ) { remote_head = rh; }
    virtual int poll( double dt );
};

const char *platformCStringPath( const char *path );
