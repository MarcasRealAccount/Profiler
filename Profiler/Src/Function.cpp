#include "Profiler/Function.h"

namespace Profiler::Detail
{
	void FunctionBegin(void* functionPtr)
	{
		auto& event       = NewEvent<FunctionBeginEvent>();
		event.FunctionPtr = functionPtr;
		CaptureLowResTimestamp(event.Timestamp);
		++g_TState.FunctionDepth;
	}

	void FunctionEnd()
	{
		auto& event = NewEvent<FunctionEndEvent>();
		CaptureLowResTimestamp(event.Timestamp);
		--g_TState.FunctionDepth;
	}

	void HRFunctionBegin(void* functionPtr)
	{
		auto& event       = NewEvent<FunctionBeginEvent>();
		event.FunctionPtr = functionPtr;
		CaptureHighResTimestamp(event.Timestamp);
		++g_TState.FunctionDepth;
	}

	void HRFunctionEnd()
	{
		auto& event = NewEvent<FunctionEndEvent>();
		CaptureHighResTimestamp(event.Timestamp);
		--g_TState.FunctionDepth;
	}

	void BoolArg(std::uint8_t offset, bool value)
	{
		auto& event  = NewEvent<BoolArgumentEvent>();
		event.Offset = offset;
		event.Value  = value;
	}

	void PtrArg(std::uint8_t offset, void* ptr)
	{
		auto& event  = NewEvent<PtrArgumentEvent>();
		event.Offset = offset;
		event.Ptr    = ptr;
	}
} // namespace Profiler::Detail