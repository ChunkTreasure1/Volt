#include "vtpch.h"
#include "RenderGraphPass.h"

namespace Volt
{
	const bool RenderGraphPassNodeBase::ReadsResource(RenderGraphResourceHandle handle) const
	{
		auto it = std::find(resourceReads.cbegin(), resourceReads.cend(), handle);
		return it != resourceReads.end();
	}

	const bool RenderGraphPassNodeBase::WritesResource(RenderGraphResourceHandle handle) const
	{
		auto it = std::find(resourceWrites.cbegin(), resourceWrites.cend(), handle);
		return it != resourceWrites.end();
	}

	const bool RenderGraphPassNodeBase::CreatesResource(RenderGraphResourceHandle handle) const
	{
		auto it = std::find(resourceCreates.cbegin(), resourceCreates.cend(), handle);
		return it != resourceCreates.end();
	}

	const bool RenderGraphPassNodeBase::IsCulled() const
	{
		return isCulled && !hasSideEffect;
	}
}
