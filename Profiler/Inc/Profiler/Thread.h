#pragma once

#include "State.h"
#include "Timestamp.h"
#include "Utils/Core.h"

namespace Profiler
{
	namespace Detail
	{
		BUILD_NEVER_INLINE void ThreadBegin(ThreadState* state);
		BUILD_NEVER_INLINE void ThreadEnd(ThreadState* state);
		BUILD_NEVER_INLINE void HRThreadBegin(ThreadState* state);
		BUILD_NEVER_INLINE void HRThreadEnd(ThreadState* state);
	} // namespace Detail

	inline void ThreadBegin()
	{
		ThreadState* state = GetThreadState();
		g_State.addThread(state);
		if (state->Capture)
			Detail::ThreadBegin(state);
	}

	inline void ThreadEnd()
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::ThreadEnd(state);
		g_State.removeThread(state);
		FreeThreadState(state);
	}

	inline void HRThreadBegin()
	{
		ThreadState* state = GetThreadState();
		g_State.addThread(state);
		if (state->Capture)
			Detail::HRThreadBegin(state);
	}

	inline void HRThreadEnd()
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::HRThreadEnd(state);
		g_State.removeThread(state);
		FreeThreadState(state);
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