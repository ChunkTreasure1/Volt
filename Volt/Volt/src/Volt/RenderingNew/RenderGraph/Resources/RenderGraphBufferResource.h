#pragma once

#include "RenderGraphResource.h"

#include <VoltRHI/Core/RHICommon.h>

namespace Volt
{
	struct RenderGraphBufferDesc
	{
		size_t size;
		RHI::BufferUsage usage = RHI::BufferUsage::None;
	};

	struct RenderGraphBuffer
	{
		RenderGraphBufferDesc description{};
		RenderGraphResourceHandle resourceHandle{};

		inline static constexpr ResourceType GetResourceType()
		{
			return ResourceType::Buffer;
		}
	};

	struct RenderGraphUniformBuffer
	{
		RenderGraphBufferDesc description{};
		RenderGraphResourceHandle resourceHandle{};

		inline static constexpr ResourceType GetResourceType()
		{
			return ResourceType::Buffer;
		}
	};
}
