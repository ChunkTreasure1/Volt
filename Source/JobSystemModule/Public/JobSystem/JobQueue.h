#pragma once

#include "JobSystem/Config.h"

namespace Volt
{
	struct Job;
	class VTJS_API JobQueue
	{
	public:
		JobQueue();
		~JobQueue();

		void Push(Job* job);
		Job* Pop();
		Job* Steal();

	private:
		inline static constexpr uint32_t MAX_JOB_COUNT = 4096u;
		inline static constexpr uint32_t MASK = MAX_JOB_COUNT - 1u;

		Job* m_jobQueue[MAX_JOB_COUNT];
		long m_bottom = 0;
		long m_top = 0;
	};
}
