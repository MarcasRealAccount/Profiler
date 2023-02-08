#include "RuntimeView.h"
#include "ImGuiHelper.h"

#include <Profiler/Profiler.h>

#include <imgui.h>

namespace UI
{
	void UpdateRuntimeView(RuntimeViewState* state)
	{
		auto systemProcess = Profiler::GetSystemProcess();
		Profiler::PollData(systemProcess, &state->SystemRuntimeData);
		if (state->Process == systemProcess)
			state->ProcessRuntimeData.Abilities = Profiler::RuntimeAbilities::None;
		else
			Profiler::PollData(state->Process, &state->ProcessRuntimeData);
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

	void CPUUsages(RuntimeViewState* state)
	{
		ImGui::TextUnformatted("Process / System");

		auto&        systemData  = state->SystemRuntimeData;
		auto&        processData = state->ProcessRuntimeData;
		std::uint8_t selector    = (!!processData.Abilities.hasFlag(Profiler::RuntimeAbilities::CPUUsage)) << 1 | !!systemData.Abilities.hasFlag(Profiler::RuntimeAbilities::CPUUsage);
		std::uint8_t selector2   = ((!!processData.Abilities.hasFlag(Profiler::RuntimeAbilities::IndividualCPUUsages)) << 1 | !!systemData.Abilities.hasFlag(Profiler::RuntimeAbilities::IndividualCPUUsages)) & selector;

		double systemUsage  = 0.0;
		double processUsage = 0.0;

		// TODO(MarcasRealAccount): Maybe add cpu time information?
		if (selector & 0b01)
		{
			if (selector2 & 0b01)
			{
				for (std::size_t i = 0; i < systemData.CPUs.size(); ++i)
				{
					auto& cpu   = systemData.CPUs[i];
					systemUsage += cpu.Usage;
				}
				systemUsage /= systemData.CPUs.size();
			}
			else
			{
				systemUsage = systemData.CPUs[0].Usage;
			}
		}

		if (selector & 0b10)
		{
			if (selector2 & 0b10)
			{
				for (std::size_t i = 0; i < processData.CPUs.size(); ++i)
				{
					auto& cpu    = processData.CPUs[i];
					processUsage += cpu.Usage;
				}
				processUsage /= processData.CPUs.size();
			}
			else
			{
				processUsage = processData.CPUs[0].Usage;
			}
		}

		switch (selector)
		{
		case 0b00: return; // Dafuq are you doing in here???
		case 0b01:
			ImGui::TextF("Total (%): ---.-- / {:6.2f}", systemUsage * 100.0);
			break;
		case 0b10:
			ImGui::TextF("Total (%): {:6.2f} / ---.--", processUsage * 100.0);
			break;
		case 0b11:
			ImGui::TextF("Total (%): {:6.2f} / {:6.2f}", processUsage * 100.0, systemUsage * 100.0);
			break;
		}

		if (selector2 == 0)
			return; // No individual core information

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode("Individual Cores##IndividualCores"))
		{
			std::size_t numCores     = std::max(systemData.CPUs.size(), processData.CPUs.size());
			std::size_t maxCoreWidth = RequiredDigits(numCores);
			for (std::size_t i = 0; i < numCores; ++i)
			{
				switch (selector2)
				{
				case 0b00: break; // Dafuq happened here???
				case 0b01:
					ImGui::TextF("{:>{}} (%): ---.-- / {:6.2f}", i, maxCoreWidth, systemData.CPUs[i].Usage * 100.0);
					break;
				case 0b10:
					ImGui::TextF("{:>{}} (%): {:6.2f} / ---.--", i, maxCoreWidth, processData.CPUs[i].Usage * 100.0);
					break;
				case 0b11:
					ImGui::TextF("{:>{}} (%): {:6.2f} / {:6.2f}", i, maxCoreWidth, processData.CPUs[i].Usage * 100.0, systemData.CPUs[i].Usage * 100.0);
					break;
				}
			}
			ImGui::TreePop();
		}
	}

	void MemoryUsages(RuntimeViewState* state)
	{
		auto handleData = [](Profiler::RuntimeData& data, const char* nodeName) {
			if (data.Abilities.hasFlag(Profiler::RuntimeAbilities::MemoryUsage))
			{
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				if (ImGui::TreeNode(nodeName))
				{
					auto& mem = data.Memory;
					if (data.Abilities.hasFlag(Profiler::RuntimeAbilities::IndividualMemoryUsages))
					{
						std::string physicalUsageSuffix = "";
						std::string physicalTotalSuffix = "";
						std::string virtualUsageSuffix  = "";
						std::string virtualTotalSuffix  = "";
						double      physicalUsage       = GetScaledBytes(mem.PhysicalUsage, physicalUsageSuffix);
						double      physicalTotal       = GetScaledBytes(mem.PhysicalTotal, physicalTotalSuffix);
						double      virtualUsage        = GetScaledBytes(mem.VirtualUsage, virtualUsageSuffix);
						double      virtualTotal        = GetScaledBytes(mem.VirtualTotal, virtualTotalSuffix);
						ImGui::TextF("Physical:    {:6.2f} {:2} / {:6.2f} {:2} ({:6.2f}%)",
									 physicalUsage,
									 physicalUsageSuffix,
									 physicalTotal,
									 physicalTotalSuffix,
									 static_cast<double>(mem.PhysicalUsage) / static_cast<double>(mem.PhysicalTotal) * 100.0);
						ImGui::TextF("Virtual:     {:6.2f} {:2} / {:6.2f} {:2} ({:6.2f}%)",
									 virtualUsage,
									 virtualUsageSuffix,
									 virtualTotal,
									 virtualTotalSuffix,
									 static_cast<double>(mem.VirtualUsage) / static_cast<double>(mem.VirtualTotal) * 100.0);
					}
					else
					{
						std::string usageSuffix = "";
						std::string totalSuffix = "";
						double      usage       = GetScaledBytes(mem.VirtualUsage, usageSuffix);
						double      total       = GetScaledBytes(mem.VirtualTotal, totalSuffix);
						ImGui::TextF("Usage:       {:6.2f} {:2} / {:6.2f} {:2} ({:6.2f}%)",
									 usage,
									 usageSuffix,
									 total,
									 totalSuffix,
									 static_cast<double>(mem.VirtualUsage) / static_cast<double>(mem.VirtualTotal) * 100.0);
					}

					if (data.Abilities.hasFlag(Profiler::RuntimeAbilities::MemoryPageFaults))
						ImGui::TextF("Page Faults: {}", mem.PageFaultCount);

					ImGui::TreePop();
				}
			}
		};

		handleData(state->SystemRuntimeData, "System##System");
		handleData(state->ProcessRuntimeData, "Process##Process");
	}

	void IOUsages(RuntimeViewState* state)
	{
		ImGui::Text("Process / System");

		auto&        systemData  = state->SystemRuntimeData;
		auto&        processData = state->ProcessRuntimeData;
		std::uint8_t selector    = (!!processData.Abilities.hasFlag(Profiler::RuntimeAbilities::IOUsage)) << 1 | !!systemData.Abilities.hasFlag(Profiler::RuntimeAbilities::IOUsage);
		std::uint8_t selector2   = ((!!processData.Abilities.hasFlag(Profiler::RuntimeAbilities::IndividualIOUsages)) << 1 | !!systemData.Abilities.hasFlag(Profiler::RuntimeAbilities::IndividualIOUsages)) & selector;

		{
			std::string systemReadSuffix;
			std::string systemWriteSuffix;
			std::string systemOtherSuffix;
			std::string processReadSuffix;
			std::string processWriteSuffix;
			std::string processOtherSuffix;
			double      systemRead   = 0.0;
			double      systemWrite  = 0.0;
			double      systemOther  = 0.0;
			double      systemUsage  = 0.0;
			double      processRead  = 0.0;
			double      processWrite = 0.0;
			double      processOther = 0.0;
			double      processUsage = 0.0;
			if (selector & 0b01)
			{
				if (selector2 & 0b01)
				{
					std::uint64_t read  = 0;
					std::uint64_t write = 0;
					std::uint64_t other = 0;
					for (std::size_t i = 0; i < systemData.IOEndpoints.size(); ++i)
					{
						auto& usage = systemData.IOEndpoints[i];
						read        += usage.CurReadCount - usage.LastReadCount;
						write       += usage.CurWriteCount - usage.LastWriteCount;
						other       += usage.CurOtherCount - usage.LastOtherCount;
						systemUsage += usage.Usage;
					}
					systemRead  = GetScaledBytes(read, systemReadSuffix);
					systemWrite = GetScaledBytes(write, systemWriteSuffix);
					systemOther = GetScaledBytes(other, systemOtherSuffix);
					systemUsage /= systemData.IOEndpoints.size();
				}
				else
				{
					auto& usage = systemData.IOEndpoints[0];
					systemRead  = GetScaledBytes(usage.CurReadCount - usage.LastReadCount, systemReadSuffix);
					systemWrite = GetScaledBytes(usage.CurWriteCount - usage.LastWriteCount, systemWriteSuffix);
					systemOther = GetScaledBytes(usage.CurOtherCount - usage.LastOtherCount, systemOtherSuffix);
					systemUsage = usage.Usage;
				}
			}

			if (selector & 0b10)
			{
				if (selector2 & 0b10)
				{
					std::uint64_t read  = 0;
					std::uint64_t write = 0;
					std::uint64_t other = 0;
					for (std::size_t i = 0; i < processData.IOEndpoints.size(); ++i)
					{
						auto& usage  = processData.IOEndpoints[i];
						read         += usage.CurReadCount - usage.LastReadCount;
						write        += usage.CurWriteCount - usage.LastWriteCount;
						other        += usage.CurOtherCount - usage.LastOtherCount;
						processUsage += usage.Usage;
					}
					processRead  = GetScaledBytes(read, processReadSuffix);
					processWrite = GetScaledBytes(write, processWriteSuffix);
					processOther = GetScaledBytes(other, processOtherSuffix);
					processUsage /= processData.IOEndpoints.size();
				}
				else
				{
					auto& usage  = processData.IOEndpoints[0];
					processRead  = GetScaledBytes(usage.CurReadCount - usage.LastReadCount, processReadSuffix);
					processWrite = GetScaledBytes(usage.CurWriteCount - usage.LastWriteCount, processWriteSuffix);
					processOther = GetScaledBytes(usage.CurOtherCount - usage.LastOtherCount, processOtherSuffix);
					processUsage = usage.Usage;
				}
			}

			systemReadSuffix   += "/s";
			systemWriteSuffix  += "/s";
			systemOtherSuffix  += "/s";
			processReadSuffix  += "/s";
			processWriteSuffix += "/s";
			processOtherSuffix += "/s";

			switch (selector)
			{
			case 0b00: return; // Dafuq are you doing in here???
			case 0b01:
				ImGui::TextF("Total (R/W/O %): (---.-- B/s , ---.-- B/s , ---.-- B/s ) ---.--% / ({:6.2f} {:4}, {:6.2f} {:4}, {:6.2f} {:4}) {:6.2f}%",
							 systemRead,
							 systemReadSuffix,
							 systemWrite,
							 systemWriteSuffix,
							 systemOther,
							 systemOtherSuffix,
							 systemUsage * 100.0);
				break;
			case 0b10:
				ImGui::TextF("Total (R/W/O %): ({:6.2f} {:4}, {:6.2f} {:4}, {:6.2f} {:4}) {:6.2f}% / (---.-- B/s , ---.-- B/s , ---.-- B/s ) ---.--%",
							 processRead,
							 processReadSuffix,
							 processWrite,
							 processWriteSuffix,
							 processOther,
							 processOtherSuffix,
							 processUsage * 100.0);
				break;
			case 0b11:
				ImGui::TextF("Total (R/W/O %): ({:6.2f} {:4}, {:6.2f} {:4}, {:6.2f} {:4}) {:6.2f}% / ({:6.2f} {:4}, {:6.2f} {:4}, {:6.2f} {:4}) {:6.2f}%",
							 processRead,
							 processReadSuffix,
							 processWrite,
							 processWriteSuffix,
							 processOther,
							 processOtherSuffix,
							 processUsage * 100.0,
							 systemRead,
							 systemReadSuffix,
							 systemWrite,
							 systemWriteSuffix,
							 systemOther,
							 systemOtherSuffix,
							 systemUsage * 100.0);
				break;
			}
		}

		if (selector2 == 0)
			return;

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode("Individual Endpoints##IndividualEndpoints"))
		{
			std::size_t maxNameLength  = 0;
			std::size_t numIOEndpoints = std::max(processData.IOEndpoints.size(), systemData.IOEndpoints.size());
			for (std::size_t i = 0; i < numIOEndpoints; ++i)
			{
				std::string_view name;
				if (selector2 & 0b01)
				{
					auto& endpoint = systemData.IOEndpoints[i];
					name           = endpoint.Name;
				}

				if (selector2 & 0b10)
				{
					auto& endpoint = processData.IOEndpoints[i];
					if (name.empty())
						name = endpoint.Name;
				}

				maxNameLength = std::max(maxNameLength, name.size());
			}

			for (std::size_t i = 0; i < numIOEndpoints; ++i)
			{
				std::string      systemReadSuffix;
				std::string      systemWriteSuffix;
				std::string      systemOtherSuffix;
				std::string      processReadSuffix;
				std::string      processWriteSuffix;
				std::string      processOtherSuffix;
				double           systemRead   = 0.0;
				double           systemWrite  = 0.0;
				double           systemOther  = 0.0;
				double           systemUsage  = 0.0;
				double           processRead  = 0.0;
				double           processWrite = 0.0;
				double           processOther = 0.0;
				double           processUsage = 0.0;
				std::string_view name;

				if (selector2 & 0b01)
				{
					auto& endpoint = systemData.IOEndpoints[i];
					systemRead     = GetScaledBytes(endpoint.CurReadCount - endpoint.LastReadCount, systemReadSuffix);
					systemWrite    = GetScaledBytes(endpoint.CurWriteCount - endpoint.LastWriteCount, systemWriteSuffix);
					systemOther    = GetScaledBytes(endpoint.CurOtherCount - endpoint.LastOtherCount, systemOtherSuffix);
					systemUsage    = endpoint.Usage;
					name           = endpoint.Name;
				}

				if (selector2 & 0b10)
				{
					auto& endpoint = processData.IOEndpoints[i];
					processRead    = GetScaledBytes(endpoint.CurReadCount - endpoint.LastReadCount, processReadSuffix);
					processWrite   = GetScaledBytes(endpoint.CurWriteCount - endpoint.LastWriteCount, processWriteSuffix);
					processOther   = GetScaledBytes(endpoint.CurOtherCount - endpoint.LastOtherCount, processOtherSuffix);
					processUsage   = endpoint.Usage;
					if (name.empty())
						name = endpoint.Name;
				}

				systemReadSuffix   += "/s";
				systemWriteSuffix  += "/s";
				systemOtherSuffix  += "/s";
				processReadSuffix  += "/s";
				processWriteSuffix += "/s";
				processOtherSuffix += "/s";

				switch (selector2)
				{
				case 0b00: break; // Dafuq happened here???
				case 0b01:
					ImGui::TextF("{:>{}} (R/W/O %): (---.-- B/s , ---.-- B/s , ---.-- B/s ) ---.--% / ({:6.2f} {:4}, {:6.2f} {:4}, {:6.2f} {:4}) {:6.2f}%",
								 name,
								 maxNameLength,
								 systemRead,
								 systemReadSuffix,
								 systemWrite,
								 systemWriteSuffix,
								 systemOther,
								 systemOtherSuffix,
								 systemUsage * 100.0);
					break;
				case 0b10:
					ImGui::TextF("{:>{}} (R/W/O %): ({:6.2f} {:4}, {:6.2f} {:4}, {:6.2f} {:4}) {:6.2f}% / (---.-- B/s , ---.-- B/s , ---.-- B/s ) ---.--%",
								 name,
								 maxNameLength,
								 processRead,
								 processReadSuffix,
								 processWrite,
								 processWriteSuffix,
								 processOther,
								 processOtherSuffix,
								 processUsage * 100.0);
					break;
				case 0b11:
					ImGui::TextF("{:>{}} (R/W/O %): ({:6.2f} {:4}, {:6.2f} {:4}, {:6.2f} {:4}) {:6.2f}% / ({:6.2f} {:4}, {:6.2f} {:4}, {:6.2f} {:4}) {:6.2f}%",
								 name,
								 maxNameLength,
								 processRead,
								 processReadSuffix,
								 processWrite,
								 processWriteSuffix,
								 processOther,
								 processOtherSuffix,
								 processUsage * 100.0,
								 systemRead,
								 systemReadSuffix,
								 systemWrite,
								 systemWriteSuffix,
								 systemOther,
								 systemOtherSuffix,
								 systemUsage * 100.0);
					break;
				}
			}
			ImGui::TreePop();
		}
	}

	void ShowRuntimeView(bool* p_open, RuntimeViewState* state)
	{
		ImGui::Begin("Runtime View##RuntimeView", p_open);

		auto support = state->SystemRuntimeData.Abilities | state->ProcessRuntimeData.Abilities;

		if (support.hasFlag(Profiler::RuntimeAbilities::CPUUsage))
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("CPU Usage##CPUUsage"))
			{
				CPUUsages(state);
				ImGui::TreePop();
			}
		}

		if (support.hasFlag(Profiler::RuntimeAbilities::MemoryUsage))
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			if (ImGui::TreeNode("Memory Usage##MemoryUsage"))
			{
				MemoryUsages(state);
				ImGui::TreePop();
			}
		}

		if (support.hasFlag(Profiler::RuntimeAbilities::IOUsage))
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