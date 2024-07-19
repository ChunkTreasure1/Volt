#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"

namespace Volt
{
	struct DrawCullingData
	{
		RenderGraphResourceHandle countCommandBuffer;
		RenderGraphResourceHandle taskCommandsBuffer;
	};

	class RenderGraph;
	class RenderGraphBlackboard;

	class CullingTechnique
	{
	public:
		CullingTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		DrawCullingData Execute(uint32_t drawCommandCount, uint32_t meshletCount);

	private:
		DrawCullingData AddDrawCallCullingPass(uint32_t drawCommandCount, uint32_t meshletCount);
		void AddTaskSubmitSetupPass(const DrawCullingData& data);

		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
