#pragma once

#include "Context_D3D.h"

// Since the Moyai rendering engine is using fixed vertex processing, the 
// D3D implementation of FragmentShader combines vertex and pixel shaders.
class FragmentShader_D3D 
{
	friend class VertexBuffer_D3D;
	friend class ShaderManager_D3D;

public:
	
	bool isLoaded() const { return m_pVertexShader != nullptr; }

	virtual void updateUniforms() {};
	void bind();

protected:

	FragmentShader_D3D();
	virtual ~FragmentShader_D3D();

	void SetShaders(FragmentShader_D3D *shader);

	ID3D11VertexShader *m_pVertexShader;
	ID3D11PixelShader *m_pPixelShader;
	ID3DBlob *m_pVSByteCode;
};