#include "vtpch.h"
#include "RenderGraphPass.h"

#include "Volt/RenderingNew/RenderGraph/RenderContext.h"

namespace Volt
{
	const bool RenderGraphPassNodeBase::ReadsResource(RenderGraphResourceHandle handle) const
	{
		auto it = std::find_if(resourceReads.begin(), resourceReads.end(), [&](const auto& lhs) 
		{
			return lhs.handle == handle;
		});

		return it != resourceReads.end();
	}

	const bool RenderGraphPassNodeBase::WritesResource(RenderGraphResourceHandle handle) const
	{
		auto it = std::find_if(resourceWrites.begin(), resourceWrites.end(), [&](const auto& lhs)
		{
			return lhs.handle == handle;
		});
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
