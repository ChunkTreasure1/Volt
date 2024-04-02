#include "rhipch.h"
#include "Image2D.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

#include <VoltVulkan/Images/VulkanImage2D.h>
#include <VoltD3D12/Images/D3D12Image2D.h>

namespace Volt::RHI
{
	Ref<Image2D> Image2D::Create(const ImageSpecification& specification, const void* data /* = nullptr */)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::D3D12: return CreateRef<D3D12Image2D>(specification, data); break;
			case GraphicsAPI::Vulkan: return CreateRef<VulkanImage2D>(specification, data); break;
		}

		return nullptr;
	}

	Ref<Image2D> Image2D::Create(const ImageSpecification& specification, Ref<Allocator> customAllocator, const void* data)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::D3D12: return CreateRef<D3D12Image2D>(specification, customAllocator, data); break;
			case GraphicsAPI::Vulkan: return CreateRef<VulkanImage2D>(specification, customAllocator, data); break;
		}

		return nullptr;
	}

	Ref<Image2D> Image2D::Create(const SwapchainImageSpecification& specification)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
		case GraphicsAPI::MoltenVk:
		case GraphicsAPI::Mock:
		case GraphicsAPI::D3D12:
			break;

		case GraphicsAPI::Vulkan: return CreateRef<VulkanImage2D>(specification); break;
		}

		return nullptr;
	}
}
