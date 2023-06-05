#include "rhipch.h"
#include "GraphicsDevice.h"

#include "VoltRHI/Buffers/CommandBuffer.h"

namespace Volt
{
	Ref<GraphicsDevice> GraphicsDevice::Create(const GraphicsDeviceInfo& deviceInfo)
	{
		// #TODO: Implement
		return nullptr;
	}

	GraphicsDevice::GraphicsDevice()
	{
		m_deviceQueueMutexes[QueueType::Graphics];
		m_deviceQueueMutexes[QueueType::Compute];
		m_deviceQueueMutexes[QueueType::TransferCopy];
	}

	Ref<CommandBuffer> GraphicsDevice::GetSingleUseCommandBuffer(QueueType queueType)
	{
		Ref<CommandBuffer> commandBuffer = CommandBuffer::Create(1, queueType);
		return commandBuffer;
	}

	void GraphicsDevice::ExecuteSingleUseCommandBuffer(Ref<CommandBuffer> commandBuffer)
	{
		const auto queueType = commandBuffer->GetQueueType();

		std::scoped_lock lock{ m_deviceQueueMutexes[queueType] };
		m_deviceQueues[queueType]->Execute({ commandBuffer });
	}
}
