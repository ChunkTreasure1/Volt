#pragma once

#include <RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h>

namespace Volt
{
	class RenderGraph;
	class RenderGraphBlackboard;

	struct SkyAtmosphereData
	{

	};

	class SkyAtmosphereTechnique
	{
	public:
		SkyAtmosphereTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		SkyAtmosphereData Execute();

	private:
		RenderGraphResourceHandle RenderTransmittanceLUT();
		RenderGraphResourceHandle SetupAtmosphereUniformBuffer();

		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
