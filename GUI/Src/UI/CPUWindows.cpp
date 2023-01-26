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
	static constexpr float PI_HALF = 3.1415926535897932384f / 2.0f;

	void SmoothFloat::tick(float deltaTime)
	{
		Time    = std::min(Time + deltaTime, 1.0f);
		Current = std::lerp(Origin, Target, std::sinf(Time * PI_HALF));
		if (Time == 1.0f)
			Origin = Target;
	}

	struct TimelineEntry
	{
		TimelineEntry(float begin, float end)
			: Begin(begin),
			  End(end) {}

		float Begin;
		float End;
	};

	using Timeline = std::vector<TimelineEntry>;

	float TimelineTimeToPosition(std::uint64_t time, float offset, float invZoom)
	{
		return static_cast<float>(time * invZoom * 0.01f) - offset;
	}

	std::uint64_t TimelinePositionToTime(float pos, float offset, float zoom)
	{
		return static_cast<std::uint64_t>((pos + offset) * zoom * 100.0f);
	}

	void DrawTimeline(ImGuiWindow* window, const Timeline& timeline, float height, CPUCoresData* data, bool topBorder, bool bottomBorder, float offset)
	{
		if (window->SkipItems)
			return;

		auto  context = ImGui::GetCurrentContext();
		auto& style   = context->Style;

		float width = window->Size.x - style.WindowPadding.x * 2;
		if (ImGui::GetScrollMaxY() != 0)
			width -= style.ScrollbarSize;
		ImRect contentBB;
		contentBB.Min.x = window->DC.CursorPos.x;
		contentBB.Min.y = window->DC.CursorPos.y;
		contentBB.Max.x = contentBB.Min.x + width;
		contentBB.Max.y = contentBB.Min.y + height;
		ImRect totalBB  = contentBB;
		if (topBorder)
		{
			contentBB.Min.y += 2.0f;
			contentBB.Max.y += 2.0f;
			totalBB.Max.y   += 2.0f;
		}
		if (bottomBorder)
			totalBB.Max.y += 1.0f;

		ImGui::ItemAdd(totalBB, 0);
		ImGui::ItemSize(totalBB);

		if (topBorder)
			window->DrawList->AddLine(totalBB.Min, { totalBB.Max.x, totalBB.Min.y }, ImGui::GetColorU32(ImGuiCol_Border), 1.0f);
		if (bottomBorder)
			window->DrawList->AddLine({ totalBB.Min.x, totalBB.Max.y }, totalBB.Max, ImGui::GetColorU32(ImGuiCol_Border), 1.0f);

		float mouseX = ImGui::GetMousePos().x - contentBB.Min.x;

		float minTime = data->Offset + offset;
		float maxTime = data->Offset + offset + width * data->InvZoom;
		for (std::size_t i = 0; i < timeline.size(); ++i)
		{
			auto& entry = timeline[i];

			if (entry.End < minTime || entry.Begin > maxTime)
				continue;

			float begin = entry.Begin < minTime ? 0.0f : (entry.Begin - data->Offset - offset) * data->Zoom;
			float end   = entry.End > maxTime ? width : (entry.End - data->Offset - offset) * data->Zoom;

			ImVec2 min { contentBB.Min.x + begin, contentBB.Min.y };
			ImVec2 max { contentBB.Min.x + end, contentBB.Max.y };
			window->DrawList->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_PlotLines));
		}

		auto str = fmt::format("MiT: {:< 5.2e}, MaT: {:< 5.2e}, MX: {:< 5.2e}, WX: {:< 5.2e}, Z: {:< 5.2e}, O: {:< 5.2e}, Ov: {:< 5.2e}", minTime, maxTime, mouseX, data->Offset + offset + mouseX * data->InvZoom, data->Zoom, data->Offset + offset, data->OffsetVel);
		window->DrawList->AddText(contentBB.Min, ImGui::GetColorU32(ImGuiCol_Text), str.c_str());
	}
	}

	float GetMouseDragDeltaX(ImGuiMouseButton button, bool* down, float lock_threshold = -1.0f)
	{
		ImGuiContext& g = *GImGui;
		IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(g.IO.MouseDown));
		if (lock_threshold < 0.0f)
			lock_threshold = g.IO.MouseDragThreshold;
		if (down)
			*down = g.IO.MouseDown[button];
		if (g.IO.MouseDown[button] || g.IO.MouseReleased[button])
			if (g.IO.MouseDragMaxDistanceSqr[button] >= lock_threshold * lock_threshold)
				if (ImGui::IsMousePosValid(&g.IO.MousePos) && ImGui::IsMousePosValid(&g.IO.MouseClickedPos[button]))
					return g.IO.MousePos.x - g.IO.MouseClickedPos[button].x;
		return 0.0f;
	}

	void ShowCPUCores(bool* p_open, CPUCoresData* data, float deltaTime)
	{
		ImGui::Begin("CPU Cores##CPUCores", p_open);

		auto  context = ImGui::GetCurrentContext();
		auto& style   = context->Style;

		float        height = ImGui::GetTextLineHeight() + style.FramePadding.y * 2;
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		float deltaOff = data->OffsetVel * deltaTime;
		auto  mousePos = ImGui::GetMousePos() - (window->Pos + style.WindowPadding);
		if (mousePos.x >= 0.0f && mousePos.x < window->Size.x - style.WindowPadding.x * 2.0f &&
			mousePos.y >= 0.0f && mousePos.y < window->Size.y - style.WindowPadding.y * 2.0f &&
			ImGui::IsWindowHovered())
		{
			float wheelY = context->IO.MouseWheel;
			if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && wheelY != 0.0f)
			{
				float curWX = mousePos.x * data->InvZoom;
				/*data->ZoomLevel.Origin = data->ZoomLevel.Current;
				data->ZoomLevel.Target = std::floorf(data->ZoomLevel.Target) + wheelY;
				data->ZoomLevel.reset();*/
				data->ZoomLevel = std::floorf(data->ZoomLevel) + wheelY;
				float nextZoom  = 20.0f * std::powf(1.2f, data->ZoomLevel /*.Target*/);
				float nextWX    = mousePos.x / nextZoom;
				data->Offset    += curWX - nextWX;
				/*data->Offset.Origin = data->Offset.Current;
				data->Offset.Target += curWX - nextWX;
				data->Offset.reset();*/
			}

			bool  down = false;
			float drag = GetMouseDragDeltaX(ImGuiMouseButton_Right, &down);
			if (down)
			{
				if (data->PreviousDrag == 0)
					data->PreviousDrag = drag;
				float deltaDrag       = data->PreviousDrag - drag;
				float dragVel         = deltaDrag * data->InvZoom;
				data->PreviousDrag    = drag;
				data->PreviousDragVel = dragVel;
				data->OffsetVel       = 0;
				deltaOff              += dragVel;
			}
			else if (data->PreviousDrag != 0)
			{
				data->OffsetVel       = data->PreviousDragVel / deltaTime;
				data->PreviousDrag    = 0;
				data->PreviousDragVel = 0;
			}
		}
		else if (data->PreviousDrag != 0)
		{
			data->PreviousDrag = 0;
		}

		// data->ZoomLevel.tick(2.5f * deltaTime);
		data->Zoom    = 20.0f * std::powf(1.2f, data->ZoomLevel);
		data->InvZoom = 1.0f / data->Zoom;
		data->Offset  += deltaOff;
		// data->Offset.Origin += deltaOff;
		//  data->Offset.Target += deltaOff;
		//  data->Offset.tick(2.5f * deltaTime);
		data->OffsetVel = std::lerp(data->OffsetVel, 0, 6.5f * deltaTime);
		if (data->OffsetVel > -1e-4f && data->OffsetVel < 1e-4f)
			data->OffsetVel = 0.0f;

		struct Timestamp
		{
			Timestamp(std::uint64_t begin, std::uint64_t end)
			{
				Begin.Time = begin & ~(1ULL << 63ULL);
				Begin.Type = begin >> 63ULL;
				End.Time   = end & ~(1ULL << 63ULL);
				End.Type   = end >> 63ULL;
			}

			Profiler::EventTimestamp Begin;
			Profiler::EventTimestamp End;
		};

		std::vector<Timestamp> core0Timestamps {
			{10,  15},
			{ 17, 18},
			{ 25, 36},
			{ 38, 40},
			{ 41, 50},
			{ 50, 90}
		};

		Timeline core0Timeline;
		for (auto& timestamps : core0Timestamps)
		{
			float begin = 0.0f;
			float end   = 0.0f;

			switch (timestamps.Begin.Type)
			{
			case 0:
				begin = std::chrono::duration_cast<std::chrono::duration<float, std::nano>>(std::chrono::high_resolution_clock::duration { timestamps.Begin.Time }).count();
				break;
			case 1:
				begin = timestamps.Begin.Time * 5.6e-1f;
				break;
			}

			switch (timestamps.End.Type)
			{
			case 0:
				end = std::chrono::duration_cast<std::chrono::duration<float, std::nano>>(std::chrono::high_resolution_clock::duration { timestamps.End.Time }).count();
				break;
			case 1:
				end = timestamps.End.Time * 5.6e-1f;
				break;
			}

			core0Timeline.emplace_back(TimelineEntry { begin, end });
		}

		DrawTimeline(window, core0Timeline, height, data, true, true, 0.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 1.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 2.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 3.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 4.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 5.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 6.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 7.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 8.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 9.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 10.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 11.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 12.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 13.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 14.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 15.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 16.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 17.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 18.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 19.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 20.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 21.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 22.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 23.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 24.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 25.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 26.0f);
		DrawTimeline(window, core0Timeline, height, data, true, true, 27.0f);

		ImGui::End();
	}
} // namespace UI