#include "Profiler/State.h"
#include "Profiler/Utils/Core.h"
#include "Profiler/Utils/IntrinsicsThatClangDoesntSupport.h"

#include <iostream>

#include <fmt/format.h>

#if BUILD_IS_SYSTEM_WINDOWS
	#include <Windows.h>

	#include <powerbase.h>

	#pragma comment(lib, "PowrProf.lib")

typedef struct _PROCESSOR_POWER_INFORMATION
{
	ULONG Number;
	ULONG MaxMhz;
	ULONG CurrentMhz;
	ULONG MhzLimit;
	ULONG MaxIdleState;
	ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;
#endif

namespace Profiler
{
	State                    g_State {};
	thread_local ThreadState g_TState {};

	static void CheckInvariantClock()
	{
		int res[4];
		Utils::cpuid(res, 0x8000'0001);
		if (!((res[2] >> 23) & 1))
			return;

		Utils::cpuid(res, 1);
		if (!((res[3] >> 4) & 1))
			return;

		Utils::cpuid(res, 0x8000'0007);
		if (!((res[3] >> 8) & 1))
			return;

		g_State.Abilities |= Abilities::InvariantCPUClock;

#if BUILD_IS_SYSTEM_WINDOWS
		SYSTEM_INFO sysInfo {};
		GetSystemInfo(&sysInfo);

		std::size_t coreCount = sysInfo.dwNumberOfProcessors;

		PROCESSOR_POWER_INFORMATION* infos = new PROCESSOR_POWER_INFORMATION[coreCount];

		CallNtPowerInformation(ProcessorInformation, nullptr, 0, infos, static_cast<ULONG>(coreCount * sizeof(*infos)));

		g_State.InvariantClockFrequency = infos[0].MaxMhz * 1'000'000ULL;

		delete[] infos;
#else
		g_State.InvariantClockFrequency = 0;
#endif
	}

	static void CheckIBS()
	{
		int res[4];
		Utils::cpuid(res, 0x8000'0001);
		if (!((res[2] >> 10) & 1))
			return;

		g_State.Abilities |= Abilities::IBS;
		// TODO(MarcasRealAccount): Implement IBS
	}

	static void SetupTLS()
	{
	}

	static void FreeTLS()
	{
	}

	void Init()
	{
		g_State.Initialized = true;
		g_State.Capturing   = false;
		g_State.Abilities   = 0;
		g_State.Events.clear();
		g_State.CurrentFrame            = 0;
		g_State.InvariantClockFrequency = 0;
		g_State.Threads.clear();
		g_TState.ThreadID = GetThreadID();
		g_State.ThreadsMutex.lock();
		g_State.Threads.emplace_back(&g_TState);
		g_State.MainThreadID = g_State.Threads.back()->ThreadID;
		g_State.ThreadsMutex.unlock();

		CheckInvariantClock();
		CheckIBS();
		SetupTLS();
	}

	void Deinit()
	{
		g_State.Initialized = false;
		g_State.Capturing   = false;
		g_State.Events.clear();
		for (auto tstate : g_State.Threads)
		{
			tstate->Capture = false;
			g_State.pushEvents(tstate->Buffer, tstate->CurrentIndex, tstate->ThreadID);
			tstate->CurrentIndex = 0;
			FreeThreadState(tstate);
		}
		FreeTLS();
	}

	void WantCapturing(bool capture, bool instant)
	{
		if (instant)
		{
			g_State.WantCapturing = capture;
			g_State.Capturing     = capture;
			bool canCapture       = g_State.Initialized && g_State.Capturing;
			for (auto tstate : g_State.Threads)
				tstate->Capture = canCapture;
			if (!canCapture)
			{
				for (auto tstate : g_State.Threads)
				{
					g_State.pushEvents(tstate->Buffer, tstate->CurrentIndex, tstate->ThreadID);
					tstate->CurrentIndex = 0;
				}
			}
		}
		else
		{
			g_State.WantCapturing = capture;
		}
	}

	static void WriteEvent(Event* event)
	{
		switch (event->Type)
		{
		case EEventType::ThreadBounds:
		{
			ThreadBoundsEvent* data = reinterpret_cast<ThreadBoundsEvent*>(event);
			std::cout << fmt::format("Thread Bounds {}, length: {}\n", data->ThreadID, data->Length);
			break;
		}
		case EEventType::ThreadBegin:
		{
			ThreadBeginEvent* data = reinterpret_cast<ThreadBeginEvent*>(event);
			std::cout << fmt::format("Thread Begin, time: {}, type: {}\n", static_cast<std::uint64_t>(data->Timestamp.Time), data->Timestamp.Type ? "HR" : "LR");
			break;
		}
		case EEventType::ThreadEnd:
		{
			ThreadEndEvent* data = reinterpret_cast<ThreadEndEvent*>(event);
			std::cout << fmt::format("Thread End, time: {}, type: {}\n", static_cast<std::uint64_t>(data->Timestamp.Time), data->Timestamp.Type ? "HR" : "LR");
			break;
		}
		case EEventType::Frame:
		{
			FrameEvent* data = reinterpret_cast<FrameEvent*>(event);
			std::cout << fmt::format("Frame {}, time: {}, type: {}\n", data->FrameNum, static_cast<std::uint64_t>(data->Timestamp.Time), data->Timestamp.Type ? "HR" : "LR");
			break;
		}
		case EEventType::FunctionBegin:
		{
			FunctionBeginEvent* data = reinterpret_cast<FunctionBeginEvent*>(event);
			std::cout << fmt::format("Function Begin {}, time: {}, type: {}\n", data->FunctionPtr, static_cast<std::uint64_t>(data->Timestamp.Time), data->Timestamp.Type ? "HR" : "LR");
			break;
		}
		case EEventType::FunctionEnd:
		{
			FunctionEndEvent* data = reinterpret_cast<FunctionEndEvent*>(event);
			std::cout << fmt::format("Function End, time: {}, type: {}\n", static_cast<std::uint64_t>(data->Timestamp.Time), data->Timestamp.Type ? "HR" : "LR");
			break;
		}
		case EEventType::BoolArgument:
		{
			BoolArgumentEvent* data = reinterpret_cast<BoolArgumentEvent*>(event);
			std::cout << fmt::format("    Argument {} = {}\n", data->Offset, data->Value);
			break;
		}
		case EEventType::IntArgument:
		{
			IntArgumentEvent* data = reinterpret_cast<IntArgumentEvent*>(event);
			switch (data->Size)
			{
			case 1:
				std::cout << fmt::format("    Argument {} = {}\n", data->Offset, data->Data[0] & 0xFF);
				break;
			case 2:
				std::cout << fmt::format("    Argument {} = {}\n", data->Offset, data->Data[0] & 0xFFFF);
				break;
			case 4:
				std::cout << fmt::format("    Argument {} = {}\n", data->Offset, data->Data[0] & 0xFFFF'FFFF);
				break;
			case 8:
				std::cout << fmt::format("    Argument {} = {}\n", data->Offset, data->Data[0] & 0xFFFF'FFFF'FFFF'FFFF);
				break;
			}
			break;
		}
		case EEventType::FloatArgument:
		{
			FloatArgumentEvent* data = reinterpret_cast<FloatArgumentEvent*>(event);
			switch (data->Size)
			{
			case 4:
			{
				float v;
				std::memcpy(&v, data->Data, sizeof(v));
				std::cout << fmt::format("    Argument {} = {}\n", data->Offset, v);
				break;
			}
			case 8:
			{
				double v;
				std::memcpy(&v, data->Data, sizeof(v));
				std::cout << fmt::format("    Argument {} = {}\n", data->Offset, v);
				break;
			}
			}
			break;
		}
		case EEventType::FlagsArgument:
		{
			FlagsArgumentEvent* data = reinterpret_cast<FlagsArgumentEvent*>(event);
			std::cout << fmt::format("    Argument {} = {}\n", data->Offset, data->Data[0]);
			break;
		}
		case EEventType::PtrArgument:
		{
			PtrArgumentEvent* data = reinterpret_cast<PtrArgumentEvent*>(event);
			std::cout << fmt::format("    Argument {} = {}\n", data->Offset, data->Ptr);
			break;
		}
		case EEventType::ForLoopBegin:
		{
			ForLoopBeginEvent* data = reinterpret_cast<ForLoopBeginEvent*>(event);
			std::cout << fmt::format("For Loop Begin, id: {}, time: {}, type: {}\n", data->ID, static_cast<std::uint64_t>(data->Timestamp.Time), data->Timestamp.Type ? "HR" : "LR");
			break;
		}
		case EEventType::ForLoopEnd:
		{
			ForLoopEndEvent* data = reinterpret_cast<ForLoopEndEvent*>(event);
			std::cout << fmt::format("For Loop End, id: {}, time: {}, type: {}\n", data->ID, static_cast<std::uint64_t>(data->Timestamp.Time), data->Timestamp.Type ? "HR" : "LR");
			break;
		}
		case EEventType::ForLoopIterBegin:
		{
			ForLoopIterBeginEvent* data = reinterpret_cast<ForLoopIterBeginEvent*>(event);
			std::cout << fmt::format("For Loop Iter Begin {}, id: {}, time: {}, type: {}\n", data->Index[0], data->ID, static_cast<std::uint64_t>(data->Timestamp.Time), data->Timestamp.Type ? "HR" : "LR");
			break;
		}
		case EEventType::ForLoopIterEnd:
		{
			ForLoopIterEndEvent* data = reinterpret_cast<ForLoopIterEndEvent*>(event);
			std::cout << fmt::format("For Loop Iter End, id: {}, time: {}, type: {}\n", data->ID, static_cast<std::uint64_t>(data->Timestamp.Time), data->Timestamp.Type ? "HR" : "LR");
			break;
		}
		case EEventType::MemAlloc:
		{
			MemAllocEvent* data = reinterpret_cast<MemAllocEvent*>(event);
			std::cout << fmt::format("Mem Alloc {}, size: {}, time: {}, type: {}\n", data->Memory, data->Size, static_cast<std::uint64_t>(data->Timestamp.Time), data->Timestamp.Type ? "HR" : "LR");
			break;
		}
		case EEventType::MemFree:
		{
			MemFreeEvent* data = reinterpret_cast<MemFreeEvent*>(event);
			std::cout << fmt::format("Mem Free {}, time: {}, type: {}\n", data->Memory, static_cast<std::uint64_t>(data->Timestamp.Time), data->Timestamp.Type ? "HR" : "LR");
			break;
		}
		default:
			break;
		}
	}

	void WriteCaptures()
	{
		for (auto event : g_State.Events)
			WriteEvent(&event);
	}

	std::uint64_t GetThreadID()
	{
#if BUILD_IS_SYSTEM_WINDOWS
		return GetThreadId(GetCurrentThread());
#else
		return 0;
#endif
	}

	bool IsMainThread()
	{
		return g_TState.ThreadID == g_State.MainThreadID;
	}
} // namespace Profiler