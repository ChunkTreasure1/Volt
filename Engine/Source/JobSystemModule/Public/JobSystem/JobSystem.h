#pragma once

#include "JobSystem/Job.h"
#include "JobSystem/JobQueueLocking.h"
#include "JobSystem/JobAllocator.h"

#include <SubSystem/SubSystem.h>
#include <EventSystem/EventListener.h>

#include <CoreUtilities/Containers/Vector.h>

namespace Volt
{
	struct JobGroup
	{
		long unfinishedJobs = 0;
	};

	class AppUpdateEvent;
	class VTJS_API JobSystem : public SubSystem, EventListener
	{
	public:
		JobSystem();
		~JobSystem();

		static JobID CreateJob(const std::function<void()>& task);
		static JobID CreateJob(ExecutionPolicy executionPolicy, const std::function<void()>& task);
		static JobID CreateAndRunJob(const std::function<void()>& task);
		static JobID CreateAndRunJob(ExecutionPolicy executionPolicy, const std::function<void()>& task);
		static JobID CreateJobAsChild(JobID parentJob, const std::function<void()>& task);
		static JobID CreateJobAsChild(ExecutionPolicy executionPolicy, JobID parentJob, const std::function<void()>& task);

		static void DestroyJob(JobID jobId);

		static void WaitForJob(JobID jobId);
		static void RunJob(JobID jobId);

		VT_DECLARE_SUBSYSTEM("{FBB8F365-99B3-416D-84EA-702BD4F56962}"_guid)

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
			//Vector<JobQueue> jobQueues;
			Vector<JobQueueLocking> jobQueues;
			JobQueueLocking mainThreadQueue;
		};

		struct AllocatedJob
		{
			Job* ptr;
			JobID id;
		};

		inline static constexpr uint32_t MAX_JOB_GROUP_COUNT = 1024;

		void Initialize() override;
		void Shutdown() override;

		bool OnUpdate(AppUpdateEvent& event);

		void ExecuteMainThreadJobs();

		AllocatedJob AllocateJobInternal(ExecutionPolicy executionPolicy, const std::function<void()>& task, JobID parentJob = INVALID_JOB_ID);

		Job* TryGetJob(uint32_t workerId);
		void SpawnWorker(uint32_t workerId);
		void ExecuteJob(Job* job);
		void FinishJob(Job* job, JobAllocator& allocator);

		bool HasCompletedJob(Job* job);

		inline static JobSystem* s_instance = nullptr;

		InternalState m_internalState;
		JobAllocator m_allocator;
		Vector<std::thread> m_workerThreads;
	};
}
