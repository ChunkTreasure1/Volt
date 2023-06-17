#include "vtpch.h"
#include "BloomTechnique.h"

#include "Volt/Rendering/FrameGraph/FrameGraph.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/ComputePipeline.h"

#include "Volt/Rendering/RenderTechniques/TechniqueStructures.h"

#include "Volt/Utility/ImageUtility.h"

namespace Volt
{
	BloomTechnique::BloomTechnique(const glm::uvec2& renderSize)
		: myRenderSize(renderSize)
	{
	}

	void BloomTechnique::AddBloomDownsamplePass(FrameGraph& frameGraph, Ref<ComputePipeline> downsamplePipeline)
	{
		const auto& luminosityData = frameGraph.GetBlackboard().Get<LuminosityData>();

		frameGraph.AddRenderPass("Bloom Downsample Pass",
		[&](FrameGraph::Builder& builder)
		{
			builder.WriteResource(luminosityData.luminosityImage);
			builder.SetIsComputePass();
		},

		[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			const auto& targetResource = resources.GetImageResource(luminosityData.luminosityImage);

			glm::uvec2 mipSize = myRenderSize;

			struct DownsampleData
			{
				glm::vec2 texelSize = 0.f;
				uint32_t mipLevel = 0;
			} downsampleData;

			const uint32_t currentIndex = commandBuffer->GetCurrentIndex();
			downsamplePipeline->Clear(currentIndex);

			auto samplerDescriptorSet = Renderer::GetBindlessData().globalDescriptorSets[Sets::SAMPLERS]->GetOrAllocateDescriptorSet(currentIndex);

			for (uint32_t i = 1; i < targetResource.image.lock()->GetSpecification().mips; i++)
			{
				downsampleData.texelSize.x = 1.f / glm::vec2(mipSize).x;
				downsampleData.texelSize.y = 1.f / glm::vec2(mipSize).y;
				downsampleData.mipLevel = i;

				downsamplePipeline->SetImage(targetResource.image.lock(), Sets::OTHER, 1, i - 1, ImageAccess::Write);
				downsamplePipeline->SetImage(targetResource.image.lock(), Sets::OTHER, 0, i, ImageAccess::Write);

				downsamplePipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
				downsamplePipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), samplerDescriptorSet, Sets::SAMPLERS);

				constexpr uint32_t threadCount = 8;
				const uint32_t dispatchX = (uint32_t)glm::ceil((float)mipSize.x / threadCount);
				const uint32_t dispatchY = (uint32_t)glm::ceil((float)mipSize.y / threadCount);

				downsamplePipeline->Dispatch(commandBuffer->GetCurrentCommandBuffer(), dispatchX, dispatchY, 1, currentIndex);

				VkImageSubresourceRange subresource{};
				subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				subresource.baseArrayLayer = 0;
				subresource.baseMipLevel = i;
				subresource.layerCount = 1;
				subresource.levelCount = 1;
				Utility::InsertImageMemoryBarrier(commandBuffer->GetCurrentCommandBuffer(), targetResource.image.lock()->GetHandle(), VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, subresource);

				mipSize /= 2u;
			}

			downsamplePipeline->ClearAllResources();
		});
	}

	void BloomTechnique::AddBloomUpsamplePass(FrameGraph& frameGraph, Ref<ComputePipeline> upsamplePipeline)
	{
		const auto& luminosityData = frameGraph.GetBlackboard().Get<LuminosityData>();

		frameGraph.AddRenderPass("Bloom Upsample Pass",
		[&](FrameGraph::Builder& builder)
		{
			builder.WriteResource(luminosityData.luminosityImage);
			builder.SetIsComputePass();
		},

		[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			const auto& targetResource = resources.GetImageResource(luminosityData.luminosityImage);
			const uint32_t currentIndex = commandBuffer->GetCurrentIndex();
			upsamplePipeline->Clear(currentIndex);

			glm::uvec2 mipSize = myRenderSize;

			auto samplerDescriptorSet = Renderer::GetBindlessData().globalDescriptorSets[Sets::SAMPLERS]->GetOrAllocateDescriptorSet(currentIndex);
			for (uint32_t i = targetResource.image.lock()->GetSpecification().mips - 1; i >= 1; i--)
			{
				mipSize.x = (uint32_t)glm::max(1.0f, glm::floor(float(targetResource.image.lock()->GetWidth()) / glm::pow(2.0f, float(i - 1))));
				mipSize.y = (uint32_t)glm::max(1.0f, glm::floor(float(targetResource.image.lock()->GetHeight()) / glm::pow(2.0f, float(i - 1))));

				upsamplePipeline->SetImage(targetResource.image.lock(), Sets::OTHER, 0, i - 1, ImageAccess::Write);
				upsamplePipeline->SetImage(targetResource.image.lock(), Sets::OTHER, 1, i, ImageAccess::Write);
				upsamplePipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
				upsamplePipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), samplerDescriptorSet, Sets::SAMPLERS);

				constexpr float filterRadius = 0.005f;
				upsamplePipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &filterRadius, sizeof(float));

				constexpr uint32_t threadCount = 8;
				const uint32_t dispatchX = (uint32_t)glm::ceil((float)mipSize.x / threadCount);
				const uint32_t dispatchY = (uint32_t)glm::ceil((float)mipSize.y / threadCount);

				upsamplePipeline->Dispatch(commandBuffer->GetCurrentCommandBuffer(), dispatchX, dispatchY, 1, currentIndex);

				VkImageSubresourceRange subresource{};
				subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

				subresource.baseArrayLayer = 0;
				subresource.baseMipLevel = i - 1;
				subresource.layerCount = 1;
				subresource.levelCount = 1;
				Utility::InsertImageMemoryBarrier(commandBuffer->GetCurrentCommandBuffer(), targetResource.image.lock()->GetHandle(), VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, subresource);
			}

			upsamplePipeline->ClearAllResources();
		});
	}
	
	void BloomTechnique::AddBloomCompositePass(FrameGraph& frameGraph, Ref<ComputePipeline> compositePipeline)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& luminosityData = frameGraph.GetBlackboard().Get<LuminosityData>();

		frameGraph.AddRenderPass("Bloom Composite Pass",
			[&](FrameGraph::Builder& builder)
		{
			builder.WriteResource(skyboxData.outputImage);
			builder.ReadResource(luminosityData.luminosityImage);
			builder.SetIsComputePass();
		},

		[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			const auto& outputImageResource = resources.GetImageResource(skyboxData.outputImage);
			const auto& bloomSrcImageResource = resources.GetImageResource(luminosityData.luminosityImage);

			compositePipeline->SetImage(outputImageResource.image.lock(), Sets::OTHER, 0, ImageAccess::Write);
			compositePipeline->SetImage(bloomSrcImageResource.image.lock(), Sets::OTHER, 1, ImageAccess::Read);
			compositePipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			constexpr float bloomStrength = 0.5f;
			compositePipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &bloomStrength, sizeof(float));

			constexpr uint32_t threadCount = 8;
			const uint32_t dispatchX = std::max(1u, (outputImageResource.image.lock()->GetWidth() / threadCount) + 1);
			const uint32_t dispatchY = std::max(1u, (outputImageResource.image.lock()->GetHeight() / threadCount) + 1);

			Renderer::DispatchComputePipeline(commandBuffer, compositePipeline, dispatchX, dispatchY, 1);
		});
	}
}
