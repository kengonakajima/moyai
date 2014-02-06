#include "GPUMarker_D3D.h"

GPUMarker_D3D::GPUMarker_D3D(ID3D11DeviceContext *context) 
	: m_pPerf(nullptr)
{
	HRESULT hr = context->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&m_pPerf);
	if (FAILED(hr))
	{
		m_pPerf = nullptr;
	}
}

GPUMarker_D3D::~GPUMarker_D3D() 
{
	SafeRelease(m_pPerf);
}

void GPUMarker_D3D::BeginEvent(const char *msg) 
{
	wchar_t wMsg[1024];
	size_t convertedCount;
	mbstowcs_s(&convertedCount, wMsg, 1024, msg, _TRUNCATE);
	m_pPerf->BeginEvent(wMsg);
}

void GPUMarker_D3D::EndEvent() 
{
	m_pPerf->EndEvent();
}
