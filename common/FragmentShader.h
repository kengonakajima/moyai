#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/FragmentShader_OGL.h"
	typedef FragmentShader_OGL FragmentShader;
#elif defined(USE_D3D)
	#include "../direct3d/FragmentShader_D3D.h"
	typedef FragmentShader_D3D FragmentShader;
#endif