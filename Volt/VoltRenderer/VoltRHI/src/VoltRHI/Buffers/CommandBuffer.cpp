#include "rhipch.h"
#include "CommandBuffer.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Buffers/VulkanCommandBuffer.h>
#include <VoltD3D12/Buffers/D3D12CommandBuffer.h>

namespace Volt::RHI
{
	Ref<CommandBuffer> CommandBuffer::Create(const uint32_t count, QueueType queueType, bool swapchainTarget)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRef<D3D12CommandBuffer>(count, queueType, swapchainTarget); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRef<VulkanCommandBuffer>(count, queueType, swapchainTarget); break;
		}

		return nullptr;
	}

	Ref<CommandBuffer> CommandBuffer::Create()
	{
		const auto api = GraphicsContext::GetAPI();
		
		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRef<D3D12CommandBuffer>(1, QueueType::Graphics, false); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRef<VulkanCommandBuffer>(1, QueueType::Graphics, false); break;
		}

		return nullptr;
	}
	
	CommandBuffer::CommandBuffer(QueueType queueType)
		: m_queueType(queueType)
	{
	}
}
