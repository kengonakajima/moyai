#ifdef USE_D3D

#include "Viewport_D3D.h"
#include "Context_D3D.h"


Viewport_D3D::Viewport_D3D() 
	: screen_width(0)
	, screen_height(0)
	, dimension(DIMENSION_2D)
	, scl(0,0,0)
	, near_clip(0.01)
	, far_clip(100) 
{

}

Viewport_D3D::~Viewport_D3D()
{

}

void Viewport_D3D::setSize(int scrw, int scrh ) 
{
	screen_width = scrw;
	screen_height = scrh;

	D3D11_VIEWPORT viewport;
	viewport.Width = screen_width;
	viewport.Height = screen_height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	g_context.m_pDeviceContext->RSSetViewports(1, &viewport);
}

void Viewport_D3D::setScale2D( float sx, float sy )
{
	dimension = DIMENSION_2D;
	scl = Vec3(sx,sy,1);
}

void Viewport_D3D::setClip3D( float neardist, float fardist ) 
{        
	near_clip = neardist;
	far_clip = fardist;
	dimension = DIMENSION_3D;
}

void Viewport_D3D::getMinMax( Vec2 *minv, Vec2 *maxv )
{
	minv->x = -scl.x/2;
	maxv->x = scl.x/2;
	minv->y = -scl.y/2;
	maxv->y = scl.y/2;
}

#endif