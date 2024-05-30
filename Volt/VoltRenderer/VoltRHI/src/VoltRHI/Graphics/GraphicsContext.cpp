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

	RefPtr<GraphicsContext> GraphicsContext::Create(const GraphicsContextCreateInfo& createInfo)
	{
		s_graphicsAPI = createInfo.graphicsApi;
		return RHIProxy::GetInstance().CreateGraphicsContext(createInfo);
	}

	void GraphicsContext::Update()
	{
		Get().GetDefaultAllocatorImpl().Update();
		Get().GetTransientAllocatorImpl()->Update();
	}
}
