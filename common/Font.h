#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/Font_OGL.h"
	typedef Font_OGL Font;
#elif defined(USE_D3D)
	#include "../direct3d/Font_D3D.h"
	typedef Font_D3D Font;
#endif