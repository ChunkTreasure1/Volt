#include "DrawTriangleTest.h"

#include <Volt/Core/Application.h>
#include <Volt/Rendering/Shader/ShaderMap.h>

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>

#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

using namespace Volt;

RG_DrawTriangleTest::RG_DrawTriangleTest()
{
}

RG_DrawTriangleTest::~RG_DrawTriangleTest()
{
}

bool RG_DrawTriangleTest::RunTest()
{
	auto& swapchain = Volt::WindowManager::Get().GetMainWindow().GetSwapchain();

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
