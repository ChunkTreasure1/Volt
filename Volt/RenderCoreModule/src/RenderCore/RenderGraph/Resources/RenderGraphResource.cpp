#include "rcpch.h"
#include "RenderGraphResource.h"

#include "RenderCore/RenderGraph/RenderGraphPass.h"
#include "RenderCore/RenderGraph/RenderGraph.h"

namespace Volt
{
	RenderGraphPassResources::RenderGraphPassResources(RenderGraph& renderGraph, RenderGraphPassNodeBase& pass)
		: m_renderGraph(renderGraph), m_pass(pass)
	{
	}

	ResourceHandle RenderGraphPassResources::GetImage2D(const RenderGraphImage2DHandle resourceHandle, const int32_t mip, const int32_t layer) const
	{
		ValidateResourceAccess(resourceHandle);
		auto handle = m_renderGraph.GetImage2D(resourceHandle, mip, layer);

#ifdef VT_DEBUG
		m_pass.m_resourceHandleMapping[handle] = resourceHandle;
#endif

		return handle;
	}

	ResourceHandle RenderGraphPassResources::GetImage3D(const RenderGraphImage3DHandle resourceHandle, const int32_t mip, const int32_t layer) const
	{
		ValidateResourceAccess(resourceHandle);
		auto handle = m_renderGraph.GetImage3D(resourceHandle, mip, layer);

#ifdef VT_DEBUG
		m_pass.m_resourceHandleMapping[handle] = resourceHandle;
#endif

		return handle;
	}

	ResourceHandle RenderGraphPassResources::GetBuffer(const RenderGraphBufferHandle resourceHandle) const
	{
		ValidateResourceAccess(resourceHandle);
		auto handle = m_renderGraph.GetBuffer(resourceHandle);
		
#ifdef VT_DEBUG
		m_pass.m_resourceHandleMapping[handle] = resourceHandle;
#endif
		
		return handle;
	}

	ResourceHandle RenderGraphPassResources::GetUniformBuffer(const RenderGraphUniformBufferHandle resourceHandle) const
	{
		ValidateResourceAccess(resourceHandle);
		auto handle = m_renderGraph.GetUniformBuffer(resourceHandle);

#ifdef VT_DEBUG
		m_pass.m_resourceHandleMapping[handle] = resourceHandle;
#endif

		return handle;
	}

	void RenderGraphPassResources::ValidateResourceAccess(const RenderGraphResourceHandle resourceHandle) const
	{
		const bool isRegisteredForAccess = m_pass.ReadsResource(resourceHandle) || m_pass.WritesResource(resourceHandle) || m_pass.CreatesResource(resourceHandle);

		if (!isRegisteredForAccess)
		{
			VT_LOG(LogVerbosity::Error, "[RenderGraph] Pass {0} trying to access resource with handle {1}, but it has not been registered for access!", m_pass.name, resourceHandle);
		}

		VT_ENSURE(isRegisteredForAccess);
	}
}
