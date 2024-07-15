#include "vtpch.h"
#include "DirectionalShadowTechnique.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/Rendering/Shader/ShaderMap.h"
#include "Volt/Rendering/RenderScene.h"
#include "Volt/Rendering/RendererCommon.h"
#include "Volt/Rendering/Camera/Camera.h"

#include <VoltRHI/Pipelines/RenderPipeline.h>

namespace Volt
{
	DirectionalShadowTechnique::DirectionalShadowTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
		: m_renderGraph(renderGraph), m_blackboard(blackboard)
	{
	}

	DirectionalShadowData DirectionalShadowTechnique::Execute(Ref<Camera> camera, Ref<RenderScene> renderScene, const DirectionalLightData& light)
	{
		constexpr float SIZE = 1000.f;

		Ref<Camera> shadowCamera = CreateRef<Camera>(-SIZE, SIZE, -SIZE, SIZE, 1.f, 10'000.f);
		const glm::mat4 view = glm::lookAtLH(glm::vec3(0.f) - glm::vec3(light.direction) * 1000.f, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

		shadowCamera->SetView(view);

		const auto& uniformBuffers = m_blackboard.Get<UniformBuffersData>();
		const auto& gpuSceneData = m_blackboard.Get<GPUSceneData>();

		const auto& gpuMeshes = renderScene->GetGPUMeshes();
		const auto& objectDrawData = renderScene->GetObjectDrawData();

		DirectionalShadowData& dirShadowData = m_renderGraph.AddPass<DirectionalShadowData>("Directional Shadow",
		[&](RenderGraph::Builder& builder, DirectionalShadowData& data)
		{
			data.renderSize = { 1024, 1024 };

			RenderGraphImageDesc imageDesc{};
			imageDesc.format = RHI::PixelFormat::D32_SFLOAT;
			imageDesc.width = data.renderSize.x;
			imageDesc.height = data.renderSize.y;
			imageDesc.usage = RHI::ImageUsage::Attachment;
			imageDesc.layers = 5;
			imageDesc.isCubeMap = false;
			imageDesc.name = "Directional Shadow";

			data.shadowTexture = builder.CreateImage2D(imageDesc);

			GPUSceneData::SetupInputs(builder, gpuSceneData);

			builder.ReadResource(uniformBuffers.viewDataBuffer);
			builder.ReadResource(uniformBuffers.directionalLightBuffer);
		},
		[=](const DirectionalShadowData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			RenderingInfo info = context.CreateRenderingInfo(data.renderSize.x, data.renderSize.y, { data.shadowTexture });
			info.renderingInfo.layerCount = DirectionalLightData::CASCADE_COUNT;
			info.renderingInfo.depthAttachmentInfo.SetClearColor(1.f, 1.f, 1.f, 1.f);

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("DirectionalShadowMeshShader");
			pipelineInfo.depthCompareOperator = RHI::CompareOperator::LessEqual;
			pipelineInfo.cullMode = RHI::CullMode::Back;

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);
			context.BindPipeline(pipeline);

			GPUSceneData::SetupConstants(context, resources, gpuSceneData);

			context.SetConstant("viewData"_sh, resources.GetUniformBuffer(uniformBuffers.viewDataBuffer));
			context.SetConstant("directionalLight"_sh, resources.GetBuffer(uniformBuffers.directionalLightBuffer));

			struct PushConstant
			{
				uint32_t drawIndex;
				uint32_t viewIndex;
			};

			for (uint32_t i = 0; i < DirectionalLightData::CASCADE_COUNT; i++)
			{
				context.BeginMarker(std::format("Cascade: {}", i));
				for (uint32_t m = 0; const auto & objData : objectDrawData)
				{
					PushConstant pushConstants{ m, i };

					context.PushConstants(&pushConstants, sizeof(PushConstant));
					context.DispatchMeshTasks(gpuMeshes[objData.meshId].meshletCount, 1, 1);
					m++;
				}
				context.EndMarker();
			}

			context.EndRendering();
		});

		return dirShadowData;
	}
}
