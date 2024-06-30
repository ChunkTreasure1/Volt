#include "rhipch.h"
#include "ComputePipeline.h"

#include "VoltRHI/RHIProxy.h"
#include "VoltRHI/Shader/Shader.h"

namespace Volt::RHI
{
	RefPtr<ComputePipeline> ComputePipeline::Create(RefPtr<Shader> shader, bool useGlobalResources)
	{
		return RHIProxy::GetInstance().CreateComputePipeline(shader, useGlobalResources);
	}
}
