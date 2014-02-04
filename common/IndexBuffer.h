#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/IndexBuffer_OGL.h"
	typedef IndexBuffer_OGL IndexBuffer;
#elif defined(USE_D3D)
	#include "../direct3d/IndexBuffer_D3D.h"
	typedef IndexBuffer_D3D IndexBuffer;
#endif