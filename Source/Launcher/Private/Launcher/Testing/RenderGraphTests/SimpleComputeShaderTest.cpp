#include "Launcher/Testing/RenderGraphTests/SimpleComputeShaderTest.h"

#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Core/Application.h>
#include <Volt/Rendering/Renderer.h>

#include <AssetSystem/AssetManager.h>

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>
#include <RenderCore/Shader/ShaderMap.h>

using namespace Volt;

RG_SimpleComputeShaderTest::RG_SimpleComputeShaderTest()
	: m_commandBufferSet(Renderer::GetFramesInFlight())
{
	m_cubeMesh = AssetManager::GetAsset<Mesh>("Engine/Meshes/Primitives/SM_Cube.vtasset");
}

RG_SimpleComputeShaderTest::~RG_SimpleComputeShaderTest()
{
}

bool RG_SimpleComputeShaderTest::RunTest()
{
	RenderGraph renderGraph{ m_commandBufferSet.IncrementAndGetCommandBuffer() };

	struct Data
	{
		RenderGraphBufferHandle inputBuffer;
		RenderGraphBufferHandle outputBuffer;
	};

	GPUMesh gpuMesh = m_cubeMesh->GetGPUMeshes().front();

	RenderGraphBufferHandle inputHandle;

	// GPU Meshes
	{
		const auto desc = RGUtils::CreateBufferDesc<GPUMesh>(2, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "GPU Meshes");
		inputHandle = renderGraph.CreateBuffer(desc);
		renderGraph.AddStagedBufferUpload(inputHandle, &gpuMesh, sizeof(GPUMesh), "Upload GPU Meshes");
	}

	renderGraph.AddPass<Data>("Compute Shader Pass",
	[&](RenderGraph::Builder& builder, Data& data)
	{
		{
			const auto desc = RGUtils::CreateBufferDescGPU<uint32_t>(1, "Output Buffer");
			data.outputBuffer = builder.CreateBuffer(desc);
		}

		builder.ReadResource(inputHandle);
		builder.SetHasSideEffect();
		builder.SetIsComputePass();
	},
	[=](const Data& data, RenderContext& context)
	{
		auto pipeline = ShaderMap::GetComputePipeline("RG_SimpleComputeShaderTest");

		context.BindPipeline(pipeline);

		context.SetConstant("inputBuffer"_sh, inputHandle);
		context.SetConstant("outputBuffer"_sh, data.outputBuffer);

		context.Dispatch(1, 1, 1);

	});

	renderGraph.Compile();
	renderGraph.Execute();

	return true;
}
