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
			case GraphicsAPI::D3D12: return CreateRefRHI<D3D12Image2D>(specification, data); break;
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanImage2D>(specification, data); break;
		}

		return nullptr;
	}

	Ref<Image2D> Image2D::Create(const ImageSpecification& specification, Ref<MemoryPool> pool)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Mock:
				break;

			case GraphicsAPI::Vulkan: return CreateRefRHI<VulkanImage2D>(specification, pool); break;
		}

		return nullptr;
	}
}
