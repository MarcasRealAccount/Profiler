#pragma once

#include <fmt/format.h>
#include <imgui.h>

namespace ImGui
{
	template <class... Ts>
	void TextF(fmt::format_string<Ts...>&& format, Ts&&... args)
	{
		std::string str = fmt::format(format, std::forward<Ts>(args)...);
		::ImGui::TextUnformatted(str.c_str(), str.c_str() + str.size());
	}
} // namespace ImGui