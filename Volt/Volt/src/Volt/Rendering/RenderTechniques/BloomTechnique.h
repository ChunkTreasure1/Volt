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
		BloomTechnique(const gem::vec2ui& renderSize);

		void AddBloomDownsamplePass(FrameGraph& frameGraph, Ref<ComputePipeline> downsamplePipeline);
		void AddBloomUpsamplePass(FrameGraph& frameGraph, Ref<ComputePipeline> upsamplePipeline);
		void AddBloomCompositePass(FrameGraph& frameGraph, Ref<ComputePipeline> compositePipeline);

	private:
		gem::vec2ui myRenderSize;
	};
}
