#pragma once
#include <chrono>

using Clock = std::chrono::high_resolution_clock;
class Timer
{
public:
	
	Timer();

	// Call once per frame
	void Tick();

	float GetDeltaTime() const;

	std::chrono::time_point<Clock> GetTime() const;

private:

	std::chrono::time_point<Clock> m_lastTime;

	float m_deltaTime = 0.0f;

	std::chrono::time_point<Clock> m_time;

};
