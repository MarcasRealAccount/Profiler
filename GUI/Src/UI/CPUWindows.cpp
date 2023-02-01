#include "CPUWindows.h"

#include <cmath>

#include <chrono>
#include <unordered_map>
#include <vector>

#include <Profiler/State.h>

#include <fmt/format.h>
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

namespace UI
{
	template <std::floating_point T>
	T Clamp(T x, std::type_identity_t<T> min, std::type_identity_t<T> max)
	{
		return std::min(std::max(x, min), max);
	}

	template <std::floating_point T>
	T Floor(T x)
	{
		return x < T { 0 } ? std::floor(x) : std::floor(x);
	}

	template <std::floating_point T>
	T Ceil(T x)
	{
		return x < T { 0 } ? std::ceil(x) : std::ceil(x);
	}

	template <std::floating_point T>
	T FloorToBoundary(T x, T scale)
	{
		return Floor(x / scale) * scale;
	}

	template <std::floating_point T>
	T CeilToBoundary(T x, T scale)
	{
		return Ceil(x / scale) * scale;
	}

	template <std::floating_point T>
	T Frac(T x)
	{
		return x - std::floor(x);
	}

	std::string_view GetTimeSuffix(std::int32_t scale)
	{
		switch (scale)
		{
		case -30: return "qs";
		case -27: return "rs";
		case -24: return "ys";
		case -21: return "zs";
		case -18: return "as";
		case -15: return "fs";
		case -12: return "ps";
		case -9: return "ns";
		case -6: return "\xc2\xb5s";
		case -3: return "ms";
		case 0: return "s";
		case 3: return "ks";
		case 6: return "Ms";
		case 9: return "Gs";
		case 12: return "Ts";
		case 15: return "Ps";
		case 18: return "Es";
		case 21: return "Zs";
		case 24: return "Ys";
		case 27: return "Rs";
		case 30: return "Qs";
		default: return "?s";
		}
	}

	double TimestampToScreenX(std::uint64_t timestamp, TimelineOptions* options)
	{
		return (timestamp - options->Offset) * options->InvScale;
	}

	std::uint64_t ScreenXToTimestamp(double x, TimelineOptions* options)
	{
		return static_cast<std::uint64_t>(x * options->Scale + options->Offset);
	}

	double ScreenXToSeconds(double x, double spx, TimelineOptions* options)
	{
		return x * spx + options->Offset * options->InvSampleRate;
	}

	void DefaultTimelineStyle(TimelineOptions* options)
	{
		options->Borders             = true;
		options->BorderColor         = ImGui::GetColorU32(ImGuiCol_Border);
		options->DefaultEntryColor   = ImGui::GetColorU32(ImGuiCol_PlotLines);
		options->LightEntryTextColor = ImGui::GetColorU32(ImGuiCol_Text);
		options->DarkEntryTextColor  = 255 << 24 | (255 - (options->LightEntryTextColor >> 16) & 0xFF) << 16 | (255 - (options->LightEntryTextColor >> 8) & 0xFF) << 8 | 255 - options->LightEntryTextColor & 0xFF;
		options->LargeBarColor       = ImGui::GetColorU32(ImGuiCol_Separator);
		options->SmallBarColor       = ImGui::GetColorU32(ImGuiCol_Separator);
		options->TimeColor           = ImGui::GetColorU32(ImGuiCol_Separator);
		options->LargeBarHeight      = ImGui::GetTextLineHeight();
		options->SmallBarHeight      = options->LargeBarHeight * 0.5f;
	}

	void DrawLargeBar(TimelineOptions* options, ImGuiWindow* window, ImRect& barsBB, ImRect& timeBB, float x, double baseTime, std::int32_t baseScale, double t, std::int32_t scale)
	{
		std::string_view baseSuffix = GetTimeSuffix(baseScale);
		std::string_view suffix     = GetTimeSuffix(scale);
		std::string      str        = baseScale > scale ? fmt::format("{:.2f}{} + {:.2f}{}", baseTime, baseSuffix, t, suffix)
														: fmt::format("{:.2f}{}", t, suffix);
		auto             textWidth  = ImGui::CalcTextSize(str.c_str(), str.c_str() + str.size()).x;
		ImVec2           textPos    = { timeBB.Min.x + Clamp(x - textWidth * 0.5f, 0.0f, timeBB.GetWidth() - textWidth), timeBB.Min.y };

		ImVec2 p  = { barsBB.Min.x + x, barsBB.Min.y };
		ImVec2 p2 = { p.x, p.y + options->LargeBarHeight };
		window->DrawList->AddLine(p, p2, options->LargeBarColor);
		window->DrawList->AddText(textPos, options->TimeColor, str.c_str(), str.c_str() + str.size());
	}

	void DrawTimescale(TimelineOptions* options)
	{
		if (options->Scale == 0.0)
			return;

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext* ctx   = ImGui::GetCurrentContext();
		ImGuiStyle*   style = &ctx->Style;

		float width = window->Size.x - style->WindowPadding.x * 2;
		if (ImGui::GetScrollMaxY() != 0)
			width -= style->ScrollbarSize;

		ImRect barsBB;
		ImRect timeBB;
		ImRect totalBB;
		barsBB.Min  = window->DC.CursorPos;
		barsBB.Max  = { barsBB.Min.x + width, barsBB.Min.y + std::max(options->LargeBarHeight, options->SmallBarHeight) };
		timeBB.Min  = { barsBB.Min.x, barsBB.Max.y + style->FramePadding.y };
		timeBB.Max  = { barsBB.Max.x, timeBB.Min.y + ImGui::GetTextLineHeight() };
		totalBB.Min = barsBB.Min;
		totalBB.Max = timeBB.Max;
		ImGui::ItemAdd(totalBB, 0);
		ImGui::ItemSize(totalBB);

		// TODO(MarcasRealAccount): Make the deltaX range between 120px to 250px
		// TODO(MarcasRealAccount): Handle negative offsets
		// TODO(MarcasRealAccount): Fix nans when offset=0
		// TODO(MarcasRealAccount): Fix startTimeBoundary resetting to 0 when the scale is not in 100s (When deltaScale is -8 and offset goes across 10ns, the drawn text shows 0ns)

		double spp = options->InvSampleRate * options->Scale;
		double pps = options->SampleRate * options->InvScale;

		double       offsetTime       = options->Offset * options->InvSampleRate;
		double       deltaTime        = 120.0 * spp;
		std::int64_t offsetScalel     = offsetTime == 0.0 ? (1LL << 63LL) : static_cast<std::int64_t>(Ceil(std::log10(offsetTime) - 1));
		std::int64_t baseOffsetScalel = offsetTime == 0.0 ? (1LL << 63LL) : (offsetScalel < 0 ? (offsetScalel - 2) : offsetScalel) / 3 * 3;
		std::int64_t deltaScalel      = static_cast<std::int64_t>(Ceil(std::log10(deltaTime)));
		std::int64_t baseDeltaScalel  = (deltaScalel < 0 ? (deltaScalel - 2) : deltaScalel) / 3 * 3;
		double       baseOffsetScale  = std::pow(10.0, baseOffsetScalel);
		double       deltaScale       = std::pow(10.0, deltaScalel);
		if (deltaScale == 0.0)
			return;
		double           baseDeltaScale    = std::pow(10.0, baseDeltaScalel);
		double           upperDeltaScale   = deltaScale * 10.0;
		double           startTime         = Frac(offsetTime / upperDeltaScale) * upperDeltaScale;
		double           baseTime          = offsetTime - startTime;
		double           startTimeBoundary = CeilToBoundary(startTime, deltaScale);
		double           startX            = (startTimeBoundary - startTime) * pps;
		double           deltaX            = deltaScale * pps;
		std::uint64_t    largeBars         = static_cast<std::uint64_t>(Ceil(width / deltaX));
		std::string_view baseSuffix        = GetTimeSuffix(baseOffsetScalel);
		std::string_view suffix            = GetTimeSuffix(baseDeltaScalel);
		for (std::uint64_t i = 0; i < largeBars; ++i)
		{
			double x = startX + deltaX * i;
			if (x > width)
				break;
			double       t        = startTimeBoundary + deltaScale * i;
			double       bts      = baseTime / baseOffsetScale;
			double       ts       = t / baseDeltaScale;
			std::int32_t btsScale = baseOffsetScalel;
			std::int32_t tsScale  = baseDeltaScalel;
			if (ts > 900.0)
			{
				bts += 1000.0 * std::pow(10.0, tsScale - btsScale);
				if (bts > 999.9)
				{
					bts      *= 0.001;
					btsScale += 3;
				}
				ts -= 1000.0;
				ts = std::abs(ts);
			}
			DrawLargeBar(options, window, barsBB, timeBB, x, bts, btsScale, ts, tsScale);
		}
	}

	void DrawTimeline(TimelineOptions* options, std::size_t numEntries, TimelineEntry* entries)
	{
		if (options->Scale == 0.0)
			return;

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext* ctx   = ImGui::GetCurrentContext();
		ImGuiStyle*   style = &ctx->Style;

		float width = window->Size.x - style->WindowPadding.x * 2;
		if (ImGui::GetScrollMaxY() != 0)
			width -= style->ScrollbarSize;

		ImRect contentBB;
		contentBB.Min = window->DC.CursorPos;
		contentBB.Max = { contentBB.Min.x + width, contentBB.Min.y + ImGui::GetTextLineHeight() + style->ItemInnerSpacing.y * 2 };
		ImRect totalBB;
		totalBB.Min = contentBB.Min;
		totalBB.Max = contentBB.Max;

		if (options->Borders)
		{
			contentBB.Min.y += 3.0f;
			contentBB.Max.y += 3.0f;
			totalBB.Max.y   += 6.0f;
		}

		ImGui::ItemAdd(totalBB, 0);
		ImGui::ItemSize(totalBB);

		if (options->Borders)
		{
			window->DrawList->AddLine(totalBB.Min, { totalBB.Max.x, totalBB.Min.y }, options->BorderColor, style->FrameBorderSize);
			window->DrawList->AddLine({ totalBB.Min.x, totalBB.Max.y }, totalBB.Max, options->BorderColor, style->FrameBorderSize);
		}

		auto mp = ImGui::GetMousePos();

		double endOffset = options->Offset + width * options->Scale;
		for (std::size_t i = 0; i < numEntries; ++i)
		{
			TimelineEntry& entry = entries[i];
			if (entry.End < options->Offset)
				continue;
			if (entry.Begin > endOffset)
				continue;

			double bx = Clamp((entry.Begin - options->Offset) * options->InvScale, 0.0, width);
			double ex = Clamp((entry.End - options->Offset) * options->InvScale, 0.0, width);
			double dx = ex - bx;
			if (dx < 5.0)
				continue;

			std::uint8_t alpha = 255;
			if (dx < 10.0)
				alpha = static_cast<std::uint8_t>((dx - 5.0) / 5.0 * 255.0);
			std::uint32_t color = entry.Color & 0xFF'FF'FF | alpha << 24;

			ImVec2 entryMin { static_cast<float>(contentBB.Min.x + bx), contentBB.Min.y };
			ImVec2 entryMax { static_cast<float>(contentBB.Min.x + ex), contentBB.Max.y };

			window->DrawList->AddRectFilled(entryMin, entryMax, color);
			if (mp.x >= entryMin.x && mp.y >= entryMin.y && mp.x <= entryMax.x && mp.y <= entryMax.y)
				window->DrawList->AddRect(entryMin, entryMax, 0xFF'FF'FF'FF);

			double textWidth = ImGui::CalcTextSize(entry.Text).x;
			double innerSize = ex - bx - style->ItemInnerSpacing.x * 2;
			if (textWidth < innerSize)
			{
				double   luminance = 0.2126 / 255 * ((entry.Color >> 16) & 0xFF) + 0.7152 / 255 * ((entry.Color >> 8) & 0xFF) + 0.0722 / 255 * (entry.Color & 0xFF);
				uint32_t textColor = luminance > 0.5 ? options->DarkEntryTextColor : options->LightEntryTextColor;
				window->DrawList->AddText({ static_cast<float>(contentBB.Min.x + bx + (ex - bx - textWidth) * 0.5f),
											static_cast<float>(contentBB.Min.y + style->ItemInnerSpacing.y) },
										  textColor,
										  entry.Text);
			}
		}
	}
} // namespace UI