#pragma once

#if defined(_DEBUG)

	#if defined(USE_OPENGL)
		#include "../opengl/GPUMarker_OGL.h"
	#elif defined (USE_D3D)
		#include "../direct3d/GPUMarker_D3D.h"
	#endif

#else

	#define GPU_BEGIN_EVENT(msg)
	#define GPU_END_EVENT()

#endif

