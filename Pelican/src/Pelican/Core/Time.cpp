#include "PelicanPCH.h"
#include "Time.h"

namespace Pelican
{
	float Time::m_DeltaTime = 0.0f;
	std::chrono::high_resolution_clock::time_point Time::m_StartTime;

	std::chrono::high_resolution_clock::time_point Time::GetTime()
	{
		return std::chrono::high_resolution_clock::now();
	}

	float Time::GetDeltaTime()
	{
		return m_DeltaTime;
	}

	float Time::GetTotalTime()
	{
		const auto currentTime = std::chrono::high_resolution_clock::now();
		return std::chrono::duration<float>(currentTime - m_StartTime).count();
	}

	void Time::SetDeltaTime(float deltaTime)
	{
		m_DeltaTime = deltaTime;
	}

	void Time::Update(std::chrono::high_resolution_clock::time_point lastTime)
	{
		const auto currentTime = std::chrono::high_resolution_clock::now();
		m_DeltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
	}
}
