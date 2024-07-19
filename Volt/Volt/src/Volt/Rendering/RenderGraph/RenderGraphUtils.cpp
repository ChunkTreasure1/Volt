#include "vtpch.h"
#include "RenderGraphUtils.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"

namespace Volt::RGUtils
{
	void ClearImage(RenderGraph& renderGraph, RenderGraphResourceHandle image, const glm::vec4& clearColor, const std::string& passName)
	{
		renderGraph.AddPass(passName.empty() ? "Clear Image Pass" : passName,
		[&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(image, RenderGraphResourceState::Clear);
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources) 
		{
			context.ClearImage(image, clearColor);
		});
	}
}
