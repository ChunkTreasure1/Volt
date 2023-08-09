#include "rhipch.h"
#include "DescriptorTable.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Descriptors/VulkanDescriptorTable.h>

namespace Volt::RHI
{
	Ref<DescriptorTable> DescriptorTable::Create(const DescriptorTableSpecification& specification)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanDescriptorTable>(specification); break;
		}

		return nullptr;
	}
}
