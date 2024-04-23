#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"

namespace Volt
{
	struct LightCullingData
	{
		RenderGraphResourceHandle visiblePointLightsBuffer;
		RenderGraphResourceHandle visibleSpotLightsBuffer;
	};

	class RenderGraph;
	class RenderGraphBlackboard;

	class LightCullingTechnique
	{
	public:
		LightCullingTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		LightCullingData Execute(const glm::uvec2& renderSize);

	private:
		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
