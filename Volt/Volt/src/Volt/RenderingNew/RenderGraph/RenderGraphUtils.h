#pragma once

#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphTextureResource.h"

namespace Volt
{
	namespace RGUtils
	{
		template<typename T>
		inline static RenderGraphBufferDesc CreateBufferDesc(const size_t count, const RHI::BufferUsage usage, const RHI::MemoryUsage memoryUsage, std::string_view name)
		{
			return { sizeof(T) * count, usage, memoryUsage, name};
		}

		template<typename T>
		inline static RenderGraphBufferDesc CreateBufferDescGPU(const size_t count, std::string_view name)
		{
			return { sizeof(T) * count, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, name };
		}

		template<RHI::PixelFormat format>
		inline static RenderGraphImageDesc CreateImage2DDesc(const uint32_t width, const uint32_t height, const RHI::ImageUsage usage, std::string_view name)
		{
			return { format, width, height, usage, name };
		}
	}
}
