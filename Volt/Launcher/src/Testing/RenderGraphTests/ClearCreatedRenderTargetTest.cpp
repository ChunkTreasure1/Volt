#include "ClearCreatedRenderTargetTest.h"

#include <Volt/Core/Application.h>
#include <Volt/Rendering/Shader/ShaderMap.h>

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>

using namespace Volt;

RG_ClearCreatedRenderTargetTest::RG_ClearCreatedRenderTargetTest()
{
}

RG_ClearCreatedRenderTargetTest::~RG_ClearCreatedRenderTargetTest()
{
}

bool RG_ClearCreatedRenderTargetTest::RunTest()
{
	RenderGraph renderGraph{ m_commandBuffer };
	
	auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::R16G16B16A16_SFLOAT>(1280, 720, RHI::ImageUsage::AttachmentStorage, "TestImage");
	RenderGraphImage2DHandle testImageHandle = renderGraph.CreateImage2D(desc);

	renderGraph.AddPass("Clear Test Image",
	[&](RenderGraph::Builder& builder)
	{
		builder.WriteResource(testImageHandle, RenderGraphResourceState::Clear);
		builder.SetHasSideEffect();
	},
	[=](RenderContext& context)
	{
		context.ClearImage(testImageHandle, { 0.f, 0.f, 0.f, 0.f });
	});

	renderGraph.Compile();
	renderGraph.Execute();

	return true;
}
