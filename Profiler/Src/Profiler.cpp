#include "Profiler/Profiler.h"

#include <chrono>
#include <fstream>
#include <variant>

#include <format>
#include <iostream>

namespace Profiler
{
	enum class EEventType : std::uint8_t
	{
		Frame         = 0,
		Function      = 2,
		TimespanEvent = 3,
		Argument      = 4
	};

	struct ProfilerEvent
	{
	public:
		EEventType   Type;
		std::uint8_t Data[31];
	};

	struct FrameEvent
	{
	public:
		EEventType    Type;
		std::uint64_t FrameNum;
	};

	struct FunctionEvent
	{
	public:
		EEventType    Type;
		void*         FunctionPtr;
		std::uint64_t Length;
	};

	struct TimespanEvent
	{
	public:
		EEventType    Type;
		std::uint64_t Start, End;
	};

	struct ArgumentEvent
	{
	public:
		EEventType    Type;
		std::uint8_t  ArgType;
		std::uint16_t Flags;
		std::uint16_t Offset;
		std::uint64_t Values[3];
	};

	struct ProfilerState
	{
	public:
		bool               Initialized = false;
		bool               Capturing   = false;
		EProfilerAbilities Abilities   = 0;

		std::vector<ProfilerEvent> Events;
		EventID                    StartEvent   = 0;
		EventID                    CurrentEvent = 0;

		std::uint64_t CurrentFrame = 0;
	};

	static ProfilerState s_ProfilerState;

	static constexpr std::uint64_t CurrentTime()
	{
		return std::chrono::high_resolution_clock::now().time_since_epoch().count();
	}

	static EventID AddEvent(ProfilerEvent&& event)
	{
		++s_ProfilerState.CurrentEvent;
		s_ProfilerState.Events.emplace_back(std::move(event));
		return s_ProfilerState.CurrentEvent - 1;
	}

	FunctionRAII::FunctionRAII()
		: m_Event(~0ULL) {}

	FunctionRAII::FunctionRAII(void* functionPtr)
		: m_Event(~0ULL)
	{
		if (!s_ProfilerState.Abilities.hasFlag(ProfilerAbilities::Functions)) [[unlikely]]
			return;

		ProfilerEvent event {};
		event.Type          = EEventType::Function;
		FunctionEvent* data = reinterpret_cast<FunctionEvent*>(&event);
		data->FunctionPtr   = functionPtr;
		data->Length        = 0;
		m_Event             = AddEvent(std::move(event));

		ProfilerEvent event2 {};
		event2.Type          = EEventType::TimespanEvent;
		TimespanEvent* data2 = reinterpret_cast<TimespanEvent*>(&event2);
		data2->Start         = CurrentTime();
		data2->End           = 0;
		AddEvent(std::move(event2));
	}

	FunctionRAII::FunctionRAII(FunctionRAII&& move) noexcept
		: m_Event(move.m_Event)
	{
		move.m_Event = ~0ULL;
	}

	FunctionRAII::~FunctionRAII()
	{
		if (m_Event != ~0ULL) [[unlikely]]
		{
			FunctionEvent* data  = reinterpret_cast<FunctionEvent*>(&s_ProfilerState.Events[m_Event - s_ProfilerState.StartEvent]);
			TimespanEvent* data2 = reinterpret_cast<TimespanEvent*>(&s_ProfilerState.Events[m_Event + 1 - s_ProfilerState.StartEvent]);
			data->Length         = s_ProfilerState.CurrentEvent - m_Event;
			data2->End           = CurrentTime();
		}
	}

	void Init()
	{
		s_ProfilerState.Initialized  = true;
		s_ProfilerState.Capturing    = false;
		s_ProfilerState.Abilities    |= ProfilerAbilities::Functions;
		s_ProfilerState.Abilities    |= ProfilerAbilities::Arguments;
		s_ProfilerState.Events       = {};
		s_ProfilerState.StartEvent   = 0;
		s_ProfilerState.CurrentEvent = ~0ULL;
		s_ProfilerState.CurrentFrame = 0;
	}

	void Deinit()
	{
		s_ProfilerState.Capturing   = false;
		s_ProfilerState.Initialized = false;
		s_ProfilerState.Events      = {};
	}

	void BeginCapturing(std::size_t minEventCount)
	{
		s_ProfilerState.Capturing = true;
		s_ProfilerState.Events.clear();
		s_ProfilerState.Events.reserve(minEventCount);
		s_ProfilerState.StartEvent   = s_ProfilerState.CurrentEvent + 1;
		s_ProfilerState.CurrentEvent = s_ProfilerState.StartEvent;
	}

	void EndCapturing()
	{
		s_ProfilerState.Capturing = false;
	}

	void WriteCaptures()
	{
		/*std::ofstream file("Result.captures");
		if (!file)
			return;

		std::uint64_t numEvents = s_ProfilerState.Events.size();
		file.write(reinterpret_cast<const char*>(&s_ProfilerState.StartEvent), sizeof(s_ProfilerState.StartEvent));
		file.write(reinterpret_cast<const char*>(&numEvents), sizeof(numEvents));
		file.write(reinterpret_cast<const char*>(s_ProfilerState.Events.data()), s_ProfilerState.Events.size() * sizeof(ProfilerEvent));
		file.close();*/

		std::cout << std::format("Event {} to {}\n", s_ProfilerState.StartEvent, s_ProfilerState.StartEvent + s_ProfilerState.Events.size());
		for (std::size_t i = 0; i < s_ProfilerState.Events.size(); ++i)
		{
			auto& ev = s_ProfilerState.Events[i];
			switch (ev.Type)
			{
			case EEventType::Frame:
			{
				FrameEvent* data = reinterpret_cast<FrameEvent*>(&ev);
				std::cout << std::format("Frame {}\n", data->FrameNum);
				break;
			}
			case EEventType::Function:
			{
				FunctionEvent* data = reinterpret_cast<FunctionEvent*>(&ev);
				std::cout << std::format("Function {}, {} events\n", data->FunctionPtr, data->Length);
				break;
			}
			case EEventType::TimespanEvent:
			{
				TimespanEvent* data = reinterpret_cast<TimespanEvent*>(&ev);
				std::cout << std::format("    Timespan {} to {}\n", data->Start, data->End);
				break;
			}
			case EEventType::Argument:
			{
				ArgumentEvent* data = reinterpret_cast<ArgumentEvent*>(&ev);
				switch (data->ArgType)
				{
				case 0:
				{
					bool value = data->Values[0];
					std::cout << std::format("    Argument {} = {}\n", data->Offset, value ? "true" : "false");
					break;
				}
				case 1:
				{
					void* value = std::bit_cast<void*>(data->Values[0]);
					std::cout << std::format("    Argument {} = {}\n", data->Offset, value);
					break;
				}
				case 2:
				{
					std::uint64_t value = data->Values[0]; // oof
					std::cout << std::format("    Argument {} = {}\n", data->Offset, value);
					break;
				}
				case 3:
				{
					switch (data->Flags >> 8)
					{
					case 2:
					{
						float value = std::bit_cast<float>(static_cast<std::uint32_t>(data->Values[0]));
						std::cout << std::format("    Argument {} = {}\n", data->Offset, value);
						break;
					}
					case 3:
					{
						double value = std::bit_cast<double>(data->Values[0]);
						std::cout << std::format("    Argument {} = {}\n", data->Offset, value);
						break;
					}
					default:
						std::cout << std::format("    Argument {} = unknown float\n", data->Offset);
						break;
					}
					break;
				}
				case 4:
				{
					std::uint64_t value = data->Values[0]; // oof
					std::cout << std::format("    Argument {} = {}\n", data->Offset, value);
					break;
				}
				default:
					std::cout << std::format("    Argument {} = unknown type\n", data->Offset);
					break;
				}
				break;
			}
			}
		}
	}

	void Frame()
	{
		if (s_ProfilerState.Capturing) [[unlikely]]
		{
			ProfilerEvent event {};
			event.Type       = EEventType::Frame;
			FrameEvent* data = reinterpret_cast<FrameEvent*>(&event);
			data->FrameNum   = s_ProfilerState.CurrentFrame++;
			AddEvent(std::move(event));
		}
	}

	namespace Detail
	{
		void IntArg(std::uint16_t flags, std::uint16_t offset, std::uint64_t (&values)[3])
		{
			if (s_ProfilerState.Capturing) [[unlikely]]
			{
				if (!s_ProfilerState.Abilities.hasFlag(ProfilerAbilities::Arguments)) [[unlikely]]
					return;

				ProfilerEvent event {};
				event.Type          = EEventType::Argument;
				ArgumentEvent* data = reinterpret_cast<ArgumentEvent*>(&event);
				data->ArgType       = 2;
				data->Flags         = flags;
				data->Offset        = offset;
				std::memcpy(data->Values, values, sizeof(values));
				AddEvent(std::move(event));
			}
		}

		void FloatArg(std::uint16_t flags, std::uint16_t offset, std::uint64_t (&values)[3])
		{
			if (s_ProfilerState.Capturing) [[unlikely]]
			{
				if (!s_ProfilerState.Abilities.hasFlag(ProfilerAbilities::Arguments)) [[unlikely]]
					return;

				ProfilerEvent event {};
				event.Type          = EEventType::Argument;
				ArgumentEvent* data = reinterpret_cast<ArgumentEvent*>(&event);
				data->ArgType       = 3;
				data->Flags         = flags;
				data->Offset        = offset;
				std::memcpy(data->Values, values, sizeof(values));
				AddEvent(std::move(event));
			}
		}

		void FlagsArg(std::uint16_t flags, std::uint16_t offset, const std::type_info* type, std::uint64_t (&values)[2])
		{
			if (s_ProfilerState.Capturing) [[unlikely]]
			{
				if (!s_ProfilerState.Abilities.hasFlag(ProfilerAbilities::Arguments)) [[unlikely]]
					return;

				ProfilerEvent event {};
				event.Type          = EEventType::Argument;
				ArgumentEvent* data = reinterpret_cast<ArgumentEvent*>(&event);
				data->ArgType       = 4;
				data->Flags         = flags;
				data->Offset        = offset;
				data->Values[0]     = std::bit_cast<std::uint64_t>(type);
				std::memcpy(data->Values + 1, values, sizeof(values));
				AddEvent(std::move(event));
			}
		}
	} // namespace Detail

	FunctionRAII Function(void* functionPtr)
	{
		return s_ProfilerState.Capturing ? FunctionRAII { functionPtr } : FunctionRAII {};
	}

	void BoolArg(std::uint16_t offset, bool value)
	{
		if (s_ProfilerState.Capturing) [[unlikely]]
		{
			if (!s_ProfilerState.Abilities.hasFlag(ProfilerAbilities::Arguments)) [[unlikely]]
				return;

			ProfilerEvent event {};
			event.Type          = EEventType::Argument;
			ArgumentEvent* data = reinterpret_cast<ArgumentEvent*>(&event);
			data->ArgType       = 0;
			data->Flags         = 0;
			data->Offset        = offset;
			data->Values[0]     = value;
			AddEvent(std::move(event));
		}
	}

	void PtrArg(std::uint16_t offset, void* value)
	{
		if (s_ProfilerState.Capturing) [[unlikely]]
		{
			if (!s_ProfilerState.Abilities.hasFlag(ProfilerAbilities::Arguments)) [[unlikely]]
				return;

			ProfilerEvent event {};
			event.Type          = EEventType::Argument;
			ArgumentEvent* data = reinterpret_cast<ArgumentEvent*>(&event);
			data->ArgType       = 1;
			data->Flags         = 0;
			data->Offset        = offset;
			data->Values[0]     = std::bit_cast<std::uint64_t>(value);
			AddEvent(std::move(event));
		}
	}

	EProfilerAbilities GetAbilities()
	{
		return s_ProfilerState.Abilities;
	}
} // namespace Profiler