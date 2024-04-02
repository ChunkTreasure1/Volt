#pragma once

#include <functional>

namespace Volt
{
	class RenderContext;

	namespace RHI
	{
		class RenderPipeline;
	}

	namespace RCUtils
	{
		extern void DrawFullscreenTriangle(RenderContext& context, Weak<RHI::RenderPipeline> pipeline, const std::function<void(RenderContext& context)>& setConstantsFunc = {});
	}
}
