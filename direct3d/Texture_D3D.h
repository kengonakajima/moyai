#pragma once

#include "../common.h"
#include "Context_D3D.h"

class Texture_D3D 
{

public:

	Image *image;
	UINT tex;

	Texture_D3D();
	~Texture_D3D();

	void setImage( Image *img );
	bool load( const char *path );
	void setLinearMagFilter();
	void setLinearMinFilter();    
	void getSize( int *w, int *h);

	inline Vec2 getSize() // use this vector direct to Prop2D::setScl(v)
	{ 
		int w,h;
		getSize( &w, &h );
		return Vec2((float)w, (float)h);
	}

	void bind();

private:

	bool createD3DTexture();

	ID3D11Texture2D *m_pTexture;
	ID3D11ShaderResourceView *m_pTextureSRV;
	ID3D11SamplerState *m_pSamplerState;
};