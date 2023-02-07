#pragma once

#include <vector>

#include <Profiler/Profiler.h>

namespace UI
{
	struct RuntimeViewState
	{
		Profiler::RuntimeData SystemRuntimeData;
		Profiler::RuntimeData ProcessRuntimeData;
		Profiler::Process     Process;
	};

	void UpdateRuntimeView(RuntimeViewState* state);

	void ShowRuntimeView(bool* p_open, RuntimeViewState* state);
} // namespace UI