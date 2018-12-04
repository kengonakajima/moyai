#pragma once

#include "common.h"
#include "Layer.h"
#include "DrawBatch.h"

#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#include "GLFWemu.h"
#endif

#ifdef __linux__
#include "GLFWemu.h"
#include "GLemu.h"
#endif

#if defined(__APPLE__) || defined(WIN32)
#include "GLFW/glfw3.h"                                                                                 
#endif

class MoyaiClient : public Moyai {
public:
    GLFWwindow *window;
    DrawBatchList batch_list;
    int last_draw_call_count;
    RemoteHead *remote_head;
    int window_width, window_height;
	MoyaiClient(GLFWwindow *win, int w, int h ) : Moyai(), window(win), last_draw_call_count(0), remote_head(0), window_width(w), window_height(h) {
	}
	int render(bool clear=true, bool swap_and_flush=true);
	void capture( Image *img );
	void insertLayer( Layer *l ) {
        int hprio = getHighestPriority();
        l->priority = hprio+1;
        l->parent_client = this;
		insertGroup( l );
	}
    int getHighestPriority();
    void setRemoteHead( RemoteHead *rh ) {
        remote_head = rh;
        rh->setWindowSize(window_width, window_height);
    }
    virtual int poll( double dt );
};

const char *platformCStringPath( const char *path );
