#include "vtpch.h"
#include "UISceneRenderer.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"
#include "Volt/Rendering/Shader/ShaderMap.h"
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

		uint32_t indexCount = 0;
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
		{ 0.f, 1.f, 0.f, 1.f },
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
		0, 2, 1,
		2, 3, 1
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
	
		if (PrepareForRender(renderGraph, blackboard))
		{
			const auto& renderingData = blackboard.Get<UIRenderingData>();
		
			const glm::vec2 halfSize = glm::vec2{ targetImage->GetWidth(), targetImage->GetHeight() } / 2.f;

			const auto projection = glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y, 0.1f, 100.f);

			struct UIPassData
			{
				RenderGraphResourceHandle depthImage;
				RenderGraphResourceHandle targetImage;
			};

			renderGraph.AddPass<UIPassData>("UI Pass",
			[&](RenderGraph::Builder& builder, UIPassData& data) 
			{
				{
					const auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::D32_SFLOAT>(targetImage->GetWidth(), targetImage->GetHeight(), RHI::ImageUsage::Attachment, "UI Depth");
					data.depthImage = builder.CreateImage2D(desc);
				}

				data.targetImage = builder.AddExternalImage2D(targetImage);

				builder.WriteResource(data.targetImage);
				builder.ReadResource(renderingData.indexBuffer, RenderGraphResourceState::IndexBuffer);
				builder.ReadResource(renderingData.vertexBuffer, RenderGraphResourceState::VertexBuffer);

				builder.SetHasSideEffect();
			},
			[=](const UIPassData& data, RenderContext& context, const RenderGraphPassResources& resources)
			{
				RenderingInfo renderingInfo = context.CreateRenderingInfo(targetImage->GetWidth(), targetImage->GetHeight(), { data.targetImage, data.depthImage });

				RHI::RenderPipelineCreateInfo pipelineInfo{};
				pipelineInfo.shader = ShaderMap::Get("UIMain");

				auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

				context.BeginRendering(renderingInfo);
				context.BindPipeline(pipeline);

				context.BindIndexBuffer(renderingData.indexBuffer);
				context.BindVertexBuffers({ renderingData.vertexBuffer }, 0);

				context.SetConstant("viewProjection"_sh, projection);

				context.DrawIndexed(renderingData.indexCount, 1, 0, 0, 0);
				context.EndRendering();
			});
		}

		RenderGraphResourceHandle outputImage = renderGraph.AddExternalImage2D(targetImage);
		{
			RenderGraphBarrierInfo barrier{};
			barrier.dstAccess = RHI::BarrierAccess::ShaderRead;
			barrier.dstLayout = RHI::ImageLayout::ShaderRead;
			barrier.dstStage = RHI::BarrierStage::PixelShader;

			renderGraph.AddResourceBarrier(outputImage, barrier);
		}

		renderGraph.Compile();
		renderGraph.Execute();
	}

	bool UISceneRenderer::PrepareForRender(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		VT_PROFILE_FUNCTION();

		// We are going to dynamically size the index and vertex buffers.
		// To do this we first need to calculate the maximum index and vertex count.
		// For now we will only do this for the images, but in the future we need
		// to add this to other types

		VertexIndexCounts vertexIndexCounts = CalculateMaxVertexAndIndexCount();

		auto& renderingData = blackboard.Add<UIRenderingData>();

		if (vertexIndexCounts.indexCount == 0 || vertexIndexCounts.vertexCount == 0)
		{
			return false;
		}

		renderingData.indexCount = vertexIndexCounts.indexCount;

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
					vertex.position = (s_quadVertexPositions[i] - glm::vec4{ transComp.alignment, 0.f, 0.f }) * glm::vec4{ transComp.size, 1.f, 1.f } + glm::vec4{ transComp.position, 10.f, 0.f };
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

		return true;
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
