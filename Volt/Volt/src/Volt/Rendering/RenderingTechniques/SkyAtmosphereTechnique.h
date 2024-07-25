#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"

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

		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
