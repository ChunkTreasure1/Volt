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
		auto [jobPtr, jobId] = s_instance->AllocateJobInternal(task);
		return jobId;
	}

	JobID JobSystem::CreateAndRunJob(const std::function<void()>& task)
	{
		JobID jobId = CreateJob(task);
		RunJob(jobId);

		return jobId;
	}

	JobID JobSystem::CreateJobAsChild(JobID parentJob, const std::function<void()>& task)
	{
		VT_ENSURE(s_instance);
		VT_ENSURE_MSG(parentJob != INVALID_JOB_ID, "The parent job must be a valid job ID!");

		auto [jobPtr, jobId] = s_instance->AllocateJobInternal(task, parentJob);
		return jobId;
	}

	void JobSystem::DestroyJob(JobID jobId)
	{
		VT_ENSURE(s_instance);
		VT_ENSURE_MSG(jobId != INVALID_JOB_ID, "The job id must be a valid job ID!");

		Job* job = s_instance->m_allocator.GetJobFromID(jobId);
		job->unfinishedJobs = 0;
		s_instance->m_allocator.FreeJob(job);
	}

	void JobSystem::WaitForJob(JobID jobId)
	{
		VT_ENSURE(s_instance);
		VT_ENSURE_MSG(jobId != INVALID_JOB_ID, "The job id must be a valid job ID!");

		Job* job = s_instance->m_allocator.GetJobFromID(jobId);

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
		auto& internalState = s_instance->m_internalState;

		const uint32_t nextQueueToPush = internalState.nextQueueToPush.fetch_add(1) % internalState.workerCount;
		internalState.jobQueues.at(nextQueueToPush).Push(s_instance->m_allocator.GetJobFromID(jobId));
		internalState.wakeCondition.notify_one();
	}

	void JobSystem::Initialize()
	{
		// Initialize num cores - 1 threads.
		const uint32_t hardwareConcurrency = std::thread::hardware_concurrency() - 1;
		m_internalState.workerCount = hardwareConcurrency;
		m_internalState.jobQueues.resize(m_internalState.workerCount);
		m_internalState.jobGroups.resize(MAX_JOB_GROUP_COUNT);

		for (uint32_t i = 0; i < hardwareConcurrency; i++)
		{
			auto& worker = m_workerThreads.emplace_back(std::bind(&JobSystem::SpawnWorker, this, i));

			const uint64_t core = i + 1;
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

	JobSystem::AllocatedJob JobSystem::AllocateJobInternal(const std::function<void()>& task, JobID parentJob)
	{
		auto [newJob, jobId] = m_allocator.AllocateJob();
		newJob->unfinishedJobs = 1;
		newJob->func = task;

		if (parentJob != INVALID_JOB_ID)
		{
			Job* parentJobPtr = m_allocator.GetJobFromID(parentJob);
			Atomic::InterlockedIncrement(&parentJobPtr->unfinishedJobs);
			newJob->parentJob = parentJob;
		}

		return { newJob, jobId };
	}

	VT_OPTIMIZE_OFF
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
				Job* stolenJob = stealingQueue.Pop();
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
		FinishJob(job);
	}

	void JobSystem::FinishJob(Job* job)
	{
		const long unfinishedJobs = Atomic::InterlockedDecrement(&job->unfinishedJobs);
		if (unfinishedJobs == 0)
		{
			if (job->parentJob != INVALID_JOB_ID)
			{
				FinishJob(m_allocator.GetJobFromID(job->parentJob));
			}
		}
	}

	bool JobSystem::HasCompletedJob(Job* job)
	{
		return job->unfinishedJobs == 0;
	}
}
