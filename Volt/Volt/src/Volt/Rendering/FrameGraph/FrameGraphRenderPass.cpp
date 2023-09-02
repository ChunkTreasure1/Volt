#include "vtpch.h"
#include "FrameGraphRenderPass.h"

#include "Volt/Rendering/FrameGraph/FrameGraph.h"

namespace Volt
{
	const bool FrameGraphRenderPassNodeBase::ReadsResource(FrameGraphResourceHandle handle) const
	{
		auto it = std::find(resourceReads.cbegin(), resourceReads.cend(), handle);
		return it != resourceReads.end();
	}
	const bool FrameGraphRenderPassNodeBase::WritesResource(FrameGraphResourceHandle handle) const
	{
		auto it = std::find(resourceWrites.cbegin(), resourceWrites.cend(), handle);
		return it != resourceWrites.end();
	}
	const bool FrameGraphRenderPassNodeBase::CreatesResource(FrameGraphResourceHandle handle) const
	{
		auto it = std::find(resourceCreates.cbegin(), resourceCreates.cend(), handle);
		return it != resourceCreates.end();
	}

	const bool FrameGraphRenderPassNodeBase::IsCulled() const
	{
		return isCulled && !hasSideEffect;
	}
}
