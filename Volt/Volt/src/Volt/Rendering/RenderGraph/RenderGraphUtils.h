#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphTextureResource.h"

namespace Volt
{
	namespace RGUtils
	{
		template<typename T>
		inline static RenderGraphBufferDesc CreateBufferDesc(const size_t count, const RHI::BufferUsage usage, const RHI::MemoryUsage memoryUsage, std::string_view name)
		{
			VT_CORE_ASSERT(count > 0, "Count must not be zero!");
			return { sizeof(T) * count, usage, memoryUsage, name};
		}

		template<typename T>
		inline static RenderGraphBufferDesc CreateBufferDescGPU(const size_t count, std::string_view name)
		{
			VT_CORE_ASSERT(count > 0, "Count must not be zero!");
			return { sizeof(T) * count, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, name };
		}

		template<RHI::PixelFormat format>
		inline static RenderGraphImageDesc CreateImage2DDesc(const uint32_t width, const uint32_t height, const RHI::ImageUsage usage, std::string_view name)
		{
			VT_CORE_ASSERT(width > 0 && height > 0, "Width and height must not be zero!");
			return { format, width, height, usage, name };
		}
	}
}
