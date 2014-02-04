#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/Prop2D_OGL.h"
	typedef Prop2D_OGL Prop2D;
#elif defined(USE_D3D)
	#include "../direct3d/Prop2D_D3D.h"
	typedef Prop2D_D3D Prop2D;
#endif