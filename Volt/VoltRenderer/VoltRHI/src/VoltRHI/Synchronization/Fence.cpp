#include "rhipch.h"
#include "Fence.h"

#include "VoltRHI/Graphics/GraphicsContext.h"

namespace Volt::RHI
{
	Ref<Fence> Fence::Create(const FenceCreateInfo& createInfo)
	{
		const auto api = GraphicsContext::GetAPI();

		switch (api)
		{
			case GraphicsAPI::D3D12:
			case GraphicsAPI::Mock:
			case GraphicsAPI::MoltenVk:
			case GraphicsAPI::Vulkan:
				break;
		}

		return nullptr;
	}
}
