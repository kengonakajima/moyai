#pragma once

#define _XM_NO_INTRINSICS_

#include <stdio.h>
#include <assert.h>
#include <unordered_map>
#include <d3d11.h>

#include "Vertex_D3D.h"
#include "../common/PerformanceCounter.h"

class FragmentShader_D3D;
class VertexBuffer_D3D;

typedef void (*GLFWkeyfun)(int, int);

moyai_align(16) struct CBufferMVP
{
	CBufferMVP(const XMMATRIX &modelView, const XMMATRIX &projection)
		: ModelView(modelView)
		, Projection(projection)
	{
		MVP = ModelView * Projection;
	}

	XMMATRIX ModelView;
	XMMATRIX Projection;
	XMMATRIX MVP;
};

typedef int KeyCode;
struct KeyState
{
	KeyState() : win32KeyCode(0), glfwKeyCode(0), isPressed(false) {}

	KeyCode win32KeyCode;
	KeyCode glfwKeyCode;
	bool isPressed;
};

struct Context_D3D
{
	Context_D3D()
		: m_pSwapChain(nullptr)
		, m_pDevice(nullptr)
		, m_pFeatureLevel(D3D_FEATURE_LEVEL_11_0)
		, m_pDeviceContext(nullptr)
		, m_pRenderTargetView(nullptr)
		, m_pDepthStencil(nullptr)
		, m_hInstance(nullptr)
		, m_hWindowHandle(nullptr)
		, m_swapInterval(0) 
		, m_keyCallbackFunct(nullptr)
		, m_mouseWheelDelta(0)
	{
		m_clearColorRGBA[0] = 0.0f;
		m_clearColorRGBA[1] = 0.0f;
		m_clearColorRGBA[2] = 0.0f;
		m_clearColorRGBA[3] = 1.0f;
	}

	IDXGISwapChain *m_pSwapChain;
	ID3D11Device *m_pDevice;
	D3D_FEATURE_LEVEL m_pFeatureLevel;
	ID3D11DeviceContext *m_pDeviceContext;
	ID3D11RenderTargetView *m_pRenderTargetView;
	ID3D11Texture2D *m_pDepthStencil;
	ID3D11DepthStencilView *m_pDepthStencilView;

	ID3D11DepthStencilState *m_pNoDepthTestState;
	ID3D11DepthStencilState *m_pDepthStencilState;

	FragmentShader_D3D *m_pDefaultShader;

	HINSTANCE m_hInstance;
	HWND m_hWindowHandle;

	int m_swapInterval;
	float m_clearColorRGBA[4];

	GLFWkeyfun m_keyCallbackFunct;
	std::unordered_map<KeyCode, KeyState> m_keyStates;
	int m_mouseWheelDelta;
};

extern Context_D3D g_context;

