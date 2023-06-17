#include "vtpch.h"
#include "GTAOTechnique.h"

#include "Volt/Rendering/FrameGraph/FrameGraph.h"
#include "Volt/Rendering/RenderTechniques/TechniqueStructures.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/ComputePipeline.h"
#include "Volt/Rendering/Camera/Camera.h"


#include "Volt/Rendering/Texture/Image2D.h"

namespace Volt
{
	struct PrefilterDepthData
	{
		FrameGraphResourceHandle prefilteredDepth;
	};

	struct GTAOData
	{
		FrameGraphResourceHandle aoOutput;
		FrameGraphResourceHandle edgesOutput;
	};

	GTAOTechnique::GTAOTechnique(Ref<Camera> camera, const glm::uvec2& renderSize, uint64_t frameIndex, const GTAOSettings& settings)
		: myRenderSize(renderSize), myFrameIndex(frameIndex)
	{
		// Setup constants
		{
			GTAOConstants constants{};
			constants.ViewportSize = myRenderSize;
			constants.ViewportPixelSize = { 1.f / myRenderSize.x, 1.f / myRenderSize.y };

			const auto& projectionMatrix = camera->GetProjection();

			float depthLinearizeMul = (-projectionMatrix[3][2]);
			float depthLinearizeAdd = (projectionMatrix[2][2]);

			// correct the handedness issue
			if (depthLinearizeMul * depthLinearizeAdd < 0.f)
			{
				depthLinearizeAdd = -depthLinearizeAdd;
			}

			constants.DepthUnpackConsts = { depthLinearizeMul, depthLinearizeAdd };

			const float tanHalfFovY = 1.f / (projectionMatrix[1][1]);
			const float tanHalfFovX = 1.f / (projectionMatrix[0][0]);

			constants.CameraTanHalfFOV = { tanHalfFovX, tanHalfFovY };
			constants.NDCToViewMul = { constants.CameraTanHalfFOV.x * 2.f, constants.CameraTanHalfFOV.y * -2.f };
			constants.NDCToViewAdd = { constants.CameraTanHalfFOV.x * -1.f, constants.CameraTanHalfFOV.y * 1.f };
			constants.NDCToViewMul_x_PixelSize = { constants.NDCToViewMul.x * constants.ViewportPixelSize.x, constants.NDCToViewMul.y * constants.ViewportPixelSize.y };

			///// Settings /////
			constants.EffectRadius = settings.radius;
			constants.EffectFalloffRange = settings.falloffRange;
			constants.DenoiseBlurBeta = 1.2f; // 1 denoise pass
			constants.RadiusMultiplier = settings.radiusMultiplier;
			constants.SampleDistributionPower = 2.f;
			constants.ThinOccluderCompensation = 0.f;
			constants.FinalValuePower = settings.finalValuePower;
			constants.DepthMIPSamplingOffset = 3.3f;
			constants.NoiseIndex = frameIndex % 64;
			constants.Padding0 = 0;

			myConstants = constants;
		}
	}

	void GTAOTechnique::AddPrefilterDepthPass(FrameGraph& frameGraph, Ref<ComputePipeline> prefilterDepthPipeline, FrameGraphResourceHandle srcDepthHandle)
	{
		constexpr uint32_t GTAO_PREFILTERED_DEPTH_MIP_COUNT = 5;

		frameGraph.GetBlackboard().Add<PrefilterDepthData>() = frameGraph.AddRenderPass<PrefilterDepthData>("GTAO Prefilter Depth Pass",
		[&](FrameGraph::Builder& builder, PrefilterDepthData& data)
		{
			FrameGraphTextureSpecification spec{};
			spec.format = ImageFormat::R32F;
			spec.width = myRenderSize.x;
			spec.height = myRenderSize.y;
			spec.usage = ImageUsage::Storage;
			spec.mips = GTAO_PREFILTERED_DEPTH_MIP_COUNT;
			spec.name = "GTAO Prefiltered Depth";

			data.prefilteredDepth = builder.CreateTexture(spec);
			builder.ReadResource(srcDepthHandle);
			builder.SetIsComputePass();
		},

		[=](const PrefilterDepthData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			const auto& srcDepthResource = resources.GetImageResource(srcDepthHandle);
			const auto& targetResource = resources.GetImageResource(data.prefilteredDepth);

			prefilterDepthPipeline->SetImage(srcDepthResource.image.lock(), Sets::OTHER, 0, ImageAccess::Read);

			prefilterDepthPipeline->SetImage(targetResource.image.lock(), Sets::OTHER, 1, 0, ImageAccess::Write);
			prefilterDepthPipeline->SetImage(targetResource.image.lock(), Sets::OTHER, 2, 1, ImageAccess::Write);
			prefilterDepthPipeline->SetImage(targetResource.image.lock(), Sets::OTHER, 3, 2, ImageAccess::Write);
			prefilterDepthPipeline->SetImage(targetResource.image.lock(), Sets::OTHER, 4, 3, ImageAccess::Write);
			prefilterDepthPipeline->SetImage(targetResource.image.lock(), Sets::OTHER, 5, 4, ImageAccess::Write);
			prefilterDepthPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			prefilterDepthPipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &myConstants, sizeof(GTAOConstants));

			const uint32_t dispatchX = (myRenderSize.x + 16 - 1) / 16;
			const uint32_t dispatchY = (myRenderSize.y + 16 - 1) / 16;

			Renderer::DispatchComputePipeline(commandBuffer, prefilterDepthPipeline, dispatchX, dispatchY, 1);

			prefilterDepthPipeline->ClearAllResources();
		});
	}

	void GTAOTechnique::AddMainPass(FrameGraph& frameGraph, Ref<ComputePipeline> mainPassPipeline, FrameGraphResourceHandle viewNormalsHandle)
	{
		const auto& prefilterDepthData = frameGraph.GetBlackboard().Get<PrefilterDepthData>();

		frameGraph.GetBlackboard().Add<GTAOData>() = frameGraph.AddRenderPass<GTAOData>("GTAO Main Pass",
		[&](FrameGraph::Builder& builder, GTAOData& data)
		{
			// AO texture
			{
				FrameGraphTextureSpecification spec{};
				spec.format = ImageFormat::R32UI;
				spec.width = myRenderSize.x;
				spec.height = myRenderSize.y;
				spec.usage = ImageUsage::Storage;
				spec.name = "GTAO AO Output";

				data.aoOutput = builder.CreateTexture(spec);
			}

			// Edges texture
			{
				FrameGraphTextureSpecification spec{};
				spec.format = ImageFormat::R8U;
				spec.width = myRenderSize.x;
				spec.height = myRenderSize.y;
				spec.usage = ImageUsage::Storage;
				spec.name = "GTAO Edges Output";

				data.edgesOutput = builder.CreateTexture(spec);
			}

			builder.ReadResource(prefilterDepthData.prefilteredDepth);
			builder.ReadResource(viewNormalsHandle);
			builder.SetIsComputePass();
		},

		[=](const GTAOData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			const auto& prefilteredDepthResource = resources.GetImageResource(prefilterDepthData.prefilteredDepth);
			const auto& viewNormalsResource = resources.GetImageResource(viewNormalsHandle);

			const auto& aoOutputResource = resources.GetImageResource(data.aoOutput);
			const auto& edgesOutputResource = resources.GetImageResource(data.edgesOutput);

			const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

			mainPassPipeline->SetImage(prefilteredDepthResource.image.lock(), Sets::OTHER, 0, ImageAccess::Read);

			mainPassPipeline->SetImage(aoOutputResource.image.lock(), Sets::OTHER, 1, ImageAccess::Write);
			mainPassPipeline->SetImage(edgesOutputResource.image.lock(), Sets::OTHER, 2, ImageAccess::Write);

			mainPassPipeline->SetImage(viewNormalsResource.image.lock(), Sets::OTHER, 3, ImageAccess::Read);

			mainPassPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
			mainPassPipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &myConstants, sizeof(GTAOConstants));

			const uint32_t dispatchX = (myRenderSize.x + 16 - 1) / 16;
			const uint32_t dispatchY = (myRenderSize.y + 16 - 1) / 16;

			Renderer::DispatchComputePipeline(commandBuffer, mainPassPipeline, dispatchX, dispatchY, 1);

			mainPassPipeline->ClearAllResources();
		});
	}

	void GTAOTechnique::AddDenoisePass(FrameGraph& frameGraph, const std::vector<Ref<ComputePipeline>>& denoisePipelines)
	{
		const auto& mainData = frameGraph.GetBlackboard().Get<GTAOData>();

		frameGraph.GetBlackboard().Add<GTAOOutput>() = frameGraph.AddRenderPass<GTAOOutput>("GTAO Denoise Pass",
		[&](FrameGraph::Builder& builder, GTAOOutput& data)
		{
			FrameGraphTextureSpecification spec{};
			spec.format = ImageFormat::R32UI;
			spec.width = myRenderSize.x;
			spec.height = myRenderSize.y;
			spec.usage = ImageUsage::Storage;
			spec.name = "Final GTAO Output";

			data.outputImage = builder.CreateTexture(spec);

			builder.WriteResource(mainData.aoOutput);
			builder.ReadResource(mainData.edgesOutput);
			builder.SetIsComputePass();
		},

		[=](const GTAOOutput& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			const auto& aoTermResource = resources.GetImageResource(mainData.aoOutput);
			const auto& edgeTermResource = resources.GetImageResource(mainData.edgesOutput);
			const auto& finalOutputResource = resources.GetImageResource(data.outputImage);

			const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

			VkImageSubresourceRange subresourceRange{};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.layerCount = 1;
			subresourceRange.levelCount = 1;

			ImageBarrierInfo writeReadBarrierInfo{};
			writeReadBarrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			writeReadBarrierInfo.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
			writeReadBarrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			writeReadBarrierInfo.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
			writeReadBarrierInfo.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			writeReadBarrierInfo.subresourceRange = subresourceRange;

			ImageBarrierInfo readWriteBarrierInfo{};
			readWriteBarrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			readWriteBarrierInfo.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
			readWriteBarrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			readWriteBarrierInfo.dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
			readWriteBarrierInfo.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			readWriteBarrierInfo.subresourceRange = subresourceRange;

			for (size_t i = 0; i < denoisePipelines.size(); i++)
			{
				auto currentPipeline = denoisePipelines.at(i);

				if ((i % 2) == 0)
				{
					currentPipeline->SetImage(aoTermResource.image.lock(), Sets::OTHER, 0, ImageAccess::Write);
					currentPipeline->SetImage(finalOutputResource.image.lock(), Sets::OTHER, 2, ImageAccess::Write);
				}
				else
				{
					currentPipeline->SetImage(finalOutputResource.image.lock(), Sets::OTHER, 0, ImageAccess::Write);
					currentPipeline->SetImage(aoTermResource.image.lock(), Sets::OTHER, 2, ImageAccess::Write);
				}

				currentPipeline->SetImage(edgeTermResource.image.lock(), Sets::OTHER, 1, ImageAccess::Read);

				currentPipeline->InsertImageBarrier(Sets::OTHER, 0, readWriteBarrierInfo);
				currentPipeline->InsertImageBarrier(Sets::OTHER, 2, writeReadBarrierInfo);

				currentPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
				currentPipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &myConstants, sizeof(GTAOConstants));

				const uint32_t dispatchX = (myRenderSize.x + 8 - 1) / 8;
				const uint32_t dispatchY = (myRenderSize.y + 8 - 1) / 8;

				Renderer::DispatchComputePipeline(commandBuffer, currentPipeline, dispatchX, dispatchY, 1);

				currentPipeline->ClearAllResources();
			}
		});
	}
}

