#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/Viewport_OGL.h"
	typedef Viewport_OGL Viewport;
#elif defined(USE_D3D)
	#include "../direct3d/Viewport_D3D.h"
	typedef Viewport_D3D Viewport;
#endif