#pragma once

#include "State.h"
#include "Utils/Core.h"

namespace Profiler
{
	namespace Detail
	{
		BUILD_NEVER_INLINE std::uint64_t Data(ThreadState* state, void* data, std::size_t size);
	}

	inline std::uint64_t Data(void* data, std::size_t size)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			return Detail::Data(state, data, size);
	}
} // namespace Profiler