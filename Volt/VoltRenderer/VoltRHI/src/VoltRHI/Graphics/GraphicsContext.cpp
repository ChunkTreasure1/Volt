#include "rhipch.h"
#include "GraphicsContext.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	GraphicsContext::GraphicsContext()
	{
		s_context = this;
	}

	GraphicsContext::~GraphicsContext()
	{
		s_context = nullptr;
	}

	Ref<GraphicsContext> GraphicsContext::Create(const GraphicsContextCreateInfo& createInfo)
	{
		s_graphicsAPI = createInfo.graphicsApi;
		s_logHook = createInfo.loghookInfo;
		s_resourceManagementInfo = createInfo.resourceManagementInfo;

		return RHIProxy::GetInstance().CreateGraphicsContext(createInfo);
	}

	void GraphicsContext::DestroyResource(std::function<void()>&& function)
	{
		if (s_resourceManagementInfo.resourceDeletionCallback)
		{
			s_resourceManagementInfo.resourceDeletionCallback(std::move(function));
		}
		else
		{
			function();
		}
	}

	void GraphicsContext::Update()
	{
		Get().GetDefaultAllocatorImpl().Update();
		Get().GetTransientAllocatorImpl()->Update();
	}

	void GraphicsContext::LogUnformatted(Severity logSeverity, std::string_view message)
	{
		if (!s_logHook.enabled || !s_logHook.logCallback)
		{
			return;
		}

		s_logHook.logCallback(logSeverity, message);
	}
}
