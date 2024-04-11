#include "rhipch.h"
#include "CommandBuffer.h"

#include "VoltRHI/RHIProxy.h"

namespace Volt::RHI
{
	Ref<CommandBuffer> CommandBuffer::Create(const uint32_t count, QueueType queueType)
	{
		return RHIProxy::GetInstance().CreateCommandBuffer(count, queueType);
	}

	Ref<CommandBuffer> CommandBuffer::Create(Weak<Swapchain> swapchain)
	{
		return RHIProxy::GetInstance().CreateCommandBuffer(swapchain);
	}

	Ref<CommandBuffer> CommandBuffer::Create()
	{
		return RHIProxy::GetInstance().CreateCommandBuffer(1, QueueType::Graphics);
	}
}
