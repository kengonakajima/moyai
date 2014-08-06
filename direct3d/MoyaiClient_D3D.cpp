#ifdef USE_D3D

#include "MoyaiClient_D3D.h"

MoyaiClient_D3D::MoyaiClient_D3D() 
	: Moyai() 
	, m_pBlendState(nullptr)
	, m_pQuadVertexBuffer(nullptr)
{
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;

	HRESULT hr = g_context.m_pDevice->CreateBlendState(&blendDesc, &m_pBlendState);
	assert(SUCCEEDED(hr) && "Failed to create blend state");

	VertexFormat format;
	format.declareCoordVec3();
	format.declareUV();

	m_pQuadVertexBuffer = new VertexBuffer_D3D(&format, 6, g_context.m_pDefaultShader);
}

MoyaiClient_D3D::~MoyaiClient_D3D()
{
	if (m_pBlendState)
	{
		m_pBlendState->Release();
		m_pBlendState = nullptr;
	}

	SafeDelete(m_pQuadVertexBuffer);
}

int MoyaiClient_D3D::render()
{
	g_context.m_pDeviceContext->ClearRenderTargetView(g_context.m_pRenderTargetView, g_context.m_clearColorRGBA);
	g_context.m_pDeviceContext->ClearDepthStencilView(g_context.m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	g_context.m_pDeviceContext->OMSetBlendState(m_pBlendState, NULL, 0xFFFFFFFF);

	int cnt=0;
	for(int i=0;i<elementof(groups);i++)
	{
		Group *g = groups[i];
		if( g && g->to_render ) 
		{
			Layer *l = (Layer*) g;
			cnt += l->renderAllProps();
		}
	}

	glfw_d3d::glfwSwapBuffers();

	return cnt;
}

void MoyaiClient_D3D::capture( Image *img ) 
{
	float *buf = (float*)MALLOC( img->width * img->height * 3 * sizeof(float) );
	//glReadPixels( 0, 0, img->width, img->height, GL_RGB, GL_FLOAT, buf );

	for(int y=0;y<img->height;y++)
	{
		for(int x=0;x<img->width;x++)
		{
			int ind = (x + y * img->width)*3;
			float r = buf[ind+0], g = buf[ind+1], b = buf[ind+2];
			Color c( r,g,b,1);
			img->setPixel(x,img->height-1-y,c);
		}
	}   

	FREE(buf);
}

#endif