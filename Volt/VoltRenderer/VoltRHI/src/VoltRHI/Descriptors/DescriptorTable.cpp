#include "rhipch.h"
#include "DescriptorTable.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Descriptors/VulkanDescriptorTable.h>
#include <VoltVulkan/Descriptors/VulkanDescriptorBufferTable.h>

// #TODO_Ivar: Temp
#include <VoltVulkan/Graphics/VulkanPhysicalGraphicsDevice.h>

namespace Volt::RHI
{
	Ref<DescriptorTable> DescriptorTable::Create(const DescriptorTableCreateInfo& specification)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
				break;

			case GraphicsAPI::Vulkan:
			{
				if (GraphicsContext::GetPhysicalDevice()->AsRef<VulkanPhysicalGraphicsDevice>().AreDescriptorBuffersEnabled())
				{
					return CreateRef<VulkanDescriptorBufferTable>(specification);
				}
				else
				{
					return CreateRef<VulkanDescriptorTable>(specification);
				}
				break;
			}
		}

		return nullptr;
	}
}
