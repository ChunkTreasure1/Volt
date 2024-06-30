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
		LightCullingData Execute();

		inline static constexpr uint32_t TILE_SIZE = 16;

	private:
		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
