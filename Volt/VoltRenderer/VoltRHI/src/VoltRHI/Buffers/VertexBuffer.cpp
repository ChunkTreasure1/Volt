#include "rhipch.h"
#include "VertexBuffer.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Buffers/VulkanVertexBuffer.h>

namespace Volt::RHI
{
	Ref<VertexBuffer> VertexBuffer::Create(const void* data, const uint32_t size)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanVertexBuffer>(data, size); break;
		}

		return nullptr;
	}
}
