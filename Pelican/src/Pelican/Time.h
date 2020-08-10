#pragma once
#include <chrono>

namespace Pelican
{
	class Time
	{
	public:
		static std::chrono::high_resolution_clock::time_point GetTime();

		static float GetDeltaTime();
		static void SetDeltaTime(float deltaTime);

		static void Update(std::chrono::high_resolution_clock::time_point lastTime);

	private:
		static float m_DeltaTime;
		static std::chrono::high_resolution_clock::time_point m_StartTime;
	};
}
