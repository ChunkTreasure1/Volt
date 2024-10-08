#include "jspch.h"
#include "JobSystem.h"

#include <CoreUtilities/ThreadUtilities.h>
#include <CoreUtilities/Atomic.h>
#include <CoreUtilities/Random.h>

namespace Volt
{
	JobSystem::JobSystem()
	{
		VT_ENSURE(s_instance == nullptr);
		s_instance = this;

		Initialize();
	}

	JobSystem::~JobSystem()
	{
		Shutdown();
		s_instance = nullptr;
	}

	JobID JobSystem::CreateJob(const std::function<void()>& task)
	{
		VT_ENSURE(s_instance);
		return CreateJob(ExecutionPolicy::WorkerThread, task);
	}

	JobID JobSystem::CreateJob(ExecutionPolicy executionPolicy, const std::function<void()>& task)
	{
		auto [jobPtr, jobId] = s_instance->AllocateJobInternal(executionPolicy, task);
		return jobId;
	}

	JobID JobSystem::CreateAndRunJob(const std::function<void()>& task)
	{
		return CreateAndRunJob(ExecutionPolicy::WorkerThread, task);
	}

	JobID JobSystem::CreateAndRunJob(ExecutionPolicy executionPolicy, const std::function<void()>& task)
	{
		JobID jobId = CreateJob(executionPolicy, task);
		RunJob(jobId);

		return jobId;
	}

	JobID JobSystem::CreateJobAsChild(JobID parentJob, const std::function<void()>& task)
	{
		return CreateJobAsChild(ExecutionPolicy::WorkerThread, parentJob, task);
	}

	JobID JobSystem::CreateJobAsChild(ExecutionPolicy executionPolicy, JobID parentJob, const std::function<void()>& task)
	{
		VT_ENSURE(s_instance);
		VT_ENSURE_MSG(parentJob != INVALID_JOB_ID, "The parent job must be a valid job ID!");

		auto [jobPtr, jobId] = s_instance->AllocateJobInternal(executionPolicy, task, parentJob);
		return jobId;
	}

	void JobSystem::DestroyJob(JobID jobId)
	{
		VT_ENSURE(s_instance);
		VT_ENSURE_MSG(jobId != INVALID_JOB_ID, "The job id must be a valid job ID!");
		s_instance->m_allocator.FreeJob(jobId);
	}

	void JobSystem::WaitForJob(JobID jobId)
	{
		VT_ENSURE(s_instance);
		VT_ENSURE_MSG(jobId != INVALID_JOB_ID, "The job id must be a valid job ID!");

		Job* job = s_instance->m_allocator.GetJobFromID(jobId);
		if (!job)
		{
			return;
		}

		while (!s_instance->HasCompletedJob(job))
		{
			Job* nextJob = s_instance->TryGetJob(0);
			if (nextJob)
			{
				s_instance->ExecuteJob(nextJob);
			}
		}
	}

	void JobSystem::RunJob(JobID jobId)
	{
		VT_ENSURE(s_instance);
		VT_ENSURE_MSG(jobId != INVALID_JOB_ID, "The job id must be a valid job ID!");

		auto& internalState = s_instance->m_internalState;
		Job* jobPtr = s_instance->m_allocator.GetJobFromID(jobId);

		if (jobPtr->executionPolicy == ExecutionPolicy::WorkerThread)
		{
			const uint32_t nextQueueToPush = internalState.nextQueueToPush.fetch_add(1) % internalState.workerCount;
			internalState.jobQueues.at(nextQueueToPush).Push(jobPtr);
			internalState.wakeCondition.notify_one();
		}
		else if (jobPtr->executionPolicy == ExecutionPolicy::MainThread)
		{
			internalState.mainThreadQueue.Push(jobPtr);
		}
	}

	void JobSystem::ExecuteMainThreadJobs()
	{
		auto& internalState = m_internalState;

		Job* jobPtr = internalState.mainThreadQueue.Pop();
		while (jobPtr)
		{
			jobPtr->func();
			FinishJob(jobPtr, m_allocator);
			jobPtr = internalState.mainThreadQueue.Pop();
		}
	}

	void JobSystem::Initialize()
	{
		// Initialize num cores - 2 threads.
		const uint32_t hardwareConcurrency = std::thread::hardware_concurrency() - 2;
		m_internalState.workerCount = hardwareConcurrency;
		m_internalState.jobQueues.resize(m_internalState.workerCount);
		m_internalState.jobGroups.resize(MAX_JOB_GROUP_COUNT);

		for (uint32_t i = 0; i < hardwareConcurrency; i++)
		{
			auto& worker = m_workerThreads.emplace_back(std::bind(&JobSystem::SpawnWorker, this, i));

			const uint64_t core = i + 2;
			Thread::AssignThreadToCore(worker.native_handle(), 1ull << core);
			Thread::SetThreadPriority(worker.native_handle(), ThreadPriority::High);

			std::string threadName = std::format("Volt::Worker {}", i);
			Thread::SetThreadName(worker.native_handle(), threadName);
		}
	}

	void JobSystem::Shutdown()
	{
		// Kill the job system, and wake all threads
		m_internalState.alive = false;
		m_internalState.wakeCondition.notify_all();

		for (auto& w : m_workerThreads)
		{
			w.join();
		}
	}

	JobSystem::AllocatedJob JobSystem::AllocateJobInternal(ExecutionPolicy executionPolicy, const std::function<void()>& task, JobID parentJob)
	{
		auto [newJob, jobId] = m_allocator.AllocateJob();
		newJob->unfinishedJobs = 1;
		newJob->func = task;
		newJob->executionPolicy = executionPolicy;

		if (parentJob != INVALID_JOB_ID)
		{
			Job* parentJobPtr = m_allocator.GetJobFromID(parentJob);
			Atomic::InterlockedIncrement(&parentJobPtr->unfinishedJobs);
			newJob->parentJob = parentJob;
		}

		return { newJob, jobId };
	}

	Job* JobSystem::TryGetJob(uint32_t workerId)
	{
		auto& workerQueue = m_internalState.jobQueues.at(workerId);

		Job* job = workerQueue.Pop();
		if (!job)
		{
			uint32_t currentQueue = (workerId + 1) % m_internalState.workerCount;

			while (currentQueue != workerId)
			{
				auto& stealingQueue = m_internalState.jobQueues.at(currentQueue);
				Job* stolenJob = stealingQueue.Steal();
				if (stolenJob)
				{
					return stolenJob;
				}

				currentQueue = (currentQueue + 1) % m_internalState.workerCount;
			}
		}

		return job;
	}

	void JobSystem::SpawnWorker(uint32_t workerId)
	{
		while (m_internalState.alive.load())
		{
			Job* job = TryGetJob(workerId);
			const bool foundWork = job != nullptr;

			if (foundWork)
			{
				ExecuteJob(job);
			}
			else
			{
				std::unique_lock<std::mutex> lock{ m_internalState.wakeMutex };
				m_internalState.wakeCondition.wait(lock);
			}
		}
	}

	void JobSystem::ExecuteJob(Job* job)
	{
		job->func();
		FinishJob(job, m_allocator);
	}

	void JobSystem::FinishJob(Job* job, JobAllocator& allocator)
	{
		const long unfinishedJobs = Atomic::InterlockedDecrement(&job->unfinishedJobs);
		if (unfinishedJobs == 0)
		{
			if (job->parentJob != INVALID_JOB_ID)
			{
				FinishJob(allocator.GetJobFromID(job->parentJob), allocator);
			}

			allocator.FreeJob(job);
		}
	}

	bool JobSystem::HasCompletedJob(Job* job)
	{
		return job->unfinishedJobs == 0;
	}
}
