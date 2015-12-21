#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/DrawBatch_OGL.h"
	typedef DrawBatchList_OGL DrawBatchList;
#elif defined(USE_D3D)
	// not implemented
#endif
