#pragma once

#include "RenderGraphResource.h"

#include "Volt/Math/Math.h"

#include <VoltRHI/Images/Image2D.h>

namespace Volt
{
	struct RenderGraphImageDesc
	{
		RenderGraphImageDesc() = default;

		RenderGraphImageDesc(const RHI::PixelFormat format, const uint32_t width, const uint32_t height, const RHI::ImageUsage usage, std::string_view name)
		{ 
			this->format = format;
			this->width = width;
			this->height = height;
			this->usage = usage;
			this->name = name;
		}

		RHI::PixelFormat format = RHI::PixelFormat::R8G8B8A8_UNORM;
		RHI::ImageUsage usage = RHI::ImageUsage::Attachment;
		RHI::ClearMode clearMode = RHI::ClearMode::Clear;

		std::string_view name;

		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t depth = 1;

		uint32_t layers = 1;
		uint32_t mips = 1;

		bool isCubeMap = false;
	};

	namespace Utility
	{
		inline static size_t GetHashFromImageDesc(const RenderGraphImageDesc& desc)
		{
			size_t result = std::hash<uint32_t>()(static_cast<uint32_t>(desc.format));
			result = Math::HashCombine(result, std::hash<uint32_t>()(static_cast<uint32_t>(desc.usage)));
			result = Math::HashCombine(result, std::hash<uint32_t>()(static_cast<uint32_t>(desc.clearMode)));
			result = Math::HashCombine(result, std::hash<uint32_t>()(desc.width));
			result = Math::HashCombine(result, std::hash<uint32_t>()(desc.height));
			result = Math::HashCombine(result, std::hash<uint32_t>()(desc.depth));
			result = Math::HashCombine(result, std::hash<uint32_t>()(desc.layers));
			result = Math::HashCombine(result, std::hash<uint32_t>()(desc.mips));
			result = Math::HashCombine(result, std::hash<bool>()(desc.isCubeMap));

			return result;
		}
	}

	struct RenderGraphImage2D
	{
		RenderGraphImageDesc description{};
		bool isExternal = false;
		bool trackGlobalResource = true;

		inline static constexpr ResourceType GetResourceType()
		{
			return ResourceType::Image2D;
		}
	};

	struct RenderGraphTexture3D
	{
		RenderGraphImageDesc description{};
		bool isExternal = false;

		inline static constexpr ResourceType GetResourceType()
		{
			return ResourceType::Image3D;
		}
	};
}
