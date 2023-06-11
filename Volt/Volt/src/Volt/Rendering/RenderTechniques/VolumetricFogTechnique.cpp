#include "vtpch.h"
#include "VolumetricFogTechnique.h"

#include "Volt/Rendering/RenderTechniques/TechniqueStructures.h"

#include "Volt/Rendering/FrameGraph/FrameGraph.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/ComputePipeline.h"

#include "Volt/Rendering/SceneRenderer.h"

namespace Volt
{
	struct InjectionData
	{
		FrameGraphResourceHandle injectionImage;
	};

	VolumetricFogTechnique::VolumetricFogTechnique(const glm::uvec2& renderSize, const GlobalDescriptorMap& descriptorMap, const VolumetricFogSettings& settings, bool dirLightShadows)
		: myRenderSize(renderSize), myGlobalDescriptorMap(descriptorMap), mySettings(settings), myDirLightShadows(dirLightShadows)
	{
	}

	void VolumetricFogTechnique::AddInjectionPass(FrameGraph& frameGraph, Ref<ComputePipeline> injectionPipeline, Ref<Image3D> resultImage)
	{
		const auto& dirShadowData = frameGraph.GetBlackboard().Get<DirectionalShadowData>();

		frameGraph.GetBlackboard().Add<InjectionData>() = frameGraph.AddRenderPass<InjectionData>("Fog Injection Pass",
			[&](FrameGraph::Builder& builder, InjectionData& data)
		{
			if (resultImage)
			{
				data.injectionImage = builder.AddExternalTexture(resultImage, "Injection image");
			}

			builder.WriteResource(data.injectionImage);

			if (myDirLightShadows)
			{
				builder.ReadResource(dirShadowData.shadowMap);
			}

			if (frameGraph.GetBlackboard().Contains<SpotLightShadowData>())
			{
				const auto& spotlightShadowData = frameGraph.GetBlackboard().Get<SpotLightShadowData>();
				for (const auto& lightHandle : spotlightShadowData.spotLightShadows)
				{
					builder.ReadResource(lightHandle);
				}
			}
		},

		[=](const InjectionData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			struct PushConstants
			{
				float anisotropy = 0.f;
				float density = 0.04f;
				float globalDensity = 0.f;
				float padding;
				glm::vec4 sizeMultiplier = 1.f;
				glm::vec4 globalColor = { 1.f };
			} pushConstants;

			pushConstants.anisotropy = mySettings.anisotropy;
			pushConstants.density = mySettings.density;
			pushConstants.globalDensity = mySettings.globalDensity;
			pushConstants.globalColor = glm::vec4(mySettings.globalColor, 1.f);
			pushConstants.sizeMultiplier.x = static_cast<float>(myRenderSize.x) / static_cast<float>(SceneRenderer::VOLUMETRIC_FOG_WIDTH);
			pushConstants.sizeMultiplier.y = static_cast<float>(myRenderSize.y) / static_cast<float>(SceneRenderer::VOLUMETRIC_FOG_HEIGHT);

			Renderer::BeginSection(commandBuffer, "Fog Injection Pass", TO_NORMALIZEDRGB(6, 71, 24));

			const auto& outputInjectionResource = resources.GetImageResource(data.injectionImage);

			const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

			injectionPipeline->SetImage(outputInjectionResource.image3D.lock(), Sets::OTHER, 0, ImageAccess::Write);
			injectionPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
			injectionPipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &pushConstants, sizeof(PushConstants));

			// Bind descriptor sets
			{
				injectionPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myGlobalDescriptorMap.at(Sets::RENDERER_BUFFERS)->GetOrAllocateDescriptorSet(currentIndex), Sets::RENDERER_BUFFERS);
				injectionPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myGlobalDescriptorMap.at(Sets::PBR_RESOURCES)->GetOrAllocateDescriptorSet(currentIndex), Sets::PBR_RESOURCES);
			}

			constexpr uint32_t localSizeX = 16;
			constexpr uint32_t localSizeY = 16;
			constexpr uint32_t localSizeZ = 1;

			const uint32_t groupX = (uint32_t)std::ceil((float)SceneRenderer::VOLUMETRIC_FOG_WIDTH / (float)localSizeX);
			const uint32_t groupY = (uint32_t)std::ceil((float)SceneRenderer::VOLUMETRIC_FOG_HEIGHT / (float)localSizeY);
			const uint32_t groupZ = (uint32_t)std::ceil((float)SceneRenderer::VOLUMETRIC_FOG_DEPTH / (float)localSizeZ);

			Renderer::DispatchComputePipeline(commandBuffer, injectionPipeline, groupX, groupY, groupZ);
			Renderer::EndSection(commandBuffer);
		});
	}

	void VolumetricFogTechnique::AddRayMarchPass(FrameGraph& frameGraph, Ref<ComputePipeline> rayMarchPipeline, Ref<Image3D> resultImage)
	{
		const auto& injectionData = frameGraph.GetBlackboard().Get<InjectionData>();

		frameGraph.GetBlackboard().Add<VolumetricFogData>() = frameGraph.AddRenderPass<VolumetricFogData>("Fog Ray March Pass",
			[&](FrameGraph::Builder& builder, VolumetricFogData& data)
		{
			if (resultImage)
			{
				data.rayMarched = builder.AddExternalTexture(resultImage, "Ray March image");
			}

			builder.WriteResource(data.rayMarched);
			builder.ReadResource(injectionData.injectionImage);
		},

		[=](const VolumetricFogData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			Renderer::BeginSection(commandBuffer, "Fog Ray March Pass", TO_NORMALIZEDRGB(6, 71, 24));

			const auto& outputRayMarchedResource = resources.GetImageResource(data.rayMarched);
			const auto& injectionResource = resources.GetImageResource(injectionData.injectionImage);

			const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

			rayMarchPipeline->SetImage(outputRayMarchedResource.image3D.lock(), Sets::OTHER, 0, ImageAccess::Write);
			rayMarchPipeline->SetImage(injectionResource.image3D.lock(), Sets::OTHER, 1, ImageAccess::Write);

			rayMarchPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			// Bind descriptor sets
			{
				rayMarchPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myGlobalDescriptorMap.at(Sets::RENDERER_BUFFERS)->GetOrAllocateDescriptorSet(currentIndex), Sets::RENDERER_BUFFERS);
			}

			constexpr uint32_t localSizeX = 8;
			constexpr uint32_t localSizeY = 8;
			constexpr uint32_t localSizeZ = 1;

			const uint32_t groupX = (uint32_t)std::ceil((float)SceneRenderer::VOLUMETRIC_FOG_WIDTH / (float)localSizeX);
			const uint32_t groupY = (uint32_t)std::ceil((float)SceneRenderer::VOLUMETRIC_FOG_HEIGHT / (float)localSizeY);
			const uint32_t groupZ = 1;

			Renderer::DispatchComputePipeline(commandBuffer, rayMarchPipeline, groupX, groupY, groupZ);
			Renderer::EndSection(commandBuffer);
		});
	}
}
