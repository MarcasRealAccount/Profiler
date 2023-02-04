#pragma once

#include "State.h"
#include "Timestamp.h"
#include "Utils/Core.h"

#include <cstring>

namespace Profiler
{
	namespace Detail
	{
		BUILD_NEVER_INLINE std::uint64_t ForLoopBegin(ThreadState* state);
		BUILD_NEVER_INLINE void          ForLoopEnd(ThreadState* state, std::uint64_t id);
		BUILD_NEVER_INLINE std::uint64_t HRForLoopBegin(ThreadState* state);
		BUILD_NEVER_INLINE void          HRForLoopEnd(ThreadState* state, std::uint64_t id);
		BUILD_NEVER_INLINE void          ForLoopIterBegin(ThreadState* state, std::uint64_t id, std::uint8_t size, std::uint64_t (&values)[2]);
		BUILD_NEVER_INLINE void          ForLoopIterEnd(ThreadState* state, std::uint64_t id);
		BUILD_NEVER_INLINE void          HRForLoopIterBegin(ThreadState* state, std::uint64_t id, std::uint8_t size, std::uint64_t (&values)[2]);
		BUILD_NEVER_INLINE void          HRForLoopIterEnd(ThreadState* state, std::uint64_t id);
	} // namespace Detail

	inline std::uint64_t ForLoopBegin()
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			return Detail::ForLoopBegin(state);
		return 0;
	}

	inline void ForLoopEnd(std::uint64_t id)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::ForLoopEnd(state, id);
	}

	inline std::uint64_t HRForLoopBegin()
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			return Detail::HRForLoopBegin(state);
		return 0;
	}

	inline void HRForLoopEnd(std::uint64_t id)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::HRForLoopEnd(state, id);
	}

	template <std::integral T>
	inline void ForLoopIterBegin(std::uint64_t id, T index)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
		{
			std::uint64_t values[2];
			std::memcpy(values, &index, sizeof(index));
			Detail::ForLoopIterBegin(state, id, sizeof(index), values);
		}
	}

	inline void ForLoopIterEnd(std::uint64_t id)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::ForLoopIterEnd(state, id);
	}

	template <std::integral T>
	inline void HRForLoopIterBegin(std::uint64_t id, T index)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
		{
			std::uint64_t values[2];
			std::memcpy(values, &index, sizeof(index));
			Detail::HRForLoopIterBegin(state, id, sizeof(index), values);
		}
	}

	inline void HRForLoopIterEnd(std::uint64_t id)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::HRForLoopIterEnd(state, id);
	}

	struct RAIIForLoop
	{
	public:
		RAIIForLoop() { m_ID = ForLoopBegin(); }

		~RAIIForLoop() { ForLoopEnd(m_ID); }

		operator std::uint64_t() const { return m_ID; }

	private:
		std::uint64_t m_ID;
	};

	struct RAIIHRForLoop
	{
	public:
		RAIIHRForLoop() { m_ID = HRForLoopBegin(); }

		~RAIIHRForLoop() { HRForLoopEnd(m_ID); }

		operator std::uint64_t() const { return m_ID; }

	private:
		std::uint64_t m_ID;
	};

	struct RAIIForLoopIter
	{
	public:
		template <std::integral T>
		RAIIForLoopIter(std::uint64_t id, T value)
			: m_ID(id)
		{
			ForLoopIterBegin(id, value);
		}

		~RAIIForLoopIter() { ForLoopIterEnd(m_ID); }

	private:
		std::uint64_t m_ID;
	};

	struct RAIIHRForLoopIter
	{
	public:
		template <std::integral T>
		RAIIHRForLoopIter(std::uint64_t id, T value)
			: m_ID(id)
		{
			HRForLoopIterBegin(id, value);
		}

		~RAIIHRForLoopIter() { HRForLoopIterEnd(m_ID); }

	private:
		std::uint64_t m_ID;
	};

	inline RAIIForLoop ForLoop()
	{
		return RAIIForLoop {};
	}

	inline RAIIHRForLoop HRForLoop()
	{
		return RAIIHRForLoop {};
	}

	template <std::integral T>
	inline RAIIForLoopIter ForLoopIter(std::uint64_t id, T value)
	{
		return RAIIForLoopIter { id, value };
	}

	template <std::integral T>
	inline RAIIHRForLoopIter HRForLoopIter(std::uint64_t id, T value)
	{
		return RAIIHRForLoopIter { id, value };
	}
} // namespace Profiler