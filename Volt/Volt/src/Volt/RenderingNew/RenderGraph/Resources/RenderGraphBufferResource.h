#pragma once

#include "RenderGraphResource.h"

#include "Volt/Math/Math.h"

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

	namespace Utility
	{
		inline static size_t GetHashFromBufferDesc(const RenderGraphBufferDesc& desc)
		{
			size_t result = std::hash<size_t>()(desc.size);
			result = Math::HashCombine(result, std::hash<uint32_t>()(static_cast<uint32_t>(desc.usage)));
			result = Math::HashCombine(result, std::hash<uint32_t>()(static_cast<uint32_t>(desc.memoryUsage)));

			return result;
		}
	}

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
