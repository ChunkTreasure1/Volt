#pragma once

#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphResourceHandle.h"

namespace Volt
{
	namespace RHI
	{
		class ComputePipeline;
	}

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
