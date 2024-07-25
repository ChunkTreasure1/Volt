#include "vtpch.h"
#include "RenderGraphUtils.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"

namespace Volt::RGUtils
{
	void ClearImage2D(RenderGraph& renderGraph, RenderGraphResourceHandle image, const glm::vec4& clearColor, const std::string& passName)
	{
		renderGraph.AddPass(passName.empty() ? "Clear Image Pass" : passName,
		[&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(image, RenderGraphResourceState::Clear);
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources) 
		{
			context.ClearImage2D(image, clearColor);
		});
	}
	void ClearImage3D(RenderGraph& renderGraph, RenderGraphResourceHandle image, const glm::vec4& clearColor, const std::string& passName)
	{
		renderGraph.AddPass(passName.empty() ? "Clear Image Pass" : passName,
		[&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(image, RenderGraphResourceState::Clear);
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			context.ClearImage3D(image, clearColor);
		});
	}
}
