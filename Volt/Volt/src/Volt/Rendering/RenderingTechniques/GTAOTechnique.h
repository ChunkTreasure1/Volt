#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"
#include "Volt/Rendering/SceneRendererStructs.h"

namespace Volt
{
	class RenderGraph;
	class RenderGraphBlackboard;

	class Camera;
	struct GTAOSettings;

	class GTAOTechnique
	{
	public:
		GTAOTechnique(uint64_t frameIndex, const GTAOSettings& settings);

		GTAOOutput Execute(RenderGraph& frameGraph, RenderGraphBlackboard& blackboard);

	private:
		friend struct PrefilterDepthData;

		void AddPrefilterDepthPass(RenderGraph& frameGraph, RenderGraphBlackboard& blackboard);
		void AddMainPass(RenderGraph& frameGraph, RenderGraphBlackboard& blackboard);
		GTAOOutput AddDenoisePass(RenderGraph& frameGraph, RenderGraphBlackboard& blackboard);

		struct GTAOConstants
		{
			glm::ivec2 ViewportSize;
			glm::vec2 ViewportPixelSize;                  // .zw == 1.0 / ViewportSize.xy

			glm::vec2 DepthUnpackConsts = { -0.01f, -1.e10f };
			glm::vec2 CameraTanHalfFOV = { 0.99057f, 0.52057f };

			glm::vec2 NDCToViewMul = { 1.98115f, -1.04113f };
			glm::vec2 NDCToViewAdd = { -0.99057f, 0.52057f };

			glm::vec2 NDCToViewMul_x_PixelSize = { NDCToViewMul.x * ViewportPixelSize.x, NDCToViewMul.y * ViewportPixelSize.y };
			float EffectRadius = 0.5f;                       // world (viewspace) maximum size of the shadow
			float EffectFalloffRange = 0.615f;

			float RadiusMultiplier = 1.457f;
			float Padding0;
			float FinalValuePower = 2.2f;
			float DenoiseBlurBeta = 1.2f;

			float SampleDistributionPower = 2.f;
			float ThinOccluderCompensation = 0.f;
			float DepthMIPSamplingOffset = 3.3f;
			int NoiseIndex = 0;                         // frameIndex % 64 if using TAA or 0 otherwise
		} m_constants;

		uint64_t m_frameIndex = 0;
	};
}
