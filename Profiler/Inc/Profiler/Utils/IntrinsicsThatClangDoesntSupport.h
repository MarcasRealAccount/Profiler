#pragma once

#include "Core.h"

#include <cstdint>

#if BUILD_IS_TOOLSET_MSVC
	#include <intrin.h>
#elif BUILD_IS_TOOLSET_GCC || BUILD_IS_TOOLSET_CLANG
	#include <cpuid.h>
#endif

namespace Profiler::Utils
{
	inline void cpuid(int (&out)[4], int code)
	{
#if BUILD_IS_TOOLSET_MSVC
		__cpuid(out, code);
#elif BUILD_IS_TOOLSET_GCC || BUILD_IS_TOOLSET_CLANG
		__cpuid(code, out[0], out[1], out[2], out[3]);
#endif
	}

	inline std::uint64_t rdtsc()
	{
#if BUILD_IS_TOOLSET_MSVC
		return __rdtsc();
#elif BUILD_IS_TOOLSET_GCC || BUILD_IS_TOOLSET_CLANG
		std::uint64_t result {};
		asm("rdtsc\n"
			"shlq $32, %%rdx\n"
			"orq %%rax, %%rdx\n"
			"movq %%rdx, %0\n"
			: "=r"(result)
			:
			: "rax", "rdx");
		return result;
#endif
	}
} // namespace Profiler::Utils