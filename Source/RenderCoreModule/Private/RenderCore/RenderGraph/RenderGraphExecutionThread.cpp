#include "rcpch.h"
#include "RenderCore/RenderGraph/RenderGraphExecutionThread.h"

#include "RenderCore/RenderGraph/RenderGraph.h"
#include "RenderCore/Resources/BindlessResourcesManager.h"

#include <RHIModule/Graphics/GraphicsContext.h>
#include <RHIModule/Graphics/GraphicsDevice.h>
#include <RHIModule/Graphics/DeviceQueue.h>

#include <RHIModule/Graphics/Swapchain.h>

#include <CoreUtilities/ThreadUtilities.h>
#include <CoreUtilities/Containers/FunctionQueue.h>
#include <CoreUtilities/Containers/ThreadSafeQueue.h>

namespace Volt
{
	struct RenderGraphThreadData
	{
		Scope<std::thread> executionThread;

		ThreadSafeQueue<std::function<void()>> executionQueue;

		std::mutex mutex;
		std::condition_variable executeVariable;
		std::condition_variable waitForExecutionVariable;

		std::atomic_bool isExecuting;
		std::atomic_bool isRunning;

		RenderGraphExecutionThread::ExecutionMode executionMode;
	};

	inline static Scope<RenderGraphThreadData> s_data;

	void RenderGraphExecutionThread::Initialize(RenderGraphExecutionThread::ExecutionMode executionMode)
	{
		s_data = CreateScope<RenderGraphThreadData>();
		s_data->isRunning = true;
		s_data->executionMode = executionMode;

		if (executionMode == RenderGraphExecutionThread::ExecutionMode::Multithreaded)
		{
			InitializeThread();
		}
	}

	void RenderGraphExecutionThread::Shutdown()
	{
		s_data->isRunning = false;

		ShutdownThread();

		s_data = nullptr;
	}

	void RenderGraphExecutionThread::ExecuteRenderGraph(RenderGraph&& renderGraph)
	{
		s_data->executionQueue.emplace([rg = std::move(renderGraph)]() mutable
		{
			rg.ExecuteInternal(false);
		});

		s_data->executeVariable.notify_one();
	}

	void RenderGraphExecutionThread::WaitForFinishedExecution()
	{
		if (s_data->executionMode == RenderGraphExecutionThread::ExecutionMode::Multithreaded)
		{
			std::unique_lock lock{ s_data->mutex };
			s_data->waitForExecutionVariable.wait(lock, []() { return !s_data->isExecuting.load() && s_data->executionQueue.empty(); });
		}
		else
		{
			ExecuteGraphs();
		}
	}

	void RenderGraphExecutionThread::InitializeThread()
	{
		s_data->executionThread = CreateScope<std::thread>(&RenderGraphExecutionThread::RT_ExecuteGraphs);
		SetThreadName(s_data->executionThread->native_handle(), "RenderGraphExecutionThread");
	}

	void RenderGraphExecutionThread::ShutdownThread()
	{
		if (s_data->executionMode == RenderGraphExecutionThread::ExecutionMode::Multithreaded)
		{
			s_data->executeVariable.notify_one();
			s_data->executionThread->join();
		}
	}

	void RenderGraphExecutionThread::RT_ExecuteGraphs()
	{
		VT_PROFILE_THREAD("RenderGraphExecutionThread");

		while (s_data->isRunning)
		{
			VT_PROFILE_SCOPE("Execute Graph");

			std::unique_lock<std::mutex> lock{ s_data->mutex };
			s_data->executeVariable.wait(lock, []() { return !s_data->executionQueue.empty() || !s_data->isRunning; });

			if (!s_data->isRunning)
			{
				break;
			}

			// #TODO_Ivar: This really shouldn't be here
			RHI::GraphicsContext::GetDevice()->GetDeviceQueue(RHI::QueueType::Graphics)->WaitForQueue();

			std::function<void()> executeFunction{};
			while (s_data->executionQueue.try_pop(executeFunction))
			{
				s_data->isExecuting = true;
				executeFunction();
				s_data->isExecuting = false;
			}

			s_data->waitForExecutionVariable.notify_one();
		}
	}
	void RenderGraphExecutionThread::ExecuteGraphs()
	{
		std::function<void()> executeFunction{};
		while (s_data->executionQueue.try_pop(executeFunction))
		{
			s_data->isExecuting = true;
			executeFunction();
			s_data->isExecuting = false;
		}
	}
}
