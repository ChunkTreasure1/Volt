#pragma once

#include "Volt/Core/Base.h"

namespace Volt 
{
	class VulkanFramebuffer;
	struct VulkanRenderPass
	{
		Ref<VulkanFramebuffer> framebuffer;
		Ref<RenderPipeline> overridePipeline;

		std::string debugName;
		gem::vec4 debugColor = { 1.f };
	};
}