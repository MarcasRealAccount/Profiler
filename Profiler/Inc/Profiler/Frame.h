#pragma once

#include "State.h"
#include "Timestamp.h"
#include "Utils/Core.h"

namespace Profiler
{
	namespace Detail
	{
		BUILD_NEVER_INLINE void Frame(ThreadState* state);
		BUILD_NEVER_INLINE void HRFrame(ThreadState* state);
	} // namespace Detail

	inline void Frame()
	{
		if (!g_State.Initialized)
			return;

		if (!IsMainThread())
			throw std::runtime_error("Frame has to be called from the main thread!");

		bool newCapture = g_State.WantCapturing;
		if (g_State.Capturing != newCapture)
		{
			for (auto tstate : g_State.Threads)
				tstate->Capture = newCapture;
		}
		g_State.Capturing = newCapture;

		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::Frame(state);
	}

	inline void HRFrame()
	{
		if (!g_State.Initialized)
			return;

		if (!IsMainThread())
			throw std::runtime_error("Frame has to be called from the main thread!");

		bool newCapture = g_State.WantCapturing;
		if (g_State.Capturing != newCapture)
		{
			for (auto tstate : g_State.Threads)
				tstate->Capture = newCapture;
		}
		g_State.Capturing = newCapture;

		ThreadState* state = GetThreadState();
		if (state->Capture)
			Detail::HRFrame(state);
	}
} // namespace Profiler