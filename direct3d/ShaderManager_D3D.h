#pragma once

#include "FragmentShader_D3D.h"

class ShaderManager_D3D
{
public:

	enum ShaderId
	{
		SHADER_DEFAULT,
		SHADER_PRIMITIVE,
		SHADER_COUNT
	};

	static ShaderManager_D3D& GetInstance();
	static void Destroy();

	ShaderManager_D3D();
	~ShaderManager_D3D();

	FragmentShader_D3D* GetShader(ShaderId shaderId);

private:

	static ShaderManager_D3D *s_instance;
	FragmentShader_D3D m_shaders[SHADER_COUNT];
};