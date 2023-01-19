#pragma once

#include "Utils/Core.h"
#include "Utils/Flags.h"

#include <cstddef>
#include <cstdint>

#include <atomic>
#include <mutex>
#include <vector>

namespace Profiler
{
	using EAbilities = Utils::Flags<std::uint32_t>;

	namespace Abilities
	{
		static constexpr EAbilities InvariantCPUClock = 1;
		static constexpr EAbilities IBS               = 2;
	} // namespace Abilities

	enum class EEventType : std::uint8_t
	{
		Unknown = 0,
		ThreadBounds,
		ThreadBegin,
		ThreadEnd,
		Frame,
		FunctionBegin,
		FunctionEnd,
		Callstack,
		BoolArgument,
		IntArgument,
		FloatArgument,
		FlagsArgument,
		PtrArgument,
		ForLoopBegin,
		ForLoopEnd,
		ForLoopIterBegin,
		ForLoopIterEnd,
		MemAlloc,
		MemFree,
		DataHeader
	};

	struct EventTimestamp
	{
		std::uint64_t Time : 63;
		std::uint64_t Type : 1;
	};

	struct Event
	{
	public:
		Event()
			: Type(EEventType::Unknown),
			  Data { 0 } {}

		Event(EEventType type)
			: Type(type),
			  Data { 0 } {}

	public:
		EEventType   Type;
		std::uint8_t Data[31];
	};

	struct ThreadBoundsEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::ThreadBounds;

	public:
		EEventType    Type;
		std::uint64_t ThreadID;
		std::uint64_t Length;
	};

	struct ThreadBeginEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::ThreadBegin;

	public:
		EEventType     Type;
		EventTimestamp Timestamp;
	};

	struct ThreadEndEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::ThreadEnd;

	public:
		EEventType     Type;
		EventTimestamp Timestamp;
	};

	struct FrameEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::Frame;

	public:
		EEventType     Type;
		std::uint64_t  FrameNum;
		EventTimestamp Timestamp;
	};

	struct FunctionBeginEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::FunctionBegin;

	public:
		EEventType     Type;
		void*          FunctionPtr;
		EventTimestamp Timestamp;
	};

	struct FunctionEndEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::FunctionEnd;

	public:
		EEventType     Type;
		EventTimestamp Timestamp;
	};

	struct CallstackEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::Callstack;

	public:
		EEventType    Type;
		std::uint64_t DataID;
		std::uint64_t NumEntries;
	};

	struct BoolArgumentEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::BoolArgument;

	public:
		EEventType   Type;
		std::uint8_t Offset;
		std::uint8_t Pad[6];
		bool         Value;
	};

	struct IntArgumentEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::IntArgument;

	public:
		EEventType    Type;
		std::uint8_t  Offset;
		std::uint8_t  Size;
		std::uint8_t  Base;
		std::uint8_t  Pad[4];
		std::uint64_t Data[3];
	};

	struct FloatArgumentEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::FloatArgument;

	public:
		EEventType    Type;
		std::uint8_t  Offset;
		std::uint8_t  Size;
		std::uint8_t  Pad[5];
		std::uint64_t Data[3];
	};

	struct FlagsArgumentEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::FlagsArgument;

	public:
		EEventType    Type;
		std::uint8_t  Offset;
		std::uint8_t  Pad[6];
		std::uint64_t FlagsType;
		std::uint64_t Data[2];
	};

	struct PtrArgumentEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::PtrArgument;

	public:
		EEventType   Type;
		std::uint8_t Offset;
		std::uint8_t Pad[6];
		void*        Ptr;
	};

	struct ForLoopBeginEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::ForLoopBegin;

	public:
		EEventType     Type;
		std::size_t    ID;
		EventTimestamp Timestamp;
	};

	struct ForLoopEndEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::ForLoopEnd;

	public:
		EEventType     Type;
		std::uint64_t  ID;
		EventTimestamp Timestamp;
	};

	struct ForLoopIterBeginEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::ForLoopIterBegin;

	public:
		EEventType     Type;
		std::uint8_t   Size;
		std::uint8_t   Pad[6];
		std::uint64_t  ID;
		EventTimestamp Timestamp;
		std::uint64_t  Index[2];
	};

	struct ForLoopIterEndEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::ForLoopIterEnd;

	public:
		EEventType     Type;
		std::uint8_t   Pad[7];
		std::uint64_t  ID;
		EventTimestamp Timestamp;
	};

	struct MemAllocEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::MemAlloc;

	public:
		EEventType     Type;
		void*          Memory;
		std::uint64_t  Size;
		EventTimestamp Timestamp;
	};

	struct MemFreeEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::MemFree;

	public:
		EEventType     Type;
		void*          Memory;
		EventTimestamp Timestamp;
	};

	struct DataHeaderEvent
	{
	public:
		static constexpr EEventType c_Type = EEventType::DataHeader;

	public:
		EEventType    Type;
		std::uint64_t Size;
		std::uint64_t ID;
	};

	struct DataSectionEvent
	{
	public:
		std::uint8_t Data[32];
	};

	class alignas(32) ThreadState
	{
	public:
		std::uint64_t    ThreadID      = 0;
		std::uint64_t    FunctionDepth = 0;
		std::uint64_t    ForLoopDepth  = 0;
		std::uint8_t     CurrentIndex  = 0;
		std::atomic_bool Capture       = false;
		Event            Buffer[128];
	};

	class State
	{
	public:
		void pushEvents(Event* events, std::size_t count, std::uint64_t threadID)
		{
			EventMutex.lock();
			ThreadBoundsEvent* bounds = reinterpret_cast<ThreadBoundsEvent*>(&Events.emplace_back(ThreadBoundsEvent::c_Type));
			bounds->ThreadID          = threadID;
			bounds->Length            = count;
			Events.insert(Events.end(), events, events + count);
			EventMutex.unlock();
		}

		void addThread(ThreadState* state)
		{
			ThreadsMutex.lock();
			Threads.emplace_back(state);
			ThreadsMutex.unlock();
			state->Capture = Initialized && Capturing;
		}

		void removeThread(ThreadState* state)
		{
			ThreadsMutex.lock();
			std::erase(Threads, state);
			ThreadsMutex.unlock();
		}

	public:
		bool Initialized   = false;
		bool Capturing     = false;
		bool WantCapturing = false;

		EAbilities Abilities = 0;

		std::vector<Event> Events;
		std::mutex         EventMutex;

		std::uint64_t        CurrentFrame  = 0;
		std::atomic_uint64_t CurrentDataID = 0;

		std::uint64_t InvariantClockFrequency = 0;

		std::vector<ThreadState*> Threads;
		std::mutex                ThreadsMutex;
		std::uint64_t             MainThreadID;
	};

	void Init();
	void Deinit();
	void WantCapturing(bool capture, bool instant = false);
	void WriteCaptures();

	std::uint64_t GetThreadID();
	bool          IsMainThread();

	extern State g_State;

	extern thread_local ThreadState g_TState;

	inline std::uint64_t NewDataID()
	{
		return g_State.CurrentDataID++;
	}

	template <class T>
	inline T& NewEvent(ThreadState* state)
	{
		std::uint8_t ci = state->CurrentIndex;
		if (ci & 0x80)
		{
			g_State.pushEvents(state->Buffer, 128, state->ThreadID);
			ci = 0;
		}
		T& elem = *reinterpret_cast<T*>(&state->Buffer[ci]);
		if constexpr (requires { T::c_Type; })
			elem.Type = T::c_Type;
		state->CurrentIndex = ci + 1;
		return elem;
	}

	inline void FlushEvents(ThreadState* state)
	{
		g_State.pushEvents(state->Buffer, state->CurrentIndex, state->ThreadID);
		state->CurrentIndex = 0;
	}

	inline ThreadState* GetThreadState()
	{
		return &g_TState;
	}

	inline void FreeThreadState([[maybe_unused]] ThreadState* state)
	{
	}
} // namespace Profiler