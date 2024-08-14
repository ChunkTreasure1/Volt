#pragma once

#include <RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h>
#include <RHIModule/Images/Image.h>

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
		RenderGraphImageHandle taaOutput;
		RenderGraphImageHandle previousColor;
	};

	class TAATechnique
	{
	public:
		TAATechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		TAAData Execute(RefPtr<RHI::Image> previousColor, RenderGraphImageHandle velocityTexture);

	private:
		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
