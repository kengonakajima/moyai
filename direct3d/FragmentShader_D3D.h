#pragma once

#include "Context_D3D.h"

// Since the Moyai rendering engine is using fixed vertex processing, the 
// D3D implementation of FragmentShader combines vertex and pixel shaders.
class FragmentShader_D3D 
{
	friend class VertexBuffer_D3D;

public:
	
	FragmentShader_D3D();
	virtual ~FragmentShader_D3D();

	bool load(const char *src);
	virtual void updateUniforms() {};

	void bind();

private:

	ID3D11VertexShader *m_pVertexShader;
	ID3D11PixelShader *m_pPixelShader;
	ID3DBlob *m_pVSByteCode;
};