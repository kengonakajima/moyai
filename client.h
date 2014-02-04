#ifndef _MOYAI_CLIENT_H_
#define _MOYAI_CLIENT_H_


#if !defined(USE_OPENGL) && !defined(USE_D3D) || defined(USE_OPENGL) && defined(USE_D3D)
	#error You have to define either USE_OPENGL or USE_D3D
#endif

#if defined(USE_OPENGL)
	#define GLEW_STATIC
#endif

// GFX API specific includes
#include "common/Viewport.h"
#include "common/VertexBuffer.h"
#include "common/IndexBuffer.h"
#include "common/FragmentShader.h"
#include "common/ColorReplacerShader.h"
#include "common/Prim.h"
#include "common/Prop3D.h"
#include "common/Prop2D.h"
#include "common/Layer.h"
#include "common/Font.h"
#include "common/TextBox.h"
#include "common/MoyaiClient.h"

// Common includes
#include "common/Animation.h"
#include "common/AnimCurve.h"
#include "common/Camera.h"
#include "common/CharGrid.h"
#include "common/Enums.h"
#include "common/Grid.h"
#include "common/Light.h"
#include "common/Material.h"
#include "common/Mesh.h"
#include "common/Pad.h"
#include "common/PrimDrawer.h"
#include "common/Renderable.h"
#include "common/Sound.h"
#include "common/SoundSystem.h"
#include "common/TileDeck.h"
#include "common/VertexFormat.h"

#if defined(_MSC_VER)
	#include "common/Dependencies.h"
#endif

#if defined(USE_D3D)
 using namespace glfw_d3d;
#endif

#endif