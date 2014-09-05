#include "ShaderManager_D3D.h"
#include "ShaderCode_D3D.h"

ShaderManager_D3D *ShaderManager_D3D::s_instance = nullptr;

ShaderManager_D3D& ShaderManager_D3D::GetInstance()
{
	if (!s_instance)
	{
		s_instance = new ShaderManager_D3D();
	}

	return *s_instance;
}

void ShaderManager_D3D::Destroy()
{
	delete s_instance;
	s_instance = nullptr;
}

ShaderManager_D3D::ShaderManager_D3D()
{

}

ShaderManager_D3D::~ShaderManager_D3D()
{
	
}

FragmentShader_D3D* ShaderManager_D3D::GetShader(ShaderId shaderId)
{
	if (!m_shaders[shaderId].isLoaded())
	{
		switch(shaderId)
		{
		case SHADER_DEFAULT:
			m_shaders[shaderId].load(default_vertex_shader, default_pixel_shader);
			break;
		case SHADER_PRIMITIVE:
			m_shaders[shaderId].load(primitive_shader);
			break;
		}
	}

	return &m_shaders[shaderId];
}
