#pragma once

#include "Viewport_D3D.h"
#include "../common/Light.h"
#include "../common/Mesh.h"
#include "Context_D3D.h"

class Camera;
class TileDeck;
class Material;
class Texture_D3D;
class VertexBuffer_D3D;

class Layer_D3D : public Group 
{
	friend class Prop2D_D3D;

public:

	struct RenderData
	{
		RenderData() {}
		Color color;
		Vec2 offset;
		Vec2 scale;
		Vec2 uvOffset;
		Vec2 uvScale;
		float rotation;
		FragmentShader_D3D *shader;
		Texture_D3D *texture;
	};

	Camera *camera;
	Viewport_D3D *viewport;
	Light *light;

	// working area to avoid allocation in inner loops
	SorterEntry sorter_opaque[Prop::CHILDREN_ABS_MAX];
	SorterEntry sorter_transparent[Prop::CHILDREN_ABS_MAX];

	Layer_D3D();
	virtual ~Layer_D3D();

	inline void setViewport( Viewport_D3D *vp )\
	{
		viewport = vp;
	}

	inline void setCamera(Camera *cam)
	{
		camera = cam;
	}

	inline void setLight(Light *l)
	{
		light = l;
	}

	int renderAllProps();
	void selectCenterInside( Vec2 minloc, Vec2 maxloc, Prop*out[], int *outlen );

	inline void selectCenterInside( Vec2 center, float dia, Prop *out[], int *outlen)
	{
		selectCenterInside( center - Vec2(dia,dia),
			center + Vec2(dia,dia),
			out, outlen );
	}

	void drawMesh( int dbg, Mesh *mesh, TileDeck *deck, Vec3 *loc, Vec3 *locofs, Vec3 *scl, Vec3 *rot, Vec3 *localloc, Vec3 *localscl, Vec3 *localrot, Material *material  );
	void drawBillboard(int billboard_index, TileDeck *deck, Vec3 *loc, Vec3 *scl  );

	void setupProjectionMatrix3D();
	Vec2 getScreenPos( Vec3 at );
	Vec3 getWorldPos( Vec2 scrpos );

	int getHighestPriority();

private:

	void init();
	int sendDrawCalls();

	Texture_D3D *m_pLastTexture;
	VertexBuffer_D3D *m_pBillboardVertexBuffer;
	VertexBuffer_D3D *m_pQuadVertexBuffer;
	ID3D11Buffer *m_pMatrixConstantBuffer;	
	std::vector<RenderData> m_renderData;
};