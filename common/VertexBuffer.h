#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/VertexBuffer_OGL.h"
	typedef VertexBuffer_OGL VertexBuffer;
#elif defined(USE_D3D)
	#include "../direct3d/VertexBuffer_D3D.h"
	typedef VertexBuffer_D3D VertexBuffer;
#endif