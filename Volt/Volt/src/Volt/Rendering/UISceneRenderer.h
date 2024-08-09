#pragma once

#include <CoreUtilities/Core.h>

#include <RHIModule/Images/Image2D.h>
#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Buffers/CommandBufferSet.h>

namespace Volt
{
	class UIScene;
	class RenderGraph;
	class RenderGraphBlackboard;

	struct UISceneRendererSpecification
	{
		Ref<UIScene> scene;

		bool isEditor = false;
	};

	class UISceneRenderer
	{
	public:
		UISceneRenderer(const UISceneRendererSpecification& specification);
		~UISceneRenderer();

		void OnRender(RefPtr<RHI::Image2D> targetImage, const glm::mat4& projectionMatrix);

		VT_NODISCARD VT_INLINE RefPtr<RHI::Image2D> GetIDImage() const { return m_widgetIDImage; }

	private:
		struct VertexIndexCounts
		{
			uint32_t vertexCount;
			uint32_t indexCount;
		};
		
		bool PrepareForRender(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard);
		VertexIndexCounts CalculateMaxVertexAndIndexCount();

		bool m_isEditor = false;

		Ref<UIScene> m_scene;
		RefPtr<RHI::Image2D> m_widgetIDImage;
		RHI::CommandBufferSet m_commandBufferSet;
	};
}
