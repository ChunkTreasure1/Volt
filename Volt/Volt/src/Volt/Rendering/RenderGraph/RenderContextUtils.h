#pragma once

#include <RHIModule/Pipelines/RenderPipeline.h>

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
		extern void DrawFullscreenTriangle(RenderContext& context, WeakPtr<RHI::RenderPipeline> pipeline, const std::function<void(RenderContext& context)>& setConstantsFunc = {});
	}
}
