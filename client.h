#ifndef _MOYAI_CLIENT_H_
#define _MOYAI_CLIENT_H_

#define GLEW_STATIC

#ifdef WIN32
#include "GL/glew.h"
#define USE_OPENAL 1
#define ALUT_BUILD_LIBRARY
#endif

#if defined(__APPLE__)
#define USE_OPENAL 1
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
#include "OpenGLES/ES1/gl.h"
#include "OpenGLES/ES1/glext.h"
#elif TARGET_OS_IPHONE
#include "OpenGLES/ES1/gl.h"
#include "OpenGLES/ES1/glext.h"
#elif TARGET_OS_MAC
#include <OpenGL/gl.h>
#include <Glut/glut.h>
#else
#   error "Unknown Apple platform"
#endif
#endif

#if defined(__linux__)
#include "GLemu.h"
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
#include "Keyboard.h"
#include "Mouse.h"




#endif
