#include "jspch.h"
#include "JobSystem.h"

namespace Volt
{
	JobSystem::JobSystem()
	{
		VT_ENSURE(s_instance == nullptr);
		s_instance = this;

		m_threadPool.Initialize(std::thread::hardware_concurrency());
	}

	JobSystem::~JobSystem()
	{
		m_threadPool.Shutdown();
		s_instance = nullptr;
	}

	uint32_t JobSystem::GetThreadCount()
	{
		VT_ENSURE(s_instance);
		return s_instance->m_threadPool.GetThreadCount();
	}
}
