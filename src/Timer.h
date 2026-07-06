#pragma once
#include <chrono>

using Clock = std::chrono::high_resolution_clock;
class Timer
{
public:
	
	Timer();

	// Called once per frame
	void Tick();

	float GetDeltaTime() const;

	std::chrono::time_point<Clock> GetTime() const;

	float GetCurrentFrameExecTime() const;

private:

	std::chrono::time_point<Clock> m_previousFrameStart;

	std::chrono::time_point<Clock> m_frameStart;

	float m_deltaTime = 0.0f;


};
