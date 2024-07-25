#include "vtpch.h"
#include "SkyAtmosphereTechnique.h"

namespace Volt
{
	SkyAtmosphereTechnique::SkyAtmosphereTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
		: m_renderGraph(renderGraph), m_blackboard(blackboard)
	{
	}

	SkyAtmosphereData SkyAtmosphereTechnique::Execute()
	{
		return SkyAtmosphereData();
	}

	RenderGraphResourceHandle SkyAtmosphereTechnique::RenderTransmittanceLUT()
	{
		return RenderGraphResourceHandle();
	}
}
