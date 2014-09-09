#ifdef USE_D3D

#include "ColorReplacerShader_D3D.h"
#include "ShaderCode_D3D.h"
#include "ShaderManager_D3D.h"

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

	FragmentShader_D3D *shader = g_context.m_pShaderManager->GetShader(ShaderManager_D3D::SHADER_INSTANCING_COLOR_REPLACE);
	SetShaders(shader);

	return true;
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