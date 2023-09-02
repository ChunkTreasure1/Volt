#pragma once

#include "RenderGraphResourceHandle.h"

#include <VoltRHI/Images/Image2D.h>

namespace Volt
{
	struct RenderGraphImageDesc
	{
		RHI::Format format = RHI::Format::R8G8B8A8_UNORM;
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

	struct RenderGraphTexture2D
	{
		RenderGraphImageDesc description{};
		bool isExternal = false;
	};

	struct RenderGraphTexture3D
	{
		RenderGraphImageDesc description{};
		bool isExternal = false;
	};
}
