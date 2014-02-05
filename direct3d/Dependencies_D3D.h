#pragma once

#define DX_LIB_PATH "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, DX_LIB_PATH "xinput.lib") // Make sure we use XInput 1.3 even if we compile using VS2012 (which defaults to XInput 1.4 only supported on Win8).
