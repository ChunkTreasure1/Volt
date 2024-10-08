#include "jspch.h"
#include "JobAllocator.h"

namespace Volt
{
	JobAllocator::JobAllocator()
	{
		m_jobAllocator.resize(MAX_JOB_COUNT);
	}

	AllocatedJob JobAllocator::AllocateJob()
	{
		JobID jobId = INVALID_JOB_ID;

		{
			std::scoped_lock lock{ m_freeJobsMutex };
			if (!m_freeJobs.empty())
			{
				jobId = m_freeJobs.back();
				m_freeJobs.pop_back();
			}
		}

		if (jobId == INVALID_JOB_ID)
		{
			jobId = m_currentTailIndex.fetch_add(1u);
		}

		VT_ENSURE(jobId < MAX_JOB_COUNT);
		VT_ENSURE(m_jobAllocator[jobId].unfinishedJobs == 0);
		return { &m_jobAllocator[jobId], jobId };
	}

	void JobAllocator::FreeJob(JobID id)
	{
		m_jobAllocator[id].unfinishedJobs = 0;
		m_jobAllocator[id].func = nullptr;
		m_jobAllocator[id].parentJob = INVALID_JOB_ID;
	
		std::scoped_lock lock{ m_freeJobsMutex };
		m_freeJobs.emplace_back(id);
	}

	void JobAllocator::FreeJob(Job* job)
	{
		// As the job index matches with the job ID, this will work
		FreeJob(static_cast<JobID>(std::distance(m_jobAllocator.begin(), job)));
	}

	Job* JobAllocator::GetJobFromID(JobID id)
	{
		VT_ENSURE(id < MAX_JOB_COUNT);
		return &m_jobAllocator[id];
	}
}
