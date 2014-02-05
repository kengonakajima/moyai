#pragma once

#pragma comment(lib, "freetype2410_D.lib")
#pragma comment(lib, "fmodex_vc.lib")
#pragma comment(lib, "wsock32.lib")

#if defined(USE_OPENGL)
	#include "../opengl/Dependencies_OGL.h"
#elif defined(USE_D3D)
	#include "../direct3d/Dependencies_D3D.h"
#endif