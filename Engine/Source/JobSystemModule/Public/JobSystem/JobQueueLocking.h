#pragma once

#include "JobSystem/Job.h"
#include "JobSystem/Config.h"

#include <CoreUtilities/Containers/Vector.h>

#include <mutex>

namespace Volt
{
	class VTJS_API JobQueueLocking
	{
	public:
		JobQueueLocking();
		~JobQueueLocking();

		void Push(Job* job);
		Job* Pop();
		Job* Steal();

	private:
		inline static constexpr uint32_t MAX_JOB_COUNT = 4096u;
		inline static constexpr uint32_t MASK = MAX_JOB_COUNT - 1u;

		Job* m_jobQueue[MAX_JOB_COUNT];
		long m_bottom = 0;
		long m_top = 0;

		std::mutex* m_mutex = nullptr;
	};
}
