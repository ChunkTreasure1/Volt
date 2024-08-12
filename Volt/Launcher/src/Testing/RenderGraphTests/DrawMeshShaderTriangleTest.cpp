#include "DrawMeshShaderTriangleTest.h"

#include <Volt/Core/Application.h>
#include <Volt/Rendering/Shader/ShaderMap.h>

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>

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
	RenderGraphImageHandle targetImageHandle = renderGraph.AddExternalImage(targetImage);

	renderGraph.AddPass("Triangle Pass",
	[&](RenderGraph::Builder& builder)
	{
		builder.WriteResource(targetImageHandle);
		builder.SetHasSideEffect();
	},
	[=](RenderContext& context)
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
