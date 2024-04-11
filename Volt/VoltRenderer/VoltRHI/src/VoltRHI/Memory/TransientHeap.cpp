#include "rhipch.h"
#include "TransientHeap.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	Ref<TransientHeap> TransientHeap::Create(const TransientHeapCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateTransientHeap(createInfo);
	}
}
