#include "Profiler/Utils/Core.h"

#if BUILD_IS_SYSTEM_LINUX
	#include "Profiler/Runtime.h"

	#include <chrono>
	#include <fstream>
	#include <filesystem>
	#include <thread>
	#include <mutex>

	#include <cstdio>

	#include <unistd.h>

namespace Profiler
{
	static struct LinuxRuntimeData
	{
		std::mutex Mutex;

		std::uint64_t LastTime = 0;
		std::uint64_t CurTime  = 0;

		std::vector<IOEndpointData> IOEndpoints;
	} s_LinuxRuntimeData;

	static std::string ReadAllContent(const std::filesystem::path& filePath, bool* opened)
	{
		std::ifstream file(filePath);
		if (!file)
		{
			if (opened)
				*opened = false;
			return "";
		}
		if (opened)
			*opened = true;

		std::string content;
		char        buf[4096];
		while (true)
		{
			file.read(buf, 4096);
			std::size_t readCount = file.gcount();
			if (readCount == 0)
				break;

			content.append(buf, buf + readCount);
		}

		file.close();
		return content;
	}

	static std::uint64_t GetCurrentTime()
	{
		return std::chrono::duration_cast<std::chrono::duration<std::uint64_t, std::nano>>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	static std::uint64_t ConvertUserHZ(std::uint64_t uhz)
	{
		double clockRate = 1e9 / static_cast<double>(sysconf(_SC_CLK_TCK));
		return static_cast<std::uint64_t>(uhz * clockRate);
	}

	static void UpdateDrives()
	{
		bool opened  = false;
		auto content = ReadAllContent("/proc/diskstats", &opened);
		if (!opened)
			return;

		auto& ioEndpoints = s_LinuxRuntimeData.IOEndpoints;

		std::string_view contentView(content);
		std::size_t      offset = 0;
		while (offset < contentView.size())
		{
			std::size_t lineEnd = contentView.find_first_of('\n', offset + 1);
			if (lineEnd == std::string::npos)
				break;
			std::string_view line = contentView.substr(offset, lineEnd - offset);
			offset                = lineEnd + 1;
			if (line.empty())
				continue;

			IOEndpointData endpoint {};
			endpoint.Type = EIOEndpointType::PhysicalDrive;

			std::size_t typeBegin = line.find_first_not_of(' ');
			std::size_t typeEnd   = line.find_first_of(' ', typeBegin + 1);
			std::size_t idBegin   = line.find_first_not_of(' ', typeEnd + 1);
			std::size_t idEnd     = line.find_first_of(' ', idBegin + 1);
			endpoint.ID           = std::strtoull(line.data() + typeBegin, nullptr, 10) << 32 | std::strtoull(line.data() + idBegin, nullptr, 10);
			std::size_t nameBegin = line.find_first_not_of(' ', idEnd + 1);
			std::size_t nameEnd   = line.find_first_of(' ', nameBegin + 1);
			endpoint.Name         = line.substr(nameBegin, nameEnd - nameBegin);

			std::size_t fieldIndex = 0;
			std::size_t subOffset  = line.find_first_not_of(' ', nameEnd + 1);
			while (subOffset < line.size())
			{
				std::size_t end = line.find_first_of(' ', subOffset);
				if (end == std::string::npos)
					break;
				std::size_t start = subOffset;
				subOffset         = end + 1;
				++fieldIndex;

				if (!(fieldIndex == 3 || fieldIndex == 4 || fieldIndex == 7 || fieldIndex == 8))
					continue;

				// TODO(MarcasRealAccount): Stop assuming sector size is always 512 bytes, (tho it most likely is 99% of the time due to backwards support)
				std::uint64_t val = std::strtoull(line.data() + start, nullptr, 10);
				switch (fieldIndex)
				{
				case 3: endpoint.CurReadCount = val * 512; break;
				case 4: endpoint.CurReadTime = val * 1'000'000; break;
				case 7: endpoint.CurWriteCount = val * 512; break;
				case 8: endpoint.CurWriteTime = val * 1'000'000; break;
				}
			}

			if (endpoint.CurReadCount != 0 || endpoint.CurWriteCount != 0)
				ioEndpoints.emplace_back(std::move(endpoint));
		}
	}

	static void UpdateNetworkAdapters()
	{
		// TODO(MarcasRealAccount): Enumerate network adapters and update them
	}

	static void UpdateIOEndpoints(std::uint64_t timeSpent)
	{
		auto& ioEndpoints = s_LinuxRuntimeData.IOEndpoints;

		std::vector<IOEndpointData> previousData;
		ioEndpoints.swap(previousData);
		UpdateDrives();
		UpdateNetworkAdapters();

		std::sort(ioEndpoints.begin(),
				  ioEndpoints.end(),
				  [](const IOEndpointData& lhs, const IOEndpointData& rhs) -> bool {
					  std::uint64_t tl = static_cast<std::uint64_t>(lhs.Type);
					  std::uint64_t tr = static_cast<std::uint64_t>(rhs.Type);
					  if (tl < tr)
						  return true;
					  else if (tl > tr)
						  return false;
					  return lhs.ID < rhs.ID;
				  });

		for (std::size_t i = 0; i < ioEndpoints.size(); ++i)
		{
			auto& endpoint = ioEndpoints[i];
			for (std::size_t j = 0; j < previousData.size(); ++j)
			{
				auto& pEndpoint = previousData[j];
				if (endpoint.Type == pEndpoint.Type &&
					endpoint.ID == pEndpoint.ID)
				{
					endpoint.LastReadCount  = pEndpoint.CurReadCount;
					endpoint.LastWriteCount = pEndpoint.CurWriteCount;
					endpoint.LastOtherCount = pEndpoint.CurOtherCount;
					endpoint.LastReadTime   = pEndpoint.CurReadTime;
					endpoint.LastWriteTime  = pEndpoint.CurWriteTime;
					endpoint.LastOtherTime  = pEndpoint.CurOtherTime;
					break;
				}
			}

			std::uint64_t totalTime = (endpoint.CurReadTime - endpoint.LastReadTime) + (endpoint.CurWriteTime - endpoint.LastWriteTime) + (endpoint.CurOtherTime - endpoint.LastOtherTime);
			endpoint.IdleTime       = timeSpent - totalTime;
			endpoint.Usage          = static_cast<double>(totalTime) / static_cast<double>(timeSpent);
		}
	}

	static void PollSystemIOData(RuntimeData* runtimeData, std::uint64_t curGetTime)
	{
		if (!runtimeData)
			return;

		s_LinuxRuntimeData.Mutex.lock();
		std::uint64_t timeSpent = curGetTime - s_LinuxRuntimeData.CurTime;

		if (timeSpent > 250'000'000ULL)
		{
			s_LinuxRuntimeData.LastTime = s_LinuxRuntimeData.CurTime;
			s_LinuxRuntimeData.CurTime  = curGetTime;
			UpdateIOEndpoints(timeSpent);
		}

		runtimeData->IOEndpoints.resize(s_LinuxRuntimeData.IOEndpoints.size());
		for (std::size_t i = 0; i < runtimeData->IOEndpoints.size(); ++i)
		{
			auto& data      = runtimeData->IOEndpoints[i];
			auto& linuxData = s_LinuxRuntimeData.IOEndpoints[i];
			data            = *static_cast<IOEndpointData*>(&linuxData);
		}
		s_LinuxRuntimeData.Mutex.unlock();
	}

	static bool ParseProcStat(RuntimeData* runtimeData)
	{
		bool opened  = false;
		auto content = ReadAllContent("/proc/stat", &opened);
		if (!opened)
			return false;

		for (std::size_t i = 0; i < runtimeData->CPUs.size(); ++i)
		{
			auto& data        = runtimeData->CPUs[i];
			data.LastSysTime  = data.CurSysTime;
			data.LastUserTime = data.CurUserTime;
		}

		std::size_t totalCPUs = 0;

		std::string_view contentView(content);
		std::size_t      offset = 0;
		while (offset < contentView.size())
		{
			std::size_t lineEnd = contentView.find_first_of('\n', offset + 1);
			if (lineEnd == std::string::npos)
				break;
			std::string_view line = contentView.substr(offset, lineEnd - offset);
			offset                = lineEnd + 1;
			if (line.empty())
				continue;

			std::size_t      keyEnd = line.find_first_of(' ');
			std::string_view key    = line.substr(0, keyEnd);
			if (!key.starts_with("cpu"))
				continue;

			std::uint64_t coreIndex = 0;
			if (keyEnd > 3)
				coreIndex = std::strtoull(key.data() + 3, nullptr, 10);

			totalCPUs = std::max(totalCPUs, coreIndex + 1);
			if (totalCPUs > runtimeData->CPUs.size())
				runtimeData->CPUs.resize(totalCPUs);
			auto&         data       = runtimeData->CPUs[coreIndex];
			std::uint64_t fieldIndex = 0;
			std::size_t   subOffset  = line.find_first_not_of(' ', keyEnd + 1);
			while (subOffset < line.size())
			{
				std::size_t end = line.find_first_of(' ', subOffset + 1);
				if (end == std::string::npos)
					break;
				std::size_t start = subOffset;
				subOffset         = end + 1;
				++fieldIndex;

				if (fieldIndex != 1 && fieldIndex != 3)
					continue;

				std::uint64_t val = std::strtoull(line.data() + start, nullptr, 10);
				switch (fieldIndex)
				{
				case 1: data.CurUserTime = ConvertUserHZ(val); break;
				case 3: data.CurSysTime = ConvertUserHZ(val); break;
				}
			}
		}
		runtimeData->CPUs.resize(totalCPUs);
		return true;
	}

	static bool ParseProcMemInfo(RuntimeData* runtimeData)
	{
		bool opened  = false;
		auto content = ReadAllContent("/proc/meminfo", &opened);
		if (!opened)
			return false;

		auto& mem          = runtimeData->Memory;
		mem.PageFaultCount = 0;
		mem.PhysicalTotal  = 0;
		mem.PhysicalUsage  = 0;
		mem.VirtualTotal   = 0;
		mem.VirtualUsage   = 0;
		mem.PagedUsage     = 0;
		mem.NonPagedUsage  = 0;

		std::uint64_t MemTotal = 0, MemFree = 0, VmallocTotal = 0, VmallocUsed = 0;

		std::string_view contentView(content);
		std::size_t      offset = 0;
		while (offset < contentView.size())
		{
			std::size_t lineEnd = contentView.find_first_of('\n', offset + 1);
			if (lineEnd == std::string::npos)
				break;
			std::string_view line = contentView.substr(offset, lineEnd - offset);
			offset                = lineEnd + 1;
			if (line.empty())
				continue;

			std::size_t      keyEnd = line.find_first_of(':');
			std::string_view key    = line.substr(0, keyEnd);

			std::uint8_t index;
			if (key == "MemTotal")
				index = 0;
			else if (key == "MemFree")
				index = 1;
			else if (key == "VmallocTotal")
				index = 2;
			else if (key == "VmallocUsed")
				index = 3;
			else
				continue;

			std::size_t      valueBegin = line.find_first_not_of(' ', keyEnd + 1);
			std::size_t      valueEnd   = line.find_first_of(' ', valueBegin);
			std::string_view value      = line.substr(valueBegin, valueEnd - valueBegin);
			std::uint64_t    val        = std::strtoull(value.data(), nullptr, 10);
			switch (index)
			{
			case 0: MemTotal = val; break;
			case 1: MemFree = val; break;
			case 2: VmallocTotal = val; break;
			case 3: VmallocUsed = val; break;
			}
		}

		mem.PhysicalTotal = MemTotal * 1024;
		mem.PhysicalUsage = (MemTotal - MemFree) * 1024;
		mem.VirtualTotal  = VmallocTotal * 1024;
		mem.VirtualUsage  = VmallocUsed * 1024;
		return true;
	}

	Process GetSystemProcess()
	{
		return 0;
	}

	Process GetCurrentProcess()
	{
		return static_cast<Process>(getpid());
	}

	bool PollData(Process process, RuntimeData* runtimeData)
	{
		if (!runtimeData)
			return false;

		std::uint64_t curGetTime = GetCurrentTime();
		std::uint64_t timeSpent  = curGetTime - runtimeData->CurTime;

		if (timeSpent < 250'000'000ULL)
			return false;

		runtimeData->LastTime = runtimeData->CurTime;
		runtimeData->CurTime  = curGetTime;

		runtimeData->Abilities = RuntimeAbilities::None;
		if (process == 0)
		{
		}
		else
		{
		}

		// Gather CPU data
		{
			bool success = false;
			if (process == 0)
				success = ParseProcStat(runtimeData);
			else
				success = false; // TODO(MarcasRealAccount): Implement per process cpu data!

			if (success)
			{
				double timePerThread   = static_cast<double>(timeSpent) * std::thread::hardware_concurrency();
				runtimeData->Abilities |= RuntimeAbilities::CPUUsage;
				if (runtimeData->CPUs.size() > 1)
					runtimeData->Abilities |= RuntimeAbilities::IndividualCPUUsages;

				for (std::size_t i = 0; i < runtimeData->CPUs.size(); ++i)
				{
					auto&         cpu       = runtimeData->CPUs[i];
					std::uint64_t totalTime = ((cpu.CurSysTime - cpu.LastSysTime) + (cpu.CurUserTime - cpu.LastUserTime));
					cpu.IdleTime            = timePerThread - totalTime;
					cpu.Usage               = static_cast<double>(totalTime) / timePerThread;
				}
			}
			else
			{
				for (std::size_t i = 0; i < runtimeData->CPUs.size(); ++i)
				{
					auto& cpu    = runtimeData->CPUs[i];
					cpu.IdleTime = 0;
					cpu.Usage    = 0.0;
				}
			}
		}

		// Gather Memory data
		{
			bool success = false;
			if (process == 0)
				success = ParseProcMemInfo(runtimeData);
			else
				success = false; // TODO(MarcasRealAccount): Implement per process memory data!

			if (success)
			{
				runtimeData->Abilities |= RuntimeAbilities::MemoryUsage | RuntimeAbilities::IndividualMemoryUsages;
			}
		}

		// Gather IO Endpoint data
		{
			if (process == 0)
			{
				PollSystemIOData(runtimeData, curGetTime);

				if (!runtimeData->IOEndpoints.empty())
				{
					runtimeData->Abilities |= RuntimeAbilities::IOUsage;
					if (runtimeData->IOEndpoints.size() > 1)
						runtimeData->Abilities |= RuntimeAbilities::IndividualIOUsages;
				}
			}
			else
			{
			}
		}

		return true;
	}
} // namespace Profiler

#endif