#include "rcpch.h"
#include "RenderCore/RenderGraph/RenderGraphPass.h"

#include "RenderCore/RenderGraph/RenderContext.h"

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
		auto it = std::find_if(resourceCreates.begin(), resourceCreates.end(), [&](const auto& lhs)
		{
			return lhs.handle == handle;
		});
		return it != resourceCreates.end();
	}

	const bool RenderGraphPassNodeBase::IsCulled() const
	{
		return isCulled && !hasSideEffect;
	}

#ifdef VT_DEBUG
	const bool RenderGraphPassNodeBase::ReadsResource(ResourceHandle handle) const
	{
		return ReadsResource(m_resourceHandleMapping.at(handle));
	}

	const bool RenderGraphPassNodeBase::WritesResource(ResourceHandle handle) const
	{
		return WritesResource(m_resourceHandleMapping.at(handle));
	}

	const bool RenderGraphPassNodeBase::CreatesResource(ResourceHandle handle) const
	{
		return CreatesResource(m_resourceHandleMapping.at(handle));
	}
#endif
}
