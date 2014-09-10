#pragma once

#include "FragmentShader_D3D.h"

class ShaderManager_D3D
{
public:

	enum ShaderId
	{
		SHADER_DEFAULT,
		SHADER_PRIMITIVE,
		SHADER_INSTANCING,
		SHADER_INSTANCING_COLOR_REPLACE,
		SHADER_COUNT
	};

	ShaderManager_D3D();
	~ShaderManager_D3D();

	FragmentShader_D3D* GetShader(ShaderId shaderId);

private:

	enum VertexShaderId
	{
		VS_DEFAULT,
		VS_PRIMITIVE,
		VS_INSTANCING,
		VS_COUNT
	};

	enum PixelShaderId
	{
		PS_DEFAULT,
		PS_PRIMITIVE,
		PS_COLOR_REPLACE,
		PS_COUNT
	};

	struct VertexShader
	{
		~VertexShader()
		{
			SafeRelease(vertexShader);
			SafeRelease(vsByteCode);
		}
		ID3D11VertexShader *vertexShader;
		ID3DBlob *vsByteCode;
	};

	bool CreateVertexShader(VertexShaderId id, const char *src);
	bool CreatePixelShader(PixelShaderId id, const char *src);
	void LoadShader(ShaderId shaderId, VertexShaderId vsId, PixelShaderId psId);

	VertexShader m_vertexShaders[VS_COUNT];
	ID3D11PixelShader *m_pixelShader[PS_COUNT];
	FragmentShader_D3D m_shaders[SHADER_COUNT];
};