#pragma once

namespace Volt
{
	class CommandBuffer;
	class GlobalDescriptorSet;
	class ComputePipeline;
	class FrameGraph;

	using GlobalDescriptorMap = std::unordered_map<uint32_t, Ref<GlobalDescriptorSet>>;

	class SSRTechnique
	{
	public:
		SSRTechnique(const GlobalDescriptorMap& descriptorMap, const glm::uvec2& renderSize);

		void AddSSRPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline);
		void AddSSRComposite(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline);

	private:
		const GlobalDescriptorMap& myGlobalDescriptorMap;
		const glm::uvec2& myRenderSize;
	};
}
