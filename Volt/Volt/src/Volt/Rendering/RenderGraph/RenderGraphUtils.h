#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphTextureResource.h"

#include <string_view>

namespace Volt::RGUtils
{
	template<typename T, typename COUNT_TYPE>
	inline static RenderGraphBufferDesc CreateBufferDesc(const COUNT_TYPE count, const RHI::BufferUsage usage, const RHI::MemoryUsage memoryUsage, std::string_view name)
	{
		VT_CORE_ASSERT(count > 0, "Count must not be zero!");
		return { static_cast<uint32_t>(count), sizeof(T), usage, memoryUsage, name };
	}

	template<typename T, typename COUNT_TYPE>
	inline static RenderGraphBufferDesc CreateBufferDescGPU(const COUNT_TYPE count, std::string_view name)
	{
		VT_CORE_ASSERT(count > 0, "Count must not be zero!");
		return { static_cast<uint32_t>(count), sizeof(T), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, name };
	}

	template<RHI::PixelFormat format>
	inline static RenderGraphImageDesc CreateImage2DDesc(const uint32_t width, const uint32_t height, const RHI::ImageUsage usage, std::string_view name)
	{
		VT_CORE_ASSERT(width > 0 && height > 0, "Width and height must not be zero!");
		return { format, width, height, usage, name };
	}

	extern void ClearImage(RenderGraph& renderGraph, RenderGraphResourceHandle image, const glm::vec4& clearColor, const std::string& passName = "");
}
