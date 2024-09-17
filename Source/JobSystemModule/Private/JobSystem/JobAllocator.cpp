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
		const uint32_t index = m_jobAllocationIndex.fetch_add(1u);
		const uint32_t allocatorIndex = (index - 1) % MAX_JOB_COUNT;

		VT_ENSURE(m_jobAllocator[allocatorIndex].unfinishedJobs == 0);
		return { &m_jobAllocator[allocatorIndex], allocatorIndex };
	}

	void JobAllocator::FreeJob(Job* job)
	{
		job->unfinishedJobs = 0;
		job->func = 0;
		job->parentJob = INVALID_JOB_ID;
	}

	Job* JobAllocator::GetJobFromID(JobID id)
	{
		VT_ENSURE(id < MAX_JOB_COUNT);
		return &m_jobAllocator[id];
	}
}
