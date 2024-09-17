#pragma once

#include "JobSystem/Job.h"
#include "JobSystem/JobQueue.h"
#include "JobSystem/JobAllocator.h"

#include <CoreUtilities/Containers/Vector.h>
#include <CoreUtilities/Containers/Map.h>

namespace Volt
{
	struct JobGroup
	{
		long unfinishedJobs = 0;
	};

	class VTJS_API JobSystem
	{
	public:
		JobSystem();
		~JobSystem();

		static JobID CreateJob(const std::function<void()>& task);
		static JobID CreateAndRunJob(const std::function<void()>& task);
		static JobID CreateJobAsChild(JobID parentJob, const std::function<void()>& task);

		static void DestroyJob(JobID jobId);

		static void WaitForJob(JobID jobId);
		static void RunJob(JobID jobId);

	private:
		struct InternalState
		{
			std::atomic_bool alive = true;
			std::atomic_uint32_t nextQueueToPush = 0;
			std::atomic_uint32_t nextJobGroupIndex = 0;
			std::atomic_uint32_t aliveJobGroupCount = 0;
			std::mutex wakeMutex;
			std::condition_variable wakeCondition;
			uint32_t workerCount = 0;

			Vector<JobGroup> jobGroups;
			Vector<JobQueue> jobQueues;
		};

		struct AllocatedJob
		{
			Job* ptr;
			JobID id;
		};

		inline static constexpr uint32_t MAX_JOB_GROUP_COUNT = 1024;

		void Initialize();
		void Shutdown();

		AllocatedJob AllocateJobInternal(const std::function<void()>& task, JobID parentJob = INVALID_JOB_ID);

		Job* TryGetJob(uint32_t workerId);
		void SpawnWorker(uint32_t workerId);
		void ExecuteJob(Job* job);
		void FinishJob(Job* job);

		bool HasCompletedJob(Job* job);

		inline static JobSystem* s_instance = nullptr;

		InternalState m_internalState;
		JobAllocator m_allocator;
		Vector<std::thread> m_workerThreads;
	};
}
