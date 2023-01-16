#pragma once

#include "State.h"

#include <chrono>

#include <intrin.h>

namespace Profiler
{
	inline void CaptureLowResTimestamp(EventTimestamp& timestamp)
	{
		timestamp.Time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		timestamp.Type = 0;
	}

	inline void CaptureHighResTimestamp(EventTimestamp& timestamp)
	{
		timestamp.Time = __rdtsc();
		timestamp.Type = 1;
	}
} // namespace Profiler