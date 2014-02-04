#pragma once

#include "FragmentShader_D3D.h"
#include "../common.h"

class ColorReplacerShader_D3D : public FragmentShader_D3D 
{

public:

	ColorReplacerShader_D3D();
	virtual ~ColorReplacerShader_D3D();

	bool init();
	void setColor( Color from, Color to, float eps );

	virtual void updateUniforms();

private:

	moyai_align(16) struct ReplaceValues
	{
		XMFLOAT4 from_color;
		XMFLOAT4 to_color;
		float epsilon;
	} m_values;

	ID3D11Buffer *m_pConstantBuffer;

};