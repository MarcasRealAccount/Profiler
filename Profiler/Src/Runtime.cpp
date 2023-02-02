#include "Profiler/Runtime.h"
#include "Profiler/Utils/Core.h"

#include <thread>
#include <unordered_map>

#if BUILD_IS_SYSTEM_WINDOWS
	#include <Windows.h>

	#include <psapi.h>
#endif

namespace Profiler
{
	static std::size_t s_CoreCount = std::thread::hardware_concurrency();

#if BUILD_IS_SYSTEM_WINDOWS
	struct ProcessData
	{
		std::uint64_t LastGetTime;
		std::uint64_t LastSysTime;
		std::uint64_t LastUserTime;

		double LastUsage;
	};

	static std::unordered_map<Process, ProcessData> s_ProcessDatas;
#endif

	ERuntimeAbilities GetRuntimeAbilities()
	{
#if BUILD_IS_SYSTEM_WINDOWS
		return RuntimeAbilities::ProcessCoreUsage |
			   RuntimeAbilities::ProcessMemoryUsage |
			   RuntimeAbilities::ProcessIndividualMemoryUsages;
#else
		return RuntimeAbilities::None;
#endif
	}

	Process GetCurrentProcess()
	{
#if BUILD_IS_SYSTEM_WINDOWS
		return ::GetCurrentProcessId();
#else
		return 0;
#endif
	}

	std::size_t GetTotalCoreCount()
	{
		return s_CoreCount;
	}

	std::size_t GetDriveCount()
	{
		return 0;
	}

	void GetDriveInfos(std::size_t driveCount, DriveInfo* infos)
	{
		for (std::size_t i = 0; i < driveCount; ++i)
		{
			DriveInfo& info = infos[i];
			info.Name       = "";
			info.Size       = 0;
		}
	}

	void GetCoreUsages(Process process, std::size_t coreCount, CoreCounter* coreCounters)
	{
#if BUILD_IS_SYSTEM_WINDOWS
		if (coreCount == 0 || !coreCounters)
			return;

		auto itr = s_ProcessDatas.find(process);
		if (itr == s_ProcessDatas.end())
		{
			FILETIME curTime, kernelTime, userTime;

			GetSystemTimeAsFileTime(&curTime);
			ProcessData data {};
			data.LastGetTime = std::bit_cast<std::uint64_t>(curTime);
			if (process == 0)
			{
				FILETIME idleTime;
				GetSystemTimes(&idleTime, &kernelTime, &userTime);
				kernelTime = std::bit_cast<FILETIME>(std::bit_cast<std::uint64_t>(kernelTime) - std::bit_cast<std::uint64_t>(idleTime));
			}
			else
			{
				FILETIME startTime, exitTime;
				HANDLE   processHandle = OpenProcess(0, false, process);
				GetProcessTimes(processHandle, &startTime, &exitTime, &kernelTime, &userTime);
				CloseHandle(processHandle);
			}
			data.LastSysTime  = std::bit_cast<std::uint64_t>(kernelTime);
			data.LastUserTime = std::bit_cast<std::uint64_t>(userTime);
			data.LastUsage    = 0.0;
			s_ProcessDatas.insert({ process, data });
		}
		else
		{
			FILETIME curTime;
			GetSystemTimeAsFileTime(&curTime);

			ProcessData& data       = itr->second;
			auto         curGetTime = std::bit_cast<std::uint64_t>(curTime);
			using namespace std::chrono_literals;
			if ((curGetTime - data.LastGetTime) > 2'500'000ULL)
			{
				FILETIME kernelTime, userTime;
				if (process == 0)
				{
					FILETIME idleTime;
					GetSystemTimes(&idleTime, &kernelTime, &userTime);
					kernelTime = std::bit_cast<FILETIME>(std::bit_cast<std::uint64_t>(kernelTime) - std::bit_cast<std::uint64_t>(idleTime));
				}
				else
				{
					FILETIME startTime, exitTime;
					HANDLE   processHandle = OpenProcess(PROCESS_QUERY_INFORMATION |
                                                           PROCESS_QUERY_LIMITED_INFORMATION |
                                                           PROCESS_VM_READ,
                                                       false,
                                                       process);
					GetProcessTimes(processHandle, &startTime, &exitTime, &kernelTime, &userTime);
					CloseHandle(processHandle);
				}

				auto curSysTime  = std::bit_cast<std::uint64_t>(kernelTime);
				auto curUserTime = std::bit_cast<std::uint64_t>(userTime);

				double usage = ((curSysTime - data.LastSysTime) + (curUserTime - data.LastUserTime));
				usage        /= (curGetTime - data.LastGetTime);
				usage        /= s_CoreCount;

				data.LastGetTime  = curGetTime;
				data.LastSysTime  = curSysTime;
				data.LastUserTime = curUserTime;
				data.LastUsage    = usage;
			}
			coreCounters[0].Usage = data.LastUsage;
		}
#else
		for (std::size_t i = 0; i < coreCount; ++i)
		{
			CoreCounter& cnt = coreCounters[i];
			cnt.Usage        = 0.0;
		}
#endif
	}

	void GetMemoryUsage(Process process, MemoryCounters& counters)
	{
#if BUILD_IS_SYSTEM_WINDOWS
		if (process == 0)
		{
			MEMORYSTATUSEX memoryCounters {};
			memoryCounters.dwLength = sizeof(memoryCounters);
			if (GlobalMemoryStatusEx(&memoryCounters))
			{
				counters.PageFaultCount    = 0;
				counters.PeakWorkingSet    = memoryCounters.ullTotalPhys;
				counters.CurrentWorkingSet = memoryCounters.ullTotalPhys - memoryCounters.ullAvailPhys;
				counters.PeakPrivate       = memoryCounters.ullTotalPageFile;
				counters.Private           = memoryCounters.ullTotalPageFile - memoryCounters.ullAvailPageFile;
				counters.PeakNonPagedPool = counters.CurrentNonPagedPool = 0;
				counters.PeakPagedPool = counters.CurrentPagedPool = 0;
			}
			else
			{
				// TODO(MarcasRealAccount): Add error logging
				counters.PageFaultCount      = 0;
				counters.PeakWorkingSet      = 0;
				counters.CurrentWorkingSet   = 0;
				counters.PeakPagedPool       = 0;
				counters.CurrentPagedPool    = 0;
				counters.PeakNonPagedPool    = 0;
				counters.CurrentNonPagedPool = 0;
				counters.PeakPrivate         = 0;
				counters.Private             = 0;
			}
		}
		else
		{
			HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION |
												   PROCESS_QUERY_LIMITED_INFORMATION |
												   PROCESS_VM_READ,
											   false,
											   process);

			PROCESS_MEMORY_COUNTERS_EX memCounters {};
			memCounters.cb = sizeof(memCounters);
			if (GetProcessMemoryInfo(processHandle,
									 reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&memCounters),
									 sizeof(memCounters)))
			{
				counters.PageFaultCount      = memCounters.PageFaultCount;
				counters.PeakWorkingSet      = memCounters.PeakWorkingSetSize;
				counters.CurrentWorkingSet   = memCounters.WorkingSetSize;
				counters.PeakPagedPool       = memCounters.QuotaPeakPagedPoolUsage;
				counters.CurrentPagedPool    = memCounters.QuotaPagedPoolUsage;
				counters.PeakNonPagedPool    = memCounters.QuotaPeakNonPagedPoolUsage;
				counters.CurrentNonPagedPool = memCounters.QuotaNonPagedPoolUsage;
				counters.PeakPrivate         = memCounters.PeakPagefileUsage;
				counters.Private             = memCounters.PagefileUsage;
			}
			else
			{
				// TODO(MarcasRealAccount): Add error logging
				counters.PageFaultCount      = 0;
				counters.PeakWorkingSet      = 0;
				counters.CurrentWorkingSet   = 0;
				counters.PeakPagedPool       = 0;
				counters.CurrentPagedPool    = 0;
				counters.PeakNonPagedPool    = 0;
				counters.CurrentNonPagedPool = 0;
				counters.PeakPrivate         = 0;
				counters.Private             = 0;
			}

			CloseHandle(processHandle);
		}
#else
		counters.PageFaultCount      = 0;
		counters.PeakWorkingSet      = 0;
		counters.CurrentWorkingSet   = 0;
		counters.PeakPagedPool       = 0;
		counters.CurrentPagedPool    = 0;
		counters.PeakNonPagedPool    = 0;
		counters.CurrentNonPagedPool = 0;
		counters.PeakPrivate         = 0;
		counters.Private             = 0;
#endif
	}

	void GetDriveUsages(Process process, std::size_t driveCount, DriveCounter* driveCounters)
	{
		for (std::size_t i = 0; i < driveCount; ++i)
		{
			DriveCounter& cnt = driveCounters[i];
			cnt.BytesRead     = 0;
			cnt.BytesWritten  = 0;
			cnt.Usage         = 0.0;
		}
	}
} // namespace Profiler