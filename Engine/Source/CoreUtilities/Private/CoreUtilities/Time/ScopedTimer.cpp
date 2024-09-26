#include "cupch.h"
#include "Time/ScopedTimer.h"

ScopedTimer::ScopedTimer()
{
	m_startTime = std::chrono::steady_clock::now();
}
