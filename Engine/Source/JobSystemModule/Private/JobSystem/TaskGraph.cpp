#include "jspch.h"
#include "TaskGraph.h"

#include "JobSystem/JobSystem.h"

namespace Volt
{
	TaskGraph::TaskGraph(size_t predictedTaskCount)
		: m_predictedTaskCount(predictedTaskCount)
	{
		CreateNewSubGraph();
	}

	TaskGraph::~TaskGraph()
	{
		if (!m_executed)
		{
			for (auto& subGraph : m_subGraphs)
			{
				JobSystem::DestroyJob(subGraph.parentJobId);
				for (const auto& jobId : subGraph.createdJobs)
				{
					JobSystem::DestroyJob(jobId);
				}
			}
		}
	}

	TaskID TaskGraph::AddTask(const std::function<void()>& task)
	{
		auto& currentSubGraph = m_subGraphs.back();

		JobID newJobId = JobSystem::CreateJobAsChild(currentSubGraph.parentJobId, task);
		currentSubGraph.createdJobs.emplace_back(newJobId);
		return newJobId;
	}

	void TaskGraph::Barrier()
	{
		CreateNewSubGraph();
	}

	void TaskGraph::Execute()
	{
		m_executed = true;

		for (size_t i = 0; auto& subGraph : m_subGraphs)
		{
			for (const auto& jobId : subGraph.createdJobs)
			{
				JobSystem::RunJob(jobId);
			}

			JobSystem::RunJob(subGraph.parentJobId);

			if (i < m_subGraphs.size() - 1)
			{
				JobSystem::WaitForJob(subGraph.parentJobId);
			}

			i++;
		}
	}

	void TaskGraph::ExecuteAndWait()
	{
		Execute();
		JobSystem::WaitForJob(m_subGraphs.back().parentJobId);
	}

	void TaskGraph::Wait()
	{
		VT_ENSURE(m_executed);

		for (const auto& subGraph : m_subGraphs)
		{
			JobSystem::WaitForJob(subGraph.parentJobId);
		}
	}

	void TaskGraph::CreateNewSubGraph()
	{
		auto& newSubGraph = m_subGraphs.emplace_back();
		newSubGraph.createdJobs.reserve(m_predictedTaskCount);
		newSubGraph.parentJobId = JobSystem::CreateJob([]() {});
	}
}
