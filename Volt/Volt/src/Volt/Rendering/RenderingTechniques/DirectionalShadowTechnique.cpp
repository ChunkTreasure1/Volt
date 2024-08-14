#include "vtpch.h"
#include "DirectionalShadowTechnique.h"

#include "Volt/Rendering/Shader/ShaderMap.h"
#include "Volt/Rendering/RenderScene.h"
#include "Volt/Rendering/RendererCommon.h"
#include "Volt/Rendering/Camera/Camera.h"

#include "Volt/Rendering/RenderingTechniques/CullingTechnique.h"

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderGraphBlackboard.h>
#include <RenderCore/RenderGraph/Resources/RenderGraphTextureResource.h>

#include <RHIModule/Pipelines/RenderPipeline.h>

namespace Volt
{
	DirectionalShadowTechnique::DirectionalShadowTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
		: m_renderGraph(renderGraph), m_blackboard(blackboard)
	{
	}

	DirectionalShadowData DirectionalShadowTechnique::Execute(Ref<Camera> camera, Ref<RenderScene> renderScene)
	{
		constexpr float SIZE = 1000.f;

		const auto& uniformBuffers = m_blackboard.Get<UniformBuffersData>();
		const auto& gpuSceneData = m_blackboard.Get<GPUSceneData>();
		const auto& lightInfo = m_blackboard.Get<DirectionalLightInfo>();

		Ref<Camera> shadowCamera = CreateRef<Camera>(-SIZE, SIZE, -SIZE, SIZE, 1.f, 100'000.f);
		const glm::mat4 view = glm::lookAtLH(glm::vec3(0.f) - glm::vec3(lightInfo.data.direction) * 1000.f, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

		shadowCamera->SetView(view);

		m_renderGraph.BeginMarker("Directional Light Shadow");

		DirectionalShadowData dirShadowData{};
		{
			dirShadowData.renderSize = { 2048, 2048 };

			RenderGraphImageDesc imageDesc{};
			imageDesc.format = RHI::PixelFormat::D32_SFLOAT;
			imageDesc.width = dirShadowData.renderSize.x;
			imageDesc.height = dirShadowData.renderSize.y;
			imageDesc.usage = RHI::ImageUsage::Attachment;
			imageDesc.layers = DirectionalLightData::CASCADE_COUNT;
			imageDesc.isCubeMap = false;
			imageDesc.name = "Directional Light Shadow";

			dirShadowData.shadowTexture = m_renderGraph.CreateImage(imageDesc);
		}

		for (uint32_t i = 0; i < DirectionalLightData::CASCADE_COUNT; i++)
		{
			CullingTechnique::Info cullingInfo{};
			cullingInfo.type = CullingTechnique::Type::Orthographic;
			cullingInfo.viewMatrix = lightInfo.views[i];
			cullingInfo.cullingFrustum = lightInfo.projectionBounds[i];

			cullingInfo.nearPlane = shadowCamera->GetNearPlane();
			cullingInfo.farPlane = shadowCamera->GetFarPlane();
			cullingInfo.drawCommandCount = renderScene->GetDrawCount();
			cullingInfo.meshletCount = renderScene->GetMeshletCount();

			CullingTechnique cullTechnique{ m_renderGraph, m_blackboard };
			DrawCullingData cullingData = cullTechnique.Execute(cullingInfo);

			m_renderGraph.AddPass(std::format("Directional Light Shadow Cascade {}", i),
			[&](RenderGraph::Builder& builder)
			{
				GPUSceneData::SetupInputs(builder, gpuSceneData);
				
				builder.WriteResource(dirShadowData.shadowTexture);
				builder.ReadResource(uniformBuffers.directionalLightBuffer);
				builder.ReadResource(cullingData.countCommandBuffer, RenderGraphResourceState::IndirectArgument);
				builder.ReadResource(cullingData.taskCommandsBuffer);
				builder.SetHasSideEffect();
			},
			[=](RenderContext& context)
			{
				RenderingInfo info = context.CreateRenderingInfo(dirShadowData.renderSize.x, dirShadowData.renderSize.y, { dirShadowData.shadowTexture });
				info.renderingInfo.layerCount = DirectionalLightData::CASCADE_COUNT;
				info.renderingInfo.depthAttachmentInfo.SetClearColor(1.f, 1.f, 1.f, 1.f);
				if (i != 0)
				{
					info.renderingInfo.depthAttachmentInfo.clearMode = RHI::ClearMode::Load;
				}

				RHI::RenderPipelineCreateInfo pipelineInfo{};
				pipelineInfo.shader = ShaderMap::Get("DirectionalShadowMeshShader");
				pipelineInfo.depthCompareOperator = RHI::CompareOperator::LessEqual;
				pipelineInfo.cullMode = RHI::CullMode::Back;

				auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

				context.BeginRendering(info);
				context.BindPipeline(pipeline);

				GPUSceneData::SetupConstants(context, gpuSceneData);

				context.SetConstant("directionalLight"_sh, uniformBuffers.directionalLightBuffer);
				context.SetConstant("taskCommands"_sh, cullingData.taskCommandsBuffer);
				context.SetConstant("viewMatrix"_sh, cullingInfo.viewMatrix);
				context.SetConstant("cullingFrustum"_sh, cullingInfo.cullingFrustum);
				context.SetConstant("renderSize"_sh, dirShadowData.renderSize);

				struct PushConstant
				{
					uint32_t viewIndex;
				};

				PushConstant pushConstants{ i };
				context.PushConstants(&pushConstants, sizeof(PushConstant));
				context.DispatchMeshTasksIndirect(cullingData.countCommandBuffer, sizeof(uint32_t), 1, 0);

				context.EndRendering();
			});
		}

		m_renderGraph.EndMarker();

		return dirShadowData;
	}
}
