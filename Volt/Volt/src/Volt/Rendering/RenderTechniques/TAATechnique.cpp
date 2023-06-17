#include "vtpch.h"
#include "TAATechnique.h"

#include "Volt/Rendering/FrameGraph/FrameGraph.h"

#include "Volt/Rendering/RenderTechniques/TechniqueStructures.h"

#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/ComputePipeline.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/GlobalDescriptorSet.h"

#include "Volt/Utility/Noise.h"

namespace Volt
{
	struct TAAData
	{
		FrameGraphResourceHandle tempColor;
		FrameGraphResourceHandle historyColor;
		FrameGraphResourceHandle historyDepth;
		FrameGraphResourceHandle currentColor;
	};

	TAATechnique::TAATechnique(Ref<Camera> camera, const gem::mat4& reprojectionMatrix, const gem::vec2ui renderSize, uint64_t frameIndex, const gem::vec2& jitterDelta)
		: myCamera(camera), myRenderSize(renderSize), myReprojectionMatrix(reprojectionMatrix), myJitterDelta(jitterDelta), myFrameIndex(frameIndex)
	{
	}

	void TAATechnique::AddGenerateMotionVectorsPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline, Ref<GlobalDescriptorSet> rendererBuffersSet, FrameGraphResourceHandle srcDepthHandle)
	{
		frameGraph.GetBlackboard().Add<MotionVectorData>() = frameGraph.AddRenderPass<MotionVectorData>("Generate Motion Vectors",
		[&](FrameGraph::Builder& builder, MotionVectorData& data)
		{
			data.motionVectors = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, "Motion Vectors", ImageUsage::AttachmentStorage });
			data.currentDepth = builder.CreateTexture({ ImageFormat::R32F, myRenderSize, "TAA Depth", ImageUsage::Storage });

			builder.ReadResource(srcDepthHandle);
			builder.SetIsComputePass();
		},

		[=](const MotionVectorData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			struct PushConstantData 
			{
				gem::mat4 reprojectionMatrix;
				gem::vec2 viewportSize2x;
				gem::vec2 jitter;
			} pushConstants;

			pushConstants.reprojectionMatrix = myReprojectionMatrix;
			pushConstants.viewportSize2x = { 2.f / myRenderSize.x, 2.f / myRenderSize.y };
			pushConstants.jitter = myJitterDelta;

			const auto& srcDepthResource = resources.GetImageResource(srcDepthHandle);

			const auto& motionVectorResource = resources.GetImageResource(data.motionVectors);
			const auto& currentDepthResource = resources.GetImageResource(data.currentDepth);

			pipeline->SetImage(srcDepthResource.image.lock(), Sets::OTHER, 0, ImageAccess::Read);

			pipeline->SetImage(motionVectorResource.image.lock(), Sets::OTHER, 1, ImageAccess::Read);
			pipeline->SetImage(currentDepthResource.image.lock(), Sets::OTHER, 2, ImageAccess::Write);

			pipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
			pipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), rendererBuffersSet->GetOrAllocateDescriptorSet(commandBuffer->GetCurrentIndex()), Sets::RENDERER_BUFFERS);
			pipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &pushConstants, sizeof(PushConstantData));

			const uint32_t dispatchX = (myRenderSize.x + 8 - 1) / 8;
			const uint32_t dispatchY = (myRenderSize.y + 8 - 1) / 8;

			Renderer::DispatchComputePipeline(commandBuffer, pipeline, dispatchX, dispatchY, 1);

			pipeline->ClearAllResources();
		});
	}

	void TAATechnique::AddTAAPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline, Ref<Image2D> colorHistory, FrameGraphResourceHandle srcDepthHandle)
	{
		const auto& motionVectorData = frameGraph.GetBlackboard().Get<MotionVectorData>();
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();

		frameGraph.GetBlackboard().Add<TAAData>() = frameGraph.AddRenderPass<TAAData>("TAA Pass",
			[&](FrameGraph::Builder& builder, TAAData& data)
		{
			data.tempColor = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, "TAA Output", ImageUsage::Storage });
			data.historyColor = builder.AddExternalTexture(colorHistory, "Color History", ClearMode::Load);

			builder.ReadResource(data.historyColor);
			builder.ReadResource(skyboxData.outputImage);
			builder.ReadResource(srcDepthHandle);

			builder.SetIsComputePass();
		},

			[=](const TAAData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			struct PushConstantData
			{
				gem::mat4 reprojectionMatrix;
				gem::vec2 texelSize;

			} pushConstants;

			pushConstants.texelSize = { 1.f / myRenderSize.x, 1.f / myRenderSize.y };
			pushConstants.reprojectionMatrix = myReprojectionMatrix;

			const auto& depthResource = resources.GetImageResource(srcDepthHandle);
			const auto& currentColorResource = resources.GetImageResource(skyboxData.outputImage);
			const auto& historyColorResource = resources.GetImageResource(data.historyColor);
			const auto& resultResource = resources.GetImageResource(data.tempColor);

			pipeline->SetImage(resultResource.image.lock(), Sets::OTHER, 0, ImageAccess::Write);

			pipeline->SetImage(depthResource.image.lock(), Sets::OTHER, 1, ImageAccess::Read);
			pipeline->SetImage(currentColorResource.image.lock(), Sets::OTHER, 2, ImageAccess::Read);
			pipeline->SetImage(historyColorResource.image.lock(), Sets::OTHER, 3, ImageAccess::Read);

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
	
	void TAATechnique::AddTAAApplyPass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& taaMainData = frameGraph.GetBlackboard().Get<TAAData>();

		frameGraph.AddRenderPass("TAA Apply Pass",
			[&](FrameGraph::Builder& builder)
		{
			builder.ReadResource(taaMainData.tempColor);
			builder.WriteResource(skyboxData.outputImage);

			builder.SetIsComputePass();
		},

			[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			const auto& tempColorResource = resources.GetImageResource(taaMainData.tempColor);
			const auto& resultResource = resources.GetImageResource(skyboxData.outputImage);

			pipeline->SetImage(tempColorResource.image.lock(), Sets::OTHER, 0, ImageAccess::Read);
			pipeline->SetImage(resultResource.image.lock(), Sets::OTHER, 1, ImageAccess::Write);

			pipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			const uint32_t dispatchX = (myRenderSize.x + 8 - 1) / 8;
			const uint32_t dispatchY = (myRenderSize.y + 8 - 1) / 8;

			Renderer::DispatchComputePipeline(commandBuffer, pipeline, dispatchX, dispatchY, 1);

			pipeline->ClearAllResources();
		});
	}
}
