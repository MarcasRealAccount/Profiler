#pragma once

#include <vector>

#include <Profiler/Profiler.h>

namespace UI
{
	struct RuntimeViewState
	{
		Profiler::ERuntimeAbilities Abilities;
		Profiler::Process           Process;

		std::vector<Profiler::IOEndpointInfo> IOEndpointInfos;

		std::vector<Profiler::CoreCounter> TotalCoreUsages;
		Profiler::MemoryCounters           TotalMemoryUsages;
		std::vector<Profiler::IOCounter>   TotalIOUsages;

		std::vector<Profiler::CoreCounter> ProcessCoreUsages;
		Profiler::MemoryCounters           ProcessMemoryUsages;
		std::vector<Profiler::IOCounter>   ProcessIOUsages;
	};

	void UpdateRuntimeView(RuntimeViewState* state);

	void ShowRuntimeView(bool* p_open, RuntimeViewState* state);
} // namespace UI