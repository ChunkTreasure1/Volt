#include "jspch.h"
#include "JobQueueLocking.h"

namespace Volt
{
	JobQueueLocking::JobQueueLocking()
	{
		m_mutex = new std::mutex();
		memset(m_jobQueue, 0, MAX_JOB_COUNT * sizeof(Job*));
	}

	JobQueueLocking::~JobQueueLocking()
	{
		delete m_mutex;
	}
	
	void JobQueueLocking::Push(Job* job)
	{
		std::scoped_lock lock{ *m_mutex };
		m_jobQueue[m_bottom] = job;
		++m_bottom;
	}
	
	Job* JobQueueLocking::Pop()
	{
		std::scoped_lock lock{ *m_mutex };
		const long jobCount = m_bottom - m_top;
		if (jobCount <= 0)
		{
			// No jobs in queue
			return nullptr;
		}

		--m_bottom;
		return m_jobQueue[m_bottom];
	}
	
	Job* JobQueueLocking::Steal()
	{
		std::scoped_lock lock{ *m_mutex };

		const long jobCount = m_bottom - m_top;
		if (jobCount <= 0)
		{
			return nullptr;
		}

		Job* job = m_jobQueue[m_top];
		++m_top;
		return job;
	}
}
