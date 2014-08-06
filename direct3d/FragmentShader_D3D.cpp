#ifdef USE_D3D

#include "FragmentShader_D3D.h"
#include <d3dcompiler.h>

const static char default_vertex_shader[] = 
	"struct VS_Input\n"
	"{\n"
	"   float3 pos : POSITION;\n"
	"   float2 uv : TEXCOORD0;\n"
	"   float4 color : COLOR0;\n"
	"   float3 posRot : TEXCOORD1;\n"
	"};\n"
	"struct VS_Output\n"
	"{\n"
	"   float4 pos : SV_POSITION;\n"
	"   float4 color : COLOR0;\n"
	"   float2 uv : TEXCOORD0;\n"
	"};\n"
	"struct PS_Output\n"
	"{\n"
	"   float4 color : SV_Target;\n"
	"};\n"
	"cbuffer PerLayer : register(b0)\n"
	"{\n"
	"   float4x4 Projection;\n"
	"}\n"
	"VS_Output VSMain(VS_Input input)\n"
	"{\n"
	"   VS_Output output;\n"
	"   output.color = input.color;\n"
	"   output.uv = input.uv;\n"
	"   float4 pos = float4(input.pos, 1.0f);\n"
	"	float2 trans = input.posRot.xy;\n"
	"	float rot = input.posRot.z;\n"
	"	float cosRot = cos(rot);\n"
	"	float sinRot = sin(rot);\n"
	"	float3x2 rotTransMat = { cosRot, sinRot, -sinRot, cosRot, trans.x, trans.y };\n"
	"   pos.xy = mul(float3(pos.xy, 1.0f), rotTransMat);\n"
	"   pos = mul(pos, Projection);\n"
	"   output.pos = pos;\n"
	"   return output;\n"
	"}\n";

FragmentShader_D3D::FragmentShader_D3D()
	: m_pPixelShader(nullptr)
	, m_pVertexShader(nullptr)
	, m_pVSByteCode(nullptr)
{

}

FragmentShader_D3D::~FragmentShader_D3D()
{
	SafeRelease(m_pPixelShader);
	SafeRelease(m_pVertexShader);
	SafeRelease(m_pVSByteCode);
}

bool FragmentShader_D3D::load(const char *src) 
{
	size_t srcLength = strlen(src);
	ID3DBlob *psByteCode = nullptr;
	ID3DBlob *vsErrorMsgs = nullptr;
	ID3DBlob *psErrorMsgs = nullptr;

	HRESULT hr = D3DCompile(
					src,							// Shader code
					srcLength,						// Shader code string length
					nullptr,						// Shader source name
					nullptr,						// Macro definitions
					nullptr,						// Includes
					"VSMain",						// Entry point
					"vs_5_0",						// Compiler target
				#if defined(DEBUG) || defined(_DEBUG)	// Compile options
					D3DCOMPILE_DEBUG,
				#else
					0,
				#endif
					0,								// FX compile options
					&m_pVSByteCode,					// Compiled code (out)
					&vsErrorMsgs);					// Error messages (out)

	CheckFailure(hr, "Unable to compile vertex shader: %s.", vsErrorMsgs->GetBufferPointer());

	hr = D3DCompile(
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

	hr = g_context.m_pDevice->CreateVertexShader(m_pVSByteCode->GetBufferPointer(), m_pVSByteCode->GetBufferSize(), nullptr, &m_pVertexShader);
	CheckFailure(hr, "Unable to create vertex shader.");
	hr = g_context.m_pDevice->CreatePixelShader(psByteCode->GetBufferPointer(), psByteCode->GetBufferSize(), nullptr, &m_pPixelShader);
	CheckFailure(hr, "Unable to create pixel shader.");

	SafeRelease(psByteCode);
	SafeRelease(vsErrorMsgs);
	SafeRelease(psErrorMsgs);

	return true;
}

void FragmentShader_D3D::bind()
{
	g_context.m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	g_context.m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
}

#endif