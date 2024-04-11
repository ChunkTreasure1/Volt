#include "rhipch.h"
#include "ComputePipeline.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	Ref<ComputePipeline> ComputePipeline::Create(Ref<Shader> shader, bool useGlobalResources)
	{
		return RHIProxy::GetInstance().CreateComputePipeline(shader, useGlobalResources);
	}
}
