#ifdef USE_D3D

#include "FragmentShader_D3D.h"

FragmentShader_D3D::FragmentShader_D3D()
	: m_pVertexShader(nullptr)
	, m_pPixelShader(nullptr)
	, m_pVSByteCode(nullptr)
{

}

FragmentShader_D3D::~FragmentShader_D3D()
{
	SafeRelease(m_pPixelShader);
	SafeRelease(m_pVertexShader);
	SafeRelease(m_pVSByteCode);
}

void FragmentShader_D3D::bind()
{
	g_context.m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	g_context.m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
}

void FragmentShader_D3D::SetShaders(FragmentShader_D3D *shader)
{
	m_pVertexShader = shader->m_pVertexShader;
	m_pPixelShader = shader->m_pPixelShader;
	m_pVSByteCode = shader->m_pVSByteCode;

	m_pVertexShader->AddRef();
	m_pPixelShader->AddRef();
	m_pVSByteCode->AddRef();
}

#endif