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
		RenderGraphResourceHandle taaOutput;
		RenderGraphResourceHandle previousColor;
	};

	class TAATechnique
	{
	public:
		TAATechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		TAAData Execute(Ref<RHI::Image2D> previousColor);

	private:
		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
