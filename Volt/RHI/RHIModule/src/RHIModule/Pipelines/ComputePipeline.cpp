#include "rhipch.h"
#include "ComputePipeline.h"

#include "RHIModule/RHIProxy.h"
#include "RHIModule/Shader/Shader.h"

namespace Volt::RHI
{
	RefPtr<ComputePipeline> ComputePipeline::Create(RefPtr<Shader> shader, bool useGlobalResources)
	{
		return RHIProxy::GetInstance().CreateComputePipeline(shader, useGlobalResources);
	}
}
