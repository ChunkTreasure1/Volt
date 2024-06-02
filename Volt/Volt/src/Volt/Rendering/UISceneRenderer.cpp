#include "vtpch.h"
#include "UISceneRenderer.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/Utility/StagedBufferUpload.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Rendering/Texture/Texture2D.h"

#include "Volt/GameUI/UIScene.h"
#include "Volt/GameUI/UIComponents.h"

namespace Volt
{
	struct UIRenderingData
	{
		RenderGraphResourceHandle vertexBuffer = 0;
		RenderGraphResourceHandle indexBuffer = 0;
	};

	struct UIVertex
	{
		glm::vec4 position;
		glm::vec2 texCoords;
		ResourceHandle imageHandle = Resource::Invalid;
	};

	static constexpr uint32_t s_quadVertexCount = 4;
	static constexpr uint32_t s_quadIndexCount = 6;
	static constexpr glm::vec4 s_quadVertexPositions[s_quadVertexCount] =
	{
		{ 0.f, 0.f, 0.f, 1.f },
		{ 1.f, 0.f, 0.f, 1.f },
		{ 0.f, -1.f, 0.f, 1.f },
		{ 1.f, 1.f, 0.f, 1.f }
	};

	static constexpr glm::vec2 s_quadVertexTexCoords[s_quadVertexCount] =
	{
		{ 0.f, 0.f },
		{ 1.f, 0.f },
		{ 0.f, 1.f },
		{ 1.f, 1.f }
	};

	static constexpr uint32_t s_quadVertexIndices[s_quadIndexCount] =
	{
		0, 1, 2,
		2, 1, 3
	};

	UISceneRenderer::UISceneRenderer(const UISceneRendererSpecification& specification)
		: m_scene(specification.scene)
	{

	}

	UISceneRenderer::~UISceneRenderer()
	{
	}

	void UISceneRenderer::OnRender(RefPtr<RHI::Image2D> targetImage)
	{
		if (!m_commandBuffer)
		{
			m_commandBuffer = RHI::CommandBuffer::Create(Renderer::GetFramesInFlight(), RHI::QueueType::Graphics);
		}

		RenderGraphBlackboard blackboard;
		RenderGraph renderGraph{ m_commandBuffer };
	
		PrepareForRender(renderGraph, blackboard);
	}

	void UISceneRenderer::PrepareForRender(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		VT_PROFILE_FUNCTION();

		// We are going to dynamically size the index and vertex buffers.
		// To do this we first need to calculate the maximum index and vertex count.
		// For now we will only do this for the images, but in the future we need
		// to add this to other types

		VertexIndexCounts vertexIndexCounts = CalculateMaxVertexAndIndexCount();

		auto& renderingData = blackboard.Add<UIRenderingData>();

		{
			const auto desc = RGUtils::CreateBufferDesc<UIVertex>(vertexIndexCounts.vertexCount, RHI::BufferUsage::VertexBuffer, RHI::MemoryUsage::GPU, "UI Vertex Buffer");
			renderingData.vertexBuffer = renderGraph.CreateBuffer(desc);
		}
	
		{
			const auto desc = RGUtils::CreateBufferDesc<uint32_t>(vertexIndexCounts.indexCount, RHI::BufferUsage::IndexBuffer, RHI::MemoryUsage::GPU, "UI Index Buffer");
			renderingData.indexBuffer = renderGraph.CreateBuffer(desc);
		}

		// All images
		{
			StagedBufferUpload<UIVertex> stagedVertexBufferUpload{ vertexIndexCounts.vertexCount };
			StagedBufferUpload<uint32_t> stagedIndexBufferUpload{ vertexIndexCounts.indexCount };
			m_scene->ForEachWithComponents<const UITransformComponent, const UIImageComponent>([&](entt::entity handle, const UITransformComponent& transComp, const UIImageComponent& imageComp)
			{
				auto texture = AssetManager::GetAsset<Texture2D>(imageComp.imageHandle);
				if (!texture || !texture->IsValid())
				{
					texture = Renderer::GetDefaultResources().whiteTexture;
				}

				const ResourceHandle resourceHandle = texture->GetResourceHandle();

				for (uint32_t i = 0; i < s_quadVertexCount; i++)
				{
					UIVertex& vertex = stagedVertexBufferUpload.AddUploadItem();
					vertex.position = (s_quadVertexPositions[i] - glm::vec4{ transComp.alignment, 0.f, 0.f } + glm::vec4{ transComp.position, 0.f, 0.f }) * glm::vec4{ transComp.scale, 1.f, 1.f };
					vertex.texCoords = s_quadVertexTexCoords[i];
					vertex.imageHandle = resourceHandle;
				}

				for (uint32_t i = 0; i < s_quadIndexCount; i++)
				{
					stagedIndexBufferUpload.AddUploadItem() = s_quadVertexIndices[i];
				}
			});

			stagedVertexBufferUpload.UploadTo(renderGraph, renderingData.vertexBuffer);
			stagedIndexBufferUpload.UploadTo(renderGraph, renderingData.indexBuffer);
		}
	}

	UISceneRenderer::VertexIndexCounts UISceneRenderer::CalculateMaxVertexAndIndexCount()
	{
		VT_PROFILE_FUNCTION();

		VertexIndexCounts result{};

		// Images
		{
			m_scene->ForEachWithComponents<const UITransformComponent, const UIImageComponent>([&](entt::entity handle, const UITransformComponent& transComp, const UIImageComponent& uiImageComp) 
			{
				result.vertexCount += s_quadVertexCount;
				result.indexCount += s_quadIndexCount;
			});
		}

		return result;
	}
}
