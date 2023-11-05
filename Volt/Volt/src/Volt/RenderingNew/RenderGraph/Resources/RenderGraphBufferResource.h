#pragma once

#include "RenderGraphResource.h"

#include <VoltRHI/Core/RHICommon.h>

namespace Volt
{
	struct RenderGraphBufferDesc
	{
		size_t size;
		RHI::BufferUsage usage = RHI::BufferUsage::None;
		RHI::MemoryUsage memoryUsage = RHI::MemoryUsage::GPU;

		std::string_view name;
	};

	struct RenderGraphBuffer
	{
		RenderGraphBufferDesc description{};
		bool isExternal = false;
		bool trackGlobalResource = true;

		inline static constexpr ResourceType GetResourceType()
		{
			return ResourceType::Buffer;
		}
	};

	struct RenderGraphUniformBuffer
	{
		RenderGraphBufferDesc description{};
		bool isExternal = false;
		bool trackGlobalResource = true;

		inline static constexpr ResourceType GetResourceType()
		{
			return ResourceType::UniformBuffer;
		}
	};
}
