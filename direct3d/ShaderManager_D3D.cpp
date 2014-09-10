#include "ShaderManager_D3D.h"
#include "ShaderCode_D3D.h"
#include "../cumino.h"
#include <d3dcompiler.h>


ShaderManager_D3D::ShaderManager_D3D()
{
	memset(m_vertexShaders, 0, sizeof(m_vertexShaders));
	memset(m_pixelShader, 0, sizeof(m_pixelShader));
}

ShaderManager_D3D::~ShaderManager_D3D()
{
	for (int i = 0; i < PS_COUNT; ++i)
	{
		SafeRelease(m_pixelShader[i]);
	}
}

bool ShaderManager_D3D::CreateVertexShader(VertexShaderId id, const char *src)
{
	size_t srcLength = strlen(src);
	ID3DBlob *vsErrorMsgs = nullptr;

	HRESULT hr = D3DCompile(
		src,								// Shader code
		srcLength,							// Shader code string length
		nullptr,							// Shader source name
		nullptr,							// Macro definitions
		nullptr,							// Includes
		"VSMain",							// Entry point
		"vs_5_0",							// Compiler target
#if defined(DEBUG) || defined(_DEBUG)		// Compile options
		D3DCOMPILE_DEBUG,
#else
		0,
#endif
		0,									// FX compile options
		&m_vertexShaders[id].vsByteCode,	// Compiled code (out)
		&vsErrorMsgs);						// Error messages (out)

	CheckFailure(hr, "Unable to compile vertex shader: %s.", vsErrorMsgs->GetBufferPointer());
	hr = g_context.m_pDevice->CreateVertexShader(m_vertexShaders[id].vsByteCode->GetBufferPointer(), 
		m_vertexShaders[id].vsByteCode->GetBufferSize(), nullptr, &m_vertexShaders[id].vertexShader);
	CheckFailure(hr, "Unable to create vertex shader.");

	SafeRelease(vsErrorMsgs);

	return true;
}

bool ShaderManager_D3D::CreatePixelShader(PixelShaderId id, const char *src)
{
	size_t srcLength = strlen(src);
	ID3DBlob *psByteCode = nullptr;
	ID3DBlob *psErrorMsgs = nullptr;

	HRESULT hr = D3DCompile(
		src,								// Shader code
		srcLength,							// Shader code string length
		nullptr,							// Shader source name
		nullptr,							// Macro definitions
		nullptr,							// Includes
		"PSMain",							// Entry point
		"ps_5_0",							// Compiler target
#if defined(DEBUG) || defined(_DEBUG)	// Compile options
		D3DCOMPILE_DEBUG,
#else
		0,
#endif
		0,									// FX compile options
		&psByteCode,						// Compiled code (out)
		&psErrorMsgs);						// Error messages (out)

	CheckFailure(hr, "Unable to compile pixel shader.");
	hr = g_context.m_pDevice->CreatePixelShader(psByteCode->GetBufferPointer(), psByteCode->GetBufferSize(), nullptr, &m_pixelShader[id]);
	CheckFailure(hr, "Unable to create pixel shader.");

	SafeRelease(psByteCode);
	SafeRelease(psErrorMsgs);

	return true;
}

FragmentShader_D3D* ShaderManager_D3D::GetShader(ShaderId shaderId)
{
	if (!m_shaders[shaderId].isLoaded())
	{
		switch(shaderId)
		{
		case SHADER_DEFAULT:
			LoadShader(shaderId, VS_DEFAULT, PS_DEFAULT);
			break;
		case SHADER_PRIMITIVE:
			LoadShader(shaderId, VS_PRIMITIVE, PS_PRIMITIVE);
			break;
		case SHADER_INSTANCING:
			LoadShader(shaderId, VS_INSTANCING, PS_DEFAULT);
			break;
		case SHADER_INSTANCING_COLOR_REPLACE:
			LoadShader(shaderId, VS_INSTANCING, PS_COLOR_REPLACE);
			break;
		default:
			assertmsg(false, "Invalid shader");
		}
	}

	return &m_shaders[shaderId];
}

void ShaderManager_D3D::LoadShader(ShaderId shaderId, VertexShaderId vsId, PixelShaderId psId)
{
	static const char *VertexShaderStrings[] =
	{
		default_vertex_shader,		// VS_DEFAULT
		primitive_shader,			// VS_PRIMITIVE
		instancing_vertex_shader,	// VS_INSTANCING
	};
	static_assert(ARRAYSIZE(VertexShaderStrings) == VS_COUNT, "Missing vertex shader string");

	if (!m_vertexShaders[vsId].vertexShader)
	{
		CreateVertexShader(vsId, VertexShaderStrings[vsId]);
	}

	static const char *PixelShaderStrings[] =
	{
		default_pixel_shader,	// PS_DEFAULT
		primitive_shader,		// PS_PRIMITIVE
		replacer_pixel_shader,	// PS_COLOR_REPLACE
	};
	static_assert(ARRAYSIZE(PixelShaderStrings) == PS_COUNT, "Missing pixel shader string");

	if (!m_pixelShader[psId])
	{
		CreatePixelShader(psId, PixelShaderStrings[psId]);
	}

	if (!m_shaders[shaderId].isLoaded())
	{
		m_shaders[shaderId].m_pPixelShader = m_pixelShader[psId];
		m_shaders[shaderId].m_pVertexShader = m_vertexShaders[vsId].vertexShader;
		m_shaders[shaderId].m_pVSByteCode = m_vertexShaders[vsId].vsByteCode;
	}
}
