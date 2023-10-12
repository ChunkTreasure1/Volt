#include "vtpch.h"
#include "SSRTechnique.h"

#include "Volt/Rendering/RenderTechniques/TechniqueStructures.h"

#include "Volt/Rendering/FrameGraph/FrameGraph.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/CommandBuffer.h"

#include "Volt/Asset/Mesh/Material.h"

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

	void SSRTechnique::AddSSRPass(FrameGraph& frameGraph, Ref<Material> material)
	{
		const auto& preDepthData = frameGraph.GetBlackboard().Get<PreDepthData>();
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();

		frameGraph.GetBlackboard().Add<SSROutput>() = frameGraph.AddRenderPass<SSROutput>("SSR Pass", 
			[&](FrameGraph::Builder& builder, SSROutput& data) 
		{
			data.output = builder.CreateTexture({ ImageFormat::RGBA16F, { myRenderSize }, "SSR Output", ImageUsage::Attachment });

			builder.ReadResource(preDepthData.preDepth);
			builder.ReadResource(preDepthData.viewNormals);
			builder.ReadResource(skyboxData.outputImage);

			builder.SetHasSideEffect();
		}, 
			[=](const SSROutput& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			//const auto& preDepthResource = resources.GetImageResource(preDepthData.preDepth);
			//const auto& viewNormalsResource = resources.GetImageResource(preDepthData.viewNormals);
			//const auto& colorResource = resources.GetImageResource(skyboxData.outputImage);
			//const auto& outputResource = resources.GetImageResource(data.output);

			//FrameGraphRenderPassInfo renderPassInfo{};
			//renderPassInfo.color = TO_NORMALIZEDRGB(115, 237, 190);
			//renderPassInfo.name = "SSR Pass";

			//FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
			//	{
			//		resources.GetImageResource(data.output)
			//	});

			//renderingInfo.width = myRenderSize.x;
			//renderingInfo.height = myRenderSize.y;

			////material->GetSubMaterialAt(0)->Set(0, preDepthResource.image);
			////material->GetSubMaterialAt(0)->Set(1, viewNormalsResource.image);
			////material->GetSubMaterialAt(0)->Set(2, colorResource.image);

			//Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			//Renderer::DrawFullscreenTriangleWithMaterial(commandBuffer, material, myGlobalDescriptorMap);
			//Renderer::EndFrameGraphPass(commandBuffer);
		});
	}

	void SSRTechnique::AddSSRCompositePass(FrameGraph& frameGraph, Ref<Material> material)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& ssrData = frameGraph.GetBlackboard().Get<SSROutput>();

		frameGraph.AddRenderPass("SSR Composite Pass",
			[&](FrameGraph::Builder& builder)
		{
			builder.ReadResource(ssrData.output);
			builder.WriteResource(skyboxData.outputImage);

			builder.SetHasSideEffect();
		},
			[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			//const auto& ssrResource = resources.GetImageResource(ssrData.output);
			const auto& colorResource = resources.GetImageResource(skyboxData.outputImage);

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(115, 237, 190);
			renderPassInfo.name = "SSR Pass";

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					colorResource
				});

			renderingInfo.colorAttachmentInfo.front().loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			//material->GetSubMaterialAt(0)->Set(0, ssrResource.image);

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawFullscreenTriangleWithMaterial(commandBuffer, material, myGlobalDescriptorMap);
			Renderer::EndFrameGraphPass(commandBuffer);
		});
	}
}
