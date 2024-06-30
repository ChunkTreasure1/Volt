#include "rhipch.h"
#include "Semaphore.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<Semaphore> Semaphore::Create(const SemaphoreCreateInfo& createInfo)
	{
		return RHIProxy::GetInstance().CreateSemaphore(createInfo);
	}
}
