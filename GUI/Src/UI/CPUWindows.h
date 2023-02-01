#pragma once

#include <cstddef>
#include <cstdint>

namespace UI
{
	struct TimelineOptions
	{
	public:
		// State
		double SampleRate    = 4.5e9;       // Timesteps per second
		double InvSampleRate = 1.0 / 4.5e9; // Seconds per timestep
		double Offset        = 0.0;         // Timestep offset
		double Scale         = 1.0;         // Timesteps per pixel
		double InvScale      = 1.0;         // Pixels per timestep

		// std::uint64_t Offset     = 0;           // Timestep offset

		// Style
		bool          Borders             = false;
		std::uint32_t BorderColor         = 0;
		std::uint32_t DefaultEntryColor   = 0;
		std::uint32_t LightEntryTextColor = 0;
		std::uint32_t DarkEntryTextColor  = 0;
		std::uint32_t LargeBarColor       = 0;
		std::uint32_t SmallBarColor       = 0;
		std::uint32_t TimeColor           = 0;
		float         LargeBarHeight      = 0.0f;
		float         SmallBarHeight      = 0.0f;
		std::uint8_t  SmallBars           = 9;
	};

	struct TimelineEntry
	{
	public:
		std::uint64_t Begin;
		std::uint64_t End;
		const char*   Text;
		std::uint32_t Color;
	};

	void DefaultTimelineStyle(TimelineOptions* options);
	void DrawTimescale(TimelineOptions* options);
	void DrawTimeline(TimelineOptions* options, std::size_t numEntries, TimelineEntry* entries);
} // namespace UI