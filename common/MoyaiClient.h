#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/MoyaiClient_OGL.h"
	typedef MoyaiClient_OGL MoyaiClient;
#elif defined(USE_D3D)
	#include "../direct3d/MoyaiClient_D3D.h"
	typedef MoyaiClient_D3D MoyaiClient;
#endif