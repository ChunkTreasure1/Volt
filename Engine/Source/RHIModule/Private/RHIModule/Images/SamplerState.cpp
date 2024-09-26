#include "rhipch.h"

#include "RHIModule/Images/SamplerState.h"
#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<SamplerState> SamplerState::Create(const SamplerStateCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateSamplerState(createInfo);
	}
}
