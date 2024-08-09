#include "DrawMeshShaderMultipleMeshesTest.h"

#include <Volt/Core/Application.h>
#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Asset/AssetManager.h>

#include <Volt/Rendering/Shader/ShaderMap.h>

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>

using namespace Volt;

RG_DrawMeshShaderMultipleMeshesTest::RG_DrawMeshShaderMultipleMeshesTest()
{
	m_cubeMesh = AssetManager::GetAsset<Mesh>("Engine/Meshes/Primitives/SM_Cube.vtasset");
	m_sphereMesh = AssetManager::GetAsset<Mesh>("Engine/Meshes/Primitives/SM_Sphere.vtasset");
}

RG_DrawMeshShaderMultipleMeshesTest::~RG_DrawMeshShaderMultipleMeshesTest()
{
}

bool RG_DrawMeshShaderMultipleMeshesTest::RunTest()
{
	auto& swapchain = Application::Get().GetWindow().GetSwapchain();

	RenderGraph renderGraph{ m_commandBuffer };

	auto targetImage = swapchain.GetCurrentImage();
	RenderGraphImage2DHandle targetImageHandle = renderGraph.AddExternalImage2D(targetImage);

	struct Data
	{
		RenderGraphBufferHandle gpuMeshesBuffer;
		RenderGraphBufferHandle transformsBuffer;
	} data;

	std::array<GPUMesh, 2> gpuMeshes = { m_cubeMesh->GetGPUMeshes().front(), m_sphereMesh->GetGPUMeshes().front() };

	// GPU Meshes
	{
		const auto desc = RGUtils::CreateBufferDesc<GPUMesh>(2, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "GPU Meshes");
		data.gpuMeshesBuffer = renderGraph.CreateBuffer(desc);
		renderGraph.AddStagedBufferUpload(data.gpuMeshesBuffer, gpuMeshes.data(), sizeof(GPUMesh) * gpuMeshes.size(), "Upload GPU Meshes");
	}

	// Transforms
	{
		std::array<glm::mat4, 2> transforms = { glm::translate(glm::mat4{ 1.f }, { -100.f, 0.f, 0.f }), glm::translate(glm::mat4{ 1.f }, { 100.f, 0.f, 0.f }) * glm::scale(glm::mat4{ 1.f }, { 50.f })};

		const auto desc = RGUtils::CreateBufferDesc<glm::mat4>(2, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Transforms");
		data.transformsBuffer = renderGraph.CreateBuffer(desc);
		renderGraph.AddStagedBufferUpload(data.transformsBuffer, transforms.data(), sizeof(glm::mat4) * transforms.size(), "Upload Transforms");
	}

	renderGraph.AddPass("Mesh Pass",
	[&](RenderGraph::Builder& builder)
	{
		builder.WriteResource(targetImageHandle);

		builder.ReadResource(data.gpuMeshesBuffer);
		builder.ReadResource(data.transformsBuffer);

		builder.SetHasSideEffect();
	},
	[=](RenderContext& context)
	{
		RenderingInfo renderingInfo = context.CreateRenderingInfo(targetImage->GetWidth(), targetImage->GetHeight(), { targetImageHandle });

		RHI::RenderPipelineCreateInfo pipelineInfo{};
		pipelineInfo.shader = ShaderMap::Get("RG_DrawMeshShaderMultipleMeshesTest");
		pipelineInfo.cullMode = RHI::CullMode::None;

		auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

		glm::mat4 viewProj = glm::perspective(glm::radians(60.f), 16.f / 9.f, 1000.f, 0.1f) * glm::lookAt({ 0.f, 200.f, -500.f }, { 0.f }, glm::vec3(0.f, 1, 0.f));

		context.BeginRendering(renderingInfo);
		context.BindPipeline(pipeline);

		context.SetConstant("viewProjection"_sh, viewProj);
		context.SetConstant("gpuMeshesBuffer"_sh, data.gpuMeshesBuffer);
		context.SetConstant("transformsBuffer"_sh, data.transformsBuffer);

		for (uint32_t i = 0; i < 2; i++)
		{
			context.PushConstants(&i, sizeof(uint32_t));
			context.DispatchMeshTasks(gpuMeshes[i].meshletCount, 1, 1);
		}

		context.EndRendering();
	});

	renderGraph.Compile();
	renderGraph.Execute();

	return true;
}
