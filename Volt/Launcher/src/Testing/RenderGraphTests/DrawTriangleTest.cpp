#include "DrawTriangleTest.h"

#include <Volt/Core/Application.h>
#include <Volt/Rendering/RenderGraph/RenderGraph.h>
#include <Volt/Rendering/RenderGraph/RenderContextUtils.h>
#include <Volt/Rendering/Shader/ShaderMap.h>

using namespace Volt;

RG_DrawTriangleTest::RG_DrawTriangleTest()
{
}

RG_DrawTriangleTest::~RG_DrawTriangleTest()
{
}

bool RG_DrawTriangleTest::RunTest()
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
		pipelineInfo.shader = ShaderMap::Get("RG_DrawTriangleTest");
		auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

		context.BeginRendering(renderingInfo);
		RCUtils::DrawFullscreenTriangle(context, pipeline);
		context.EndRendering();
	});

	renderGraph.Compile();
	renderGraph.Execute();

	return true;
}
