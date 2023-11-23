#include "rhipch.h"
#include "BufferView.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Buffers/VulkanBufferView.h>

namespace Volt::RHI 
{
	Ref<BufferView> BufferView::Create(const BufferViewSpecification& specification)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRef<VulkanBufferView>(specification); break;
		}

		return nullptr;
	}
}
