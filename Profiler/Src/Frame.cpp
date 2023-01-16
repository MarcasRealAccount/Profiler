#include "Profiler/Frame.h"

namespace Profiler::Detail
{
	void Frame()
	{
		if (g_TState.FunctionDepth)
			throw std::runtime_error("Previous frame ended with unended functions, FIX YOUR FUCKING FUNCTION CALLS!");

		auto& event    = NewEvent<FrameEvent>();
		event.FrameNum = g_State.CurrentFrame++;
		CaptureLowResTimestamp(event.Timestamp);
	}

	void HRFrame()
	{
		if (g_TState.FunctionDepth)
			throw std::runtime_error("Previous frame ended with unended functions, FIX YOUR FUCKING FUNCTION CALLS!");

		auto& event    = NewEvent<FrameEvent>();
		event.FrameNum = g_State.CurrentFrame++;
		CaptureHighResTimestamp(event.Timestamp);
	}
} // namespace Profiler::Detail