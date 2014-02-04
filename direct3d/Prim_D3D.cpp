#ifdef USE_D3D

#include "Prim_D3D.h"
#include "VertexBuffer_D3D.h"
#include "FragmentShader_D3D.h"
#include "../common/VertexFormat.h"
#include "../common/Enums.h"


const static char primitive_shader[] = 
	"struct VS_Input\n"
	"{\n"
	"   float4 pos : POSITION;\n"
	"   float4 color : COLOR0;\n"
	"};\n"
	"struct VS_Output\n"
	"{\n"
	"   float4 pos : SV_POSITION;\n"
	"   float4 color : COLOR0;\n"
	"};\n"
	"struct PS_Output\n"
	"{\n"
	"   float4 color : SV_Target;\n"
	"};\n"
	"cbuffer Matrices : register(b0)\n"
	"{\n"
	"   float4x4 ModelView;\n"
	"   float4x4 Projection;\n"
	"   float4x4 MVP;\n"
	"}\n"
	"VS_Output VSMain(VS_Input input)\n"
	"{\n"
	"   VS_Output output;\n"
	"   float3 pos = mul(input.pos, MVP);\n"
	"   output.pos = float4(pos, 1.0f);\n"
	"   output.color = input.color;\n"
	"   return output;"
	"}\n"
	"PS_Output PSMain(VS_Output input)\n" 
	"{\n"
	"   PS_Output output;\n"
	"	output.color = input.color;\n"
	"   return output;\n"
	"}\n";

Prim_D3D::Prim_D3D(PRIMTYPE t, Vec2 a, Vec2 b, Color c, int line_width) 
	: type(t)
	, a(a)
	, b(b)
	, color(c)
	, line_width(line_width)
	, m_pVertexBuffer(nullptr)
	, m_pShader(nullptr)
{
	// Create primitive shader
	{
		m_pShader = new FragmentShader_D3D();
		m_pShader->load(primitive_shader);
	}

	// Create vertex buffer
	{
		VertexFormat format;
		format.declareCoordVec3();
		format.declareColor();

		m_pVertexBuffer = new VertexBuffer_D3D(&format, 6, m_pShader);
	}
}

void Prim_D3D::draw(Vec2 ofs)
{
	assert(line_width > 0 && "Invalid line width");

	m_pShader->bind();
	m_pShader->updateUniforms();

	if (type == PRIMTYPE_LINE)
	{
		drawLine(ofs);
	}
	else if (type == PRIMTYPE_RECTANGLE)
	{
		drawRectangle(ofs);
	}
}

void Prim_D3D::drawLine(Vec2 ofs)
{
	if (line_width == 1)
	{
		Vertex_PC vertices[] = 
		{
			{ XMFLOAT3(ofs.x + a.x, ofs.y + a.y, 0.0f), XMFLOAT4(color.r, color.g, color.b, color.a) },
			{ XMFLOAT3(ofs.x + b.x, ofs.y + b.y, 0.0f), XMFLOAT4(color.r, color.g, color.b, color.a) },
		};

		m_pVertexBuffer->copyFromBuffer(vertices, 2);
		m_pVertexBuffer->copyToGPU();
		m_pVertexBuffer->setTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		m_pVertexBuffer->bind();
		g_context.m_pDeviceContext->Draw(2, 0);
	}
	else
	{
		XMVECTOR perpVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMVECTOR lineVector = XMVectorSet(b.x - a.x, b.y - a.y, 0.0f, 0.0f);
		lineVector = XMVector3Normalize(lineVector);
		XMVECTOR lineWidthDirection = XMVector3Cross(perpVector, lineVector);

		XMVECTOR lineStart = XMVectorSet(ofs.x + a.x, ofs.y + a.y, 0.0f, 0.0f);
		XMVECTOR lineEnd = XMVectorSet(ofs.x + b.x, ofs.y + b.y, 0.0f, 0.0f);

		float halfLineWidth = line_width * 0.5f;
		XMFLOAT3 p1, p2, p3, p4;
		XMStoreFloat3(&p1, lineStart - lineWidthDirection * halfLineWidth);
		XMStoreFloat3(&p2, lineStart + lineWidthDirection * halfLineWidth);
		XMStoreFloat3(&p3, lineEnd + lineWidthDirection * halfLineWidth);
		XMStoreFloat3(&p4, lineEnd - lineWidthDirection * halfLineWidth);

		Vertex_PC vertices[] = 
		{
			{ p1, XMFLOAT4(color.r, color.g, color.b, color.a) },
			{ p2, XMFLOAT4(color.r, color.g, color.b, color.a) },
			{ p3, XMFLOAT4(color.r, color.g, color.b, color.a) },

			{ p1, XMFLOAT4(color.r, color.g, color.b, color.a) },
			{ p3, XMFLOAT4(color.r, color.g, color.b, color.a) },
			{ p4, XMFLOAT4(color.r, color.g, color.b, color.a) }
		};

		m_pVertexBuffer->copyFromBuffer(vertices, 6);
		m_pVertexBuffer->copyToGPU();
		m_pVertexBuffer->bind();
		g_context.m_pDeviceContext->Draw(6, 0);
	}
}

void Prim_D3D::drawRectangle(Vec2 ofs)
{
	Vertex_PC vertices[] = 
	{
		{ XMFLOAT3(ofs.x + a.x, ofs.y + a.y, 0.0f), XMFLOAT4(color.r, color.g, color.b, color.a) },
		{ XMFLOAT3(ofs.x + b.x, ofs.y + a.y, 0.0f), XMFLOAT4(color.r, color.g, color.b, color.a) },
		{ XMFLOAT3(ofs.x + a.x, ofs.y + b.y, 0.0f), XMFLOAT4(color.r, color.g, color.b, color.a) },

		{ XMFLOAT3(ofs.x + a.x, ofs.y + b.y, 0.0f), XMFLOAT4(color.r, color.g, color.b, color.a) },
		{ XMFLOAT3(ofs.x + b.x, ofs.y + a.y, 0.0f), XMFLOAT4(color.r, color.g, color.b, color.a) },
		{ XMFLOAT3(ofs.x + b.x, ofs.y + b.y, 0.0f), XMFLOAT4(color.r, color.g, color.b, color.a) }
	};

	m_pVertexBuffer->copyFromBuffer(vertices, 6);
	m_pVertexBuffer->copyToGPU();
	m_pVertexBuffer->setTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pVertexBuffer->bind();

	g_context.m_pDeviceContext->Draw(6, 0);
}

#endif