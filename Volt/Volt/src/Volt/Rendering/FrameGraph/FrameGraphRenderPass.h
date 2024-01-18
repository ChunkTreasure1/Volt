#pragma once

#include "Volt/Rendering/FrameGraph/FrameGraphResource.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace Volt
{
	class FrameGraph;
	class RenderPipeline;
	class CommandBuffer;

	struct FrameGraphRenderPassNodeBase
	{
		std::string name;
		uint32_t index = 0;
		uint32_t refCount = 0;

		bool isComputePass = false;
		bool isCulled = false;
		bool hasSideEffect = false;

		std::vector<FrameGraphResourceHandle> resourceReads; // These are inserted before pass execution
		std::vector<FrameGraphResourceHandle> resourceWrites;
		std::vector<FrameGraphResourceHandle> resourceCreates;

		virtual void Execute(FrameGraph& frameGraph, Ref<CommandBuffer> commandBuffer) = 0;

		const bool ReadsResource(FrameGraphResourceHandle handle) const;
		const bool WritesResource(FrameGraphResourceHandle handle) const;
		const bool CreatesResource(FrameGraphResourceHandle handle) const;
		const bool IsCulled() const;
	};

	template<typename T>
	struct FrameGraphRenderPassNode : public FrameGraphRenderPassNodeBase
	{
		T data{};
		std::function<void(const T& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)> executeFunction;

		void Execute(FrameGraph& frameGraph, Ref<CommandBuffer> commandBuffer) override
		{
			FrameGraphRenderPassResources resources(frameGraph, *this);
			executeFunction(data, resources, commandBuffer);
		}
	};

	struct FrameGraphRenderingInfo
	{
		std::vector<VkRenderingAttachmentInfo> colorAttachmentInfo{};
		VkRenderingAttachmentInfo depthAttachmentInfo{};
		bool hasDepth = false;

		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t layers = 1;
	};

	struct FrameGraphRenderPassInfo
	{
		std::string_view name;
		glm::vec4 color;
		Ref<RenderPipeline> overridePipeline;
	};
}
