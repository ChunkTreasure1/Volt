#pragma once

namespace Volt
{
	class CommandBuffer;
	class GlobalDescriptorSet;
	class Material;
	class FrameGraph;

	using GlobalDescriptorMap = std::unordered_map<uint32_t, Ref<GlobalDescriptorSet>>;

	class SSRTechnique
	{
	public:
		SSRTechnique(const GlobalDescriptorMap& descriptorMap, const glm::uvec2& renderSize);

		void AddSSRPass(FrameGraph& frameGraph, Ref<Material> material);
		void AddSSRCompositePass(FrameGraph& frameGraph, Ref<Material> material);

	private:
		const GlobalDescriptorMap& myGlobalDescriptorMap;
		const glm::uvec2& myRenderSize;
	};
}
