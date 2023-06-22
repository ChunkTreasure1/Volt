#pragma once

#include "Volt/Core/Base.h"
#include "Volt/Rendering/FrameGraph/FrameGraphResourceHandle.h"

#include <glm/glm.hpp>

namespace Volt
{
	class CommandBuffer;
	class ComputePipeline;
	class FrameGraph;

	class FXAATechnique
	{
	public:
		FXAATechnique(const glm::uvec2& renderSize);

		void AddFXAAPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline);
		void AddFXAAApplyPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline);

	private:
		glm::uvec2 myRenderSize;
	};
}
