#ifdef USE_D3D

#include "glfw_D3D.h"
#include "VertexBuffer_D3D.h"
#include "FragmentShader_D3D.h"
#include "ShaderManager_D3D.h"
#include "KeyBindings.h"
#include "../common.h"
#include "../common/GPUMarker.h"

#include <Xinput.h>

Context_D3D g_context;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_KEYUP:
		{
			KeyCode win32 = wParam;
			KeyCode glfw = KEY_Win32_GLFW[wParam];
			g_context.m_keyStates[glfw].glfwKeyCode = glfw;
			g_context.m_keyStates[glfw].win32KeyCode = win32;
			g_context.m_keyStates[glfw].isPressed = false;

			if (g_context.m_keyCallbackFunct)
			{
				g_context.m_keyCallbackFunct(glfw, GLFW_RELEASE);
			}

			return 0;
		}
	case WM_KEYDOWN:
		{
			KeyCode win32 = wParam;
			KeyCode glfw = KEY_Win32_GLFW[wParam];
			g_context.m_keyStates[glfw].glfwKeyCode = glfw;
			g_context.m_keyStates[glfw].win32KeyCode = win32;
			g_context.m_keyStates[glfw].isPressed = true;

			if (g_context.m_keyCallbackFunct)
			{
				g_context.m_keyCallbackFunct(glfw, GLFW_PRESS);
			}

			return 0;
		}
	case WM_MOUSEWHEEL:
		{
			g_context.m_mouseWheelDelta += GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			return 0;
		}
	case WM_QUIT:
	case WM_CLOSE:
		{
            if( g_context.m_windowCloseCallbackFunct ) g_context.m_windowCloseCallbackFunct();
			// This is not clean and should be changed.
			exit(0);
		}
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

namespace glfw_d3d
{
	int glfwInit( void )
	{
		assert(!g_context.m_hInstance && "glfwInit called twice");

		g_context.m_hInstance = (HINSTANCE)GetModuleHandle(nullptr);
		if (!g_context.m_hInstance)
		{
			assert(false && "Unable to retrieve application handle.");
			return 0;
		}

		WNDCLASSEX wndClass;
		::ZeroMemory(&wndClass, sizeof(WNDCLASSEX));
		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = WindowProc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = g_context.m_hInstance;
		wndClass.hIcon = LoadIcon(g_context.m_hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
		wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wndClass.lpszMenuName = nullptr;
		wndClass.lpszClassName = TEXT("GameWindow");
		wndClass.hIconSm = LoadIcon(g_context.m_hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

		if (!RegisterClassEx(&wndClass))
		{
			assert(false && "Enable to register WNDCLASS");
			return 0;
		}

		return 1;
	}

	void glfwTerminate( void )
	{
		SafeRelease(g_context.m_pDepthStencilView);
		SafeRelease(g_context.m_pRenderTargetView);
		SafeRelease(g_context.m_pDepthStencil);
		SafeRelease(g_context.m_pSwapChain);
		SafeRelease(g_context.m_pDeviceContext);
		SafeRelease(g_context.m_pDevice);

		SafeDelete(g_context.m_pShaderManager);

		SafeRelease(g_context.m_pNoDepthTestState);
		SafeRelease(g_context.m_pDepthStencilState);

		SafeRelease(g_context.m_pRasterizerState);
		SafeRelease(g_context.m_pNoCullingRasterizerState);

		#if GPU_DEBUG
			SafeDelete(g_context.m_pGPUMarker);
		#endif
	}

	void glfwEnable( int token )
	{
		
	}

	int glfwOpenWindow( int width, int height, int redbits, int greenbits, int bluebits, int alphabits, int depthbits, int stencilbits, int mode )
	{
		if (g_context.m_pDevice)
		{
			assert(false && "glfwOpenWindow called twice");
			return 0;
		}
		
		// Create window
		{
			RECT adjustedRect;
			SetRect(&adjustedRect, 0, 0, width, height);
			AdjustWindowRect(&adjustedRect, WS_OVERLAPPEDWINDOW, false);

			// TODO: Center window on screen.
			g_context.m_hWindowHandle = CreateWindow(
				TEXT("GameWindow"),		// Window class name
				TEXT(""),				// Window title
				WS_OVERLAPPEDWINDOW,	// Window style
				0,						// x position
				0,						// y position
				adjustedRect.right - adjustedRect.left,
				adjustedRect.bottom - adjustedRect.top,
				nullptr,				// Parent window handle
				nullptr,				// Window menu handle
				g_context.m_hInstance,
				nullptr					// Optional creation params
				);

			if (!g_context.m_hWindowHandle)
			{
				assert(false && "Unable to create window");
				return 0;
			}
		}

		// Create d3d device and swap chain
		{
			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

			DXGI_SWAP_CHAIN_DESC swapChainDesc;
			ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
			swapChainDesc.BufferDesc.Width = width;
			swapChainDesc.BufferDesc.Height = height;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferCount = 2;
			swapChainDesc.OutputWindow = g_context.m_hWindowHandle;
			swapChainDesc.Windowed = TRUE;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

			HRESULT hr = D3D11CreateDeviceAndSwapChain(
				nullptr,							// IDXGIAdapter (nullptr for default)
				D3D_DRIVER_TYPE_HARDWARE,// REFERENCE,			// Driver Type
				nullptr,							// Software renderer DLL handle
#if defined(DEBUG) || defined(_DEBUG)	// Flags
				D3D11_CREATE_DEVICE_DEBUG,
#else
				0,
#endif
				&featureLevel,						// Desired feature levels in order of preference
				1,									// Number of feature levels in desired list
				D3D11_SDK_VERSION,
				&swapChainDesc,
				&g_context.m_pSwapChain,
				&g_context.m_pDevice,
				&g_context.m_pFeatureLevel,
				&g_context.m_pDeviceContext
				);

			CheckFailure(hr, "Unable to create device and swap chain.");
		}

		// Create RTV on back buffer
		{
			ID3D11Texture2D *backbuffer;
			HRESULT hr = g_context.m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)(&backbuffer));
			CheckFailure(hr, "Unable to retrieve back buffer from swap chain.");
			hr = g_context.m_pDevice->CreateRenderTargetView(backbuffer, nullptr, &g_context.m_pRenderTargetView);
			CheckFailure(hr, "Unable to create render target view on back buffer.");
			backbuffer->Release();
		}

		// Create depth-stencil buffer and associated DSV
		{
			ID3D11Texture2D* depthStencil = nullptr;
			D3D11_TEXTURE2D_DESC descDepth;
			descDepth.Width = width;
			descDepth.Height = height;
			descDepth.MipLevels = 1;
			descDepth.ArraySize = 1;
			descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			descDepth.SampleDesc.Count = 1;
			descDepth.SampleDesc.Quality = 0;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			descDepth.CPUAccessFlags = 0;
			descDepth.MiscFlags = 0;

			HRESULT hr = g_context.m_pDevice->CreateTexture2D(&descDepth, nullptr, &g_context.m_pDepthStencil);
			CheckFailure(hr, "Unable to create depth-stencil buffer");

			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
			descDSV.Format = descDepth.Format;
			descDSV.Flags = 0;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0;
			hr = g_context.m_pDevice->CreateDepthStencilView(g_context.m_pDepthStencil, &descDSV, &g_context.m_pDepthStencilView);
			CheckFailure(hr, "Unable to create depth-stencil view");
		}

		// Create rasterizer states
		{
			D3D11_RASTERIZER_DESC desc;
			desc.FillMode = D3D11_FILL_SOLID;
			desc.CullMode = D3D11_CULL_BACK;
			desc.FrontCounterClockwise = FALSE;
			desc.DepthBias = 0;
			desc.SlopeScaledDepthBias = 0.0f;
			desc.DepthBiasClamp = 0.0f;
			desc.DepthClipEnable = TRUE;
			desc.ScissorEnable = FALSE;
			desc.MultisampleEnable = FALSE;
			desc.AntialiasedLineEnable = FALSE;

			g_context.m_pDevice->CreateRasterizerState(&desc, &g_context.m_pRasterizerState);
			g_context.m_pDeviceContext->RSSetState(g_context.m_pRasterizerState);

			desc.CullMode = D3D11_CULL_NONE;
			g_context.m_pDevice->CreateRasterizerState(&desc, &g_context.m_pNoCullingRasterizerState);
		}

		// Create depth-stencil states
		{
			// No depth test
			D3D11_DEPTH_STENCIL_DESC desc;
			ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
			desc.DepthEnable = FALSE;
			desc.StencilEnable = FALSE;
			g_context.m_pDevice->CreateDepthStencilState(&desc, &g_context.m_pNoDepthTestState);

			desc.DepthEnable = TRUE;
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
			g_context.m_pDevice->CreateDepthStencilState(&desc, &g_context.m_pDepthStencilState);
		}

		g_context.m_pDeviceContext->OMSetRenderTargets(1, &g_context.m_pRenderTargetView, g_context.m_pDepthStencilView);

		ShowWindow(g_context.m_hWindowHandle, SW_SHOW);
		UpdateWindow(g_context.m_hWindowHandle);

		// Create GPU Marker
		#if GPU_DEBUG
			g_context.m_pGPUMarker = new GPUMarker_D3D(g_context.m_pDeviceContext);
		#endif

		g_context.m_pShaderManager = new ShaderManager_D3D();

		return 1;
	}

	void glfwSetWindowTitle( const char *title )
	{
		SetWindowTextA(g_context.m_hWindowHandle, title);
	}

	void glfwGetWindowSize( int *width, int *height )
	{
		RECT windowRect;
		if (GetWindowRect(g_context.m_hWindowHandle, &windowRect))
		{
			*width = windowRect.right - windowRect.left;
			*height = windowRect.bottom - windowRect.top;
		}
	}
    void glfwSetWindowCloseCallback( GLFWwindowclosefun  cbfun )
    {
        g_context.m_windowCloseCallbackFunct = cbfun;
    }

	void glfwSwapInterval( int interval )
	{
		assert(interval >= 0 && "Swap interval must be >= 0");
		g_context.m_swapInterval = interval;
	}

	void glfwSwapBuffers( void )
	{
		HRESULT hr = g_context.m_pSwapChain->Present(g_context.m_swapInterval, 0);
		assert(SUCCEEDED(hr) && "Failed to present the swap chain");

		MSG msg;
		while (PeekMessage(&msg, g_context.m_hWindowHandle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void glfwSleep( double time )
	{
		DWORD timeMS = static_cast<DWORD>(time * 1000);
		Sleep(timeMS);
	}

	void glfwSetKeyCallback( GLFWkeyfun cbfun )
	{
		g_context.m_keyCallbackFunct = cbfun;
	}

	int glfwGetKey( int key )
	{
		return g_context.m_keyStates[key].isPressed ? GLFW_PRESS : GLFW_RELEASE;
	}

	Vec2 applyDeadZone(float x, float y)
	{
		static const unsigned int INPUT_DEAD_ZONE = 10000;

		Vec2 output;
		float magnitude = sqrt(x * x + y * y);
		
		if (magnitude < INPUT_DEAD_ZONE)
		{
			return Vec2(0.0f, 0.0f);
		}

		return Vec2(x, y);
	}

	int glfwGetJoystickPos( int joy, float *pos, int numaxes )
	{
		int assignedAxes = 0;
		ZeroMemory(pos, sizeof(float) * numaxes);

		if (joy < GLFW_JOYSTICK_5)
		{
			XINPUT_STATE state;
			if (XInputGetState(joy, &state) == ERROR_SUCCESS)
			{
				Vec2 leftStick = applyDeadZone((float)state.Gamepad.sThumbLX, (float)state.Gamepad.sThumbLY);
				Vec2 rightStick = applyDeadZone((float)state.Gamepad.sThumbRX, -(float)state.Gamepad.sThumbRY);

				if (numaxes > 0) { ++assignedAxes; pos[0] = leftStick.x / SHRT_MAX; }
				if (numaxes > 1) { ++assignedAxes; pos[1] = leftStick.y / SHRT_MAX; }
				if (numaxes > 2) { ++assignedAxes; pos[2] = rightStick.x / SHRT_MAX; }
				if (numaxes > 3) { ++assignedAxes; pos[3] = rightStick.y / SHRT_MAX; }

				return assignedAxes;
			}
		}
		
		return 0;
	}

	int glfwGetJoystickParam( int joy, int param )
	{
		switch (param)
		{
		case GLFW_PRESENT:
			{
				if (joy < GLFW_JOYSTICK_5)
				{
					XINPUT_STATE state;
					return XInputGetState(joy, &state) == ERROR_SUCCESS ? 1 : 0;
				}

				return 0;
			}
		case GLFW_AXES:
			{
				if (joy < GLFW_JOYSTICK_5)
				{
					XINPUT_STATE state;
					if (XInputGetState(joy, &state) == ERROR_SUCCESS)
					{
						return 4;
					}
				}

				return 0;
			}
		case GLFW_BUTTONS:
			{
				if (joy < GLFW_JOYSTICK_5)
				{
					XINPUT_STATE state;
					if (XInputGetState(joy, &state) == ERROR_SUCCESS)
					{
						return 14;
					}
				}

				return 0;
			}
		}
		
		return 0;
	}

	int glfwGetJoystickButtons( int joy, unsigned char *buttons, int numbuttons )
	{
		int assignedButtons = 0;
		ZeroMemory(buttons, numbuttons);

		if (joy < GLFW_JOYSTICK_5)
		{
			XINPUT_STATE state;
			if (XInputGetState(joy, &state) == ERROR_SUCCESS)
			{
				if (numbuttons > 1) { ++assignedButtons; buttons[1] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 1 : 0; }
				if (numbuttons > 2) { ++assignedButtons; buttons[2] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 1 : 0; }
				if (numbuttons > 3) { ++assignedButtons; buttons[3] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 1 : 0; }
				if (numbuttons > 4) { ++assignedButtons; buttons[4] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1 : 0; }
				if (numbuttons > 5) { ++assignedButtons; buttons[5] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1 : 0; }

				return assignedButtons;
			}
		}

		return 0;
	}

	int glfwGetMouseWheel( void )
	{
		return g_context.m_mouseWheelDelta;
	}

	void glClearColor(float r, float g, float b, float a)
	{
		g_context.m_clearColorRGBA[0] = r;
		g_context.m_clearColorRGBA[1] = g;
		g_context.m_clearColorRGBA[2] = b;
		g_context.m_clearColorRGBA[3] = a;
	}

	void glewInit()
	{

	}
}

#endif
