#pragma once

#include "JobSystem/Job.h"

#include <CoreUtilities/Containers/Vector.h>

#include <atomic>

namespace Volt
{
	struct AllocatedJob
	{
		Job* job;
		JobID id;
	};

	class JobAllocator
	{
	public:
		JobAllocator();

		AllocatedJob AllocateJob();
		void FreeJob(Job* job);
		Job* GetJobFromID(JobID id);

	private:
		inline static constexpr uint32_t MAX_JOB_COUNT = 8096;
		
		std::atomic_uint32_t m_jobAllocationIndex = 0;

		Vector<Job> m_jobAllocator;
	};
}
