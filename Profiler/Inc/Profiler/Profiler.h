#pragma once

#include "Utils/Flags.h"

#include <cstddef>
#include <cstdint>

#include <concepts>
#include <string>
#include <typeinfo>

namespace Profiler
{
	namespace Detail
	{
		template <class T>
		concept Pod = std::is_pod_v<T>;
	}

	using EventID = std::uint64_t;

	using EProfilerAbilities = Utils::Flags<std::uint32_t>;

	namespace ProfilerAbilities
	{
		static constexpr EProfilerAbilities Functions = 1;
		static constexpr EProfilerAbilities Arguments = 2;
	} // namespace ProfilerAbilities

	struct FunctionRAII
	{
	public:
		FunctionRAII();
		FunctionRAII(void* functionPtr);
		FunctionRAII(const FunctionRAII& copy) = delete;
		FunctionRAII(FunctionRAII&& move) noexcept;
		~FunctionRAII();

	private:
		EventID m_Event;
	};

	void Init();
	void Deinit();

	void BeginCapturing(std::size_t minEventCount = 1024ULL * 1024ULL);
	void EndCapturing();
	void WriteCaptures();

	void Frame();

	namespace Detail
	{
		void IntArg(std::uint16_t flags, std::uint16_t offset, std::uint64_t (&values)[3]);
		void FloatArg(std::uint16_t flags, std::uint16_t offset, std::uint64_t (&values)[3]);
		void FlagsArg(std::uint16_t flags, std::uint16_t offset, const std::type_info* type, std::uint64_t (&values)[2]);
	} // namespace Detail

	FunctionRAII Function(void* functionPtr);
	void         BoolArg(std::uint16_t offset, bool value);
	void         PtrArg(std::uint16_t offset, void* value);

	void IntArg(std::uint16_t offset, std::integral auto value, std::uint8_t base = 10)
	{
		std::uint64_t values[3] { 0, 0, 0 };
		std::memcpy(values, &value, sizeof(value));
		Detail::IntArg((std::bit_width(sizeof(value)) - 1) << 8 | base, offset, values);
	}

	void FloatArg(std::uint16_t offset, std::floating_point auto value)
	{
		std::uint64_t values[3] { 0, 0, 0 };
		std::memcpy(values, &value, sizeof(value));
		Detail::FloatArg((std::bit_width(sizeof(value)) - 1) << 8, offset, values);
	}

	template <class T>
	void FlagsArg(std::uint16_t offset, Utils::Flags<T> flags)
	{
		std::uint64_t values[2] { 0, 0 };
		std::memcpy(values, &flags, sizeof(flags));
		Detail::FlagsArg((std::bit_width(sizeof(flags)) - 1) << 8, offset, typeid(flags), values);
	}

	EProfilerAbilities GetAbilities();
} // namespace Profiler