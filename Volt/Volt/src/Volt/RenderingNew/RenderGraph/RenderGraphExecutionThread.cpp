#include "vtpch.h"
#include "RenderGraphExecutionThread.h"

#include "Volt/Core/Application.h"
#include "Volt/Core/Threading/ThreadSafeQueue.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraph.h"

#include "Volt/Core/Profiling.h"

#include "Volt/Utility/FunctionQueue.h"
#include "Volt/Platform/ThreadUtility.h"

#include <VoltRHI/Graphics/Swapchain.h>

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

		std::vector<FunctionQueue> deletionQueue;
		uint32_t currentFrame = 0;
		uint32_t framesInFlight = 0;
	};

	inline static Scope<RenderGraphThreadData> s_data;

	void RenderGraphExecutionThread::Initialize()
	{
		s_data = CreateScope<RenderGraphThreadData>();
		s_data->isRunning = true;

		const uint32_t frameCount = Application::Get().GetWindow().GetSwapchain().GetFramesInFlight();
		s_data->deletionQueue.resize(frameCount);
		s_data->framesInFlight = frameCount;

		InitializeThread();
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
			rg.ExecuteInternal();
		});

		s_data->executeVariable.notify_one();
	}

	void RenderGraphExecutionThread::WaitForFinishedExecution()
	{
		std::unique_lock lock{ s_data->mutex };
		s_data->waitForExecutionVariable.wait(lock, []() { return !s_data->isExecuting.load() && s_data->executionQueue.empty(); });
	}

	void RenderGraphExecutionThread::DestroyRenderGraphResource(std::function<void()>&& function)
	{
		if (!s_data)
		{
			function();
			return;
		}

		s_data->deletionQueue.at(s_data->currentFrame).Push(std::move(function));
	}

	void RenderGraphExecutionThread::InitializeThread()
	{
		s_data->executionThread = CreateScope<std::thread>(&RenderGraphExecutionThread::RT_ExecuteGraph);
		SetThreadName(s_data->executionThread->native_handle(), "RenderGraphExecutionThread");
	}

	void RenderGraphExecutionThread::ShutdownThread()
	{
		s_data->executeVariable.notify_one();
		s_data->executionThread->join();
	}

	void RenderGraphExecutionThread::RT_ExecuteGraph()
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

			s_data->deletionQueue.at(s_data->currentFrame).Flush();

			std::function<void()> executeFunction{};
			if (s_data->executionQueue.try_pop(executeFunction))
			{
				s_data->isExecuting = true;
				executeFunction();
				s_data->isExecuting = false;
				s_data->waitForExecutionVariable.notify_one();
			}

			s_data->currentFrame = (s_data->currentFrame + 1) % s_data->framesInFlight;
		}
	}
}