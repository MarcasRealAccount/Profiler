#pragma once

#include "State.h"
#include "Utils/Core.h"

namespace Profiler
{
	namespace Detail
	{
		BUILD_NEVER_INLINE void Callstack(ThreadState* state, void** callstack, std::size_t callstackSize);
	}

	inline void Callstack(void** callstack, std::size_t callstackSize)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::Callstack(state, callstack, callstackSize);
	}

	void** CollectCallstack(std::size_t& size);
	void   FreeCallstack(void** callstack);
} // namespace Profiler