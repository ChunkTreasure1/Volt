#include "rhipch.h"

#include "RHIModule/Buffers/CommandBuffer.h"
#include "RHIModule/RHIProxy.h"

namespace Volt::RHI
{
	RefPtr<CommandBuffer> CommandBuffer::Create(QueueType queueType)
	{
		return RHIProxy::GetInstance().CreateCommandBuffer(queueType);
	}

	RefPtr<CommandBuffer> CommandBuffer::Create()
	{
		return RHIProxy::GetInstance().CreateCommandBuffer(QueueType::Graphics);
	}
}
