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
	class Image3D;

	using GlobalDescriptorMap = std::unordered_map<uint32_t, Ref<GlobalDescriptorSet>>;

	struct VolumetricFogSettings;

	class VolumetricFogTechnique
	{
	public:
		VolumetricFogTechnique(const glm::uvec2& renderSize, const GlobalDescriptorMap& descriptorMap, const VolumetricFogSettings& settings, bool dirLightShadows);

		void AddInjectionPass(FrameGraph& frameGraph, Ref<ComputePipeline> injectionPipeline, Ref<Image3D> resultImage);
		void AddRayMarchPass(FrameGraph& frameGraph, Ref<ComputePipeline> rayMarchPipeline, Ref<Image3D> resultImage);

	private:
		const GlobalDescriptorMap& myGlobalDescriptorMap;
		const VolumetricFogSettings& mySettings;
		bool myDirLightShadows = false;

		glm::uvec2 myRenderSize;
	};
}
