#include "rhipch.h"

#include "RHIModule/Memory/TransientHeap.h"
#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<TransientHeap> TransientHeap::Create(const TransientHeapCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateTransientHeap(createInfo);
	}
}
