#include "PerformanceCounter.h"

#if defined(WIN32) && PROFILE

#include <stdio.h>

double PerformanceCounter::g_perfFreq = -1.0;

PerformanceCounter::PerformanceCounter(const char *msg)
	: m_msg(msg)
{
	LARGE_INTEGER perfFreq;
	if (g_perfFreq < 0 && QueryPerformanceFrequency(&perfFreq))
	{
		g_perfFreq = double(perfFreq.QuadPart) / 1000.0;
	}

	QueryPerformanceCounter(&m_perfCounterStart);
}

PerformanceCounter::~PerformanceCounter()
{
	LARGE_INTEGER perfCounterEnd;
	QueryPerformanceCounter(&perfCounterEnd);

	char debugStr[512];
	sprintf_s(debugStr, sizeof(debugStr), "%s: %f ms\n", m_msg, double(perfCounterEnd.QuadPart - m_perfCounterStart.QuadPart) / g_perfFreq);
	OutputDebugStringA(debugStr);
}

#endif