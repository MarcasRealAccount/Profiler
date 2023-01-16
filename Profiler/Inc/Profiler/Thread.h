#pragma once

#include "State.h"
#include "Timestamp.h"
#include "Utils/Core.h"

namespace Profiler
{
	namespace Detail
	{
		BUILD_NEVER_INLINE void ThreadBegin();
		BUILD_NEVER_INLINE void ThreadEnd();
		BUILD_NEVER_INLINE void HRThreadBegin();
		BUILD_NEVER_INLINE void HRThreadEnd();
	} // namespace Detail

	inline void ThreadBegin()
	{
		g_TState.Capture = g_State.Initialized && g_State.Capturing;
		if (g_TState.Capture)
			Detail::ThreadBegin();
	}

	inline void ThreadEnd()
	{
		if (g_TState.Capture)
			Detail::ThreadEnd();
	}

	inline void HRThreadBegin()
	{
		g_TState.Capture = g_State.Initialized && g_State.Capturing;
		if (g_TState.Capture)
			Detail::HRThreadBegin();
	}

	inline void HRThreadEnd()
	{
		if (g_TState.Capture)
			Detail::HRThreadEnd();
	}

	struct RAIIThread
	{
	public:
		RAIIThread() { ThreadBegin(); }

		~RAIIThread() { ThreadEnd(); }
	};

	struct RAIIHRThread
	{
	public:
		RAIIHRThread() { HRThreadBegin(); }

		~RAIIHRThread() { HRThreadEnd(); }
	};

	inline RAIIThread Thread()
	{
		return RAIIThread {};
	}

	inline RAIIHRThread HRThread()
	{
		return RAIIHRThread {};
	}
} // namespace Profiler