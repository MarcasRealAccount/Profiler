#include "Profiler/Function.h"

namespace Profiler::Detail
{
	void FunctionBegin(ThreadState* state, void* functionPtr)
	{
		auto& event       = NewEvent<FunctionBeginEvent>(state);
		event.FunctionPtr = functionPtr;
		CaptureLowResTimestamp(event.Timestamp);
		++state->FunctionDepth;
	}

	void FunctionEnd(ThreadState* state)
	{
		auto& event = NewEvent<FunctionEndEvent>(state);
		CaptureLowResTimestamp(event.Timestamp);
		--state->FunctionDepth;
	}

	void HRFunctionBegin(ThreadState* state, void* functionPtr)
	{
		auto& event       = NewEvent<FunctionBeginEvent>(state);
		event.FunctionPtr = functionPtr;
		CaptureHighResTimestamp(event.Timestamp);
		++state->FunctionDepth;
	}

	void HRFunctionEnd(ThreadState* state)
	{
		auto& event = NewEvent<FunctionEndEvent>(state);
		CaptureHighResTimestamp(event.Timestamp);
		--state->FunctionDepth;
	}

	void BoolArg(ThreadState* state, std::uint8_t offset, bool value)
	{
		auto& event  = NewEvent<BoolArgumentEvent>(state);
		event.Offset = offset;
		event.Value  = value;
	}

	void IntArg(ThreadState* state, std::uint8_t offset, std::uint8_t size, std::uint64_t (&values)[3], std::uint8_t base)
	{
		auto& event  = NewEvent<IntArgumentEvent>(state);
		event.Offset = offset;
		event.Size   = size;
		event.Base   = base;
		std::memcpy(event.Data, values, sizeof(values));
	}

	void FloatArg(ThreadState* state, std::uint8_t offset, std::uint8_t size, std::uint64_t (&values)[3])
	{
		auto& event  = NewEvent<FloatArgumentEvent>(state);
		event.Offset = offset;
		event.Size   = size;
		std::memcpy(event.Data, values, sizeof(values));
	}

	void FlagsArg(ThreadState* state, std::uint8_t offset, std::uint64_t flagsType, std::uint64_t (&values)[2])
	{
		auto& event     = NewEvent<FlagsArgumentEvent>(state);
		event.Offset    = offset;
		event.FlagsType = flagsType;
		std::memcpy(event.Data, values, sizeof(values));
	}

	void PtrArg(ThreadState* state, std::uint8_t offset, void* ptr)
	{
		auto& event  = NewEvent<PtrArgumentEvent>(state);
		event.Offset = offset;
		event.Ptr    = ptr;
	}
} // namespace Profiler::Detail