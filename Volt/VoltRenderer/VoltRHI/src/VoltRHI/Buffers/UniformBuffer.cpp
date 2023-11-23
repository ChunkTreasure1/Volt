#include "rhipch.h"
#include "UniformBuffer.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Buffers/VulkanUniformBuffer.h>

namespace Volt::RHI
{
	Ref<UniformBuffer> UniformBuffer::Create(const uint32_t size, const void* data)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRef<VulkanUniformBuffer>(size, data); break;
		}

		return nullptr;
	}
}
