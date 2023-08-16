#include "rhipch.h"
#include "ImageView.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Images/VulkanImageView.h>
#include <VoltD3D12/Images/D3D12ImageView.h>

namespace Volt::RHI
{
	Ref<ImageView> ImageView::Create(const ImageViewSpecification specification)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12: return CreateRefRHI<D3D12ImageView>(specification); break;
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanImageView>(specification); break;
		}

		return nullptr;
	}
}
