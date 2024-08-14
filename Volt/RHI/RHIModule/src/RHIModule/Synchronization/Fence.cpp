#include "rhipch.h"
#include "Fence.h"

#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<Fence> Fence::Create(const FenceCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateFence(createInfo);
	}
}
