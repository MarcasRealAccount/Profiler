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
	// static constexpr float PI_HALF = 3.1415926535897932384f / 2.0f;

	// void SmoothFloat::tick(float deltaTime)
	//{
	//	Time    = std::min(Time + deltaTime, 1.0f);
	//	Current = std::lerp(Origin, Target, std::sinf(Time * PI_HALF));
	//	if (Time == 1.0f)
	//		Origin = Target;
	// }

	// struct TimelineEntry
	//{
	//	TimelineEntry(float begin, float end)
	//		: Begin(begin),
	//		  End(end) {}

	//	float Begin;
	//	float End;
	//};

	// using Timeline = std::vector<TimelineEntry>;

	// float TimelineTimeToPosition(std::uint64_t time, float offset, float invZoom)
	//{
	//	return static_cast<float>(time * invZoom * 0.01f) - offset;
	// }

	// std::uint64_t TimelinePositionToTime(float pos, float offset, float zoom)
	//{
	//	return static_cast<std::uint64_t>((pos + offset) * zoom * 100.0f);
	// }

	// std::string_view GetTimeStringForExponent(std::int32_t exponent)
	//{
	//	switch (exponent)
	//	{
	//	case -9: return "ps";
	//	case -6: return "ns";
	//	case -3: return "mis";
	//	case 0: return "ms";
	//	case 3: return "s";
	//	case 6: return "ks";
	//	case 9: return "Ms";
	//	case 12: return "Gs";
	//	default: return "?";
	//	}
	// }

	// void DrawTimeline(ImGuiWindow* window, const Timeline& timeline, float height, CPUCoresData* data, bool topBorder, bool bottomBorder)
	//{
	//	if (window->SkipItems)
	//		return;

	//	auto  context = ImGui::GetCurrentContext();
	//	auto& style   = context->Style;

	//	float width = window->Size.x - style.WindowPadding.x * 2;
	//	if (ImGui::GetScrollMaxY() != 0)
	//		width -= style.ScrollbarSize;
	//	ImRect contentBB;
	//	contentBB.Min.x = window->DC.CursorPos.x;
	//	contentBB.Min.y = window->DC.CursorPos.y;
	//	contentBB.Max.x = contentBB.Min.x + width;
	//	contentBB.Max.y = contentBB.Min.y + height;
	//	ImRect totalBB  = contentBB;
	//	if (topBorder)
	//	{
	//		contentBB.Min.y += 2.0f;
	//		contentBB.Max.y += 2.0f;
	//		totalBB.Max.y   += 2.0f;
	//	}
	//	if (bottomBorder)
	//		totalBB.Max.y += 1.0f;

	//	ImGui::ItemAdd(totalBB, 0);
	//	ImGui::ItemSize(totalBB);

	//	if (topBorder)
	//		window->DrawList->AddLine(totalBB.Min, { totalBB.Max.x, totalBB.Min.y }, ImGui::GetColorU32(ImGuiCol_Border), 1.0f);
	//	if (bottomBorder)
	//		window->DrawList->AddLine({ totalBB.Min.x, totalBB.Max.y }, totalBB.Max, ImGui::GetColorU32(ImGuiCol_Border), 1.0f);

	//	float mouseX = ImGui::GetMousePos().x - contentBB.Min.x;

	//	float minTime = data->Offset;
	//	float maxTime = data->Offset + width * data->InvZoom;
	//	for (std::size_t i = 0; i < timeline.size(); ++i)
	//	{
	//		auto& entry = timeline[i];

	//		if (entry.End < minTime || entry.Begin > maxTime)
	//			continue;

	//		float begin = entry.Begin < minTime ? 0.0f : (entry.Begin - data->Offset) * data->Zoom;
	//		float end   = entry.End > maxTime ? width : (entry.End - data->Offset) * data->Zoom;

	//		ImVec2 min { contentBB.Min.x + begin, contentBB.Min.y };
	//		ImVec2 max { contentBB.Min.x + end, contentBB.Max.y };
	//		window->DrawList->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_PlotLines));
	//	}

	//	auto str = fmt::format("MiT: {:< 5.2e}, MaT: {:< 5.2e}, MX: {:< 5.2e}, WX: {:< 5.2e}, Z: {:< 5.2e}, O: {:< 5.2e}, Ov: {:< 5.2e}", minTime, maxTime, mouseX, data->Offset + mouseX * data->InvZoom, data->Zoom, data->Offset, data->OffsetVel);
	//	window->DrawList->AddText(contentBB.Min, ImGui::GetColorU32(ImGuiCol_Text), str.c_str());
	//}

	// void DrawTimescale(ImGuiWindow* window, CPUCoresData* data)
	//{
	//	if (window->SkipItems)
	//		return;

	//	auto  context = ImGui::GetCurrentContext();
	//	auto& style   = context->Style;

	//	float width = window->Size.x - style.WindowPadding.x * 2;
	//	if (ImGui::GetScrollMaxY() != 0)
	//		width -= style.ScrollbarSize;
	//	ImRect barsBB;
	//	barsBB.Min   = window->DC.CursorPos;
	//	barsBB.Max.x = barsBB.Min.x + width;
	//	barsBB.Max.y = barsBB.Min.y + ImGui::GetTextLineHeight() + style.ItemSpacing.y;
	//	ImRect timeBB;
	//	timeBB.Min = { barsBB.Min.x, barsBB.Max.y };
	//	timeBB.Max = { barsBB.Max.x, timeBB.Min.y + ImGui::GetTextLineHeight() + style.FramePadding.y };
	//	ImRect totalBB;
	//	totalBB.Min = barsBB.Min;
	//	totalBB.Max = timeBB.Max;

	//	ImGui::ItemAdd(totalBB, 0);
	//	ImGui::ItemSize(totalBB);

	//	float        minTime  = data->Offset;
	//	float        maxTime  = data->Offset + width * data->InvZoom;
	//	std::int32_t exponent = static_cast<std::int32_t>(std::log10f(100 * data->InvZoom));
	//	float        invScale = std::powf(10.0f, exponent);
	//	float        scale    = 1.0f / invScale;
	//	minTime               *= scale;
	//	maxTime               *= scale;
	//	for (float time = std::ceilf(minTime); time < std::ceilf(maxTime); time += 1.0f)
	//	{
	//		float x = (time * invScale - data->Offset) * data->Zoom;

	//		window->DrawList->AddLine({ barsBB.Min.x + x, barsBB.Min.y }, { barsBB.Min.x + x, barsBB.Max.y }, ImGui::GetColorU32(ImGuiCol_Border));
	//		std::string str  = fmt::format("{}{}", time, GetTimeStringForExponent(exponent));
	//		auto        size = ImGui::CalcTextSize(str.c_str());
	//		float       tx   = std::clamp(x - size.x * 0.5f, 0.0f, width - size.x);
	//		window->DrawList->AddText({ timeBB.Min.x + tx, timeBB.Min.y }, ImGui::GetColorU32(ImGuiCol_Text), str.c_str());
	//	}

	//	std::string dbg = fmt::format("Min: {}, Max: {}, 10^{}, Scale: {}", minTime, maxTime, exponent, scale);
	//	window->DrawList->AddText(barsBB.Min, ImGui::GetColorU32(ImGuiCol_Text), dbg.c_str());
	//}

	// float GetMouseDragDeltaX(ImGuiMouseButton button, bool* down, float lock_threshold = -1.0f)
	//{
	//	ImGuiContext& g = *GImGui;
	//	IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
	//	if (lock_threshold < 0.0f)
	//		lock_threshold = g.IO.MouseDragThreshold;
	//	if (down)
	//		*down = g.IO.MouseDown[button];
	//	if (g.IO.MouseDown[button] || g.IO.MouseReleased[button])
	//		if (g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold)
	//			if (ImGui::IsMousePosValid(&g.IO.MousePos) && ImGui::IsMousePosValid(&g.IO.MouseClickedPos[button]))
	//				return g.IO.MousePos.x - g.IO.MouseClickedPos[button].x;
	//	return 0.0f;
	// }

	// void ShowCPUCores(bool* p_open, CPUCoresData* data, float deltaTime)
	//{
	//	ImGui::Begin("CPU Cores##CPUCores", p_open);

	//	auto  context = ImGui::GetCurrentContext();
	//	auto& style   = context->Style;

	//	float        height = ImGui::GetTextLineHeight() + style.FramePadding.y * 2;
	//	ImGuiWindow* window = ImGui::GetCurrentWindow();

	//	float deltaOff = data->OffsetVel * deltaTime;
	//	auto  mousePos = ImGui::GetMousePos() - (window->Pos + style.WindowPadding);
	//	if (mousePos.x >= 0.0f && mousePos.x < window->Size.x - style.WindowPadding.x * 2.0f &&
	//		mousePos.y >= 0.0f && mousePos.y < window->Size.y - style.WindowPadding.y * 2.0f &&
	//		ImGui::IsWindowHovered())
	//	{
	//		float wheelY = context->IO.MouseWheel;
	//		if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && wheelY != 0.0f)
	//		{
	//			float curWX = mousePos.x * data->InvZoom;
	//			/*data->ZoomLevel.Origin = data->ZoomLevel.Current;
	//			data->ZoomLevel.Target = std::floorf(data->ZoomLevel.Target) + wheelY;
	//			data->ZoomLevel.reset();*/
	//			data->ZoomLevel = std::floorf(data->ZoomLevel) + wheelY;
	//			float nextZoom  = 20.0f * std::powf(1.2f, data->ZoomLevel /*.Target*/);
	//			float nextWX    = mousePos.x / nextZoom;
	//			data->Offset    += curWX - nextWX;
	//			/*data->Offset.Origin = data->Offset.Current;
	//			data->Offset.Target += curWX - nextWX;
	//			data->Offset.reset();*/
	//		}

	//		bool  down = false;
	//		float drag = GetMouseDragDeltaX(ImGuiMouseButton_Right, &down);
	//		if (down)
	//		{
	//			if (data->PreviousDrag == 0)
	//				data->PreviousDrag = drag;
	//			float deltaDrag       = data->PreviousDrag - drag;
	//			float dragVel         = deltaDrag * data->InvZoom;
	//			data->PreviousDrag    = drag;
	//			data->PreviousDragVel = dragVel;
	//			data->OffsetVel       = 0;
	//			deltaOff              += dragVel;
	//		}
	//		else if (data->PreviousDrag != 0)
	//		{
	//			data->OffsetVel       = data->PreviousDragVel / deltaTime;
	//			data->PreviousDrag    = 0;
	//			data->PreviousDragVel = 0;
	//		}
	//	}
	//	else if (data->PreviousDrag != 0)
	//	{
	//		data->PreviousDrag = 0;
	//	}

	//	// data->ZoomLevel.tick(2.5f * deltaTime);
	//	data->Zoom    = 20.0f * std::powf(1.2f, data->ZoomLevel);
	//	data->InvZoom = 1.0f / data->Zoom;
	//	data->Offset  += deltaOff;
	//	// data->Offset.Origin += deltaOff;
	//	//  data->Offset.Target += deltaOff;
	//	//  data->Offset.tick(2.5f * deltaTime);
	//	data->OffsetVel = std::lerp(data->OffsetVel * data->Zoom, 0, 6.5f * deltaTime) * data->InvZoom;
	//	if (data->OffsetVel > -1e-4f && data->OffsetVel < 1e-4f)
	//		data->OffsetVel = 0.0f;

	//	struct Timestamp
	//	{
	//		Timestamp(std::uint64_t begin, std::uint64_t end)
	//		{
	//			Begin.Time = begin & ~(1ULL << 63ULL);
	//			Begin.Type = begin >> 63ULL;
	//			End.Time   = end & ~(1ULL << 63ULL);
	//			End.Type   = end >> 63ULL;
	//		}

	//		Profiler::EventTimestamp Begin;
	//		Profiler::EventTimestamp End;
	//	};

	//	std::vector<Timestamp> core0Timestamps {
	//		{10,  15},
	//		{ 17, 18},
	//		{ 25, 36},
	//		{ 38, 40},
	//		{ 41, 50},
	//		{ 50, 90}
	//	};

	//	Timeline core0Timeline;
	//	for (auto& timestamps : core0Timestamps)
	//	{
	//		float begin = 0.0f;
	//		float end   = 0.0f;

	//		switch (timestamps.Begin.Type)
	//		{
	//		case 0:
	//			begin = std::chrono::duration_cast<std::chrono::duration<float, std::nano>>(std::chrono::high_resolution_clock::duration { timestamps.Begin.Time }).count();
	//			break;
	//		case 1:
	//			begin = timestamps.Begin.Time * 5.6e-1f;
	//			break;
	//		}

	//		switch (timestamps.End.Type)
	//		{
	//		case 0:
	//			end = std::chrono::duration_cast<std::chrono::duration<float, std::nano>>(std::chrono::high_resolution_clock::duration { timestamps.End.Time }).count();
	//			break;
	//		case 1:
	//			end = timestamps.End.Time * 5.6e-1f;
	//			break;
	//		}

	//		core0Timeline.emplace_back(TimelineEntry { begin, end });
	//	}

	//	DrawTimescale(window, data);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);
	//	DrawTimeline(window, core0Timeline, height, data, true, true);

	//	ImGui::End();
	//}

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

		double spp = options->InvSampleRate * options->Scale;
		double pps = options->SampleRate * options->InvScale;

		double           offsetTime        = options->Offset * options->InvSampleRate;
		double           deltaTime         = 120.0 * spp;
		std::int64_t     offsetScalel      = offsetTime == 0.0 ? (1LL << 63LL) : static_cast<std::int64_t>(Ceil(std::log10(offsetTime) - 1));
		std::int64_t     baseOffsetScalel  = offsetTime == 0.0 ? (1LL << 63LL) : (offsetScalel < 0 ? (offsetScalel - 2) : offsetScalel) / 3 * 3;
		std::int64_t     deltaScalel       = static_cast<std::int64_t>(Ceil(std::log10(deltaTime)));
		std::int64_t     baseDeltaScalel   = (deltaScalel < 0 ? (deltaScalel - 2) : deltaScalel) / 3 * 3;
		double           baseOffsetScale   = std::pow(10.0, baseOffsetScalel);
		double           deltaScale        = std::pow(10.0, deltaScalel);
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
	}
} // namespace UI