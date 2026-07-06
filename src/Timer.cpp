#include "Timer.h"

Timer::Timer()
{
	m_frameStart = Clock::now();
	m_previousFrameStart = m_frameStart;
}

void Timer::Tick()
{
	m_previousFrameStart = m_frameStart;
	m_frameStart = Clock::now();
	std::chrono::duration<float> delta = m_frameStart - m_previousFrameStart;
	m_deltaTime = delta.count();
}

float Timer::GetDeltaTime() const
{
	return m_deltaTime;
}

std::chrono::time_point<Clock> Timer::GetTime() const
{
	return m_frameStart;
}

float Timer::GetCurrentFrameExecTime() const
{
	return std::chrono::duration<float>(Clock::now() - m_frameStart).count();
}


