#pragma once

#include "Volt/Rendering/RendererCommon.h"

namespace Volt
{
	class CommandBuffer;
	class ComputePipeline;
	class Camera;
	class FrameGraph;
	class RenderPipeline;
	class GlobalDescriptorSet;

	using GlobalDescriptorMap = std::unordered_map<uint32_t, Ref<GlobalDescriptorSet>>;

	class OutlineTechnique
	{
	public:
		OutlineTechnique(const gem::vec2ui& renderSize, const GlobalDescriptorMap& descriptorMap);

		void AddOutlineGeometryPass(FrameGraph& frameGraph, Ref<RenderPipeline> pipeline, const std::vector<SubmitCommand>& outlineCmds);
		void AddJumpFloodInitPass(FrameGraph& frameGraph, Ref<Material> material);
		void AddJumpFloodPass(FrameGraph& frameGraph, Ref<Material> material);
		void AddOutlineCompositePass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline);

	private:
		const GlobalDescriptorMap& myGlobalDescriptorMap;

		gem::vec2ui myRenderSize;
	};
}
