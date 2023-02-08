#include "Profiler/Utils/Core.h"

#if BUILD_IS_SYSTEM_WINDOWS
	#include "Profiler/Runtime.h"

	#include <bit>
	#include <thread>
	#include <mutex>
	#include <algorithm>

	#include <Windows.h>

	#include <winioctl.h>
	#include <psapi.h>

namespace Profiler
{
	struct WindowsIOEndpointData : public IOEndpointData
	{
		std::wstring UNCPath;
	};

	static struct WindowsRuntimeData
	{
		WindowsRuntimeData()
			: VolumeNameBuffer(new wchar_t[32767]) {}

		~WindowsRuntimeData()
		{
			delete[] VolumeNameBuffer;
		}

		std::mutex Mutex;
		wchar_t*   VolumeNameBuffer;

		std::uint64_t LastTime = 0;
		std::uint64_t CurTime  = 0;

		std::vector<WindowsIOEndpointData> IOEndpoints;
	} s_WindowsRuntimeData;

	static void ProcessDrive(DISK_EXTENT& drive)
	{
		auto& ioEndpoints = s_WindowsRuntimeData.IOEndpoints;
		for (auto& endpoint : ioEndpoints)
			if (endpoint.Type == EIOEndpointType::PhysicalDrive &&
				endpoint.ID == drive.DiskNumber)
				return;

		WindowsIOEndpointData endpoint {};
		endpoint.Type    = EIOEndpointType::PhysicalDrive;
		endpoint.ID      = drive.DiskNumber;
		endpoint.UNCPath = L"//./PhysicalDrive" + std::to_wstring(drive.DiskNumber);
		endpoint.Name    = "Drive " + std::to_string(drive.DiskNumber);
		ioEndpoints.emplace_back(std::move(endpoint));
	}

	static void ProcessVolume(wchar_t* volumeName)
	{
		std::size_t truncateBegin = 0;
		std::size_t end           = 0;
		for (; end < 32767 && volumeName[end] != L'\0'; ++end)
			if (volumeName[end] == '\\')
				truncateBegin = end;
		for (std::size_t i = truncateBegin; i < end; ++i)
			volumeName[i] = L'\0';

		auto volumeHandle = CreateFileW(volumeName,
										0,
										FILE_SHARE_READ | FILE_SHARE_WRITE,
										nullptr,
										OPEN_EXISTING,
										0,
										0);
		if (volumeHandle == INVALID_HANDLE_VALUE)
			return;

		DWORD                writtenSize = 0;
		VOLUME_DISK_EXTENTS  extents {};
		VOLUME_DISK_EXTENTS* pExtents  = nullptr;
		bool                 allocated = false;

		if (!DeviceIoControl(volumeHandle,
							 IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
							 nullptr,
							 0,
							 &extents,
							 sizeof(extents),
							 &writtenSize,
							 nullptr))
		{
			if (GetLastError() == ERROR_MORE_DATA)
			{
				std::size_t extentsSize = 4 + sizeof(DISK_EXTENT) * extents.NumberOfDiskExtents;
				pExtents                = reinterpret_cast<VOLUME_DISK_EXTENTS*>(new std::uint8_t[extentsSize]);
				allocated               = true;
				if (!DeviceIoControl(volumeHandle,
									 IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
									 nullptr,
									 0,
									 pExtents,
									 extentsSize,
									 &writtenSize,
									 nullptr))
				{
					delete[] pExtents;
					CloseHandle(volumeHandle);
					return;
				}
			}
			else
			{
				CloseHandle(volumeHandle);
				return;
			}
		}
		else
		{
			pExtents = &extents;
		}
		CloseHandle(volumeHandle);

		for (std::size_t i = 0; i < pExtents->NumberOfDiskExtents; ++i)
			ProcessDrive(pExtents->Extents[i]);

		if (allocated)
			delete[] pExtents;
	}

	static void EnumeratePhysicalDrives()
	{
		auto volumeName = s_WindowsRuntimeData.VolumeNameBuffer;
		auto handle     = FindFirstVolumeW(volumeName, 32767);
		if (handle)
		{
			ProcessVolume(volumeName);
			while (FindNextVolumeW(handle, volumeName, 32767))
				ProcessVolume(volumeName);
			FindVolumeClose(handle);
		}
	}

	static void EnumerateNetworkAdapters()
	{
		// TODO(MarcasRealAccount): Enumerate network adapters
	}

	static bool QueryDrivePerformance(WindowsIOEndpointData& endpoint, std::uint64_t timeSpent)
	{
		auto diskHandle = CreateFileW(endpoint.UNCPath.c_str(),
									  0,
									  FILE_SHARE_READ | FILE_SHARE_WRITE,
									  nullptr,
									  OPEN_EXISTING,
									  0,
									  0);

		if (diskHandle == INVALID_HANDLE_VALUE)
			return false;

		DWORD            bytesReturned = 0;
		DISK_PERFORMANCE diskPerf {};
		if (!DeviceIoControl(diskHandle,
							 IOCTL_DISK_PERFORMANCE,
							 nullptr,
							 0,
							 &diskPerf,
							 sizeof(diskPerf),
							 &bytesReturned,
							 nullptr))
		{
			CloseHandle(diskHandle);
			return false;
		}

		endpoint.CurReadCount  = diskPerf.ReadCount;
		endpoint.CurWriteCount = diskPerf.WriteCount;
		endpoint.CurOtherCount = 0;
		endpoint.CurReadTime   = diskPerf.ReadTime.QuadPart;
		endpoint.CurWriteTime  = diskPerf.WriteTime.QuadPart;
		endpoint.CurOtherTime  = 0;

		CloseHandle(diskHandle);
		return true;
	}

	static bool QueryNetworkAdapterPerformance(WindowsIOEndpointData& endpoint, std::uint64_t timeSpent)
	{
		// TODO(MarcasRealAccount): Query network adapter performance
		endpoint.CurReadCount  = 0;
		endpoint.CurWriteCount = 0;
		endpoint.CurOtherCount = 0;
		endpoint.CurReadTime   = 0;
		endpoint.CurWriteTime  = 0;
		endpoint.CurOtherTime  = 0;
		endpoint.IdleTime      = timeSpent;
		endpoint.Usage         = 0.0;
		return true;
	}

	static void UpdateIOEndpoints(std::uint64_t timeSpent)
	{
		auto& ioEndpoints = s_WindowsRuntimeData.IOEndpoints;

		std::vector<WindowsIOEndpointData> previousData;
		ioEndpoints.swap(previousData);
		EnumeratePhysicalDrives();
		EnumerateNetworkAdapters();

		std::sort(ioEndpoints.begin(),
				  ioEndpoints.end(),
				  [](const WindowsIOEndpointData& lhs, const WindowsIOEndpointData& rhs) -> bool {
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

			bool success = false;
			switch (endpoint.Type)
			{
			case EIOEndpointType::PhysicalDrive:
				success = QueryDrivePerformance(endpoint, timeSpent);
				break;
			case EIOEndpointType::NetworkAdapter:
				success = QueryNetworkAdapterPerformance(endpoint, timeSpent);
				break;
			}

			if (success)
			{
				std::uint64_t totalTime = (endpoint.CurReadTime - endpoint.LastReadTime) + (endpoint.CurWriteTime - endpoint.LastWriteTime) + (endpoint.CurOtherTime - endpoint.LastOtherTime);
				endpoint.IdleTime       = timeSpent - totalTime;
				endpoint.Usage          = static_cast<double>(totalTime) / static_cast<double>(timeSpent);
			}
			else
			{
				endpoint.IdleTime = timeSpent;
				endpoint.Usage    = 0.0;
			}
		}
	}

	static void PollSystemIOData(RuntimeData* runtimeData, std::uint64_t curGetTime)
	{
		if (!runtimeData)
			return;

		s_WindowsRuntimeData.Mutex.lock();
		std::uint64_t timeSpent = curGetTime - s_WindowsRuntimeData.CurTime;

		if (timeSpent > 2'500'000ULL)
		{
			s_WindowsRuntimeData.LastTime = s_WindowsRuntimeData.CurTime;
			s_WindowsRuntimeData.CurTime  = curGetTime;
			UpdateIOEndpoints(timeSpent);
		}

		runtimeData->IOEndpoints.resize(s_WindowsRuntimeData.IOEndpoints.size());
		for (std::size_t i = 0; i < runtimeData->IOEndpoints.size(); ++i)
		{
			auto& data    = runtimeData->IOEndpoints[i];
			auto& winData = s_WindowsRuntimeData.IOEndpoints[i];
			data          = *static_cast<IOEndpointData*>(&winData);
		}
		s_WindowsRuntimeData.Mutex.unlock();
	}

	Process GetSystemProcess()
	{
		return 0;
	}

	Process GetCurrentProcess()
	{
		return static_cast<Process>(GetCurrentProcessId());
	}

	bool PollData(Process process, RuntimeData* runtimeData)
	{
		if (!runtimeData)
			return false;

		FILETIME curTime;
		GetSystemTimeAsFileTime(&curTime);
		std::uint64_t curGetTime = std::bit_cast<std::uint64_t>(curTime);
		std::uint64_t timeSpent  = curGetTime - runtimeData->CurTime;

		if (timeSpent < 2'500'000ULL)
			return false;

		runtimeData->LastTime = runtimeData->CurTime;
		runtimeData->CurTime  = curGetTime;

		runtimeData->Abilities = RuntimeAbilities::CPUUsage |
								 RuntimeAbilities::MemoryUsage |
								 RuntimeAbilities::IOUsage |
								 RuntimeAbilities::IndividualMemoryUsages;
		if (process == 0)
			runtimeData->Abilities |= RuntimeAbilities::IndividualIOUsages;
		else
			runtimeData->Abilities |= RuntimeAbilities::MemoryPageFaults;

		HANDLE processHandle = process != 0 ? OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, false, process) : INVALID_HANDLE_VALUE;

		// Gather CPU data
		{
			runtimeData->CPUs.resize(1); // TODO(MarcasRealAccount): Implement support for individual cpu usages (possibly with per process individual cpu usages too)

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
				GetProcessTimes(processHandle, &startTime, &exitTime, &kernelTime, &userTime);
			}

			auto& cpu               = runtimeData->CPUs[0];
			cpu.LastSysTime         = cpu.CurSysTime;
			cpu.LastUserTime        = cpu.CurUserTime;
			cpu.CurSysTime          = std::bit_cast<std::uint64_t>(kernelTime);
			cpu.CurUserTime         = std::bit_cast<std::uint64_t>(userTime);
			std::uint64_t totalTime = ((cpu.CurSysTime - cpu.LastSysTime) + (cpu.CurUserTime - cpu.LastUserTime));
			cpu.IdleTime            = (timeSpent * std::thread::hardware_concurrency()) - totalTime;
			cpu.Usage               = static_cast<double>(totalTime) / static_cast<double>(timeSpent * std::thread::hardware_concurrency());
		}

		// Gather Memory data
		{
			auto& mem = runtimeData->Memory;
			if (process == 0)
			{
				MEMORYSTATUSEX memoryCounters {};
				memoryCounters.dwLength = sizeof(memoryCounters);
				if (GlobalMemoryStatusEx(&memoryCounters))
				{
					mem.PageFaultCount = 0;
					mem.PhysicalTotal  = memoryCounters.ullTotalPhys;
					mem.PhysicalUsage  = memoryCounters.ullTotalPhys - memoryCounters.ullAvailPhys;
					mem.VirtualTotal   = memoryCounters.ullTotalPageFile;
					mem.VirtualTotal   = memoryCounters.ullTotalPageFile - memoryCounters.ullAvailPageFile;
				}
				else
				{
					// TODO(MarcasRealAccount): Implement error logging or something?
					mem.PageFaultCount = 0;
					mem.PhysicalTotal  = 0;
					mem.PhysicalUsage  = 0;
					mem.VirtualTotal   = 0;
					mem.VirtualTotal   = 0;
				}
			}
			else
			{
				PROCESS_MEMORY_COUNTERS_EX memCounters {};
				memCounters.cb = sizeof(memCounters);
				if (GetProcessMemoryInfo(processHandle, reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&memCounters), sizeof(memCounters)))
				{
					mem.PageFaultCount = memCounters.PageFaultCount;
					mem.PhysicalTotal  = memCounters.PeakWorkingSetSize;
					mem.PhysicalUsage  = memCounters.WorkingSetSize;
					mem.VirtualTotal   = memCounters.PeakPagefileUsage;
					mem.VirtualTotal   = memCounters.PagefileUsage;
				}
				else
				{
					// TODO(MarcasRealAccount): Implement error logging or something?
					mem.PageFaultCount = 0;
					mem.PhysicalTotal  = 0;
					mem.PhysicalUsage  = 0;
					mem.VirtualTotal   = 0;
					mem.VirtualTotal   = 0;
				}
			}
		}

		// Gather IO Endpoint data
		{
			if (process == 0)
			{
				PollSystemIOData(runtimeData, curGetTime);
			}
			else
			{
				runtimeData->IOEndpoints.resize(1); // TODO(MarcasRealAccount): Figure out per process individual io endpoints!

				auto& endpoint = runtimeData->IOEndpoints[0];

				IO_COUNTERS ioCounters {};
				if (GetProcessIoCounters(processHandle, &ioCounters))
				{
					endpoint.Type = EIOEndpointType::Total;
					endpoint.ID   = 0;
					endpoint.Name = "";

					endpoint.LastReadCount  = endpoint.CurReadCount;
					endpoint.LastWriteCount = endpoint.CurWriteCount;
					endpoint.LastOtherCount = endpoint.CurOtherCount;
					endpoint.CurReadCount   = ioCounters.ReadTransferCount;
					endpoint.CurWriteCount  = ioCounters.WriteTransferCount;
					endpoint.CurOtherCount  = ioCounters.OtherTransferCount;

					endpoint.LastReadTime  = endpoint.CurReadTime;
					endpoint.LastWriteTime = endpoint.CurWriteTime;
					endpoint.LastOtherTime = endpoint.CurOtherTime;
					endpoint.CurReadTime   = 0;
					endpoint.CurWriteTime  = 0;
					endpoint.CurOtherTime  = 0;
					endpoint.IdleTime      = timeSpent;
					endpoint.Usage         = 0.0;
				}
				else
				{
					endpoint.Type = EIOEndpointType::Total;
					endpoint.ID   = 0;
					endpoint.Name = "";

					endpoint.LastReadCount  = endpoint.CurReadCount;
					endpoint.LastWriteCount = endpoint.CurWriteCount;
					endpoint.LastOtherCount = endpoint.CurOtherCount;
					endpoint.CurReadCount   = 0;
					endpoint.CurWriteCount  = 0;
					endpoint.CurOtherCount  = 0;

					endpoint.LastReadTime  = endpoint.CurReadTime;
					endpoint.LastWriteTime = endpoint.CurWriteTime;
					endpoint.LastOtherTime = endpoint.CurOtherTime;
					endpoint.CurReadTime   = 0;
					endpoint.CurWriteTime  = 0;
					endpoint.CurOtherTime  = 0;
					endpoint.IdleTime      = timeSpent;
					endpoint.Usage         = 0.0;
				}
			}
		}

		if (processHandle != INVALID_HANDLE_VALUE)
			CloseHandle(processHandle);

		return true;
	}
} // namespace Profiler
#endif