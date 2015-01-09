#pragma once

#include "../common.h"
#include "../common/Layer.h"
#include "GL/glfw.h"

class MoyaiClient_OGL : public Moyai {
public:
	MoyaiClient_OGL() : Moyai() {
	}
	int render();
	void capture( Image *img );
	void insertLayer( Layer *l ) {
		insertGroup( l );
	}
};
