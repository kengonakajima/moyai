#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/Prop3D_OGL.h"
	typedef Prop3D_OGL Prop3D;
#elif defined(USE_D3D)
	#include "../direct3d/Prop3D_D3D.h"
	typedef Prop3D_D3D Prop3D;
#endif