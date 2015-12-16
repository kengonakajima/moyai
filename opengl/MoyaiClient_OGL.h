#pragma once

#include "../common.h"
#include "../common/Layer.h"
#include "GLFW/glfw3.h"

class MoyaiClient_OGL : public Moyai {
public:
    GLFWwindow *window;
	MoyaiClient_OGL(GLFWwindow *win) : Moyai(), window(win) {
	}
	int render();
	void capture( Image *img );
	void insertLayer( Layer *l ) {
		insertGroup( l );
	}
};
