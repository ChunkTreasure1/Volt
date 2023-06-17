#pragma once

#include "Volt/Rendering/RendererCommon.h"


namespace Volt
{
	class CommandBuffer;
	class ComputePipeline;
	class FrameGraph;

	class BloomTechnique
	{
	public:
		BloomTechnique(const glm::uvec2& renderSize);

		void AddBloomDownsamplePass(FrameGraph& frameGraph, Ref<ComputePipeline> downsamplePipeline);
		void AddBloomUpsamplePass(FrameGraph& frameGraph, Ref<ComputePipeline> upsamplePipeline);
		void AddBloomCompositePass(FrameGraph& frameGraph, Ref<ComputePipeline> compositePipeline);

	private:
		glm::uvec2 myRenderSize;
	};
}
