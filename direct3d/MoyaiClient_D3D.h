#pragma once

#include "../common.h"
#include "../common/Layer.h"
#include "glfw_D3D.h"

#include "Context_D3D.h"

class MoyaiClient_D3D : public Moyai 
{
public:

	MoyaiClient_D3D();
	virtual ~MoyaiClient_D3D();

	int render();
	void capture( Image *img );

	void insertLayer( Layer *l ) 
	{
		insertGroup( l );
	}

private:

	ID3D11BlendState *m_pBlendState;
};