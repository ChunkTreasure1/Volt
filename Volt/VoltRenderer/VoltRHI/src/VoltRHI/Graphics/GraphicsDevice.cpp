#include "rhipch.h"
#include "GraphicsDevice.h"

#include "VoltRHI/Buffers/CommandBuffer.h"
#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltMock/Graphics/MockGraphicsDevice.h>
#include <VoltVulkan/Graphics/VulkanGraphicsDevice.h>
#include <VoltD3D12/Graphics/D3D12GraphicsDevice.h>

namespace Volt
{
	Ref<GraphicsDevice> GraphicsDevice::Create(const GraphicsDeviceCreateInfo& deviceInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRefRHI<D3D12GraphicsDevice>(deviceInfo); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanGraphicsDevice>(deviceInfo); break;
			case GraphicsAPI::Mock: return CreateRefRHI<MockGraphicsDevice>(deviceInfo); break;
		}

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
