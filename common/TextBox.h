#pragma once

#if defined(USE_OPENGL)
	#include "../opengl/TextBox_OGL.h"
	typedef TextBox_OGL TextBox;
#elif defined(USE_D3D)
	#include "../direct3d/TextBox_D3D.h"
	typedef TextBox_D3D TextBox;
#endif