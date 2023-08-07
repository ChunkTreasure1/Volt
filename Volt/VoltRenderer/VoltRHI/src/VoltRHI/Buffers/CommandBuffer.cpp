#include "rhipch.h"
#include "CommandBuffer.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltMock/Buffers/MockCommandBuffer.h>
#include <VoltVulkan/Buffers/VulkanCommandBuffer.h>
#include <VoltD3D12/Buffers/D3D12CommandBuffer.h>

namespace Volt::RHI
{
	Ref<CommandBuffer> CommandBuffer::Create(const uint32_t count, QueueType queueType, bool swapchainTarget)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRefRHI<D3D12CommandBuffer>(count, queueType, swapchainTarget); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanCommandBuffer>(count, queueType, swapchainTarget); break;
			case GraphicsAPI::Mock: return CreateRefRHI<MockCommandBuffer>(count, queueType); break;
		}

		return nullptr;
	}

	Ref<CommandBuffer> CommandBuffer::Create()
	{
		const auto api = GraphicsContext::GetAPI();
		
		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRefRHI<D3D12CommandBuffer>(1, QueueType::Graphics, false); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanCommandBuffer>(1, QueueType::Graphics, false); break;
			case GraphicsAPI::Mock: return CreateRefRHI<MockCommandBuffer>(1, QueueType::Graphics); break;
		}

		return nullptr;
	}
	
	CommandBuffer::CommandBuffer(QueueType queueType)
		: m_queueType(queueType)
	{
	}
}
