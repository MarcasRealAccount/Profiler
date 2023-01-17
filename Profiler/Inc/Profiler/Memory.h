#pragma once

#include "State.h"
#include "Timestamp.h"
#include "Utils/Core.h"

namespace Profiler
{
	namespace Detail
	{
		BUILD_NEVER_INLINE void MemAlloc(ThreadState* state, void* memory, std::uint64_t size);
		BUILD_NEVER_INLINE void HRMemAlloc(ThreadState* state, void* memory, std::uint64_t size);
		BUILD_NEVER_INLINE void MemFree(ThreadState* state, void* memory);
		BUILD_NEVER_INLINE void HRMemFree(ThreadState* state, void* memory);
	} // namespace Detail

	inline void MemAlloc(void* memory, std::uint64_t size)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::MemAlloc(state, memory, size);
	}

	inline void HRMemAlloc(void* memory, std::uint64_t size)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::HRMemAlloc(state, memory, size);
	}

	inline void MemFree(void* memory)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::MemFree(state, memory);
	}

	inline void HRMemFree(void* memory)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::HRMemFree(state, memory);
	}

	struct RAIIMemory
	{
	public:
		RAIIMemory(void* memory, std::uint64_t size)
			: m_Memory(memory)
		{
			MemAlloc(m_Memory, size);
		}

		~RAIIMemory() { MemFree(m_Memory); }

	private:
		void* m_Memory;
	};

	struct RAIIHRMemory
	{
	public:
		RAIIHRMemory(void* memory, std::uint64_t size)
			: m_Memory(memory)
		{
			HRMemAlloc(m_Memory, size);
		}

		~RAIIHRMemory() { HRMemFree(m_Memory); }

	private:
		void* m_Memory;
	};

	inline RAIIMemory Memory(void* memory, std::uint64_t size)
	{
		return RAIIMemory { memory, size };
	}

	inline RAIIHRMemory HRMemory(void* memory, std::uint64_t size)
	{
		return RAIIHRMemory { memory, size };
	}
} // namespace Profiler