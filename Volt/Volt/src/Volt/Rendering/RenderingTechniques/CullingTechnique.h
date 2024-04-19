#pragma once

#include "Volt/Rendering/SceneRendererStructs.h"
#include "Volt/Core/Base.h"

namespace Volt
{
	class RenderGraph;
	class RenderGraphBlackboard;

	class Camera;
	class RenderScene;

	class CullingTechnique
	{
	public:
		CullingTechnique(RenderGraph& rg, RenderGraphBlackboard& blackboard);

		CullPrimitivesData Execute(Ref<Camera> camera, Ref<RenderScene> renderScene, const uint32_t instanceCount = 1);
	
	private:
		CullObjectsData AddCullObjectsPass(Ref<Camera> camera, Ref<RenderScene> renderScene);
		CullMeshletsData AddCullMeshletsPass(Ref<Camera> camera, Ref<RenderScene> renderScene, const CullObjectsData& cullObjectsData);
		CullPrimitivesData AddCullPrimitivesPass(Ref<Camera> camera, Ref<RenderScene> renderScene, const CullMeshletsData& cullMeshletsData, const uint32_t instanceCount);

		RenderGraph& m_renderGraph;
		RenderGraphBlackboard& m_blackboard;
	};
}
