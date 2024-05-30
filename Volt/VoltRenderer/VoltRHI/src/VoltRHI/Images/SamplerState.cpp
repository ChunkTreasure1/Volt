#include "rhipch.h"
#include "SamplerState.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<SamplerState> SamplerState::Create(const SamplerStateCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateSamplerState(createInfo);
	}
}
