#include "cupch.h"
#include "ScopedTimer.h"

ScopedTimer::ScopedTimer()
{
	m_startTime = std::chrono::steady_clock::now();
}
