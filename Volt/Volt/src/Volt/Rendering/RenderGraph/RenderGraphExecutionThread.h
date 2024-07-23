#pragma once

#include <functional>

namespace Volt
{
	class RenderGraph;

	class RenderGraphExecutionThread
	{
	public:
		enum class ExecutionMode
		{
			Multithreaded,
			Singlethreaded
		};

		static void Initialize(ExecutionMode executionMode);
		static void Shutdown();

		static void ExecuteRenderGraph(RenderGraph&& renderGraph);
		static void WaitForFinishedExecution();

	private:
		static void InitializeThread();
		static void ShutdownThread();

		static void RT_ExecuteGraphs();

		// Note: Used for single threaded graph execution
		static void ExecuteGraphs();

		~RenderGraphExecutionThread() = delete;
		RenderGraphExecutionThread() = delete;
	};
}
