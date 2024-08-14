#pragma once

#include <RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h>

#include <string_view>

namespace Volt
{
	class RenderGraph;

	namespace RenderingUtils
	{
		extern RenderGraphResourceHandle GenerateIndirectArgs(RenderGraph& renderGraph, RenderGraphBufferHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName);
		extern RenderGraphResourceHandle GenerateIndirectArgsWrapped(RenderGraph& renderGraph, RenderGraphBufferHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName);

		extern void CopyImage(RenderGraph& renderGraph, RenderGraphImageHandle imageToCopy, RenderGraphImageHandle destinationImage, const glm::uvec2& renderSize);
	}
}
