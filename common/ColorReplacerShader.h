#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/ColorReplacerShader_OGL.h"
	typedef ColorReplacerShader_OGL ColorReplacerShader;
#elif defined (USE_D3D)
	#include "../direct3d/ColorReplacerShader_D3D.h"
	typedef ColorReplacerShader_D3D ColorReplacerShader;
#endif

