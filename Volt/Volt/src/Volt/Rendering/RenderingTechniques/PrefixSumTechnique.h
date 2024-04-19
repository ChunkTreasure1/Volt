#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"

namespace Volt
{
	class RenderGraph;
	class RenderGraphBlackboard;

	class PrefixSumTechnique
	{
	public:
		PrefixSumTechnique(RenderGraph& rg);
		void Execute(RenderGraphResourceHandle inputBuffer, RenderGraphResourceHandle outputBuffer, const uint32_t valueCount);

		RenderGraph& m_renderGraph;
	};
}
