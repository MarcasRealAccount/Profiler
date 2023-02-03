#include "Profiler/Runtime.h"
#include "Profiler/Utils/Core.h"
#include "Profiler/Utils/UTF.h"

#include <thread>
#include <unordered_map>

#include <fmt/format.h>

#if BUILD_IS_SYSTEM_WINDOWS
	#include <Windows.h>

	#include <winioctl.h>
	#include <psapi.h>
#endif

namespace Profiler
{
	static std::size_t s_CoreCount = std::thread::hardware_concurrency();

#if BUILD_IS_SYSTEM_WINDOWS
	enum class EIOEndpointType
	{
		PhysicalDrive /*,
		 NetworkAdapter*/
	};

	struct IOEndpointData
	{
		EIOEndpointType Type;
		std::uint64_t   ID;
		std::wstring    UNCPath;
		std::string     Name;

		std::uint64_t LastGetTime;
		std::uint64_t LastBytesRead;
		std::uint64_t LastBytesWritten;
		std::uint64_t CurBytesRead;
		std::uint64_t CurBytesWritten;
		std::uint64_t LastReadTime;
		std::uint64_t LastWriteTime;
		double        LastUsage;
	};

	struct ProcessData
	{
		std::uint64_t LastGetTime;
		std::uint64_t LastSysTime;
		std::uint64_t LastUserTime;

		double LastUsage;

		std::uint64_t LastIOGetTime;
		std::uint64_t LastBytesRead;
		std::uint64_t LastBytesWritten;
		std::uint64_t CurBytesRead;
		std::uint64_t CurBytesWritten;
	};

	static std::unordered_map<Process, ProcessData> s_ProcessDatas;
	static std::uint64_t                            s_LastIOEndpointEnumeration;
	static std::vector<IOEndpointData>              s_IOEndpointDatas;

	void ProcessDisk(DISK_EXTENT disk)
	{
		for (auto& endpoint : s_IOEndpointDatas)
			if (endpoint.Type == EIOEndpointType::PhysicalDrive && endpoint.ID == disk.DiskNumber)
				return;

		IOEndpointData endpoint {};
		endpoint.Type    = EIOEndpointType::PhysicalDrive;
		endpoint.ID      = disk.DiskNumber;
		endpoint.UNCPath = Utils::UTF::CToW(fmt::format("//./PhysicalDrive{}", disk.DiskNumber));
		endpoint.Name    = fmt::format("Drive {}", disk.DiskNumber);
		s_IOEndpointDatas.emplace_back(std::move(endpoint));
	}

	void ProcessVolume(wchar_t* volumeName)
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
				DeviceIoControl(volumeHandle,
								IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
								nullptr,
								0,
								pExtents,
								extentsSize,
								&writtenSize,
								nullptr);
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

		for (std::size_t i = 0; i < pExtents->NumberOfDiskExtents; ++i)
			ProcessDisk(pExtents->Extents[i]);

		CloseHandle(volumeHandle);

		if (allocated)
			delete[] pExtents;
	}

	void EnumeratePhysicalDisks()
	{
		wchar_t* volumeName = new wchar_t[32767];
		auto     handle     = FindFirstVolumeW(volumeName, 32767);
		if (handle)
		{
			ProcessVolume(volumeName);
			while (FindNextVolumeW(handle, volumeName, 32767))
				ProcessVolume(volumeName);
			FindVolumeClose(handle);
		}
		delete[] volumeName;
	}

	bool UpdateIOEndpoints()
	{
		std::vector<IOEndpointData> previousData;
		s_IOEndpointDatas.swap(previousData);
		EnumeratePhysicalDisks();

		std::sort(s_IOEndpointDatas.begin(),
				  s_IOEndpointDatas.end(),
				  [](const IOEndpointData& lhs, const IOEndpointData& rhs) -> bool {
					  std::uint32_t tl = static_cast<std::uint32_t>(lhs.Type);
					  std::uint32_t tr = static_cast<std::uint32_t>(rhs.Type);
					  if (tl < tr)
						  return true;
					  else if (tl > tr)
						  return false;
					  return lhs.ID < rhs.ID;
				  });

		for (std::size_t i = 0; i < s_IOEndpointDatas.size(); ++i)
		{
			auto& endpoint = s_IOEndpointDatas[i];
			for (std::size_t j = 0; j < previousData.size(); ++j)
			{
				auto& pEndpoint = previousData[j];
				if (endpoint.Type == pEndpoint.Type && endpoint.ID == pEndpoint.ID)
				{
					endpoint.LastGetTime      = pEndpoint.LastGetTime;
					endpoint.LastBytesRead    = pEndpoint.LastBytesRead;
					endpoint.LastBytesWritten = pEndpoint.LastBytesWritten;
					endpoint.CurBytesRead     = pEndpoint.CurBytesRead;
					endpoint.CurBytesWritten  = pEndpoint.CurBytesWritten;
					endpoint.LastReadTime     = pEndpoint.LastReadTime;
					endpoint.LastWriteTime    = pEndpoint.LastWriteTime;
					endpoint.LastUsage        = pEndpoint.LastUsage;
					break;
				}
			}
		}

		if (previousData.size() != s_IOEndpointDatas.size())
			return true;
		for (std::size_t i = 0; i < previousData.size(); ++i)
		{
			auto& p = previousData[i];
			auto& n = s_IOEndpointDatas[i];
			if (p.Type != n.Type ||
				p.ID != n.ID ||
				p.UNCPath != n.UNCPath ||
				p.Name != n.Name)
				return true;
		}
		return false;
	}
#endif

	ERuntimeAbilities GetRuntimeAbilities()
	{
#if BUILD_IS_SYSTEM_WINDOWS
		return RuntimeAbilities::CoreUsage |
			   RuntimeAbilities::MemoryUsage |
			   RuntimeAbilities::IOUsage |
			   RuntimeAbilities::IndividualMemoryUsages |
			   RuntimeAbilities::IndividualIOUsages |
			   RuntimeAbilities::PerProcessCoreUsage |
			   RuntimeAbilities::PerProcessMemoryUsage |
			   RuntimeAbilities::PerProcessIOUsage |
			   RuntimeAbilities::PerProcessIndividualMemoryUsages |
			   RuntimeAbilities::PerProcessMemoryPageFaults;
#else
		return RuntimeAbilities::None;
#endif
	}

	Process SystemProcess()
	{
#if BUILD_IS_SYSTEM_WINDOWS
		return 0;
#else
		return 0;
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

	std::size_t GetIOEndpointCount(bool* pChanged)
	{
#if BUILD_IS_SYSTEM_WINDOWS
		FILETIME curTime;
		GetSystemTimeAsFileTime(&curTime);
		auto curGetTime = std::bit_cast<std::uint64_t>(curTime);
		if ((curGetTime - s_LastIOEndpointEnumeration) > 2'500'000ULL)
		{
			bool changed = UpdateIOEndpoints();
			if (pChanged)
				*pChanged = changed;
			s_LastIOEndpointEnumeration = curGetTime;
		}
		else if (pChanged)
			*pChanged = false;
		return s_IOEndpointDatas.size();
#else
		if (pChanged)
			*pChanged = false;
		return 0;
#endif
	}

	void GetIOEndpointInfos(std::size_t endpointCount, IOEndpointInfo* infos)
	{
#if BUILD_IS_SYSTEM_WINDOWS
		std::size_t toCopy = std::min(endpointCount, s_IOEndpointDatas.size());
		for (std::size_t i = 0; i < toCopy; ++i)
		{
			auto& info     = infos[i];
			auto& endpoint = s_IOEndpointDatas[i];
			info.Name      = endpoint.Name;
		}
		for (std::size_t i = toCopy; i < endpointCount; ++i)
		{
			auto& info = infos[i];
			info.Name  = "";
		}
#else
		for (std::size_t i = 0; i < endpointCount; ++i)
		{
			auto& info = infos[i];
			info.Name  = "";
		}
#endif
	}

	void GetCoreUsages(Process process, std::size_t coreCount, CoreCounter* counters)
	{
#if BUILD_IS_SYSTEM_WINDOWS
		if (coreCount == 0 || !counters)
			return;

		FILETIME curTime;
		GetSystemTimeAsFileTime(&curTime);
		auto curGetTime = std::bit_cast<std::uint64_t>(curTime);

		ProcessData& data = s_ProcessDatas[process];
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
		counters[0].Usage = data.LastUsage;
#else
		for (std::size_t i = 0; i < coreCount; ++i)
		{
			auto& cnt = counters[i];
			cnt.Usage = 0.0;
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

	void GetIOUsages(Process process, std::size_t endpointCount, IOCounter* counters)
	{
#if BUILD_IS_SYSTEM_WINDOWS
		if (endpointCount == 0 || !counters)
			return;

		if (process == 0)
		{
			FILETIME curTime;
			GetSystemTimeAsFileTime(&curTime);
			auto curGetTime = std::bit_cast<std::uint64_t>(curTime);

			DISK_PERFORMANCE diskPerf {};
			std::size_t      toCopy = std::min(endpointCount, s_IOEndpointDatas.size());
			for (std::size_t i = 0; i < toCopy; ++i)
			{
				auto& counter  = counters[i];
				auto& endpoint = s_IOEndpointDatas[i];
				if ((curGetTime - endpoint.LastGetTime) > 2'500'000ULL)
				{
					switch (endpoint.Type)
					{
					case EIOEndpointType::PhysicalDrive:
					{
						auto diskHandle = CreateFileW(endpoint.UNCPath.c_str(),
													  0,
													  FILE_SHARE_READ | FILE_SHARE_WRITE,
													  nullptr,
													  OPEN_EXISTING,
													  0,
													  0);
						if (diskHandle == INVALID_HANDLE_VALUE)
							break;

						DWORD bytesReturned = 0;

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
							break;
						}

						std::uint64_t curReadTime  = diskPerf.ReadTime.QuadPart;
						std::uint64_t curWriteTime = diskPerf.WriteTime.QuadPart;

						double usage = ((curReadTime - endpoint.LastReadTime) + (curWriteTime - endpoint.LastWriteTime));
						usage        /= (curGetTime - endpoint.LastGetTime);

						endpoint.LastBytesRead    = endpoint.CurBytesRead;
						endpoint.LastBytesWritten = endpoint.CurBytesWritten;
						endpoint.CurBytesRead     = diskPerf.BytesRead.QuadPart;
						endpoint.CurBytesWritten  = diskPerf.BytesWritten.QuadPart;
						endpoint.LastReadTime     = curReadTime;
						endpoint.LastWriteTime    = curWriteTime;
						endpoint.LastUsage        = usage;

						CloseHandle(diskHandle);
						break;
					}
					}

					endpoint.LastGetTime = curGetTime;
				}

				counter.BytesRead    = endpoint.CurBytesRead - endpoint.LastBytesRead;
				counter.BytesWritten = endpoint.CurBytesWritten - endpoint.LastBytesWritten;
				counter.Usage        = endpoint.LastUsage;
			}
			for (std::size_t i = toCopy; i < endpointCount; ++i)
			{
				auto& counter        = counters[i];
				counter.BytesRead    = 0;
				counter.BytesWritten = 0;
				counter.Usage        = 0.0;
			}
		}
		else
		{
			FILETIME curTime;
			GetSystemTimeAsFileTime(&curTime);
			auto  curGetTime = std::bit_cast<std::uint64_t>(curTime);
			auto& data       = s_ProcessDatas[process];
			if ((curGetTime - data.LastIOGetTime) > 2'500'000ULL)
			{
				HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION |
													   PROCESS_QUERY_LIMITED_INFORMATION |
													   PROCESS_VM_READ,
												   false,
												   process);

				IO_COUNTERS ioCounters {};
				if (GetProcessIoCounters(processHandle, &ioCounters))
				{
					data.LastBytesRead    = data.CurBytesRead;
					data.LastBytesWritten = data.CurBytesWritten;
					data.CurBytesRead     = ioCounters.ReadTransferCount;
					data.CurBytesWritten  = ioCounters.WriteTransferCount;
				}

				CloseHandle(processHandle);
				data.LastIOGetTime = curGetTime;
			}

			auto& counter        = counters[0];
			counter.BytesRead    = data.CurBytesRead - data.LastBytesRead;
			counter.BytesWritten = data.CurBytesWritten - data.LastBytesWritten;
			counter.Usage        = 0.0;
		}
#else
		for (std::size_t i = 0; i < endpointCount; ++i)
		{
			auto& counter        = counters[i];
			counter.BytesRead    = 0;
			counter.BytesWritten = 0;
			counter.Usage        = 0.0;
		}
#endif
	}
} // namespace Profiler