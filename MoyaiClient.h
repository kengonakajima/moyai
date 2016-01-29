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
	MoyaiClient(GLFWwindow *win) : Moyai(), window(win), last_draw_call_count(0) {
	}
	int render();
	void capture( Image *img );
	void insertLayer( Layer *l ) {
        l->parent_client = this;
		insertGroup( l );
	}
};
