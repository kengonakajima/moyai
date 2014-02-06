#pragma once

#include <d3d11_1.h>
#include "Context_D3D.h"

#define GPU_BEGIN_EVENT(msg) g_context.m_pGPUMarker->BeginEvent(msg)
#define GPU_END_EVENT() g_context.m_pGPUMarker->EndEvent()

class GPUMarker_D3D
{
public:

	GPUMarker_D3D(ID3D11DeviceContext *context);
	~GPUMarker_D3D();

	void BeginEvent(const char *msg);
	void EndEvent();

private:

	ID3DUserDefinedAnnotation *m_pPerf;
};