#ifndef _MOYAI_CLIENT_H_
#define _MOYAI_CLIENT_H_

#define GLEW_STATIC

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#endif


// GFX API specific includes
#include "Viewport.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "FragmentShader.h"
#include "ColorReplacerShader.h"
#include "Prim.h"
#include "Prop3D.h"
#include "Prop2D.h"
#include "Layer.h"
#include "Font.h"
#include "TextBox.h"
#include "MoyaiClient.h"

// Common includes
#include "Animation.h"
#include "AnimCurve.h"
#include "Camera.h"
#include "CharGrid.h"
#include "Enums.h"
#include "Grid.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "Pad.h"
#include "PrimDrawer.h"
#include "Renderable.h"
#include "Sound.h"
#include "SoundSystem.h"
#include "TileDeck.h"
#include "VertexFormat.h"
#include "PerformanceCounter.h"

#if defined(_MSC_VER)
	#include "Dependencies.h"
#endif



#endif
