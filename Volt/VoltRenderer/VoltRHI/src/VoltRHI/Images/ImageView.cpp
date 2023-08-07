#include "rhipch.h"
#include "ImageView.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Images/VulkanImageView.h>

namespace Volt::RHI
{
	Ref<ImageView> ImageView::Create(const ImageViewSpecification specification)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanImageView>(specification); break;
		}

		return nullptr;
	}
}
