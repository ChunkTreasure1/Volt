#pragma once

#include <functional>

namespace Volt
{
	class RenderGraph;

	class RenderGraphExecutionThread
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void ExecuteRenderGraph(RenderGraph&& renderGraph);
		static void WaitForFinishedExecution();

	private:
		static void InitializeThread();
		static void ShutdownThread();

		static void RT_ExecuteGraph();

		~RenderGraphExecutionThread() = delete;
		RenderGraphExecutionThread() = delete;
	};
}
