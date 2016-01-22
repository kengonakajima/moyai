#pragma once

#include "common.h"
#include "Layer.h"
#include "DrawBatch.h"
#include "GLFW/glfw3.h"


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
