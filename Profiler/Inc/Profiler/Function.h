#pragma once

#include "State.h"
#include "Timestamp.h"
#include "Utils/Core.h"

#include <bit>
#include <concepts>

namespace Profiler
{
	namespace Detail
	{
		BUILD_NEVER_INLINE void FunctionBegin(void* functionPtr);
		BUILD_NEVER_INLINE void FunctionEnd();
		BUILD_NEVER_INLINE void HRFunctionBegin(void* functionPtr);
		BUILD_NEVER_INLINE void HRFunctionEnd();
		BUILD_NEVER_INLINE void BoolArg(std::uint8_t offset, bool value);

		template <std::integral T>
		BUILD_NEVER_INLINE void IntArg(std::uint8_t offset, T value, std::uint8_t base = 10)
		{
			auto& event  = NewEvent<IntArgumentEvent>();
			event.Offset = offset;
			event.Size   = sizeof(value);
			event.Base   = base;
			std::memcpy(event.Data, &value, sizeof(value));
		}

		template <std::floating_point T>
		BUILD_NEVER_INLINE void FloatArg(std::uint8_t offset, T value)
		{
			auto& event  = NewEvent<FloatArgumentEvent>();
			event.Offset = offset;
			event.Size   = sizeof(value);
			std::memcpy(event.Data, &value, sizeof(value));
		}

		template <class T>
		BUILD_NEVER_INLINE void FlagsArg(std::uint8_t offset, Utils::Flags<T> flags)
		{
			auto& event     = NewEvent<FlagsArgumentEvent>();
			event.Offset    = offset;
			event.FlagsType = std::bit_cast<std::uint64_t>(typeid(flags));
			std::memcpy(event.Data, &flags, sizeof(flags));
		}

		BUILD_NEVER_INLINE void PtrArg(std::uint8_t offset, void* ptr);
	} // namespace Detail

	inline void FunctionBegin(void* functionPtr)
	{
		if (g_TState.Capture)
			Detail::FunctionBegin(functionPtr);
	}

	inline void FunctionEnd()
	{
		if (g_TState.Capture)
			Detail::FunctionEnd();
	}

	inline void HRFunctionBegin(void* functionPtr)
	{
		if (g_TState.Capture)
			Detail::HRFunctionBegin(functionPtr);
	}

	inline void HRFunctionEnd()
	{
		if (g_TState.Capture)
			Detail::HRFunctionEnd();
	}

	inline void BoolArg(std::uint8_t offset, bool value)
	{
		if (g_TState.Capture)
			Detail::BoolArg(offset, value);
	}

	template <std::integral T>
	inline void IntArg(std::uint8_t offset, T value, std::uint8_t base = 10)
	{
		if (g_TState.Capture)
			Detail::IntArg<T>(offset, value, base);
	}

	template <std::floating_point T>
	inline void FloatArg(std::uint8_t offset, T value)
	{
		if (g_TState.Capture)
			Detail::FloatArg<T>(offset, value);
	}

	template <class T>
	inline void FlagsArg(std::uint8_t offset, Utils::Flags<T> flags)
	{
		if (g_TState.Capture)
			Detail::FlagsArg<T>(offset, flags);
	}

	inline void PtrArg(std::uint8_t offset, void* ptr)
	{
		if (g_TState.Capture)
			Detail::PtrArg(offset, ptr);
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