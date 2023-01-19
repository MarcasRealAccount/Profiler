#include "CPUWindows.h"

#include <Profiler/State.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <vector>

namespace UI
{
	struct TimelineEntry
	{
		std::uint64_t begin;
		std::uint64_t end;
	};

	using Timeline = std::vector<TimelineEntry>;

	float TimelineTimeToPosition(std::uint64_t time, double invZoom)
	{
		return static_cast<float>(time * 100ULL * invZoom);
	}

	void DrawTimeline(ImGuiWindow* window, const Timeline& timeline, float height, std::uint64_t zoom, std::uint64_t offset, bool topBorder, bool bottomBorder)
	{
		if (window->SkipItems)
			return;

		auto  context = ImGui::GetCurrentContext();
		auto& style   = context->Style;

		float  width = window->Size.x - style.WindowPadding.x * 2;
		ImRect contentBB;
		contentBB.Min.x = window->DC.CursorPos.x;
		contentBB.Min.y = window->DC.CursorPos.y;
		contentBB.Max.x = window->DC.CursorPos.x + width;
		contentBB.Max.y = window->DC.CursorPos.y + height;
		ImRect totalBB  = contentBB;
		if (topBorder)
		{
			contentBB.Min.y += 2.0f;
			contentBB.Max.y += 2.0f;
			totalBB.Max.y   += 2.0f;
		}
		if (bottomBorder)
			totalBB.Max.y += 1.0f;

		if (!ImGui::ItemAdd(totalBB, 0))
			return;

		if (topBorder)
			window->DrawList->AddLine(totalBB.Min, { totalBB.Max.x, totalBB.Min.y }, ImGui::GetColorU32(ImGuiCol_Border), 1.0f);
		if (bottomBorder)
			window->DrawList->AddLine({ totalBB.Min.x, totalBB.Max.y }, totalBB.Max, ImGui::GetColorU32(ImGuiCol_Border), 1.0f);

		std::uint64_t maxTime = static_cast<std::uint64_t>(((width * 0.01) + 1) * zoom);
		double        invZoom = 1.0 / zoom;
		for (std::size_t i = 0; i < timeline.size(); ++i)
		{
			auto& entry = timeline[i];

			if (entry.end < offset || entry.begin > maxTime)
				continue;

			std::uint64_t begin = entry.begin < offset ? 0 : entry.begin - offset;
			std::uint64_t end   = entry.end > maxTime ? maxTime : entry.end - offset;

			float minX = TimelineTimeToPosition(begin, invZoom);
			float maxX = TimelineTimeToPosition(end, invZoom);
			if (maxX - minX < 5)
				continue;

			ImVec2 min { contentBB.Min.x + minX, contentBB.Min.y };
			ImVec2 max { contentBB.Min.x + maxX, contentBB.Max.y };
			window->DrawList->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_PlotLines));
		}
	}

	void ShowCPUCores(bool* p_open, std::uint64_t* p_zoom, std::uint64_t* p_offset)
	{
		ImGui::Begin("CPU Cores##CPUCores", p_open);

		auto  context = ImGui::GetCurrentContext();
		auto& style   = context->Style;

		float        height = ImGui::GetTextLineHeight() + style.FramePadding.y * 2;
		ImGuiWindow* window = ImGui::GetCurrentWindow();

		// TODO(MarcasRealAccount): Do nicer zooming and dragging
		*p_zoom += static_cast<std::uint64_t>(context->IO.MouseWheel);

		auto drag = ImGui::GetMouseDragDelta(0);
		*p_offset += static_cast<std::uint64_t>(drag.x);

		Timeline core0Timeline {
			{10,  15},
			{ 17, 18},
			{ 25, 36},
			{ 38, 40},
			{ 41, 50},
			{ 50, 90}
		};
		DrawTimeline(window, core0Timeline, height, *p_zoom, *p_offset, false, true);

		ImGui::End();
	}
} // namespace UI