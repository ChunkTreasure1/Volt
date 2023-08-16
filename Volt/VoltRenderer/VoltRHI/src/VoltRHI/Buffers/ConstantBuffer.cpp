#include "rhipch.h"
#include "ConstantBuffer.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Buffers/VulkanConstantBuffer.h>

namespace Volt::RHI
{
	Ref<ConstantBuffer> ConstantBuffer::Create(const uint32_t size, const void* data)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanConstantBuffer>(size, data); break;
		}

		return nullptr;
	}
}
