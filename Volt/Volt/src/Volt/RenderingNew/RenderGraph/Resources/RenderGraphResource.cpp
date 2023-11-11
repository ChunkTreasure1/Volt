#include "vtpch.h"
#include "RenderGraphResource.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraphPass.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraph.h"

namespace Volt
{
	RenderGraphPassResources::RenderGraphPassResources(RenderGraph& renderGraph, RenderGraphPassNodeBase& pass)
		: m_renderGraph(renderGraph), m_pass(pass)
	{
	}

	Weak<RHI::ImageView> RenderGraphPassResources::GetImage2DView(const RenderGraphResourceHandle resourceHandle) const
	{
		ValidateResourceAccess(resourceHandle);
		return m_renderGraph.GetImage2DView(resourceHandle);
	}

	ResourceHandle RenderGraphPassResources::GetImage2D(const RenderGraphResourceHandle resourceHandle) const
	{
		ValidateResourceAccess(resourceHandle);
		return m_renderGraph.GetImage2D(resourceHandle);
	}

	Weak<RHI::StorageBuffer> RenderGraphPassResources::GetBufferRaw(const RenderGraphResourceHandle resourceHandle) const
	{
		ValidateResourceAccess(resourceHandle);
		return m_renderGraph.GetBufferRaw(resourceHandle);
	}

	ResourceHandle RenderGraphPassResources::GetBuffer(const RenderGraphResourceHandle resourceHandle) const
	{
		ValidateResourceAccess(resourceHandle);
		return m_renderGraph.GetBuffer(resourceHandle);
	}

	ResourceHandle RenderGraphPassResources::GetUniformBuffer(const RenderGraphResourceHandle resourceHandle) const
	{
		ValidateResourceAccess(resourceHandle);
		return m_renderGraph.GetUniformBuffer(resourceHandle);
	}

	void RenderGraphPassResources::ValidateResourceAccess(const RenderGraphResourceHandle resourceHandle) const
	{
		const bool isRegisteredForAccess = m_pass.ReadsResource(resourceHandle) || m_pass.WritesResource(resourceHandle) || m_pass.CreatesResource(resourceHandle);
		if (!isRegisteredForAccess)
		{
#ifndef VT_DIST
			VT_DEBUGBREAK();
#else
			VT_CORE_ERROR("[RenderGraph] Pass {0} trying to access resource with handle {1}, but it has not been registered for access!", m_pass.name, resourceHandle);
#endif
		}
	}
}
