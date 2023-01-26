#pragma once

#include <cstdint>

namespace UI
{
	struct SmoothFloat
	{
	public:
		void tick(float deltaTime);

		void reset() { Time = 0.0f; }

		operator float() const { return Current; }

		float Current;

		float Target;
		float Origin;
		float Time;
	};

	struct CPUCoresData
	{
	public:
		float OffsetVel       = 0.0f;
		float PreviousDrag    = 0.0f;
		float PreviousDragVel = 0.0f;

		float ZoomLevel = 1.0f;
		float Offset    = 0.0f;

		float Zoom    = 1.0f;
		float InvZoom = 1.0f;
	};

	void ShowCPUCores(bool* p_open, CPUCoresData* data, float deltaTime);
} // namespace UI