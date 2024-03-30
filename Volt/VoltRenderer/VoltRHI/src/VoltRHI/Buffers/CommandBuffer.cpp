#include "rhipch.h"
#include "CommandBuffer.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Buffers/VulkanCommandBuffer.h>
#include <VoltD3D12/Buffers/D3D12CommandBuffer.h>

namespace Volt::RHI
{
	Ref<CommandBuffer> CommandBuffer::Create(const uint32_t count, QueueType queueType)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRef<D3D12CommandBuffer>(count, queueType); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRef<VulkanCommandBuffer>(count, queueType); break;
		}

		return nullptr;
	}

	Ref<CommandBuffer> CommandBuffer::Create(Weak<Swapchain> swapchain)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
		case GraphicsAPI::D3D12:
		case GraphicsAPI::MoltenVk:
			break;

		case GraphicsAPI::Vulkan: return CreateRef<VulkanCommandBuffer>(swapchain); break;
		}

		return nullptr;
	}

	Ref<CommandBuffer> CommandBuffer::Create()
	{
		const auto api = GraphicsContext::GetAPI();
		
		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRef<D3D12CommandBuffer>(1, QueueType::Graphics); break;
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRef<VulkanCommandBuffer>(1, QueueType::Graphics); break;
		}

		return nullptr;
	}
	
	CommandBuffer::CommandBuffer(QueueType queueType)
		: m_queueType(queueType)
	{
	}
}
