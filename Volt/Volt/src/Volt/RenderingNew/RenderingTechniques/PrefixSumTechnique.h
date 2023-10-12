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
		PrefixSumTechnique(RenderGraph& rg, Ref<RHI::ComputePipeline> prefixSumPipeline);
		void Execute(RenderGraphResourceHandle inputBuffer, RenderGraphResourceHandle outputBuffer, const uint32_t valueCount);

		RenderGraph& m_renderGraph;
		Ref<RHI::ComputePipeline> m_pipeline;
	};
}
