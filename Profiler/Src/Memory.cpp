#include "Profiler/Memory.h"

namespace Profiler::Detail
{
	void MemAlloc(ThreadState* state, void* memory, std::uint64_t size)
	{
		auto& data  = NewEvent<MemAllocEvent>(state);
		data.Memory = memory;
		data.Size   = size;
		CaptureLowResTimestamp(data.Timestamp);
	}

	void HRMemAlloc(ThreadState* state, void* memory, std::uint64_t size)
	{
		auto& data  = NewEvent<MemAllocEvent>(state);
		data.Memory = memory;
		data.Size   = size;
		CaptureHighResTimestamp(data.Timestamp);
	}

	void MemFree(ThreadState* state, void* memory)
	{
		auto& data  = NewEvent<MemFreeEvent>(state);
		data.Memory = memory;
		CaptureLowResTimestamp(data.Timestamp);
	}

	void HRMemFree(ThreadState* state, void* memory)
	{
		auto& data  = NewEvent<MemFreeEvent>(state);
		data.Memory = memory;
		CaptureHighResTimestamp(data.Timestamp);
	}
} // namespace Profiler::Detail