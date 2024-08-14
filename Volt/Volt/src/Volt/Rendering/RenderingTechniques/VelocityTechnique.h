#pragma once

#include <RenderCore/RenderGraph/Resources/RenderGraphResourceHandle.h>

#include "Volt/Core/Base.h"

namespace Volt
{
	class RenderGraph;
	class RenderGraphBlackboard;

	class Camera;

	struct PreviousFrameData;

	class VelocityTechnique
	{
	public:
		VelocityTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);

		RenderGraphResourceHandle Execute();

	private:
		RenderGraphResourceHandle ExecuteReprojectVelocity();

		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
