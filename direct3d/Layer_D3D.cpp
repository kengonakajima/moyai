#ifdef USE_D3D

#include "Layer_D3D.h"

#include "../common.h"
#include "../common/TileDeck.h"
#include "../common/Camera.h"
#include "../common/Material.h"
#include "../common/Prop2D.h"
#include "../common/Prop3D.h"
#include "../common/GPUMarker.h"

#include "VertexBuffer_D3D.h"


Layer_D3D::Layer_D3D() 
	: Group()
	, camera(nullptr)
	, viewport(nullptr)
	, light(nullptr)
	, m_pLastTexture(nullptr)
	, m_pBillboardVertexBuffer(nullptr)
	, m_pQuadVertexBuffer(nullptr)
	, m_pMatrixConstantBuffer(nullptr)
	, m_renderData()
	, m_primitiveData()
	, m_sortedRenderData()
	, m_layerIndex(0)
{
	to_render = true;
	init();
}

Layer_D3D::~Layer_D3D()
{
	m_pLastTexture = nullptr;
	SafeDelete(m_pBillboardVertexBuffer);
	SafeDelete(m_pQuadVertexBuffer);
	SafeRelease(m_pMatrixConstantBuffer);
}

void Layer_D3D::init()
{
	// Create vertex buffer
	{
		VertexFormat format;
		format.declareCoordVec3();
		format.declareColor();
		format.declareUV();
		m_pBillboardVertexBuffer = new VertexBuffer_D3D(format, 6, g_context.m_pShaderManager->GetShader(ShaderManager_D3D::SHADER_DEFAULT));

		VertexFormat instanceFormat;
		instanceFormat.declareCoordVec3();
		instanceFormat.declareUV();
		instanceFormat.addInstanceElement(VertexFormat::SEMANTIC_COLOR, 0u, 4u);
		instanceFormat.addInstanceElement(VertexFormat::SEMANTIC_TEXCOORD, 1u, 4u);
		instanceFormat.addInstanceElement(VertexFormat::SEMANTIC_TEXCOORD, 2u, 4u);
		instanceFormat.addInstanceElement(VertexFormat::SEMANTIC_TEXCOORD, 3u, 2u);
		m_pQuadVertexBuffer = new VertexBuffer_D3D(instanceFormat, 6, g_context.m_pShaderManager->GetShader(ShaderManager_D3D::SHADER_INSTANCING), 2000u);

		Vertex_PUV vertices[] = 
		{
			{ XMFLOAT3(-0.5f,  0.5f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3( 0.5f, -0.5f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(-0.5f, -0.5f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

			{ XMFLOAT3( 0.5f, -0.5f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(-0.5f,  0.5f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3( 0.5f,  0.5f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
		};

		m_pQuadVertexBuffer->copyFromBuffer(vertices, ARRAYSIZE(vertices));
		m_pQuadVertexBuffer->copyToGPU();
	}

	// Create constant buffer
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ByteWidth = sizeof(CBufferMVP);
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = g_context.m_pDevice->CreateBuffer(&desc, nullptr, &m_pMatrixConstantBuffer);
		assert(SUCCEEDED(hr) && "Unable to create constant buffer");
	}
}

void Layer_D3D::drawBillboard(int billboard_index, TileDeck *deck, Vec3 *loc, Vec3 *scl) 
{
	if(billboard_index <0) return;    

	assert(deck);

	if (deck->tex != m_pLastTexture)
	{
		deck->tex->bind();
		m_pLastTexture = deck->tex;
	}

	Vec3 diff_camera = *loc - camera->loc;
	Vec3 up_v(0.0f, 1.0f, 0.0f);

	Vec3 cross_a = diff_camera.cross(up_v).normalize(1); 
	Vec3 cross_b = diff_camera.cross(cross_a).normalize(1); 

	// now you can use CrossA and CrossB and the billboard position to calculate the positions of the edges of the billboard-rectangle
	cross_a.x *= scl->x * 0.5;        
	cross_a.y *= scl->y * 0.5;
	cross_a.z *= scl->z * 0.5;
	cross_b.x *= scl->x * 0.5;        
	cross_b.y *= scl->y * 0.5;
	cross_b.z *= scl->z * 0.5;

	Vec3 p1 = *loc + cross_a + cross_b;
	Vec3 p2 = *loc - cross_a + cross_b;
	Vec3 p3 = *loc - cross_a - cross_b;
	Vec3 p4 = *loc + cross_a - cross_b;

	float u0 = 0, v0 = 0, u1 = 1, v1 = 1;
	if( deck ) deck->getUVFromIndex( billboard_index, &u0, &v0, &u1, &v1, 0, 0, 0.0001 );

	Vertex_PCUV vertices[] = 
	{
		{ XMFLOAT3(p1.x, p1.y, p1.z), XMFLOAT2(u1, v1), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(p3.x, p3.y, p3.z), XMFLOAT2(u0, v0), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(p2.x, p2.y, p2.z), XMFLOAT2(u0, v1), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },

		{ XMFLOAT3(p1.x, p1.y, p1.z), XMFLOAT2(u1, v1), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(p4.x, p4.y, p4.z), XMFLOAT2(u1, v0), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(p3.x, p3.y, p3.z), XMFLOAT2(u0, v0), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }
	};

	m_pBillboardVertexBuffer->copyFromBuffer(vertices, sizeof(vertices));
	m_pBillboardVertexBuffer->copyToGPU();
	m_pBillboardVertexBuffer->bind();

	g_context.m_pDeviceContext->Draw(6, 0);
}

void Layer_D3D::drawMesh( int dbg, Mesh *mesh, TileDeck *deck, Vec3 *loc, Vec3 *locofs, Vec3 *scl, Vec3 *rot, Vec3 *localloc, Vec3 *localscl, Vec3 *localrot, Material *material  ) 
{
	
}

int Layer_D3D::renderAllProps(int layerIndex)
{
	assertmsg( viewport != nullptr, "no viewport in a layer id:%d setViewport missed?", id );

	m_layerIndex = layerIndex;

	if( viewport->dimension == DIMENSION_2D ) 
	{
		XMMATRIX orthoProjMatrix = XMMatrixOrthographicLH(viewport->scl.x, viewport->scl.y, 0.0f, 1000.0f);
		XMMATRIX identityMatrix = XMMatrixIdentity();
		CBufferMVP cbuffer(identityMatrix, orthoProjMatrix);

		D3D11_MAPPED_SUBRESOURCE map;
		g_context.m_pDeviceContext->Map(m_pMatrixConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
		memcpy(map.pData, &cbuffer, sizeof(CBufferMVP));
		g_context.m_pDeviceContext->Unmap(m_pMatrixConstantBuffer, 0);

		g_context.m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pMatrixConstantBuffer);
		g_context.m_pDeviceContext->PSSetConstantBuffers(0, 1, &m_pMatrixConstantBuffer);

		static SorterEntry tosort[1024*32];

		int cnt = 0;
		Prop *cur = prop_top;

		Vec2 minv, maxv;
		viewport->getMinMax(&minv, &maxv);

		while(cur)
		{
			Prop2D_D3D *cur2d = (Prop2D_D3D*)cur;

			assert( cur2d->dimension == viewport->dimension );

			// culling
			float camx=0,camy=0;
			if(camera)
			{
				camx = camera->loc.x;
				camy = camera->loc.y;
			}

			float scr_maxx = cur2d->loc.x - camx + cur2d->scl.x/2 + cur2d->max_rt_cache.x;
			float scr_minx = cur2d->loc.x - camx - cur2d->scl.x/2 + cur2d->min_lb_cache.x;
			float scr_maxy = cur2d->loc.y - camy + cur2d->scl.y/2 + cur2d->max_rt_cache.y;
			float scr_miny = cur2d->loc.y - camy - cur2d->scl.y/2 + cur2d->min_lb_cache.y;

			if( scr_maxx >= minv.x && scr_minx <= maxv.x && scr_maxy >= minv.y && scr_miny <= maxv.y )
			{
				tosort[cnt].val = cur2d->priority;
				tosort[cnt].ptr = cur2d;
				cnt++;

				if(cnt>= elementof(tosort))
				{
					print("WARNING: too many props in a layer : %d", cnt );
					break;
				}
			} 

			cur = cur->next;
		}

		quickSortF( tosort, 0, cnt-1 );
		for(int i=0;i<cnt;i++) 
		{
			Prop2D_D3D *p = (Prop2D_D3D*) tosort[i].ptr;
			if(p->visible)
			{
				p->render(camera);
			}
		}

		return sendDrawCalls();
	} 
	else // 3D
	{ 
		return 0;
	}
}

Layer_D3D::RenderData& Layer_D3D::getNewRenderData()
{
	m_renderData.push_back(RenderData());
	RenderData &renderData = m_renderData.back();
	renderData.instanceData.rotationDepth.y = (float)m_layerIndex++;
	return renderData;
}

Layer_D3D::PrimitiveData& Layer_D3D::getNewPrimitiveData()
{
	m_primitiveData.push_back(PrimitiveData());
	return m_primitiveData.back();
}

void Layer_D3D::clearRenderData()
{
	for (SortedRenderData::iterator iter = m_sortedRenderData.begin(); iter != m_sortedRenderData.end(); ++iter)
	{
		SortedRenderDataPair &instances = *iter;
		instances.second.clear();
	}

	m_renderData.clear();
	m_primitiveData.clear();
}

int Layer_D3D::sendDrawCalls()
{
	// Sort draw calls per material (shader/texture)
	for (std::vector<RenderData>::const_iterator iter = m_renderData.cbegin(); iter != m_renderData.cend(); ++iter)
	{
		const RenderData &renderData = *iter;
		std::vector<InstanceData> &instances = m_sortedRenderData[renderData.materialData];
		instances.push_back(renderData.instanceData);
	}

	GPU_BEGIN_EVENT("Layer_D3D::sendDrawCalls");
	g_context.m_pDeviceContext->OMSetDepthStencilState(g_context.m_pDepthStencilState, 0);

	for (SortedRenderData::const_iterator iter = m_sortedRenderData.cbegin(); iter != m_sortedRenderData.cend(); ++iter)
	{
		const SortedRenderDataPair &pair = *iter;
		const MaterialData &material = pair.first;
		const std::vector<InstanceData> &instances = pair.second;
		UINT instanceCount = instances.size();

		if (instanceCount > 0)
		{
			material.shader->bind();
			material.shader->updateUniforms();
			material.texture->bind();

			if (m_pQuadVertexBuffer->getMaxInstanceCount() < instanceCount)
			{
				m_pQuadVertexBuffer->resetMaxInstanceCount(instanceCount);
			}

			m_pQuadVertexBuffer->copyInstanceFromBuffer(instances.data(), sizeof(InstanceData) * instanceCount);
			m_pQuadVertexBuffer->copyInstancesToGPU();
			m_pQuadVertexBuffer->bind();
			g_context.m_pDeviceContext->DrawInstanced(6, instanceCount, 0, 0);
		}
	}

	// Primitives should go over image sprites
	g_context.m_pDeviceContext->OMSetDepthStencilState(g_context.m_pNoDepthTestState, 0);
	for (std::vector<PrimitiveData>::const_iterator iter = m_primitiveData.cbegin(); iter != m_primitiveData.cend(); ++iter)
	{
		const PrimitiveData &primData = *iter;
		if (primData.drawer)
		{
			primData.drawer->drawAll(primData.offset);
		}
	}
	GPU_END_EVENT();

	int spriteCount = m_renderData.size();
	clearRenderData();

	return spriteCount;
}

void Layer_D3D::setupProjectionMatrix3D() 
{
	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective( 60, (GLdouble)viewport->screen_width/(GLdouble)viewport->screen_height, viewport->near_clip, viewport->far_clip );
	gluLookAt( camera->loc.x,camera->loc.y,camera->loc.z,
		camera->look_at.x,camera->look_at.y,camera->look_at.z,
		camera->look_up.x,camera->look_up.y,camera->look_up.z );
	*/
}

Vec2 Layer_D3D::getScreenPos( Vec3 at ) 
{
	return Vec2(0, 0);

	/*
	setupProjectionMatrix3D();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	double projection[16], modelview[16];
	double sx,sy,sz;
	int vp[4];

	glGetIntegerv( GL_VIEWPORT, vp );    
	glGetDoublev(GL_PROJECTION_MATRIX, projection );
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview );

	gluProject( at.x, at.y, at.z, modelview, projection, vp, &sx, &sy, &sz );
	return Vec2( sx,sy );
	*/
}

Vec3 Layer_D3D::getWorldPos( Vec2 scrpos ) 
{
	return Vec3(0, 0, 0);

	/*
	setupProjectionMatrix3D();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	double projection[16], modelview[16];
	int vp[4];
	glGetIntegerv( GL_VIEWPORT, vp );
	glGetDoublev(GL_PROJECTION_MATRIX, projection );
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview );

	float z;
	double ox,oy,oz;
	Vec3 out;
	glReadPixels( scrpos.x, scrpos.y, 1,1, GL_DEPTH_COMPONENT, GL_FLOAT, &z );
	gluUnProject( scrpos.x, scrpos.y, z, modelview, projection, vp, &ox, &oy, &oz );
	return Vec3(ox,oy,oz);
	*/
}

int Layer_D3D::getHighestPriority() 
{
	int prio = 0;
	Prop *cur = prop_top;
	while(cur) {
		Prop2D_D3D *p = (Prop2D_D3D*)cur;
		if( p->priority > prio ) prio = p->priority;
		cur = cur->next;
	}
	return prio;
}

void Layer_D3D::selectCenterInside( Vec2 minloc, Vec2 maxloc, Prop*out[], int *outlen )
{
	assertmsg( viewport->dimension == DIMENSION_2D, "selectCenterInside isn't implemented for 3d viewport" );

	Prop *cur = prop_top;
	int out_max = *outlen;
	int cnt=0;

	while(cur){
		Prop2D_D3D *cur2d = (Prop2D_D3D*) cur;
		if( cur2d->dimension == DIMENSION_2D ){
			if( !cur->to_clean && cur2d->isCenterInside(minloc, maxloc) ){
				if( cnt < out_max){
					out[cnt] = cur;
					cnt++;
					if(cnt==out_max)break;
				}
			}
		}
		cur = cur->next;
	}
	*outlen = cnt;
}

#endif