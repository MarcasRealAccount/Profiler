#pragma once

#include "Utils/Flags.h"

#include <cstddef>
#include <cstdint>

#include <string>
#include <vector>

namespace Profiler
{
	using ERuntimeAbilities = Utils::Flags<>;

	namespace RuntimeAbilities
	{
		static constexpr ERuntimeAbilities None = 0x0;

		static constexpr ERuntimeAbilities CPUUsage    = 0x01;
		static constexpr ERuntimeAbilities MemoryUsage = 0x02;
		static constexpr ERuntimeAbilities IOUsage     = 0x04;

		static constexpr ERuntimeAbilities IndividualCPUUsages    = 0x10;
		static constexpr ERuntimeAbilities IndividualMemoryUsages = 0x20;
		static constexpr ERuntimeAbilities IndividualIOUsages     = 0x40;

		static constexpr ERuntimeAbilities MemoryPageFaults = 0x100;
	} // namespace RuntimeAbilities

	using Process = std::uint64_t;

	struct CPUData
	{
		std::uint64_t LastUserTime = 0;
		std::uint64_t LastSysTime  = 0;
		std::uint64_t CurUserTime  = 0;
		std::uint64_t CurSysTime   = 0;
		std::uint64_t IdleTime     = 0;
		double        Usage        = 0.0;
	};

	struct MemoryData
	{
		std::uint64_t PageFaultCount = 0;
		std::uint64_t PhysicalTotal  = 0;
		std::uint64_t PhysicalUsage  = 0;
		std::uint64_t VirtualTotal   = 0;
		std::uint64_t VirtualUsage   = 0;
		std::uint64_t PagedUsage     = 0;
		std::uint64_t NonPagedUsage  = 0;
	};

	enum class EIOEndpointType
	{
		Total,
		PhysicalDrive,
		NetworkAdapter
	};

	struct IOEndpointData
	{
		EIOEndpointType Type = EIOEndpointType::Total;
		std::uint64_t   ID   = 0;
		std::string     Name;

		std::uint64_t LastReadCount  = 0;
		std::uint64_t LastWriteCount = 0;
		std::uint64_t LastOtherCount = 0;
		std::uint64_t CurReadCount   = 0;
		std::uint64_t CurWriteCount  = 0;
		std::uint64_t CurOtherCount  = 0;

		std::uint64_t LastReadTime  = 0;
		std::uint64_t LastWriteTime = 0;
		std::uint64_t LastOtherTime = 0;
		std::uint64_t CurReadTime   = 0;
		std::uint64_t CurWriteTime  = 0;
		std::uint64_t CurOtherTime  = 0;
		std::uint64_t IdleTime      = 0;
		double        Usage         = 0.0;
	};

	struct RuntimeData
	{
		std::uint64_t LastTime = 0;
		std::uint64_t CurTime  = 0;

		ERuntimeAbilities           Abilities;
		std::vector<CPUData>        CPUs;
		MemoryData                  Memory;
		std::vector<IOEndpointData> IOEndpoints;
	};

	Process GetSystemProcess();
	Process GetCurrentProcess();
	bool    PollData(Process process, RuntimeData* runtimeData);
} // namespace Profiler