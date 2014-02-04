#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/Texture_OGL.h"
	typedef Texture_OGL Texture;
#elif defined(USE_D3D)
	#include "../direct3d/Texture_D3D.h"
	typedef Texture_D3D Texture;
#endif