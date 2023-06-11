#include "vtpch.h"
#include "SSRTechnique.h"

#include "Volt/Rendering/RenderTechniques/TechniqueStructures.h"

#include "Volt/Rendering/FrameGraph/FrameGraph.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/ComputePipeline.h"

namespace Volt
{
	struct SSROutput
	{
		FrameGraphResourceHandle output;
	};

	SSRTechnique::SSRTechnique(const GlobalDescriptorMap& descriptorMap, const glm::uvec2& renderSize)
		: myGlobalDescriptorMap(descriptorMap), myRenderSize(renderSize)
	{
	}

	void SSRTechnique::AddSSRPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline)
	{
		const auto& preDepthData = frameGraph.GetBlackboard().Get<PreDepthData>();
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();

		frameGraph.GetBlackboard().Add<SSROutput>() = frameGraph.AddRenderPass<SSROutput>("SSR Pass", 
			[&](FrameGraph::Builder& builder, SSROutput& data) 
		{
			data.output = builder.CreateTexture({ ImageFormat::RGBA16F, { myRenderSize }, "SSR Output", ImageUsage::Storage });

			builder.ReadResource(preDepthData.preDepth);
			builder.ReadResource(preDepthData.viewNormals);
			builder.ReadResource(skyboxData.outputImage);
			builder.SetIsComputePass();

			builder.SetHasSideEffect();
		}, 
			[=](const SSROutput& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			Renderer::BeginSection(commandBuffer, "SSR Pass", TO_NORMALIZEDRGB(6, 71, 24));

			const auto& preDepthResource = resources.GetImageResource(preDepthData.preDepth);
			const auto& viewNormalsResource = resources.GetImageResource(preDepthData.viewNormals);
			const auto& sceneColorResource = resources.GetImageResource(skyboxData.outputImage);
			const auto& outputResource = resources.GetImageResource(data.output);

			pipeline->SetImage(outputResource.image.lock(), Sets::OTHER, 0, ImageAccess::Write);
			pipeline->SetImage(preDepthResource.image.lock(), Sets::OTHER, 1, ImageAccess::Read);
			pipeline->SetImage(viewNormalsResource.image.lock(), Sets::OTHER, 2, ImageAccess::Read);
			pipeline->SetImage(sceneColorResource.image.lock(), Sets::OTHER, 3, ImageAccess::Read);
			pipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			pipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myGlobalDescriptorMap.at(Sets::RENDERER_BUFFERS)->GetOrAllocateDescriptorSet(commandBuffer->GetCurrentIndex()), Sets::RENDERER_BUFFERS);

			const uint32_t dispatchX = (myRenderSize.x + 8 - 1) / 8;
			const uint32_t dispatchY = (myRenderSize.y + 8 - 1) / 8;

			Renderer::DispatchComputePipeline(commandBuffer, pipeline, dispatchX, dispatchY, 1);
			Renderer::EndSection(commandBuffer);

			pipeline->ClearAllResources();
		});
	}
	
	void SSRTechnique::AddSSRComposite(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline)
	{
	}
}
