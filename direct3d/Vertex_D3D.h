#pragma once

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Macro.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

struct Vertex_PUV
{
	XMFLOAT2 pos;
	XMFLOAT2 uv;
};

struct Vertex_PCUV
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
	XMFLOAT4 color;
};

struct Vertex_PC
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

struct Vertex_PNCUV
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	XMFLOAT4 color;
};