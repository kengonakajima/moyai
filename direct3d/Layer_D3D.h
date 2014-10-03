#pragma once

#include "Viewport_D3D.h"
#include "../common/Light.h"
#include "../common/Mesh.h"
#include "Context_D3D.h"
#include "Color_D3D.h"

class PrimDrawer;
class Camera;
class TileDeck;
class Material;
class Texture_D3D;
class VertexBuffer_D3D;

class Layer_D3D : public Group 
{
	friend class Prop2D_D3D;

public:

	Camera *camera;
	Viewport_D3D *viewport;
	Light *light;

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

	int renderAllProps(int layerIndex);
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

	struct InstanceData
	{
		Color_D3D color;
		Vec2 offset;
		Vec2 scale;
		Vec2 uvOffset;
		Vec2 uvScale;
		Vec2 rotationDepth;
	};

	struct MaterialData
	{
		static const unsigned int DEFAULT_ID = -1;

		MaterialData() : id(DEFAULT_ID), shader(nullptr), texture(nullptr) {}

		unsigned int id;
		FragmentShader_D3D *shader;
		Texture_D3D *texture;

		bool operator==(const MaterialData &data) const
		{
			return (id == data.id) && (shader == data.shader) && (texture == data.texture);
		}
	};

	struct PrimitiveData
	{
		PrimitiveData() :drawer(nullptr), offset() {}
		PrimDrawer *drawer;
		Vec2 offset;
	};

	struct RenderData
	{
		InstanceData instanceData;
		MaterialData materialData;
		PrimitiveData primitiveData;
	};

	struct CallData
	{
		std::vector<InstanceData> instances;
		std::vector<PrimitiveData> primitives;
	};

	struct MaterialHash
	{
		size_t operator()(const MaterialData &data)
		{
			static const size_t shaderMask = size_t(-1) << (sizeof(size_t) >> 1u);
			static const size_t textureMask = size_t(-1) >> (sizeof(size_t) >> 1u);

			size_t shaderPtr = reinterpret_cast<size_t>(data.shader);
			size_t texturePtr = reinterpret_cast<size_t>(data.texture);
			return (shaderPtr & shaderMask) | (texturePtr & textureMask);
		}

		bool operator()(const MaterialData &data0, const MaterialData &data1)
		{
			return data0 == data1;
		}
	};

	typedef std::unordered_map<MaterialData, CallData, MaterialHash, MaterialHash> SortedRenderData;
	typedef std::pair<const MaterialData, CallData> SortedRenderDataPair;


	void init();
	int sendDrawCalls();
	void clearRenderData();
	RenderData& getNewRenderData();
	const MaterialData* getLastMaterial() const;

	Texture_D3D *m_pLastTexture;
	VertexBuffer_D3D *m_pBillboardVertexBuffer;
	VertexBuffer_D3D *m_pQuadVertexBuffer;
	ID3D11Buffer *m_pMatrixConstantBuffer;	
	std::vector<RenderData> m_renderData;
	std::vector<MaterialData> m_sortedMaterialData;
	SortedRenderData m_sortedRenderData;
	int m_layerIndex;
};