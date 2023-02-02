#pragma once

#include "Utils/Flags.h"

#include <cstddef>
#include <cstdint>

#include <string>

namespace Profiler
{
	using ERuntimeAbilities = Utils::Flags<std::uint32_t>;

	namespace RuntimeAbilities
	{
		static constexpr ERuntimeAbilities None = 0x0;

		static constexpr ERuntimeAbilities ProcessCoreUsage   = 0x01;
		static constexpr ERuntimeAbilities ProcessMemoryUsage = 0x02;
		static constexpr ERuntimeAbilities ProcessDriveUsage  = 0x04;

		static constexpr ERuntimeAbilities ProcessIndividualCoreUsages   = 0x10;
		static constexpr ERuntimeAbilities ProcessIndividualMemoryUsages = 0x20;
		static constexpr ERuntimeAbilities ProcessIndividualDriveUsages  = 0x40;
	} // namespace RuntimeAbilities

	using Process = std::uint64_t;

	struct DriveInfo
	{
		std::string Name;
		std::size_t Size;
	};

	ERuntimeAbilities GetRuntimeAbilities();

	Process GetCurrentProcess();

	std::size_t GetTotalCoreCount();
	std::size_t GetDriveCount();
	void        GetDriveInfos(std::size_t driveCount, DriveInfo* infos);

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

	struct DriveCounter
	{
		std::size_t BytesRead;
		std::size_t BytesWritten;
		double      Usage;
	};

	void GetCoreUsages(Process process, std::size_t coreCount, CoreCounter* coreCounters);
	void GetMemoryUsage(Process process, MemoryCounters& counters);
	void GetDriveUsages(Process process, std::size_t driveCount, DriveCounter* driveCounters);
} // namespace Profiler