#include "rhipch.h"
#include "RenderPipeline.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	Ref<RenderPipeline> RenderPipeline::Create(const RenderPipelineCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateRenderPipeline(createInfo);
	}
}
