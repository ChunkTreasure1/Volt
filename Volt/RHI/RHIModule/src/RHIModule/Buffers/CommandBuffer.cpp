#include "rhipch.h"
#include "CommandBuffer.h"

#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<CommandBuffer> CommandBuffer::Create(const uint32_t count, QueueType queueType)
	{
		return RHIProxy::GetInstance().CreateCommandBuffer(count, queueType);
	}

	RefPtr<CommandBuffer> CommandBuffer::Create()
	{
		return RHIProxy::GetInstance().CreateCommandBuffer(1, QueueType::Graphics);
	}
}
