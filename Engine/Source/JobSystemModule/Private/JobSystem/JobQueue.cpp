#include "jspch.h"
#include "JobQueue.h"

#include <CoreUtilities/Atomic.h>

namespace Volt
{
	JobQueue::JobQueue()
	{
		memset(m_jobQueue, 0, MAX_JOB_COUNT * sizeof(Job*));
	}
	JobQueue::~JobQueue()
	{
	}

	void JobQueue::Push(Job* job)
	{
		long b = m_bottom;
		m_jobQueue[b & MASK] = job;

		// Ensure that job is written before bottom is updated.
		VT_COMPILER_BARRIER();

		m_bottom = b + 1;
	}

	Job* JobQueue::Pop()
	{        
		long b = m_bottom - 1;
		Atomic::InterlockedExchange(&m_bottom, b);

		long t = m_top;
		if (t <= b)
		{
			Job* job = m_jobQueue[b & MASK];
			if (t != b)
			{
				return job;
			}

			if (Atomic::InterlockedCompareExchange(&m_top, t + 1, t) != t)
			{
				job = nullptr;
			}

			m_bottom = t + 1;
			return job;
		}
		else
		{
			m_bottom = t;
			return nullptr;
		}
	}

	Job* JobQueue::Steal()
	{
		long t = m_top;

		// Ensure that top is always read before bottom.
		VT_COMPILER_BARRIER();

		long b = m_bottom;

		if (t < b)
		{
			Job* job = m_jobQueue[t & MASK];

			if (Atomic::InterlockedCompareExchange(&m_top, t + 1, t) != t)
			{
				return nullptr;
			}

			return job;
		}
		else
		{
			return nullptr;
		}
	}
}
