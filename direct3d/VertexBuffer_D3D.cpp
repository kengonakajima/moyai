#ifdef USE_D3D

#include "VertexBuffer_D3D.h"
#include "FragmentShader_D3D.h"

VertexBuffer_D3D::VertexBuffer_D3D(VertexFormat *format, unsigned int maxVertexCount, const FragmentShader_D3D *inputSignatureShader) 
	: m_pVertexBuffer(nullptr)
	, m_pInputLayout(nullptr)
	, m_pInputSignatureShader(inputSignatureShader)
	, m_topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
	, m_pVertexFormat(format)
	, m_pBuffer(nullptr)
	, m_vertexStride(aligned_size(m_pVertexFormat->getNumFloat() * 4, 16))
	, m_vertexCount(maxVertexCount)
	, m_bufferSize(m_vertexStride * m_vertexCount)
{
	assert(maxVertexCount > 0);

	m_pBuffer = (float*)MALLOC(m_vertexStride * m_vertexCount);
	assert(m_pBuffer);

	initD3DObjects();
}

VertexBuffer_D3D::~VertexBuffer_D3D() 
{
	SafeRelease(m_pVertexBuffer);
	SafeRelease(m_pInputLayout);
	m_pInputSignatureShader = nullptr;
	m_pVertexFormat = nullptr;

	if (m_pBuffer)
	{
		FREE(m_pBuffer);
	}
}

void VertexBuffer_D3D::initD3DObjects()
{
	// Create vertex buffer
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ByteWidth = m_bufferSize;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = g_context.m_pDevice->CreateBuffer(&desc, nullptr, &m_pVertexBuffer);
		assert(SUCCEEDED(hr) && "Unable to create vertex buffer");
	}

	// Create input layout
	{
		D3D11_INPUT_ELEMENT_DESC elements[member_size(VertexFormat, types)];
		for (int i = 0; i < m_pVertexFormat->types_used; ++i)
		{
			switch (m_pVertexFormat->types[i])
			{
			case 'v':
				setPositionElement(elements[i]); break;
			case 'c':
				setColorElement(elements[i]); break;
			case 't':
				setUVElement(elements[i]); break;
			case 'n':
				setNormalElement(elements[i]); break;
			default:
				assert(false && "Invalid vertex format element");
			}
		}

		g_context.m_pDevice->CreateInputLayout(
			elements, 
			m_pVertexFormat->types_used, 
			m_pInputSignatureShader->m_pVSByteCode->GetBufferPointer(), 
			m_pInputSignatureShader->m_pVSByteCode->GetBufferSize(), 
			&m_pInputLayout);
	}
}

void VertexBuffer_D3D::setPositionElement(D3D11_INPUT_ELEMENT_DESC &element)
{
	element.SemanticName = "POSITION";
	element.SemanticIndex = 0;
	element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	element.InputSlot = 0;
	element.AlignedByteOffset = 0; //m_vertexStride;
	element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	element.InstanceDataStepRate = 0;
}

void VertexBuffer_D3D::setUVElement(D3D11_INPUT_ELEMENT_DESC &element)
{
	element.SemanticName = "TEXCOORD";
	element.SemanticIndex = 0;
	element.Format = DXGI_FORMAT_R32G32_FLOAT;
	element.InputSlot = 0;
	element.AlignedByteOffset = 28; //m_vertexStride;
	element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	element.InstanceDataStepRate = 0;
}

void VertexBuffer_D3D::setNormalElement(D3D11_INPUT_ELEMENT_DESC &element)
{
	assert(false && "Vertex normal is not supported yet");

	/*
	element.SemanticName = "TEXCOORD";
	element.SemanticIndex = 1;
	element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	element.InputSlot = 0;
	element.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; //sizeof(VertexNormal);
	element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	element.InstanceDataStepRate = 0;
	*/
}

void VertexBuffer_D3D::setColorElement(D3D11_INPUT_ELEMENT_DESC &element)
{
	element.SemanticName = "COLOR";
	element.SemanticIndex = 0;
	element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	element.InputSlot = 0;
	element.AlignedByteOffset = 12; //m_vertexStride;
	element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	element.InstanceDataStepRate = 0;
}

void VertexBuffer_D3D::copyFromBuffer(void *data, int vert_cnt) 
{
	assert(m_bufferSize >= vert_cnt * m_vertexStride);

	m_vertexCount = vert_cnt;
	unsigned int size = m_vertexStride * m_vertexCount;
	
	memcpy(m_pBuffer, data, size);
}

void VertexBuffer_D3D::setCoord(unsigned int index, Vec3 v) 
{
	assert(index < m_vertexCount);

	int ofs = m_pVertexFormat->coord_offset;
	assertmsg( ofs >= 0, "coord have not declared in vertex format" );

	int index_in_array = index * m_vertexStride + ofs;
	float *buffer = static_cast<float*>(m_pBuffer);
	buffer[index_in_array] = v.x;
	buffer[index_in_array+1] = v.y;
	buffer[index_in_array+2] = v.z;
}

Vec3 VertexBuffer_D3D::getCoord(unsigned int index) 
{
	assert(index < m_vertexCount);

	int ofs = m_pVertexFormat->coord_offset;
	assertmsg( ofs >= 0, "coord have not declared in vertex format" );

	int index_in_array = index * m_vertexStride + ofs;
	float *buffer = static_cast<float*>(m_pBuffer);

	return Vec3(buffer[index_in_array], buffer[index_in_array+1], buffer[index_in_array+2]);
}

void VertexBuffer_D3D::setCoordBulk(Vec3 *v, unsigned int num) 
{
	for (unsigned int i = 0; i < num; ++i) 
	{
		setCoord(i, v[i]);
	}
}

void VertexBuffer_D3D::setColor(unsigned int index, Color c) 
{
	assert( index < m_vertexCount );

	int ofs = m_pVertexFormat->color_offset;
	assertmsg( ofs >= 0, "color have not declared in vertex format");

	int index_in_array = index * m_vertexStride + ofs;
	float *buffer = static_cast<float*>(m_pBuffer);

	buffer[index_in_array] = c.r;
	buffer[index_in_array+1] = c.g;
	buffer[index_in_array+2] = c.b;
	buffer[index_in_array+3] = c.a;
}

Color VertexBuffer_D3D::getColor(unsigned int index) 
{
	assert( index < m_vertexCount );

	int ofs = m_pVertexFormat->color_offset;
	assertmsg( ofs >= 0, "color have not declared in vertex format" );

	int index_in_array = index * m_vertexStride + ofs;
	float *buffer = static_cast<float*>(m_pBuffer);

	return Color(buffer[index_in_array], buffer[index_in_array+1], buffer[index_in_array+2], buffer[index_in_array+3]);
}

void VertexBuffer_D3D::setUV(unsigned int index, Vec2 uv) 
{
	assert( index < m_vertexCount );

	int ofs = m_pVertexFormat->texture_offset;
	assertmsg( ofs >= 0, "texcoord have not declared in vertex format");

	int index_in_array = index * m_vertexStride + ofs;
	float *buf = static_cast<float*>(m_pBuffer);

	buf[index_in_array] = uv.x;
	buf[index_in_array+1] = uv.y;
}

Vec2 VertexBuffer_D3D::getUV(unsigned int index) 
{
	assert( index < m_vertexCount );

	int ofs = m_pVertexFormat->texture_offset;
	assertmsg( ofs >= 0, "UV have not been declared in vertex format" );

	int index_in_array = index * m_vertexCount + ofs;
	float *buf = static_cast<float*>(m_pBuffer);

	return Vec2( buf[index_in_array], buf[index_in_array+1] );
}

void VertexBuffer_D3D::setUVBulk(Vec2 *uv, unsigned int num) 
{
	for (unsigned int i = 0; i < num; ++i) 
	{
		setUV( i, uv[i] );
	}
}

void VertexBuffer_D3D::setNormal(unsigned int index, Vec3 v) 
{ 
	assert( index < m_vertexCount );

	int ofs = m_pVertexFormat->normal_offset;
	assertmsg( ofs >= 0, "normal have not declared in vertex format" );

	int index_in_array = index * m_vertexStride + ofs;
	float *buf = static_cast<float*>(m_pBuffer);

	buf[index_in_array] = v.x;
	buf[index_in_array+1] = v.y;
	buf[index_in_array+2] = v.z;        
}

Vec3 VertexBuffer_D3D::getNormal(unsigned int index) 
{
	assert( index < m_vertexCount );

	int ofs = m_pVertexFormat->normal_offset;
	assertmsg( ofs >= 0, "normal have not declared in vertex format" );

	int index_in_array = index * m_vertexStride + ofs;
	float *buf = static_cast<float*>(m_pBuffer);

	return Vec3( buf[index_in_array], buf[index_in_array+1], buf[index_in_array+2] );    
}

void VertexBuffer_D3D::setNormalBulk(Vec3 *v, unsigned int num) 
{
	for (unsigned int i = 0; i < num; ++i) 
	{
		setNormal( i, v[i] );
	}
}

void VertexBuffer_D3D::dump()
{

}

void VertexBuffer_D3D::copyToGPU()
{
	D3D11_MAPPED_SUBRESOURCE map;
	g_context.m_pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	memcpy(map.pData, m_pBuffer, m_bufferSize);
	g_context.m_pDeviceContext->Unmap(m_pVertexBuffer, 0);
}

void VertexBuffer_D3D::bind()
{
	UINT offset = 0;
	g_context.m_pDeviceContext->IASetInputLayout(m_pInputLayout);
	g_context.m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_vertexStride, &offset);
	g_context.m_pDeviceContext->IASetPrimitiveTopology(m_topology);
}

void VertexBuffer_D3D::setTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	m_topology = topology;
}

Vec3 VertexBuffer_D3D::calcCenterOfCoords() 
{
	Vec3 c(0,0,0);

	for (unsigned int i = 0; i < m_vertexCount; ++i) 
	{
		c += getCoord(i);
	}

	c /= (float)m_vertexCount;

	return c;
}

#endif