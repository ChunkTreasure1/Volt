#include "vtpch.h"
#include "FXAATechnique.h"

#include "Volt/Rendering/RenderTechniques/TechniqueStructures.h"
#include "Volt/Rendering/FrameGraph/FrameGraph.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/ComputePipeline.h"
#include "Volt/Rendering/CommandBuffer.h"

namespace Volt
{
	struct FXAAData
	{
		FrameGraphResourceHandle tempColor;
	};

	FXAATechnique::FXAATechnique(const glm::uvec2& renderSize)
		: myRenderSize(renderSize)
	{
	}

	void FXAATechnique::AddFXAAPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();

		frameGraph.GetBlackboard().Add<FXAAData>() = frameGraph.AddRenderPass<FXAAData>("FXAA Pass",
			[&](FrameGraph::Builder& builder, FXAAData& data)
		{
			data.tempColor = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, "FXAA Output", ImageUsage::Storage });

			builder.ReadResource(skyboxData.outputImage);
			builder.SetIsComputePass();
		},

			[=](const FXAAData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			struct PushConstantData
			{
				glm::vec2 inverseViewportSize;
			} pushConstants;

			pushConstants.inverseViewportSize = { 1.f / myRenderSize.x, 1.f / myRenderSize.y };

			const auto& currentColorResource = resources.GetImageResource(skyboxData.outputImage);
			const auto& resultResource = resources.GetImageResource(data.tempColor);

			pipeline->SetImage(resultResource.image, Sets::OTHER, 0, ImageAccess::Write);
			pipeline->SetImage(currentColorResource.image, Sets::OTHER, 1, ImageAccess::Read);

			pipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			const auto samplerDescriptorSet = Renderer::GetBindlessData().globalDescriptorSets[Sets::SAMPLERS]->GetOrAllocateDescriptorSet(commandBuffer->GetCurrentIndex());
			pipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), samplerDescriptorSet, Sets::SAMPLERS);

			pipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &pushConstants, sizeof(PushConstantData));

			const uint32_t dispatchX = (myRenderSize.x + 8 - 1) / 8;
			const uint32_t dispatchY = (myRenderSize.y + 8 - 1) / 8;

			Renderer::DispatchComputePipeline(commandBuffer, pipeline, dispatchX, dispatchY, 1);

			pipeline->ClearAllResources();
		});
	}

	void FXAATechnique::AddFXAAApplyPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& fxaaMainData = frameGraph.GetBlackboard().Get<FXAAData>();

		frameGraph.AddRenderPass("FXAA Apply Pass",
			[&](FrameGraph::Builder& builder)
		{
			builder.ReadResource(fxaaMainData.tempColor);
			builder.WriteResource(skyboxData.outputImage);

			builder.SetIsComputePass();
		},

			[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			const auto& tempColorResource = resources.GetImageResource(fxaaMainData.tempColor);
			const auto& resultResource = resources.GetImageResource(skyboxData.outputImage);

			pipeline->SetImage(tempColorResource.image, Sets::OTHER, 0, ImageAccess::Read);
			pipeline->SetImage(resultResource.image, Sets::OTHER, 1, ImageAccess::Write);

			pipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			const uint32_t dispatchX = (myRenderSize.x + 8 - 1) / 8;
			const uint32_t dispatchY = (myRenderSize.y + 8 - 1) / 8;

			Renderer::DispatchComputePipeline(commandBuffer, pipeline, dispatchX, dispatchY, 1);

			pipeline->ClearAllResources();
		});
	}
}
