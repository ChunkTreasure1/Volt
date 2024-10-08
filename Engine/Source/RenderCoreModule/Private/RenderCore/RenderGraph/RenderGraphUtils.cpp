#include "rcpch.h"
#include "RenderCore/RenderGraph/RenderGraphUtils.h"

#include "RenderCore/RenderGraph/RenderGraph.h"

namespace Volt::RGUtils
{
	void ClearImage(RenderGraph& renderGraph, RenderGraphImageHandle image, const glm::vec4& clearColor, std::string_view passName)
	{
		renderGraph.AddPass(passName.empty() ? "Clear Image Pass" : std::string(passName),
		[&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(image, RenderGraphResourceState::Clear);
		},
		[=](RenderContext& context) 
		{
			context.ClearImage(image, clearColor);
		});
	}

	void ClearBuffer(RenderGraph& renderGraph, RenderGraphBufferHandle buffer, const uint32_t clearValue, std::string_view passName)
	{
		renderGraph.AddPass(passName.empty() ? "Clear Buffer Pass" : std::string(passName),
		[&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(buffer, RenderGraphResourceState::Clear);
		},
		[=](RenderContext& context) 
		{
			context.ClearBuffer(buffer, clearValue);
		});
	}
}
