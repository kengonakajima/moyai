#pragma once

#include "../common.h"
#include "../common/Enums.h"

class VertexBuffer_D3D;
class FragmentShader_D3D;

class Prim_D3D 
{

public:

	PRIMTYPE type;
	Vec2 a,b;
	Color color;
	int line_width;

	Prim_D3D(PRIMTYPE t, Vec2 a, Vec2 b, Color c, int line_width = 1);
	~Prim_D3D();

	void draw(Vec2 ofs);

private:

	void drawLine(Vec2 ofs);
	void drawRectangle(Vec2 ofs);

	VertexBuffer_D3D *m_pVertexBuffer;
	FragmentShader_D3D *m_pShader;
};