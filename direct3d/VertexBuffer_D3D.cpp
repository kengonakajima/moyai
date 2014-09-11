#ifdef USE_D3D

#include "VertexBuffer_D3D.h"
#include "FragmentShader_D3D.h"

VertexBuffer_D3D::VertexBuffer_D3D(VertexFormat &format, unsigned int maxVertexCount, const FragmentShader_D3D *inputSignatureShader, unsigned int maxInstanceCount) 
	: m_pVertexBuffer(nullptr)
	, m_pInstanceVertexBuffer(nullptr)
	, m_pInputLayout(nullptr)
	, m_pInputSignatureShader(inputSignatureShader)
	, m_topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
	, m_pVertexFormat(format)
	, m_pBuffer(nullptr)
	, m_pInstanceBuffer(nullptr)
	, m_vertexStride(m_pVertexFormat.getNumFloat() * 4)
	, m_vertexCount(maxVertexCount)
	, m_bufferSize(m_vertexStride * m_vertexCount)
	, m_instanceBufferSize(0u)
	, m_maxInstanceCount(maxInstanceCount)
	, m_nextAlignedOffset(0u)
	, m_nextInstanceAlignedOffset(0u)
{
	assert(maxVertexCount > 0);

	m_pBuffer = MALLOC(m_vertexStride * m_vertexCount);
	assert(m_pBuffer);

	initD3DObjects();
}

void VertexBuffer_D3D::resetMaxInstanceCount(unsigned int count)
{
	assert(count > 0);
	SafeRelease(m_pInstanceVertexBuffer);
	m_maxInstanceCount = count;
	m_pInstanceBuffer = nullptr;
	m_instanceBufferSize = 0u;
	
	createInstanceBuffer();
}

VertexBuffer_D3D::~VertexBuffer_D3D() 
{
	SafeRelease(m_pVertexBuffer);
	SafeRelease(m_pInstanceVertexBuffer);
	SafeRelease(m_pInputLayout);
	m_pInputSignatureShader = nullptr;

	if (m_pBuffer)
	{
		FREE(m_pBuffer);
	}
}

void VertexBuffer_D3D::createInstanceBuffer()
{
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = m_maxInstanceCount * m_pVertexFormat.getInstanceStride();
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = g_context.m_pDevice->CreateBuffer(&desc, nullptr, &m_pInstanceVertexBuffer);
	assert(SUCCEEDED(hr) && "Unable to create instance vertex buffer");
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

	// Create instance buffer
	if (m_maxInstanceCount > 0)
	{
		createInstanceBuffer();
	}

	// Create input layout
	{
		D3D11_INPUT_ELEMENT_DESC elements[member_size(VertexFormat, types) + VertexFormat::MaxInstanceElementCount];
		for (int elementIndex = 0; elementIndex < m_pVertexFormat.types_used; ++elementIndex)
		{
			switch (m_pVertexFormat.types[elementIndex])
			{
			case 'v':
				setPositionElement(elements[elementIndex]); break;
			case 'c':
				setColorElement(elements[elementIndex]); break;
			case 't':
				setUVElement(elements[elementIndex]); break;
			case 'n':
				setNormalElement(elements[elementIndex]); break;
			default:
				assert(false && "Invalid vertex format element");
			}
		}

		int vertexElementCount = m_pVertexFormat.types_used;
		int instanceElementCount = m_pVertexFormat.getInstanceElementCount();
		for (int instanceIndex = 0; instanceIndex < instanceElementCount; ++instanceIndex)
		{
			setInstanceElement(elements[vertexElementCount + instanceIndex], *m_pVertexFormat.getInstanceElement(instanceIndex));
		}

		g_context.m_pDevice->CreateInputLayout(
			elements, 
			vertexElementCount + instanceElementCount, 
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
	element.AlignedByteOffset = m_nextAlignedOffset;
	element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	element.InstanceDataStepRate = 0;

	m_nextAlignedOffset += 12u;
}

void VertexBuffer_D3D::setUVElement(D3D11_INPUT_ELEMENT_DESC &element)
{
	element.SemanticName = "TEXCOORD";
	element.SemanticIndex = 0;
	element.Format = DXGI_FORMAT_R32G32_FLOAT;
	element.InputSlot = 0;
	element.AlignedByteOffset = m_nextAlignedOffset ;
	element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	element.InstanceDataStepRate = 0;

	m_nextAlignedOffset += 8u;
}

void VertexBuffer_D3D::setNormalElement(D3D11_INPUT_ELEMENT_DESC &element)
{
	assert(false && "Vertex normal is not supported yet");

	/*
	element.SemanticName = "TEXCOORD";
	element.SemanticIndex = 1;
	element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	element.InputSlot = 0;
	element.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	element.InstanceDataStepRate = 0;

	m_nextAlignedOffset += 12u;
	*/
}

void VertexBuffer_D3D::setColorElement(D3D11_INPUT_ELEMENT_DESC &element)
{
	element.SemanticName = "COLOR";
	element.SemanticIndex = 0;
	element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	element.InputSlot = 0;
	element.AlignedByteOffset = m_nextAlignedOffset;
	element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	element.InstanceDataStepRate = 0;

	m_nextAlignedOffset += 16u;
}

void VertexBuffer_D3D::setInstanceElement(D3D11_INPUT_ELEMENT_DESC &element, const VertexFormat::Element &formatElement)
{
	if (formatElement.semantic == VertexFormat::SEMANTIC_COLOR)
	{
		element.SemanticName = "COLOR";
	}
	else
	{
		element.SemanticName = "TEXCOORD";
	}

	element.SemanticIndex = formatElement.semanticIndex;
	element.InputSlot = 1;
	element.AlignedByteOffset = m_nextInstanceAlignedOffset;
	element.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	element.InstanceDataStepRate = 1;

	switch (formatElement.floatSize)
	{
	case 1:
		element.Format = DXGI_FORMAT_R32_FLOAT;
		m_nextInstanceAlignedOffset += 4u;
		break;
	case 2:
		element.Format = DXGI_FORMAT_R32G32_FLOAT;
		m_nextInstanceAlignedOffset += 8u;
		break;
	case 3:
		element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		m_nextInstanceAlignedOffset += 12u;
		break;
	case 4:
		element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		m_nextInstanceAlignedOffset += 16u;
		break;
	default:
		assertmsg(false, "Invalid vertex element size");
		element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		m_nextInstanceAlignedOffset += 16u;
	}
}

void VertexBuffer_D3D::copyFromBuffer(void *data, int vert_cnt) 
{
	assert(m_bufferSize >= vert_cnt * m_vertexStride);

	m_vertexCount = vert_cnt;
	unsigned int size = m_vertexStride * m_vertexCount;
	
	memcpy(m_pBuffer, data, size);
}

void VertexBuffer_D3D::copyInstanceFromBuffer(const void *data, unsigned int size)
{
	assertmsg(size <= m_maxInstanceCount * m_pVertexFormat.getInstanceStride(), "Too much data from buffer size");

	m_instanceBufferSize = size;
	m_pInstanceBuffer = data;
}

void VertexBuffer_D3D::setCoord(unsigned int index, Vec3 v) 
{
	assert(index < m_vertexCount);

	int ofs = m_pVertexFormat.coord_offset;
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

	int ofs = m_pVertexFormat.coord_offset;
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

	int ofs = m_pVertexFormat.color_offset;
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

	int ofs = m_pVertexFormat.color_offset;
	assertmsg( ofs >= 0, "color have not declared in vertex format" );

	int index_in_array = index * m_vertexStride + ofs;
	float *buffer = static_cast<float*>(m_pBuffer);

	return Color(buffer[index_in_array], buffer[index_in_array+1], buffer[index_in_array+2], buffer[index_in_array+3]);
}

void VertexBuffer_D3D::setUV(unsigned int index, Vec2 uv) 
{
	assert( index < m_vertexCount );

	int ofs = m_pVertexFormat.texture_offset;
	assertmsg( ofs >= 0, "texcoord have not declared in vertex format");

	int index_in_array = index * m_vertexStride + ofs;
	float *buf = static_cast<float*>(m_pBuffer);

	buf[index_in_array] = uv.x;
	buf[index_in_array+1] = uv.y;
}

Vec2 VertexBuffer_D3D::getUV(unsigned int index) 
{
	assert( index < m_vertexCount );

	int ofs = m_pVertexFormat.texture_offset;
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

	int ofs = m_pVertexFormat.normal_offset;
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

	int ofs = m_pVertexFormat.normal_offset;
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

void VertexBuffer_D3D::copyInstancesToGPU()
{
	if (m_pInstanceVertexBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE map;
		D3D11_BUFFER_DESC desc;
		m_pInstanceVertexBuffer->GetDesc(&desc);
		HRESULT hr = g_context.m_pDeviceContext->Map(m_pInstanceVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
		memcpy(map.pData, m_pInstanceBuffer, m_instanceBufferSize);
		g_context.m_pDeviceContext->Unmap(m_pInstanceVertexBuffer, 0);
	}
}

void VertexBuffer_D3D::bind()
{
	UINT offset = 0;
	g_context.m_pDeviceContext->IASetInputLayout(m_pInputLayout);
	g_context.m_pDeviceContext->IASetPrimitiveTopology(m_topology);

	if (m_pInstanceVertexBuffer)
	{
		ID3D11Buffer *buffers[] = { m_pVertexBuffer, m_pInstanceVertexBuffer };
		UINT strides[] = { m_vertexStride, m_pVertexFormat.getInstanceStride() };
		UINT offsets[] = { 0u, 0u };
		g_context.m_pDeviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	}
	else
	{
		g_context.m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_vertexStride, &offset);
	}
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