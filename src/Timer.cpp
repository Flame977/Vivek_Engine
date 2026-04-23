#include "Timer.h"

Timer::Timer()
{
	m_lastTime = Clock::now();
}

void Timer::Tick()
{
	auto currentTime = Clock::now();
	std::chrono::duration<float> delta = currentTime - m_lastTime;
	m_deltaTime = delta.count();
	m_lastTime = currentTime;

	m_time = currentTime;
}

float Timer::GetDeltaTime() const
{
	return m_deltaTime;
}

std::chrono::time_point<Clock> Timer::GetTime() const
{
	return m_time;
}
