#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Rendering/FrameGraph/FrameGraphResourceHandle.h"

#include <GEM/gem.h>

namespace Volt
{
	class CommandBuffer;
	class ComputePipeline;
	class FrameGraph;

	class FXAATechnique
	{
	public:
		FXAATechnique(const gem::vec2ui& renderSize);

		void AddFXAAPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline);
		void AddFXAAApplyPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline);

	private:
		gem::vec2ui myRenderSize;
	};
}
