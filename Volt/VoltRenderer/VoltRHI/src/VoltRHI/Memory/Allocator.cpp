#include "rhipch.h"
#include "Allocator.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<TransientAllocator> TransientAllocator::Create()
	{
		return RHIProxy::GetInstance().CreateTransientAllocator();
	}

	Scope<DefaultAllocator> DefaultAllocator::Create()
	{
		return RHIProxy::GetInstance().CreateDefaultAllocator();
	}
}
