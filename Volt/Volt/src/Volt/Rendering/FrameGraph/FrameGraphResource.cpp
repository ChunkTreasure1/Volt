#include "vtpch.h"
#include "FrameGraphResource.h"

#include "Volt/Rendering/FrameGraph/FrameGraph.h"

namespace Volt
{
	FrameGraphRenderPassResources::FrameGraphRenderPassResources(FrameGraph& frameGraph, FrameGraphRenderPassNodeBase& renderPass)
		: myFrameGraph(frameGraph), myRenderPass(renderPass)
	{}

	const FrameGraphTexture& FrameGraphRenderPassResources::GetImageResource(FrameGraphResourceHandle handle)
	{
		VT_CORE_ASSERT(myRenderPass.ReadsResource(handle) || myRenderPass.WritesResource(handle), "[FrameGraph] Trying to access resource which has not been marked as read or write");
		return myFrameGraph.GetImageResource(handle);
	}
}