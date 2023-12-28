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
		inline static size_t GetHashFromImageDesc(const RenderGraphImageDesc& imageDesc)
		{
			size_t hash = std::hash<uint32_t>()(static_cast<uint32_t>(imageDesc.format));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(imageDesc.usage)));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(static_cast<uint32_t>(imageDesc.clearMode)));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(imageDesc.width));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(imageDesc.height));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(imageDesc.depth));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(imageDesc.layers));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(imageDesc.mips));
			hash = Math::HashCombine(hash, std::hash<uint32_t>()(imageDesc.isCubeMap));

			return hash;
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
