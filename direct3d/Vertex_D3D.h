#pragma once

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#include "Macro.h"

using namespace DirectX;
using namespace DirectX::PackedVector;

moyai_align(16) struct Vertex_PCUV
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
	XMFLOAT2 uv;
};

moyai_align(16) struct Vertex_PC
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

moyai_align(16) struct Vertex_PNCUV
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT4 color;
	XMFLOAT2 uv;
};