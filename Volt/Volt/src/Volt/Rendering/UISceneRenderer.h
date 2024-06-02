#pragma once

#include <CoreUtilities/Core.h>

#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Buffers/CommandBuffer.h>

namespace Volt
{
	class UIScene;
	class RenderGraph;
	class RenderGraphBlackboard;

	struct UISceneRendererSpecification
	{
		Ref<UIScene> scene;
	};

	class UISceneRenderer
	{
	public:
		UISceneRenderer(const UISceneRendererSpecification& specification);
		~UISceneRenderer();

		void OnRender(RefPtr<RHI::Image2D> targetImage);

	private:
		struct VertexIndexCounts
		{
			uint32_t vertexCount;
			uint32_t indexCount;
		};
		
		void PrepareForRender(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		VertexIndexCounts CalculateMaxVertexAndIndexCount();

		Ref<UIScene> m_scene;
		RefPtr<RHI::CommandBuffer> m_commandBuffer;
	};
}
