#include "Profiler/Data.h"

namespace Profiler::Detail
{
	std::uint64_t Data(ThreadState* state, void* data, std::size_t size)
	{
		std::uint8_t* ptr    = reinterpret_cast<std::uint8_t*>(data);
		std::uint64_t id     = NewDataID();
		auto&         header = NewEvent<DataHeaderEvent>(state);
		header.ID            = id;
		header.Size          = size;
		while (size > 0)
		{
			std::size_t toTransfer = std::min<std::size_t>(size, 32);
			auto&       section    = NewEvent<DataSectionEvent>(state);
			std::memcpy(section.Data, ptr, toTransfer);
			size -= toTransfer;
			ptr  += toTransfer;
		}
		return id;
	}
} // namespace Profiler::Detail