#pragma once

#include "Volt/Rendering/RenderGraph/Resources/RenderGraphResourceHandle.h"

namespace Volt
{
	struct DrawCullingData
	{
		RenderGraphResourceHandle countCommandBuffer;
		RenderGraphResourceHandle taskCommandsBuffer;
	};

	class RenderGraph;
	class RenderGraphBlackboard;

	class CullingTechnique
	{
	public:
		enum class Type : uint32_t
		{
			Perspective = 0,
			Orthographic
		};

		struct Info
		{
			Type type = Type::Perspective;
			glm::mat4 viewMatrix;
			glm::vec4 cullingFrustum;
			float nearPlane;
			float farPlane;

			uint32_t drawCommandCount;
			uint32_t meshletCount;
		};

		CullingTechnique(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		DrawCullingData Execute(const Info& info);

	private:
		DrawCullingData AddDrawCallCullingPass(const Info& info);
		void AddTaskSubmitSetupPass(const DrawCullingData& data);

		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
