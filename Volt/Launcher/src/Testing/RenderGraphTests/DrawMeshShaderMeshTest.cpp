#include "DrawMeshShaderMeshTest.h"

#include <Volt/Core/Application.h>
#include <Volt/Rendering/Shader/ShaderMap.h>
#include <Volt/Asset/Mesh/Mesh.h>

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>

#include <AssetSystem/AssetManager.h>

using namespace Volt;

RG_DrawMeshShaderMeshTest::RG_DrawMeshShaderMeshTest()
{
	m_mesh = AssetManager::GetAsset<Mesh>("Assets/Meshes/Default/SM_Cube.vtasset");
}

RG_DrawMeshShaderMeshTest::~RG_DrawMeshShaderMeshTest()
{
}

bool RG_DrawMeshShaderMeshTest::RunTest()
{
	auto& swapchain = Application::Get().GetWindow().GetSwapchain();

	RenderGraph renderGraph{ m_commandBuffer };

	auto targetImage = swapchain.GetCurrentImage();
	RenderGraphImageHandle targetImageHandle = renderGraph.AddExternalImage(targetImage);

	struct Data
	{
		RenderGraphBufferHandle vertexPositionBuffer;
		RenderGraphBufferHandle meshletsBuffer;
		RenderGraphBufferHandle meshletDataBuffer;
	};

	const auto& gpuMesh = m_mesh->GetGPUMeshes().at(0);

	renderGraph.AddPass<Data>("Mesh Pass",
	[&](RenderGraph::Builder& builder, Data& data)
	{
		data.vertexPositionBuffer = builder.AddExternalBuffer(m_mesh->GetVertexPositionsBuffer()->GetResource());
		data.meshletsBuffer = builder.AddExternalBuffer(m_mesh->GetMeshletBuffer()->GetResource());
		data.meshletDataBuffer = builder.AddExternalBuffer(m_mesh->GetMeshletDataBuffer()->GetResource());

		builder.WriteResource(targetImageHandle);

		builder.ReadResource(data.vertexPositionBuffer);
		builder.ReadResource(data.meshletsBuffer);
		builder.ReadResource(data.meshletDataBuffer);

		builder.SetHasSideEffect();
	},
	[=](const Data& data, RenderContext& context)
	{
		RenderingInfo renderingInfo = context.CreateRenderingInfo(targetImage->GetWidth(), targetImage->GetHeight(), { targetImageHandle });

		RHI::RenderPipelineCreateInfo pipelineInfo{};
		pipelineInfo.shader = ShaderMap::Get("RG_DrawMeshShaderMeshTest");
		pipelineInfo.cullMode = RHI::CullMode::None;

		auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

		glm::mat4 viewProj = glm::perspective(glm::radians(60.f), 16.f / 9.f, 1000.f, 0.1f) * glm::lookAt({ 0.f, 200.f, -500.f }, { 0.f }, glm::vec3(0.f, 1, 0.f));

		context.BeginRendering(renderingInfo);
		context.BindPipeline(pipeline);

		context.SetConstant("viewProjection"_sh, viewProj);

		context.SetConstant("vertexPositionsBuffer"_sh, data.vertexPositionBuffer);
		context.SetConstant("meshletsBuffer"_sh, data.meshletsBuffer);
		context.SetConstant("meshletDataBuffer"_sh, data.meshletDataBuffer);

		context.SetConstant("meshletStartOffset"_sh, gpuMesh.meshletStartOffset);
		context.SetConstant("vertexOffset"_sh, gpuMesh.vertexStartOffset);

		context.DispatchMeshTasks(gpuMesh.meshletCount, 1, 1);
		context.EndRendering();
	});

	renderGraph.Compile();
	renderGraph.Execute();

	return true;
}
