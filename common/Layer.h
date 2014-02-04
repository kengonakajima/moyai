#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/Layer_OGL.h"
	typedef Layer_OGL Layer;
#elif defined(USE_D3D)
	#include "../direct3d/Layer_D3D.h"
	typedef Layer_D3D Layer;
#endif