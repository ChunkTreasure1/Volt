#pragma once

#include "RenderGraphResourceHandle.h"

#include <VoltRHI/Core/RHICommon.h>

namespace Volt
{
	struct RenderGraphBufferDesc
	{
		size_t size;
		RHI::BufferUsage usage = RHI::BufferUsage::StorageBuffer;
	};

	struct RenderGraphBuffer
	{
		RenderGraphBuffer(const size_t size, RHI::BufferUsage usage)
		{
			description.size = size;
			description.usage = usage | RHI::BufferUsage::StorageBuffer;
		}

		RenderGraphBufferDesc description{};
		RenderGraphResourceHandle resourceHandle{};
	};

	struct RenderGraphUniformBuffer
	{
		RenderGraphUniformBuffer(const size_t size, RHI::BufferUsage usage)
		{ 
			description.size = size;
			description.usage = usage | RHI::BufferUsage::UniformBuffer;
		}

		RenderGraphBufferDesc description{};
		RenderGraphResourceHandle resourceHandle{};
	};
}
