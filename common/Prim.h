#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/Prim_OGL.h"
	typedef Prim_OGL Prim;
#elif defined(USE_D3D)
	#include "../direct3d/Prim_D3D.h"
	typedef Prim_D3D Prim;
#endif