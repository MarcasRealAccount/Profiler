#pragma once

#include "State.h"
#include "Utils/Core.h"
#include "Utils/IntrinsicsThatClangDoesntSupport.h"

#include <cstdint>

#include <chrono>

namespace Profiler
{
	inline void CaptureLowResTimestamp(EventTimestamp& timestamp)
	{
		timestamp.Time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		timestamp.Type = 0;
	}

	inline void CaptureHighResTimestamp(EventTimestamp& timestamp)
	{
		timestamp.Time = Utils::rdtsc();
		timestamp.Type = 1;
	}
} // namespace Profiler