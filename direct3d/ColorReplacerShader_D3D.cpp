#ifdef USE_D3D

#include "ColorReplacerShader_D3D.h"

const static char replacer_shader[] = 
	"Texture2D g_inputTexture : register(t0);\n"
	"SamplerState g_inputSampler : register(s0);\n"
	"struct VS_Input\n"
	"{\n"
	"   float3 pos : POSITION;\n"
	"   float4 color : COLOR0;\n"
	"   float2 uv : TEXCOORD0;\n"
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
	"cbuffer ReplaceValues : register(b1)\n"
	"{\n"
	"   float4 color1;\n"
	"   float4 replace1;\n"
	"	float eps;\n"
	"}\n"
	"cbuffer Matrices : register(b0)\n"
	"{\n"
	"   float4x4 ModelView;\n"
	"   float4x4 Projection;\n"
	"   float4x4 MVP;\n"
	"}\n"
	"VS_Output VSMain(VS_Input input)\n"
	"{\n"
	"   VS_Output output;\n"
	"   output.color = input.color;\n"
	"   output.uv = input.uv;\n"
	"   float4 pos = float4(input.pos, 1.0f);\n"
	"   pos = mul(pos, MVP);\n"
	"   output.pos = pos;\n"
	"   return output;\n"
	"}\n"
	"PS_Output PSMain(VS_Output input)\n" 
	"{\n"
	"   PS_Output output;\n"
	"	float4 pixel = g_inputTexture.Sample(g_inputSampler, input.uv);\n"
	"	if(pixel.r > color1.r - eps && pixel.r < color1.r + eps && pixel.g > color1.g - eps && pixel.g < color1.g + eps && pixel.b > color1.b - eps && pixel.b < color1.b + eps )\n"
	"   {\n"
	"		pixel = float4(replace1.xyz, pixel.a);\n"
	"   }\n"
	"	output.color = pixel * input.color;\n"
	"   return output;\n"
	"}\n";

ColorReplacerShader_D3D::ColorReplacerShader_D3D()
{
	
}

ColorReplacerShader_D3D::~ColorReplacerShader_D3D()
{
	SafeRelease(m_pConstantBuffer);
}

bool ColorReplacerShader_D3D::init()
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = sizeof(ReplaceValues);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	g_context.m_pDevice->CreateBuffer(&desc, nullptr, &m_pConstantBuffer);

	return load( replacer_shader );
}

void ColorReplacerShader_D3D::setColor( Color from, Color to, float eps ) 
{
	m_values.epsilon = eps;

	m_values.to_color.x = to.r;
	m_values.to_color.y = to.g;
	m_values.to_color.z = to.b;
	m_values.to_color.w = 1.0f;

	m_values.from_color.x = from.r;
	m_values.from_color.y = from.g;
	m_values.from_color.z = from.b;
	m_values.from_color.w = 1.0f;
}

void ColorReplacerShader_D3D::updateUniforms()
{
	D3D11_MAPPED_SUBRESOURCE map;
	g_context.m_pDeviceContext->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	memcpy(map.pData, &m_values, sizeof(ReplaceValues));
	g_context.m_pDeviceContext->Unmap(m_pConstantBuffer, 0);

	g_context.m_pDeviceContext->VSSetConstantBuffers(1, 1, &m_pConstantBuffer);
	g_context.m_pDeviceContext->PSSetConstantBuffers(1, 1, &m_pConstantBuffer);
}

#endif