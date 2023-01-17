#include "Profiler/Callstack.h"
#include "Profiler/Data.h"

namespace Profiler::Detail
{
	void Callstack(ThreadState* state, void** callstack, std::size_t callstackSize)
	{
		std::uint64_t id   = Data(state, callstack, callstackSize * sizeof(void*));
		auto&         data = NewEvent<CallstackEvent>(state);
		data.DataID        = id;
		data.NumEntries    = callstackSize;
	}

	void** CollectCallstack(std::size_t& size)
	{
		// TODO(MarcasRealAccount): Implement Callstack Collection
		size = 0;
		return nullptr;
	}

	void FreeCallstack([[maybe_unused]] void** callstack)
	{
		// TODO(MarcasRealAccount): Implement Callstack Freeing
	}
} // namespace Profiler::Detail