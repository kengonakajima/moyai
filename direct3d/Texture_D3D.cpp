#ifdef USE_D3D

#include "Texture_D3D.h"


Texture_D3D::Texture_D3D()
	: image(nullptr)
	, tex(0)
	, m_pTexture(nullptr)
	, m_pTextureSRV(nullptr)
	, m_pSamplerState(nullptr)
{

}

Texture_D3D::~Texture_D3D()
{
	SafeRelease(m_pTexture);
	SafeRelease(m_pSamplerState);
}

void Texture_D3D::getSize( int *w, int *h)
{
	if (image)
	{
		*w = image->width;
		*h = image->height;
	}
}

void Texture_D3D::bind()
{
	g_context.m_pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerState);
	g_context.m_pDeviceContext->PSSetShaderResources(0, 1, &m_pTextureSRV);
}

bool Texture_D3D::load( const char *path )
{
	image = new Image();
	if (!image->loadPNG(path))
	{
		assert(false && "Unable to load png");
		return false;
	}

	return createD3DTexture();
}

void Texture_D3D::setLinearMagFilter()
{
	// Not implemented... Never called.
}

void Texture_D3D::setLinearMinFilter()
{
	// Not implemented... Never called.
}

void Texture_D3D::setImage( Image *img ) 
{
	image = img;
	createD3DTexture();
}

bool Texture_D3D::createD3DTexture()
{
	// Create texture
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = image->width;
		desc.Height = image->height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = image->buffer;
		data.SysMemPitch = image->width * 4;

		HRESULT hr = g_context.m_pDevice->CreateTexture2D(&desc, &data, &m_pTexture);
		CheckFailure(hr, "Unable to create texture");

		tex = (UINT)m_pTexture;
	}

	// Create texture SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipLevels = 1;
		desc.Texture2D.MostDetailedMip = 0;

		HRESULT hr = g_context.m_pDevice->CreateShaderResourceView(m_pTexture, &desc, &m_pTextureSRV);
		CheckFailure(hr, "Unable to create texture SRV");
	}

	// Create sampler state
	{
		D3D11_SAMPLER_DESC desc;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = 1;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.BorderColor[0] = 1.0f;
		desc.BorderColor[1] = 1.0f;
		desc.BorderColor[2] = 1.0f;
		desc.BorderColor[3] = 1.0f;
		desc.MinLOD = -FLT_MAX;
		desc.MaxLOD = FLT_MAX;

		HRESULT hr = g_context.m_pDevice->CreateSamplerState(&desc, &m_pSamplerState);
		CheckFailure(hr, "Unable to create sampler state");
	}

	return true;
}

#endif