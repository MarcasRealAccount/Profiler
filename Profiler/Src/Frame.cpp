#include "Profiler/Frame.h"

namespace Profiler::Detail
{
	void Frame(ThreadState* state)
	{
		if (state->FunctionDepth)
			throw std::runtime_error("Previous frame ended with unended functions, FIX YOUR FUCKING FUNCTION CALLS!");

		auto& event    = NewEvent<FrameEvent>(state);
		event.FrameNum = g_State.CurrentFrame++;
		CaptureLowResTimestamp(event.Timestamp);
	}

	void HRFrame(ThreadState* state)
	{
		if (state->FunctionDepth)
			throw std::runtime_error("Previous frame ended with unended functions, FIX YOUR FUCKING FUNCTION CALLS!");

		auto& event    = NewEvent<FrameEvent>(state);
		event.FrameNum = g_State.CurrentFrame++;
		CaptureHighResTimestamp(event.Timestamp);
	}
} // namespace Profiler::Detail