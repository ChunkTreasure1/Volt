#include "DrawMeshShaderTriangleTest.h"

#include <Volt/Core/Application.h>
#include <Volt/Rendering/RenderGraph/RenderGraph.h>
#include <Volt/Rendering/RenderGraph/RenderContextUtils.h>
#include <Volt/Rendering/Shader/ShaderMap.h>

using namespace Volt;

RG_DrawMeshShaderTriangleTest::RG_DrawMeshShaderTriangleTest()
{
}

RG_DrawMeshShaderTriangleTest::~RG_DrawMeshShaderTriangleTest()
{
}

bool RG_DrawMeshShaderTriangleTest::RunTest()
{
	auto& swapchain = Application::Get().GetWindow().GetSwapchain();

	RenderGraph renderGraph{ m_commandBuffer };

	auto targetImage = swapchain.GetCurrentImage();
	RenderGraphResourceHandle targetImageHandle = renderGraph.AddExternalImage2D(targetImage);

	renderGraph.AddPass("Triangle Pass",
	[&](RenderGraph::Builder& builder)
	{
		builder.WriteResource(targetImageHandle);
		builder.SetHasSideEffect();
	},
	[=](RenderContext& context, const RenderGraphPassResources& resources)
	{
		RenderingInfo renderingInfo = context.CreateRenderingInfo(targetImage->GetWidth(), targetImage->GetHeight(), { targetImageHandle });

		RHI::RenderPipelineCreateInfo pipelineInfo{};
		pipelineInfo.shader = ShaderMap::Get("RG_DrawMeshShaderTriangleTest");
		pipelineInfo.cullMode = RHI::CullMode::None;

		auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

		context.BeginRendering(renderingInfo);
		context.BindPipeline(pipeline);
		context.DispatchMeshTasks(1, 1, 1);
		context.EndRendering();
	});

	renderGraph.Compile();
	renderGraph.Execute();

	return true;
}
