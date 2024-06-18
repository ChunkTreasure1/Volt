#include "vtpch.h"
#include "DirectionalShadowTechnique.h"

#include "Volt/Rendering/RenderingTechniques/CullingTechnique.h"
#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/Rendering/Shader/ShaderMap.h"
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
		m_renderGraph.BeginMarker("Directional Shadow");

		constexpr float SIZE = 1000.f;

		Ref<Camera> shadowCamera = CreateRef<Camera>(-SIZE, SIZE, -SIZE, SIZE, 1.f, 10'000.f);
		const glm::mat4 view = glm::lookAtLH(glm::vec3(0.f) - glm::vec3(light.direction) * 1000.f, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

		shadowCamera->SetView(view);

		CullingTechnique culling{ m_renderGraph, m_blackboard };
		const CullPrimitivesData cullPrimitivesData = culling.Execute(shadowCamera, renderScene, CullingMode::None, glm::vec2{ 1024, 1024 }, DirectionalLightData::CASCADE_COUNT);

		const auto& uniformBuffers = m_blackboard.Get<UniformBuffersData>();

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

			builder.ReadResource(uniformBuffers.viewDataBuffer);
			builder.ReadResource(uniformBuffers.directionalLightBuffer);
			builder.ReadResource(uniformBuffers.gpuScene);
			builder.ReadResource(cullPrimitivesData.indexBuffer, RenderGraphResourceState::IndexBuffer);
			builder.ReadResource(cullPrimitivesData.drawCommand, RenderGraphResourceState::IndirectArgument);

			builder.SetHasSideEffect();

		},
		[=](const DirectionalShadowData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			RenderingInfo info = context.CreateRenderingInfo(data.renderSize.x, data.renderSize.y, { data.shadowTexture });
			info.renderingInfo.layerCount = DirectionalLightData::CASCADE_COUNT;
			info.renderingInfo.depthAttachmentInfo.SetClearColor(1.f, 1.f, 1.f, 1.f);

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("DirectionalShadow");
			pipelineInfo.depthCompareOperator = RHI::CompareOperator::LessEqual;
			pipelineInfo.cullMode = RHI::CullMode::Back;

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);
			context.BindPipeline(pipeline);

			const auto gpuSceneHandle = resources.GetUniformBuffer(uniformBuffers.gpuScene);
			const auto viewDataHandle = resources.GetUniformBuffer(uniformBuffers.viewDataBuffer);

			context.BindIndexBuffer(cullPrimitivesData.indexBuffer);
			context.SetConstant("gpuScene"_sh, gpuSceneHandle);
			context.SetConstant("viewData"_sh, viewDataHandle);
			context.SetConstant("directionalLight"_sh, resources.GetBuffer(uniformBuffers.directionalLightBuffer));

			context.DrawIndexedIndirect(cullPrimitivesData.drawCommand, 0, 1, sizeof(RHI::IndirectIndexedCommand));
			context.EndRendering();
		});

		m_renderGraph.EndMarker();

		return dirShadowData;
	}
}
