#pragma once

#include "JobSystem/Config.h"

#include "JobSystem/Job.h"

#include <cstdint>
#include <functional>

namespace Volt
{
	using TaskID = uint32_t;

	class VTJS_API TaskGraph
	{
	public:
		TaskGraph(size_t predictedTaskCount = 100);
		~TaskGraph();

		VT_DELETE_COPY_MOVE(TaskGraph);

		TaskID AddTask(const std::function<void()>& task);

		void Barrier();

		void Execute();
		void ExecuteAndWait();
		void Wait();

	private:
		struct TaskSubGraph
		{
			JobID parentJobId;
			Vector<JobID> createdJobs;
		};

		void CreateNewSubGraph();

		Vector<TaskSubGraph> m_subGraphs;
		size_t m_predictedTaskCount;
		bool m_executed = false;
	};
}
