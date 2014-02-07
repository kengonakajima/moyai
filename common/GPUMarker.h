#pragma once

#define GPU_DEBUG_ACTIVE (0)

#if defined(_DEBUG)
	#define GPU_DEBUG GPU_DEBUG_ACTIVE
#else
	#define GPU_DEBUG (0)
#endif

#if defined(_DEBUG) && GPU_DEBUG

	#if defined(USE_OPENGL)
		#include "../opengl/GPUMarker_OGL.h"
	#elif defined (USE_D3D)
		#include "../direct3d/GPUMarker_D3D.h"
	#endif

#else

	#define GPU_BEGIN_EVENT(msg)
	#define GPU_END_EVENT()

#endif

