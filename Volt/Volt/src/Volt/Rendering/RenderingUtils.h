#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"

#include <string_view>

namespace Volt
{
	class RenderGraph;

	namespace RenderingUtils
	{
		extern RenderGraphResourceHandle GenerateIndirectArgs(RenderGraph& renderGraph, RenderGraphResourceHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName);
		extern RenderGraphResourceHandle GenerateIndirectArgsWrapped(RenderGraph& renderGraph, RenderGraphResourceHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName);

		extern void CopyImage(RenderGraph& renderGraph, RenderGraphResourceHandle imageToCopy, RenderGraphResourceHandle destinationImage, const glm::uvec2& renderSize);
	}
}
