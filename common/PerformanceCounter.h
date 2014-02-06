#pragma once

#define CPU_PROFILE (0)

#if defined(WIN32) && CPU_PROFILE

	#include <windows.h>

	class PerformanceCounter
	{
	public:

		PerformanceCounter(const char *msg);
		~PerformanceCounter();

	private:

		static double g_perfFreq;
		LARGE_INTEGER m_perfCounterStart;
		const char *m_msg;
	};

#else

	struct PerformanceCounter
	{
		PerformanceCounter(const char *msg) {}
	};

#endif
