#include "Profiler/Thread.h"

namespace Profiler::Detail
{
	void ThreadBegin(ThreadState* state)
	{
		state->ThreadID = GetThreadID();
		g_State.addThread(state);

		auto& event = NewEvent<ThreadBeginEvent>(state);
		CaptureLowResTimestamp(event.Timestamp);
	}

	void ThreadEnd(ThreadState* state)
	{
		auto& event = NewEvent<ThreadEndEvent>(state);
		CaptureLowResTimestamp(event.Timestamp);
		FlushEvents(state);

		g_State.removeThread(state);

		if (state->FunctionDepth)
			throw std::runtime_error("Thread ended with unended functions!");
	}

	void HRThreadBegin(ThreadState* state)
	{
		state->ThreadID = GetThreadID();
		g_State.addThread(state);

		auto& event = NewEvent<ThreadBeginEvent>(state);
		CaptureHighResTimestamp(event.Timestamp);
	}

	void HRThreadEnd(ThreadState* state)
	{
		auto& event = NewEvent<ThreadEndEvent>(state);
		CaptureHighResTimestamp(event.Timestamp);
		FlushEvents(state);

		g_State.removeThread(state);

		if (state->FunctionDepth)
			throw std::runtime_error("Thread ended with unended functions!");
	}
} // namespace Profiler::Detail