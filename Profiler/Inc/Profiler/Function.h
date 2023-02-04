#pragma once

#include "State.h"
#include "Timestamp.h"
#include "Utils/Core.h"

#include <cstring>

#include <bit>
#include <concepts>

namespace Profiler
{
	namespace Detail
	{
		BUILD_NEVER_INLINE void FunctionBegin(ThreadState* state, void* functionPtr);
		BUILD_NEVER_INLINE void FunctionEnd(ThreadState* state);
		BUILD_NEVER_INLINE void HRFunctionBegin(ThreadState* state, void* functionPtr);
		BUILD_NEVER_INLINE void HRFunctionEnd(ThreadState* state);
		BUILD_NEVER_INLINE void BoolArg(ThreadState* state, std::uint8_t offset, bool value);
		BUILD_NEVER_INLINE void IntArg(ThreadState* state, std::uint8_t offset, std::uint8_t size, std::uint64_t (&values)[3], std::uint8_t base = 10);
		BUILD_NEVER_INLINE void FloatArg(ThreadState* state, std::uint8_t offset, std::uint8_t size, std::uint64_t (&values)[3]);
		BUILD_NEVER_INLINE void FlagsArg(ThreadState* state, std::uint8_t offset, std::uint64_t flagsType, std::uint64_t (&values)[2]);
		BUILD_NEVER_INLINE void PtrArg(ThreadState* state, std::uint8_t offset, void* ptr);
	} // namespace Detail

	inline void FunctionBegin(void* functionPtr)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::FunctionBegin(state, functionPtr);
	}

	inline void FunctionEnd()
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::FunctionEnd(state);
	}

	inline void HRFunctionBegin(void* functionPtr)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::HRFunctionBegin(state, functionPtr);
	}

	inline void HRFunctionEnd()
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::HRFunctionEnd(state);
	}

	inline void BoolArg(std::uint8_t offset, bool value)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::BoolArg(state, offset, value);
	}

	template <std::integral T>
	inline void IntArg(std::uint8_t offset, T value, std::uint8_t base = 10)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
		{
			std::uint64_t arr[3];
			std::memcpy(arr, &value, sizeof(value));
			Detail::IntArg(state, offset, sizeof(value), arr, base);
		}
	}

	template <std::floating_point T>
	inline void FloatArg(std::uint8_t offset, T value)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
		{
			std::uint64_t arr[3];
			std::memcpy(arr, &value, sizeof(value));
			Detail::FloatArg(state, offset, sizeof(value), arr);
		}
	}

	/*template <class T>
	inline void FlagsArg(std::uint8_t offset, Utils::Flags<T> flags)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
		{
			std::uint64_t arr[2];
			std::memcpy(arr, &flags, sizeof(flags));
			Detail::FlagsArg(state, offset, std::bit_cast<std::uint64_t>(typeid(flags)), arr);
		}
	}*/

	inline void PtrArg(std::uint8_t offset, void* ptr)
	{
		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::PtrArg(state, offset, ptr);
	}

	struct RAIIFunction
	{
	public:
		RAIIFunction(void* functionPtr) { FunctionBegin(functionPtr); }

		~RAIIFunction() { FunctionEnd(); }
	};

	struct RAIIHRFunction
	{
	public:
		RAIIHRFunction(void* functionPtr) { HRFunctionBegin(functionPtr); }

		~RAIIHRFunction() { HRFunctionEnd(); }
	};

	inline RAIIFunction Function(void* functionPtr)
	{
		return RAIIFunction { functionPtr };
	}

	inline RAIIHRFunction HRFunction(void* functionPtr)
	{
		return RAIIHRFunction { functionPtr };
	}
} // namespace Profiler