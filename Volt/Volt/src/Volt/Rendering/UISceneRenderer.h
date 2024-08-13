#pragma once

#include <CoreUtilities/Core.h>

#include <RHIModule/Images/Image.h>
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

		void OnRender(RefPtr<RHI::Image> targetImage, const glm::mat4& projectionMatrix);

		VT_NODISCARD VT_INLINE RefPtr<RHI::Image> GetIDImage() const { return m_widgetIDImage; }

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
		RefPtr<RHI::Image> m_widgetIDImage;
		RHI::CommandBufferSet m_commandBufferSet;
	};
}
