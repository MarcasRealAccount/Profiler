#include "RuntimeView.h"
#include "ImGuiHelper.h"

#include <Profiler/Profiler.h>

#include <imgui.h>

namespace UI
{
	void UpdateRuntimeView(RuntimeViewState* state)
	{
		bool changed         = false;
		auto coreCount       = Profiler::GetTotalCoreCount();
		auto ioEndpointCount = Profiler::GetIOEndpointCount(&changed);

		state->Abilities = Profiler::GetRuntimeAbilities();
		state->TotalCoreUsages.resize(state->Abilities.hasFlag(Profiler::RuntimeAbilities::IndividualCoreUsages) ? coreCount : (state->Abilities.hasFlag(Profiler::RuntimeAbilities::CoreUsage) ? 1 : 0));
		state->ProcessCoreUsages.resize(state->Abilities.hasFlag(Profiler::RuntimeAbilities::PerProcessIndividualCoreUsages) ? coreCount : (state->Abilities.hasFlag(Profiler::RuntimeAbilities::PerProcessCoreUsage) ? 1 : 0));
		state->TotalIOUsages.resize(state->Abilities.hasFlag(Profiler::RuntimeAbilities::IndividualIOUsages) ? ioEndpointCount : (state->Abilities.hasFlag(Profiler::RuntimeAbilities::IOUsage) ? 1 : 0));
		state->ProcessIOUsages.resize(state->Abilities.hasFlag(Profiler::RuntimeAbilities::PerProcessIndividualIOUsages) ? ioEndpointCount : (state->Abilities.hasFlag(Profiler::RuntimeAbilities::PerProcessIOUsage) ? 1 : 0));

		if (changed)
		{
			state->IOEndpointInfos.resize(ioEndpointCount);
			Profiler::GetIOEndpointInfos(ioEndpointCount, state->IOEndpointInfos.data());
		}

		Profiler::GetCoreUsages(Profiler::SystemProcess(), state->TotalCoreUsages.size(), state->TotalCoreUsages.data());
		Profiler::GetCoreUsages(state->Process, state->ProcessCoreUsages.size(), state->ProcessCoreUsages.data());
		Profiler::GetMemoryUsage(Profiler::SystemProcess(), state->TotalMemoryUsages);
		Profiler::GetMemoryUsage(state->Process, state->ProcessMemoryUsages);
		Profiler::GetIOUsages(Profiler::SystemProcess(), state->TotalIOUsages.size(), state->TotalIOUsages.data());
		Profiler::GetIOUsages(state->Process, state->ProcessIOUsages.size(), state->ProcessIOUsages.data());
	}

	template <std::integral T>
	std::size_t RequiredDigits(T value)
	{
		std::size_t count = 0;
		if constexpr (std::is_signed_v<T>)
		{
			count = value < 0 ? 1 : 0;
			value = -value;
		}
		while (value < 10)
		{
			++count;
			value /= 10;
		}
		return count;
	}

	double GetScaledBytes(std::uint64_t bytes, std::string& outSuffix)
	{
		if (bytes < 1'000ULL)
		{
			outSuffix = "B";
			return static_cast<double>(bytes);
		}
		else if (bytes < 1'000'000ULL)
		{
			outSuffix = "kB";
			return static_cast<double>(bytes) * 1e-3;
		}
		else if (bytes < 1'000'000'000ULL)
		{
			outSuffix = "MB";
			return static_cast<double>(bytes) * 1e-6;
		}
		else if (bytes < 1'000'000'000'000ULL)
		{
			outSuffix = "GB";
			return static_cast<double>(bytes) * 1e-9;
		}
		else if (bytes < 1'000'000'000'000'000ULL)
		{
			outSuffix = "TB";
			return static_cast<double>(bytes) * 1e-12;
		}
		else if (bytes < 1'000'000'000'000'000'000ULL)
		{
			outSuffix = "PB";
			return static_cast<double>(bytes) * 1e-15;
		}
		else
		{
			outSuffix = "EB";
			return static_cast<double>(bytes) * 1e-18;
		}
	}

	void CoreUsages(RuntimeViewState* state)
	{
		ImGui::TextUnformatted("Process / Total");

		double totalUsage        = 0.0;
		double processTotalUsage = 0.0;

		std::uint8_t selector = (!state->ProcessCoreUsages.empty()) << 1 | !state->TotalCoreUsages.empty();

		if (selector & 0b01)
		{
			if (state->TotalCoreUsages.size() > 1)
			{
				for (std::size_t i = 0; i < state->TotalCoreUsages.size(); ++i)
				{
					auto& usage = state->TotalCoreUsages[i];
					totalUsage  += usage.Usage;
				}
				totalUsage /= state->TotalCoreUsages.size();
			}
			else
			{
				totalUsage = state->TotalCoreUsages[0].Usage;
			}
		}

		if (selector & 0b10)
		{
			if (state->ProcessCoreUsages.size() > 1)
			{
				for (std::size_t i = 0; i < state->ProcessCoreUsages.size(); ++i)
				{
					auto& usage       = state->ProcessCoreUsages[i];
					processTotalUsage += usage.Usage;
				}
				processTotalUsage /= state->ProcessCoreUsages.size();
			}
			else
			{
				processTotalUsage = state->ProcessCoreUsages[0].Usage;
			}
		}

		switch (selector)
		{
		case 0b00: return; // Dafuq are you doing in here???
		case 0b01:
			ImGui::TextF("Total (%): ---.-- / {:6.2}", totalUsage * 100.0);
			break;
		case 0b10:
			ImGui::TextF("Total (%): {:6.2} / ---.--", processTotalUsage * 100.0);
			break;
		case 0b11:
			ImGui::TextF("Total (%): {:6.2} / {:6.2}", processTotalUsage * 100.0, totalUsage * 100.0);
			break;
		}

		std::size_t numCores = std::max(state->TotalCoreUsages.size(), state->ProcessCoreUsages.size());
		if (numCores <= 1)
			return; // No individual core information

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode("Individual Cores##IndividualCores"))
		{
			std::size_t maxCoreWidth = RequiredDigits(numCores);
			for (std::size_t i = 0; i < numCores; ++i)
			{
				std::uint8_t selector2 = ((i < state->ProcessCoreUsages.size()) << 1 | (i < state->TotalCoreUsages.size())) & selector;
				switch (selector2)
				{
				case 0b00: break; // Dafuq happened here???
				case 0b01:
					ImGui::TextF("{:{}} (%): ---.-- / {:6.2}", i, maxCoreWidth, state->TotalCoreUsages[i].Usage);
					break;
				case 0b10:
					ImGui::TextF("{:{}} (%): {:6.2} / ---.--", i, maxCoreWidth, state->ProcessCoreUsages[i].Usage);
					break;
				case 0b11:
					ImGui::TextF("{:{}} (%): {:6.2} / {:6.2}", i, maxCoreWidth, state->ProcessCoreUsages[i].Usage, state->TotalCoreUsages[i].Usage);
					break;
				}
			}
			ImGui::TreePop();
		}
	}

	void MemoryUsages(RuntimeViewState* state)
	{
		if (state->Abilities.hasFlag(Profiler::RuntimeAbilities::MemoryUsage))
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("Total##Total"))
			{
				if (state->Abilities.hasFlag(Profiler::RuntimeAbilities::IndividualMemoryUsages))
				{
					std::string physicalUsageSuffix = "";
					std::string physicalTotalSuffix = "";
					std::string virtualUsageSuffix  = "";
					double      physicalUsage       = GetScaledBytes(state->TotalMemoryUsages.CurrentWorkingSet, physicalUsageSuffix);
					double      physicalTotal       = GetScaledBytes(state->TotalMemoryUsages.PeakWorkingSet, physicalTotalSuffix);
					double      virtualUsage        = GetScaledBytes(state->TotalMemoryUsages.Private, virtualUsageSuffix);
					ImGui::TextF("Physical:    {:6.2} {:2} / {:6.2} {:2} ({:6.2}%)",
								 physicalUsage,
								 physicalUsageSuffix.c_str(),
								 physicalTotal,
								 physicalTotalSuffix.c_str(),
								 static_cast<double>(state->TotalMemoryUsages.CurrentWorkingSet) / static_cast<double>(state->TotalMemoryUsages.PeakWorkingSet) * 100.0);
					ImGui::TextF("Virtual:     {:6.2} {:2}", virtualUsage, virtualUsageSuffix.c_str());
				}
				else
				{
					std::string usageSuffix = "";
					std::string totalSuffix = "";
					double      usage       = GetScaledBytes(state->TotalMemoryUsages.Private, usageSuffix);
					double      total       = GetScaledBytes(state->TotalMemoryUsages.PeakPrivate, totalSuffix);
					ImGui::TextF("Usage:       {:6.2} {:2} / {:6.2} {:2} ({:6.2}%)",
								 usage,
								 usageSuffix.c_str(),
								 total,
								 totalSuffix.c_str(),
								 static_cast<double>(state->TotalMemoryUsages.CurrentWorkingSet) / static_cast<double>(state->TotalMemoryUsages.PeakWorkingSet) * 100.0);
				}

				if (state->Abilities.hasFlag(Profiler::RuntimeAbilities::MemoryPageFaults))
					ImGui::TextF("Page Faults: {}", state->TotalMemoryUsages.PageFaultCount);

				ImGui::TreePop();
			}
		}

		if (state->Abilities.hasFlag(Profiler::RuntimeAbilities::PerProcessMemoryUsage))
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("Process##Process"))
			{
				if (state->Abilities.hasFlag(Profiler::RuntimeAbilities::PerProcessIndividualMemoryUsages))
				{
					std::string physicalUsageSuffix = "";
					std::string virtualUsageSuffix  = "";
					std::string pagedUsageSuffix    = "";
					std::string nonPagedUsageSuffix = "";
					double      physicalUsage       = GetScaledBytes(state->ProcessMemoryUsages.CurrentWorkingSet, physicalUsageSuffix);
					double      virtualUsage        = GetScaledBytes(state->ProcessMemoryUsages.Private, virtualUsageSuffix);
					double      pagedUsage          = GetScaledBytes(state->ProcessMemoryUsages.CurrentPagedPool, pagedUsageSuffix);
					double      nonPagedUsage       = GetScaledBytes(state->ProcessMemoryUsages.CurrentNonPagedPool, nonPagedUsageSuffix);
					ImGui::TextF("Physical:    {:6.2} {:2}", physicalUsage, physicalUsageSuffix.c_str());
					ImGui::TextF("Virtual:     {:6.2} {:2}", virtualUsage, virtualUsageSuffix.c_str());
					ImGui::TextF("Paged:       {:6.2} {:2}", pagedUsage, pagedUsageSuffix.c_str());
					ImGui::TextF("Non Paged:   {:6.2} {:2}", nonPagedUsage, nonPagedUsageSuffix.c_str());
				}
				else
				{
					std::string usageSuffix = "";
					double      usage       = GetScaledBytes(state->ProcessMemoryUsages.Private, usageSuffix);
					ImGui::TextF("Usage:       {:6.2} {:2}", usage, usageSuffix.c_str());
				}

				if (state->Abilities.hasFlag(Profiler::RuntimeAbilities::PerProcessMemoryPageFaults))
					ImGui::TextF("Page Faults: {}", state->ProcessMemoryUsages.PageFaultCount);

				ImGui::TreePop();
			}
		}
	}

	void IOUsages(RuntimeViewState* state)
	{
		ImGui::Text("Process / Total");

		std::string totalReadSuffix;
		std::string totalWrittenSuffix;
		std::string processTotalReadSuffix;
		std::string processTotalWrittenSuffix;
		double      totalRead           = 0.0;
		double      totalWritten        = 0.0;
		double      totalUsage          = 0.0;
		double      processTotalRead    = 0.0;
		double      processTotalWritten = 0.0;
		double      processTotalUsage   = 0.0;

		std::uint8_t selector = (!state->ProcessIOUsages.empty()) << 1 | !state->TotalIOUsages.empty();

		if (selector & 0b01)
		{
			if (state->TotalIOUsages.size() > 1)
			{
				std::uint64_t read    = 0;
				std::uint64_t written = 0;
				for (std::size_t i = 0; i < state->TotalIOUsages.size(); ++i)
				{
					auto& usage = state->TotalIOUsages[i];
					read        += usage.BytesRead;
					written     += usage.BytesWritten;
					totalUsage  += usage.Usage;
				}
				totalUsage   /= state->TotalIOUsages.size();
				totalRead    = GetScaledBytes(read, totalReadSuffix);
				totalWritten = GetScaledBytes(written, totalWrittenSuffix);
			}
			else
			{
				auto& usage  = state->TotalIOUsages[0];
				totalRead    = GetScaledBytes(usage.BytesRead, totalReadSuffix);
				totalWritten = GetScaledBytes(usage.BytesWritten, totalWrittenSuffix);
				totalUsage   = usage.Usage;
			}
		}

		if (selector & 0b10)
		{
			if (state->ProcessIOUsages.size() > 1)
			{
				std::uint64_t read    = 0;
				std::uint64_t written = 0;
				for (std::size_t i = 0; i < state->ProcessIOUsages.size(); ++i)
				{
					auto& usage       = state->ProcessIOUsages[i];
					read              += usage.BytesRead;
					written           += usage.BytesWritten;
					processTotalUsage += usage.Usage;
				}
				processTotalUsage   /= state->ProcessIOUsages.size();
				processTotalRead    = GetScaledBytes(read, processTotalReadSuffix);
				processTotalWritten = GetScaledBytes(written, processTotalWrittenSuffix);
			}
			else
			{
				auto& usage         = state->ProcessIOUsages[0];
				processTotalRead    = GetScaledBytes(usage.BytesRead, processTotalReadSuffix);
				processTotalWritten = GetScaledBytes(usage.BytesWritten, processTotalWrittenSuffix);
				processTotalUsage   = usage.Usage;
			}
		}

		totalReadSuffix           += "/s";
		totalWrittenSuffix        += "/s";
		processTotalReadSuffix    += "/s";
		processTotalWrittenSuffix += "/s";

		switch (selector)
		{
		case 0b00: return; // Dafuq are you doing in here???
		case 0b01:
			ImGui::TextF("Total (I/O %): (---.-- B/s , ---.-- B/s ) ---.--% / ({:6.2} {:4}, {:6.2} {:4}) {:6.2}%",
						 totalRead,
						 totalReadSuffix.c_str(),
						 totalWritten,
						 totalWrittenSuffix.c_str(),
						 totalUsage * 100.0);
			break;
		case 0b10:
			ImGui::TextF("Total (I/O %): ({:6.2} {:4}, {:6.2} {:4}) {:6.2}% / (---.-- B/s , ---.-- B/s ) ---.--%",
						 processTotalRead,
						 processTotalReadSuffix.c_str(),
						 processTotalWritten,
						 processTotalWrittenSuffix.c_str(),
						 processTotalUsage * 100.0);
			break;
		case 0b11:
			ImGui::TextF("Total (I/O %): ({:6.2} {:4}, {:6.2} {:4}) {:6.2}% / ({:6.2} {:4}, {:6.2} {:4}) {:6.2}%",
						 processTotalRead,
						 processTotalReadSuffix.c_str(),
						 processTotalWritten,
						 processTotalWrittenSuffix.c_str(),
						 processTotalUsage * 100.0,
						 totalRead,
						 totalReadSuffix.c_str(),
						 totalWritten,
						 totalWrittenSuffix.c_str(),
						 totalUsage * 100.0);
			break;
		}

		std::size_t numIOEndpoints = state->IOEndpointInfos.size();
		if (numIOEndpoints <= 1)
			return;

		std::uint8_t selector2 = static_cast<std::uint8_t>(state->Abilities.hasFlag(Profiler::RuntimeAbilities::PerProcessIndividualIOUsages)) << 1 | static_cast<std::uint8_t>(state->Abilities.hasFlag(Profiler::RuntimeAbilities::IndividualIOUsages));

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode("Individual Endpoints##IndividualEndpoints"))
		{
			for (std::size_t i = 0; i < numIOEndpoints; ++i)
			{
				auto& info = state->IOEndpointInfos[i];

				std::uint8_t selector3 = ((i < state->ProcessIOUsages.size()) << 1 | (i < state->TotalIOUsages.size())) & selector2;

				std::string readSuffix;
				std::string writtenSuffix;
				std::string processReadSuffix;
				std::string processWrittenSuffix;
				double      read           = 0.0;
				double      written        = 0.0;
				double      usage          = 0.0;
				double      processRead    = 0.0;
				double      processWritten = 0.0;
				double      processUsage   = 0.0;

				if (selector3 & 0b01)
				{
					auto& usg = state->TotalIOUsages[i];
					read      = GetScaledBytes(usg.BytesRead, readSuffix);
					written   = GetScaledBytes(usg.BytesWritten, writtenSuffix);
					usage     = usg.Usage;
				}

				if (selector3 & 0b10)
				{
					auto& usg      = state->ProcessIOUsages[i];
					processRead    = GetScaledBytes(usg.BytesRead, processReadSuffix);
					processWritten = GetScaledBytes(usg.BytesWritten, processWrittenSuffix);
					processUsage   = usg.Usage;
				}

				readSuffix           += "/s";
				writtenSuffix        += "/s";
				processReadSuffix    += "/s";
				processWrittenSuffix += "/s";

				switch (selector3)
				{
				case 0b00: break; // Dafuq happened here???
				case 0b01:
					ImGui::TextF("{} (I/O %): (---.-- B/s , ---.-- B/s ) ---.--% / ({:6.2} {:4}, {:6.2} {:4}) {:6.2}%",
								 info.Name.c_str(),
								 read,
								 readSuffix.c_str(),
								 written,
								 writtenSuffix.c_str(),
								 usage * 100.0);
					break;
				case 0b10:
					ImGui::TextF("{} (I/O %): ({:6.2} {:4}, {:6.2} {:4}) {:6.2}% / (---.-- B/s , ---.-- B/s ) ---.--%",
								 info.Name.c_str(),
								 processRead,
								 processReadSuffix.c_str(),
								 processWritten,
								 processWrittenSuffix.c_str(),
								 processUsage * 100.0);
					break;
				case 0b11:
					ImGui::TextF("{} (I/O %): ({:6.2} {:4}, {:6.2} {:4}) {:6.2}% / ({:6.2} {:4}, {:6.2} {:4}) {:6.2}%",
								 info.Name.c_str(),
								 processRead,
								 processReadSuffix.c_str(),
								 processWritten,
								 processWrittenSuffix.c_str(),
								 processUsage * 100.0,
								 read,
								 readSuffix.c_str(),
								 written,
								 writtenSuffix.c_str(),
								 usage * 100.0);
					break;
				}
			}
			ImGui::TreePop();
		}
	}

	void ShowRuntimeView(bool* p_open, RuntimeViewState* state)
	{
		ImGui::Begin("Runtime View##RuntimeView", p_open);

		if (!state->TotalCoreUsages.empty() ||
			!state->ProcessCoreUsages.empty())
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("CPU Usage##CPUUsage"))
			{
				CoreUsages(state);
				ImGui::TreePop();
			}
		}

		if (state->Abilities.hasFlag(Profiler::RuntimeAbilities::MemoryUsage | Profiler::RuntimeAbilities::IndividualMemoryUsages |
									 Profiler::RuntimeAbilities::PerProcessMemoryUsage | Profiler::RuntimeAbilities::PerProcessIndividualMemoryUsages))
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("Memory Usage##MemoryUsage"))
			{
				MemoryUsages(state);
				ImGui::TreePop();
			}
		}

		if (state->Abilities.hasFlag(Profiler::RuntimeAbilities::IOUsage | Profiler::RuntimeAbilities::IndividualIOUsages |
									 Profiler::RuntimeAbilities::PerProcessIOUsage | Profiler::RuntimeAbilities::PerProcessIndividualIOUsages))
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("IO Usage##IOUsage"))
			{
				IOUsages(state);
				ImGui::TreePop();
			}
		}

		ImGui::End();
	}
} // namespace UI