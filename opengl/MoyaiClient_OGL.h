#pragma once

#include "../common.h"
#include "../common/Layer.h"
#include "../common/DrawBatch.h"
#include "GLFW/glfw3.h"


class MoyaiClient_OGL : public Moyai {
public:
    GLFWwindow *window;
    DrawBatchList batch_list;
    int last_draw_call_count;
	MoyaiClient_OGL(GLFWwindow *win) : Moyai(), window(win), last_draw_call_count(0) {
	}
	int render();
	void capture( Image *img );
	void insertLayer( Layer *l ) {
        l->parent_client = this;
		insertGroup( l );
	}
};
