#pragma once

#include <atomic>
#include <functional>

namespace Volt
{
	using JobID = uint32_t;
	using JobFunc = std::function<void()>;

	constexpr JobID INVALID_JOB_ID = std::numeric_limits<JobID>::max();
	constexpr size_t TARGET_SIZE = 128;

	enum class ExecutionPolicy
	{
		MainThread,
		WorkerThread
	};

	struct Job
	{
		// #TODO_Ivar: Switch to not using std::function as std::function is very large.
		JobFunc func;
		JobID parentJob = INVALID_JOB_ID;
		long unfinishedJobs = 0;
		ExecutionPolicy executionPolicy = ExecutionPolicy::WorkerThread;

		char alignmentPadding[TARGET_SIZE - sizeof(func) - sizeof(parentJob) - sizeof(unfinishedJobs) - sizeof(executionPolicy)];
	};
}
