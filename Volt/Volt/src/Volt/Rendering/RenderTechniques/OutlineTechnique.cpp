#include "vtpch.h"
#include "OutlineTechnique.h"

#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/SubMaterial.h"

#include "Volt/Rendering/RenderTechniques/TechniqueStructures.h"

#include "Volt/Rendering/FrameGraph/FrameGraph.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/ComputePipeline.h"

namespace Volt
{
	struct OutlineGeometryData
	{
		FrameGraphResourceHandle outputImage;
		FrameGraphResourceHandle depth;
	};

	struct JumpFloodInitData
	{
		FrameGraphResourceHandle outputImage;
	};

	struct JumpFloodPassData
	{
		FrameGraphResourceHandle outputImage[2];
	};

	OutlineTechnique::OutlineTechnique(const gem::vec2ui& renderSize, const GlobalDescriptorMap& descriptorMap)
		: myRenderSize(renderSize), myGlobalDescriptorMap(descriptorMap)
	{
	}

	void OutlineTechnique::AddOutlineGeometryPass(FrameGraph& frameGraph, Ref<RenderPipeline> pipeline, const std::vector<SubmitCommand>& outlineCmds)
	{
		frameGraph.GetBlackboard().Add<OutlineGeometryData>() = frameGraph.AddRenderPass<OutlineGeometryData>("Outline Geometry",
			[&](FrameGraph::Builder& builder, OutlineGeometryData& data)
		{
			data.outputImage = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, { 0.f }, "Outline Geometry" });
			data.depth = builder.CreateTexture({ ImageFormat::DEPTH32F, myRenderSize, { 0.f }, "Outline Depth" });
		},

		[=](const OutlineGeometryData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(45, 53, 110);
			renderPassInfo.name = "Outline Geometry";
			renderPassInfo.overridePipeline = pipeline;

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(data.outputImage),
					resources.GetImageResource(data.depth)
				});

			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);

			for (const auto& cmd : outlineCmds)
			{
				PushConstantDrawData drawData{ &cmd.transform, sizeof(gem::mat4) };
				Renderer::DrawMesh(commandBuffer, cmd.mesh, cmd.subMeshIndex, myGlobalDescriptorMap, drawData);
			}

			Renderer::EndFrameGraphPass(commandBuffer);
		});
	}

	void OutlineTechnique::AddJumpFloodInitPass(FrameGraph& frameGraph, Ref<Material> material)
	{
		const auto& outlineGeomData = frameGraph.GetBlackboard().Get<OutlineGeometryData>();

		frameGraph.GetBlackboard().Add<JumpFloodInitData>() = frameGraph.AddRenderPass<JumpFloodInitData>("Jump Flood Init",
			[&](FrameGraph::Builder& builder, JumpFloodInitData& data)
		{
			data.outputImage = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, { 0.f }, "Jump Flood Init" });

			builder.ReadResource(outlineGeomData.outputImage);
		},
			[=](const JumpFloodInitData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(6, 71, 24);
			renderPassInfo.name = "Jump Flood Init";

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(data.outputImage)
				});

			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			material->GetSubMaterialAt(0)->Set(0, resources.GetImageResource(outlineGeomData.outputImage).image.lock());

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawFullscreenTriangleWithMaterial(commandBuffer, material, myGlobalDescriptorMap);
			Renderer::EndFrameGraphPass(commandBuffer);
		});
	}

	void OutlineTechnique::AddJumpFloodPass(FrameGraph& frameGraph, Ref<Material> material)
	{
		const auto& jumpFloodInitData = frameGraph.GetBlackboard().Get<JumpFloodInitData>();

		frameGraph.GetBlackboard().Add<JumpFloodPassData>() = frameGraph.AddRenderPass<JumpFloodPassData>("Jump Flood Pass",
			[&](FrameGraph::Builder& builder, JumpFloodPassData& data)
		{
			data.outputImage[0] = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, { 1.f }, "Jump Flood Pass 0" });

			builder.ReadResource(jumpFloodInitData.outputImage);
		},
			[=](const JumpFloodPassData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			const int32_t steps = 2;
			int32_t step = (int32_t)std::round(std::pow(steps - 1, 2));
			int32_t index = 0;

			struct FloodPassData
			{
				gem::vec2 texelSize;
				int32_t step;
				int32_t padding;
			} floodPassData;

			floodPassData.texelSize = { 1.f / (float)myRenderSize.x, 1.f / (float)myRenderSize.y };
			floodPassData.step = step;

			while (step != 0)
			{
				FrameGraphRenderPassInfo renderPassInfo{};
				renderPassInfo.color = TO_NORMALIZEDRGB(6, 71, 24);
				renderPassInfo.name = "Jump Flood Pass";

				FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
					{
						resources.GetImageResource(data.outputImage[index])
					});

				renderingInfo.width = myRenderSize.x;
				renderingInfo.height = myRenderSize.y;

				if (index == 0)
				{
					material->GetSubMaterialAt(0)->Set(0, resources.GetImageResource(jumpFloodInitData.outputImage).image.lock());
				}
				else
				{
					material->GetSubMaterialAt(0)->Set(0, resources.GetImageResource(data.outputImage[0]).image.lock());
				}

				material->GetSubMaterialAt(0)->SetValue("texelSize", floodPassData.texelSize);
				material->GetSubMaterialAt(0)->SetValue("step", floodPassData.step);

				Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
				Renderer::DrawFullscreenTriangleWithMaterial(commandBuffer, material, myGlobalDescriptorMap);
				Renderer::EndFrameGraphPass(commandBuffer);

				index = (index + 1) % 2;
				step /= 2;

				floodPassData.step = step;
			}
		});
	}
	void OutlineTechnique::AddOutlineCompositePass(FrameGraph& frameGraph, Ref<ComputePipeline> pipeline)
	{
		const auto& jumpFloodPassData = frameGraph.GetBlackboard().Get<JumpFloodPassData>();
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();

		frameGraph.AddRenderPass("Outline Composite",
			[&](FrameGraph::Builder& builder)
		{
			builder.WriteResource(skyboxData.outputImage);
			builder.ReadResource(jumpFloodPassData.outputImage[0]);

			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},

		[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			Renderer::BeginSection(commandBuffer, "Outline Composite Pass", TO_NORMALIZEDRGB(6, 71, 24));

			const auto& outputImageResource = resources.GetImageResource(skyboxData.outputImage);
			const auto& jumpFloodPassResource = resources.GetImageResource(jumpFloodPassData.outputImage[0]);

			const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

			pipeline->SetImage(outputImageResource.image.lock(), Sets::OTHER, 0, ImageAccess::Write);
			pipeline->SetImage(jumpFloodPassResource.image.lock(), Sets::OTHER, 1, ImageAccess::Read);
			pipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			const gem::vec4 color = { 1.f, 0.5f, 0.f, 1.f };
			pipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &color, sizeof(gem::vec4));

			constexpr uint32_t threadCount = 8;

			const uint32_t dispatchX = std::max(1u, (myRenderSize.x / threadCount) + 1);
			const uint32_t dispatchY = std::max(1u, (myRenderSize.y / threadCount) + 1);

			Renderer::DispatchComputePipeline(commandBuffer, pipeline, dispatchX, dispatchY, 1);
			Renderer::EndSection(commandBuffer);
		});
	}
}
