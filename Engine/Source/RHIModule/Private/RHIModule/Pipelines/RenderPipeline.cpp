#include "rhipch.h"

#include "RHIModule/Pipelines/RenderPipeline.h"
#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<RenderPipeline> RenderPipeline::Create(const RenderPipelineCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateRenderPipeline(createInfo);
	}
}
