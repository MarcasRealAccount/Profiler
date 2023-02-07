#include "Profiler/Utils/Core.h"

#if !BUILD_IS_SYSTEM_WINDOWS && !BUILD_IS_SYSTEM_LINUX
	#include "Profiler/Runtime.h"

namespace Profiler
{
	Process GetSystemProcess()
	{
		return 0;
	}

	Process GetCurrentProcess()
	{
		return 0;
	}

	bool PollData(Process process, RuntimeData* runtimeData)
	{
		if (!runtimeData)
			return false;

		runtimeData->LastTime = 0;
		runtimeData->CurTime  = 0;

		runtimeData->Abilities = RuntimeAbilities::None;
		runtimeData->CPUs.clear();
		runtimeData->Memory = {};
		runtimeData->IOEndpoints.clear();
		return true;
	}
} // namespace Profiler

#endif