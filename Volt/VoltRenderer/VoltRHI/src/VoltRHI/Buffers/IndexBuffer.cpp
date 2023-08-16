#include <rhipch.h>
#include "IndexBuffer.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Buffers/VulkanIndexBuffer.h>

namespace Volt::RHI
{
	Ref<IndexBuffer> IndexBuffer::Create(std::span<uint32_t> indices)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanIndexBuffer>(indices); break;
		}

		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(const uint32_t* indices, uint32_t count)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanIndexBuffer>(indices, count); break;
		}

		return nullptr;
	}
}
