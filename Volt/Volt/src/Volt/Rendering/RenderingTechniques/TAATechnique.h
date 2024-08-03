#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"

namespace Volt
{
	namespace RHI
	{
		class Image2D;
	}

	class RenderGraph;
	class RenderGraphBlackboard;

	struct TAAData
	{
		RenderGraphImage2DHandle taaOutput;
		RenderGraphImage2DHandle previousColor;
	};

	class TAATechnique
	{
	public:
		TAATechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		TAAData Execute(RefPtr<RHI::Image2D> previousColor, RenderGraphImage2DHandle velocityTexture);

	private:
		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
