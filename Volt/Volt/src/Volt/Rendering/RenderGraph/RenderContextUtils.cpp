#include "vtpch.h"
#include "RenderContextUtils.h"

#include "Volt/Rendering/RenderGraph/RenderContext.h"

namespace Volt::RCUtils
{
	void DrawFullscreenTriangle(RenderContext& context, WeakPtr<RHI::RenderPipeline> pipeline, const std::function<void(RenderContext& context)>& setConstantsFunc)
	{
		context.BindPipeline(pipeline);

		if (setConstantsFunc)
		{
			setConstantsFunc(context);
		}

		context.Draw(3, 1, 0, 0);
	}
}
