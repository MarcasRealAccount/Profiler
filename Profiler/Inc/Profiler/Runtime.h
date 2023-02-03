#pragma once

#include "Utils/Flags.h"

#include <cstddef>
#include <cstdint>

#include <string>

namespace Profiler
{
	using ERuntimeAbilities = Utils::Flags<>;

	namespace RuntimeAbilities
	{
		static constexpr ERuntimeAbilities None = 0x0;

		static constexpr ERuntimeAbilities CoreUsage   = 0x01;
		static constexpr ERuntimeAbilities MemoryUsage = 0x02;
		static constexpr ERuntimeAbilities IOUsage     = 0x04;

		static constexpr ERuntimeAbilities IndividualCoreUsages   = 0x10;
		static constexpr ERuntimeAbilities IndividualMemoryUsages = 0x20;
		static constexpr ERuntimeAbilities IndividualIOUsages     = 0x40;

		static constexpr ERuntimeAbilities PerProcessCoreUsage   = 0x100;
		static constexpr ERuntimeAbilities PerProcessMemoryUsage = 0x200;
		static constexpr ERuntimeAbilities PerProcessIOUsage     = 0x400;

		static constexpr ERuntimeAbilities PerProcessIndividualCoreUsages   = 0x1000;
		static constexpr ERuntimeAbilities PerProcessIndividualMemoryUsages = 0x2000;
		static constexpr ERuntimeAbilities PerProcessIndividualIOUsages     = 0x4000;

		static constexpr ERuntimeAbilities MemoryPageFaults           = 0x1'0000;
		static constexpr ERuntimeAbilities PerProcessMemoryPageFaults = 0x2'0000;
	} // namespace RuntimeAbilities

	using Process = std::uint64_t;

	struct IOEndpointInfo
	{
		std::string Name;
	};

	ERuntimeAbilities GetRuntimeAbilities();

	Process SystemProcess();
	Process GetCurrentProcess();

	std::size_t GetTotalCoreCount();
	std::size_t GetIOEndpointCount(bool* pChanged = nullptr);
	void        GetIOEndpointInfos(std::size_t endpointCount, IOEndpointInfo* infos);

	struct CoreCounter
	{
		double Usage;
	};

	struct MemoryCounters
	{
		std::size_t PageFaultCount;
		std::size_t PeakWorkingSet;
		std::size_t CurrentWorkingSet;
		std::size_t PeakPagedPool;
		std::size_t CurrentPagedPool;
		std::size_t PeakNonPagedPool;
		std::size_t CurrentNonPagedPool;
		std::size_t PeakPrivate;
		std::size_t Private;
	};

	struct IOCounter
	{
		std::size_t BytesRead;
		std::size_t BytesWritten;
		double      Usage;
	};

	void GetCoreUsages(Process process, std::size_t coreCount, CoreCounter* counters);
	void GetMemoryUsage(Process process, MemoryCounters& counters);
	void GetIOUsages(Process process, std::size_t endpointCount, IOCounter* counters);
} // namespace Profiler