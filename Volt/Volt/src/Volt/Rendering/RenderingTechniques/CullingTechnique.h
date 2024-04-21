#pragma once

#include "Volt/Rendering/SceneRendererStructs.h"
#include "Volt/Core/Base.h"

namespace Volt
{
	class RenderGraph;
	class RenderGraphBlackboard;

	class Camera;
	class RenderScene;

	enum class CullingMode : uint32_t
	{
		Perspective = 0,
		Orthographic = 1,
		None = 2
	};

	class CullingTechnique
	{
	public:
		CullingTechnique(RenderGraph& rg, RenderGraphBlackboard& blackboard);

		CullPrimitivesData Execute(Ref<Camera> camera, Ref<RenderScene> renderScene, const CullingMode cullingMode, const glm::vec2& renderSize, const uint32_t instanceCount = 1);
	
	private:
		CullObjectsData AddCullObjectsPass(Ref<Camera> camera, Ref<RenderScene> renderScene, const CullingMode cullingMode);
		CullMeshletsData AddCullMeshletsPass(Ref<Camera> camera, Ref<RenderScene> renderScene, const CullingMode cullingMode, const CullObjectsData& cullObjectsData);
		CullPrimitivesData AddCullPrimitivesPass(Ref<Camera> camera, Ref<RenderScene> renderScene, const CullMeshletsData& cullMeshletsData, const glm::vec2& renderSize, const uint32_t instanceCount);

		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
