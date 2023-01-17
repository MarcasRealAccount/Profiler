#include "Profiler/ForLoop.h"

namespace Profiler::Detail
{
	std::uint64_t ForLoopBegin(ThreadState* state)
	{
		std::uint64_t id = state->ForLoopDepth++;

		auto& data = NewEvent<ForLoopBeginEvent>(state);
		data.ID    = id;
		CaptureLowResTimestamp(data.Timestamp);
		return id;
	}

	void ForLoopEnd(ThreadState* state, std::uint64_t id)
	{
		auto& data = NewEvent<ForLoopEndEvent>(state);
		data.ID    = id;
		CaptureLowResTimestamp(data.Timestamp);
	}

	std::uint64_t HRForLoopBegin(ThreadState* state)
	{
		std::uint64_t id = state->ForLoopDepth++;

		auto& data = NewEvent<ForLoopBeginEvent>(state);
		data.ID    = id;
		CaptureHighResTimestamp(data.Timestamp);
		return id;
	}

	void HRForLoopEnd(ThreadState* state, std::uint64_t id)
	{
		auto& data = NewEvent<ForLoopEndEvent>(state);
		data.ID    = id;
		CaptureHighResTimestamp(data.Timestamp);
	}

	void ForLoopIterBegin(ThreadState* state, std::uint64_t id, std::uint8_t size, std::uint64_t (&values)[2])
	{
		auto& data = NewEvent<ForLoopIterBeginEvent>(state);
		data.Size  = size;
		data.ID    = id;
		CaptureLowResTimestamp(data.Timestamp);
		std::memcpy(data.Index, values, sizeof(values));
	}

	void ForLoopIterEnd(ThreadState* state, std::uint64_t id)
	{
		auto& data = NewEvent<ForLoopIterEndEvent>(state);
		data.ID    = id;
		CaptureLowResTimestamp(data.Timestamp);
	}

	void HRForLoopIterBegin(ThreadState* state, std::uint64_t id, std::uint8_t size, std::uint64_t (&values)[2])
	{
		auto& data = NewEvent<ForLoopIterBeginEvent>(state);
		data.Size  = size;
		data.ID    = id;
		CaptureHighResTimestamp(data.Timestamp);
		std::memcpy(data.Index, values, sizeof(values));
	}

	void HRForLoopIterEnd(ThreadState* state, std::uint64_t id)
	{
		auto& data = NewEvent<ForLoopIterEndEvent>(state);
		data.ID    = id;
		CaptureHighResTimestamp(data.Timestamp);
	}
} // namespace Profiler::Detail