#include "Profiler/Thread.h"

namespace Profiler::Detail
{
	void ThreadBegin()
	{
		g_TState.ThreadID = GetThreadID();
		g_State.addThread(&g_TState);

		auto& event = NewEvent<ThreadBeginEvent>();
		CaptureLowResTimestamp(event.Timestamp);
	}

	void ThreadEnd()
	{
		auto& event = NewEvent<ThreadEndEvent>();
		CaptureLowResTimestamp(event.Timestamp);
		FlushEvents();

		g_State.removeThread(&g_TState);

		if (g_TState.FunctionDepth)
			throw std::runtime_error("Thread ended with unended functions!");
	}

	void HRThreadBegin()
	{
		g_TState.ThreadID = GetThreadID();
		g_State.addThread(&g_TState);

		auto& event = NewEvent<ThreadBeginEvent>();
		CaptureHighResTimestamp(event.Timestamp);
	}

	void HRThreadEnd()
	{
		auto& event = NewEvent<ThreadEndEvent>();
		CaptureHighResTimestamp(event.Timestamp);
		FlushEvents();

		g_State.removeThread(&g_TState);

		if (g_TState.FunctionDepth)
			throw std::runtime_error("Thread ended with unended functions!");
	}
} // namespace Profiler::Detail