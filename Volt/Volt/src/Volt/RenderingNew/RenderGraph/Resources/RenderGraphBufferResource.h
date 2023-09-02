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
		RenderGraphBufferDesc description{};
		RenderGraphResourceHandle resourceHandle{};
	};

	struct RenderGraphUniformBuffer
	{
		RenderGraphBufferDesc description{};
		RenderGraphResourceHandle resourceHandle{};
	};
}
