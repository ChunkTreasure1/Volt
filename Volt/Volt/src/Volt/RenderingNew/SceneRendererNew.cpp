#include "vtpch.h"
#include "SceneRendererNew.h"

#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/RenderingNew/RenderScene.h"
#include "Volt/RenderingNew/RendererCommon.h"
#include "Volt/RenderingNew/RendererNew.h"

#include "Volt/RenderingNew/SceneRendererStructs.h"
#include "Volt/RenderingNew/Shader/ShaderMap.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraph.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphExecutionThread.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphTextureResource.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Asset/Mesh/Mesh.h"

#include "Volt/Math/Math.h"

#include "Volt/Components/LightComponents.h"
#include "Volt/Components/Components.h"

#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Images/SamplerState.h>
#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>
#include <VoltRHI/Pipelines/ComputePipeline.h>

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Buffers/UniformBuffer.h>
#include <VoltRHI/Buffers/StorageBuffer.h>

#include <VoltRHI/Buffers/UniformBufferSet.h>
#include <VoltRHI/Buffers/StorageBufferSet.h>

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/GraphicsDevice.h>
#include <VoltRHI/Graphics/DeviceQueue.h>

#include <VoltRHI/Descriptors/DescriptorTable.h>

inline static constexpr uint32_t CAMERA_BUFFER_BINDING = 0;
inline static constexpr uint32_t DIRECTIONAL_LIGHT_BINDING = 1;

namespace Volt
{
	namespace Utility
	{
		const size_t HashMeshSubMesh(Ref<Mesh> mesh, const SubMesh& subMesh)
		{
			size_t result = std::hash<void*>()(mesh.get());
			result = Math::HashCombine(result, subMesh.GetHash());

			return result;
		}
	}

	SceneRendererNew::SceneRendererNew(const SceneRendererSpecification& specification)
		: m_scene(specification.scene)
	{
		m_commandBuffer = RHI::CommandBuffer::Create(3, RHI::QueueType::Graphics);

		CreatePipelines();

		// Create render target
		{
			RHI::ImageSpecification spec{};
			spec.width = specification.initialResolution.x;
			spec.height = specification.initialResolution.y;
			spec.usage = RHI::ImageUsage::AttachmentStorage;
			spec.generateMips = false;

			m_outputImage = RHI::Image2D::Create(spec);
		}

		// Constant buffer
		{
			m_constantBufferSet = RHI::UniformBufferSet::Create(RendererNew::GetFramesInFlight());
			m_constantBufferSet->Add<CameraDataNew>(5, CAMERA_BUFFER_BINDING);
			m_constantBufferSet->Add<DirectionalLightData>(5, DIRECTIONAL_LIGHT_BINDING);
		}

		// Storage buffers
		{
			m_indirectCommandsBuffer = RHI::StorageBuffer::Create(1, sizeof(IndirectGPUCommandNew), RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::CPUToGPU);
			m_indirectCountsBuffer = RHI::StorageBuffer::Create(2, sizeof(uint32_t), RHI::BufferUsage::IndirectBuffer);

			m_drawToInstanceOffsetBuffer = RHI::StorageBuffer::Create(1, sizeof(uint32_t));
			m_instanceOffsetToObjectIDBuffer = RHI::StorageBuffer::Create(1, sizeof(uint32_t));
			m_indirectDrawDataBuffer = RHI::StorageBuffer::Create(1, sizeof(IndirectDrawData), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU);
		}

		// Sampler state
		{
			RHI::SamplerStateCreateInfo info{};
			info.minFilter = RHI::TextureFilter::Linear;
			info.magFilter = RHI::TextureFilter::Linear;
			info.mipFilter = RHI::TextureFilter::Linear;
			info.wrapMode = RHI::TextureWrap::Repeat;

			m_samplerState = RHI::SamplerState::Create(info);
		}

		// Descriptor table
		{
			RHI::DescriptorTableCreateInfo descriptorTableSpec{};
			descriptorTableSpec.shader = ShaderMap::Get("MeshIndirect");
			m_descriptorTable = RHI::DescriptorTable::Create(descriptorTableSpec);
			m_descriptorTable->SetBufferView(m_drawToInstanceOffsetBuffer->GetView(), 0, 0);
			m_descriptorTable->SetBufferView(m_indirectDrawDataBuffer->GetView(), 0, 1);
			m_descriptorTable->SetBufferView(m_instanceOffsetToObjectIDBuffer->GetView(), 0, 2);

			m_descriptorTable->SetBufferViewSet(m_constantBufferSet->GetBufferViewSet(5, CAMERA_BUFFER_BINDING), 5, CAMERA_BUFFER_BINDING);
			m_descriptorTable->SetBufferViewSet(m_constantBufferSet->GetBufferViewSet(5, DIRECTIONAL_LIGHT_BINDING), 5, DIRECTIONAL_LIGHT_BINDING);

			m_descriptorTable->SetSamplerState(m_samplerState, 5, 3);
		}
	}

	SceneRendererNew::~SceneRendererNew()
	{
		m_commandBuffer = nullptr;
	}

	void SceneRendererNew::OnRenderEditor(Ref<Camera> camera)
	{
		OnRender(camera);
	}

	void SceneRendererNew::Resize(const uint32_t width, const uint32_t height)
	{
		m_resizeWidth = width;
		m_resizeHeight = height;

		m_shouldResize = true;
	}

	Ref<RHI::Image2D> SceneRendererNew::GetFinalImage()
	{
		return m_outputImage;
	}

	void SceneRendererNew::OnRender(Ref<Camera> camera)
	{
		VT_PROFILE_FUNCTION();

		if (m_scene->GetRenderScene()->IsInvalid())
		{
			Invalidate();
		}

		if (m_shouldResize)
		{
			m_width = m_resizeWidth;
			m_height = m_resizeHeight;

			RenderGraphExecutionThread::WaitForFinishedExecution();
			RHI::GraphicsContext::GetDevice()->GetDeviceQueue(RHI::QueueType::Graphics)->WaitForQueue();

			m_outputImage->Invalidate(m_width, m_height);

			m_shouldResize = false;
		}

		UpdateBuffers(camera);


		RenderGraphBlackboard rgBlackboard{};
		RenderGraph renderGraph{ m_commandBuffer };

		AddExternalResources(renderGraph, rgBlackboard);
		AddSetupIndirectPasses(renderGraph, rgBlackboard);

		renderGraph.AddResourceTransition(rgBlackboard.Get<ExternalBuffersData>().indirectCountsBuffer, RHI::ResourceState::IndirectArgument);
		renderGraph.AddResourceTransition(rgBlackboard.Get<ExternalBuffersData>().indirectCommandsBuffer, RHI::ResourceState::IndirectArgument);

		AddPreDepthPass(renderGraph, rgBlackboard);
		AddGBufferPass(renderGraph, rgBlackboard);
		AddDeferredShadingPass(renderGraph, rgBlackboard);

		renderGraph.AddResourceTransition(rgBlackboard.Get<ExternalImagesData>().outputImage, RHI::ResourceState::PixelShaderRead);

		renderGraph.Compile();
		renderGraph.Execute();
	}

	void SceneRendererNew::Invalidate()
	{
		VT_PROFILE_FUNCTION();

		auto renderScene = m_scene->GetRenderScene();

		renderScene->PrepareForUpdate();
		const uint32_t individualMeshCount = renderScene->GetIndividualMeshCount();
		const uint32_t renderObjectCount = renderScene->GetRenderObjectCount();

		if (m_indirectCommandsBuffer->GetSize() < individualMeshCount)
		{
			m_indirectCommandsBuffer->Resize(individualMeshCount);
		}

		if (m_indirectDrawDataBuffer->GetSize() < renderObjectCount)
		{
			m_indirectDrawDataBuffer->Resize(renderObjectCount);
			m_descriptorTable->SetBufferView(m_indirectDrawDataBuffer->GetView(), 0, 1);
		}

		if (m_instanceOffsetToObjectIDBuffer->GetSize() < renderObjectCount)
		{
			m_instanceOffsetToObjectIDBuffer->Resize(renderObjectCount);
			m_drawToInstanceOffsetBuffer->Resize(renderObjectCount);

			m_descriptorTable->SetBufferView(m_drawToInstanceOffsetBuffer->GetView(), 0, 0);
			m_descriptorTable->SetBufferView(m_instanceOffsetToObjectIDBuffer->GetView(), 0, 2);
		}

		UpdateDescriptorTableForMeshRendering(*renderScene);

		uint32_t currentMeshIndex = 0;

		std::unordered_map<Mesh*, uint32_t> meshDataMap;
		std::unordered_map<size_t, uint32_t> meshSubMeshIndexMap;

		IndirectDrawData* drawDataBuffer = m_indirectDrawDataBuffer->Map<IndirectDrawData>();
		IndirectGPUCommandNew* indirectCommands = m_indirectCommandsBuffer->Map<IndirectGPUCommandNew>();

		m_currentActiveCommandCount = 0;
		for (uint32_t index = 0; const auto & obj : *renderScene)
		{
			auto meshPtr = obj.mesh.get();
			uint32_t meshIndex = 0;

			if (meshDataMap.contains(meshPtr))
			{
				meshIndex = meshDataMap.at(meshPtr);
			}
			else
			{
				meshIndex = currentMeshIndex++;
				meshDataMap[meshPtr] = meshIndex;
			}

			Entity entity{ obj.entity, m_scene.Get() };

			const auto& subMesh = obj.mesh->GetSubMeshes().at(obj.subMeshIndex);

			const size_t meshSubMeshHash = Utility::HashMeshSubMesh(obj.mesh, subMesh);
			if (meshSubMeshIndexMap.contains(meshSubMeshHash))
			{
				const uint32_t cmdIndex = meshSubMeshIndexMap.at(meshSubMeshHash);
				indirectCommands[cmdIndex].command.instanceCount++;
			}
			else
			{
				meshSubMeshIndexMap[meshSubMeshHash] = m_currentActiveCommandCount;

				indirectCommands[m_currentActiveCommandCount].command.vertexCount = subMesh.indexCount;
				indirectCommands[m_currentActiveCommandCount].command.instanceCount = 1;
				indirectCommands[m_currentActiveCommandCount].command.firstVertex = subMesh.indexStartOffset;
				indirectCommands[m_currentActiveCommandCount].command.firstInstance = 0;
				indirectCommands[m_currentActiveCommandCount].objectId = index;
				indirectCommands[m_currentActiveCommandCount].meshId = meshIndex;
				m_currentActiveCommandCount++;
			}

			drawDataBuffer[index].meshId = meshIndex;
			drawDataBuffer[index].vertexStartOffset = subMesh.vertexStartOffset;
			drawDataBuffer[index].transform = entity.GetTransform();

			index++;
		}

		m_indirectDrawDataBuffer->Unmap();
		m_indirectCommandsBuffer->Unmap();

		m_scene->GetRenderScene()->SetValid();
	}

	void SceneRendererNew::UpdateBuffers(Ref<Camera> camera)
	{
		UpdateCameraBuffer(camera);
		UpdateLightBuffers();
	}

	void SceneRendererNew::UpdateCameraBuffer(Ref<Camera> camera)
	{
		if (!camera)
		{
			return;
		}

		auto cameraBuffer = m_constantBufferSet->Get(5, CAMERA_BUFFER_BINDING, m_commandBuffer->GetCurrentIndex());

		CameraDataNew* cameraData = cameraBuffer->Map<CameraDataNew>();

		cameraData->projection = camera->GetProjection();
		cameraData->view = camera->GetView();
		cameraData->inverseView = glm::inverse(camera->GetView());
		cameraData->inverseProjection = glm::inverse(camera->GetProjection());
		cameraData->position = glm::vec4(camera->GetPosition(), 1.f);

		cameraBuffer->Unmap();
	}

	void SceneRendererNew::UpdateLightBuffers()
	{
		auto& registry = m_scene->GetRegistry();

		// Directional Light
		{
			DirectionalLightData dirLightData{};

			registry.ForEach<DirectionalLightComponent, TransformComponent>([&](Wire::EntityId id, const DirectionalLightComponent& lightComp, const TransformComponent& transComp)
			{
				if (!transComp.visible)
				{
					return;
				}

				Entity entity = { id, m_scene.Get() };

				const glm::vec3 dir = glm::rotate(entity.GetRotation(), { 0.f, 0.f, 1.f }) * -1.f;
				dirLightData.direction = glm::vec4{ dir.x, dir.y, dir.z, 0.f };
				dirLightData.color = lightComp.color;
				dirLightData.intensity = lightComp.intensity;
			});

			m_constantBufferSet->Get(5, DIRECTIONAL_LIGHT_BINDING, m_commandBuffer->GetCurrentIndex())->SetData<DirectionalLightData>(dirLightData);
		}
	}

	void SceneRendererNew::UpdateDescriptorTableForMeshRendering(RenderScene& renderScene, RenderContext& renderContext)
	{
		renderContext.SetBufferViews(renderScene.GetVertexPositionViews(), 1, 0);
		renderContext.SetBufferViews(renderScene.GetVertexMaterialViews(), 2, 0);
		renderContext.SetBufferViews(renderScene.GetVertexAnimationViews(), 3, 0);
		renderContext.SetBufferViews(renderScene.GetIndexBufferViews(), 4, 0);

		renderContext.SetBufferView(m_drawToInstanceOffsetBuffer->GetView(), 0, 0);
		renderContext.SetBufferView(m_indirectDrawDataBuffer->GetView(), 0, 1);
		renderContext.SetBufferView(m_instanceOffsetToObjectIDBuffer->GetView(), 0, 2);

		renderContext.SetBufferViewSet(m_constantBufferSet->GetBufferViewSet(5, CAMERA_BUFFER_BINDING), 5, CAMERA_BUFFER_BINDING);
		renderContext.SetBufferViewSet(m_constantBufferSet->GetBufferViewSet(5, DIRECTIONAL_LIGHT_BINDING), 5, DIRECTIONAL_LIGHT_BINDING);
	}

	void SceneRendererNew::UpdateDescriptorTableForMeshRendering(RenderScene& renderScene)
	{
		m_descriptorTable->SetBufferViews(renderScene.GetVertexPositionViews(), 1, 0);
		m_descriptorTable->SetBufferViews(renderScene.GetVertexMaterialViews(), 2, 0);
		m_descriptorTable->SetBufferViews(renderScene.GetVertexAnimationViews(), 3, 0);
		m_descriptorTable->SetBufferViews(renderScene.GetIndexBufferViews(), 4, 0);
	}

	void SceneRendererNew::CreatePipelines()
	{
		m_clearIndirectCountsPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("ClearCountBuffer"));
		m_indirectSetupPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("IndirectSetup"));

		// Depth Only pipeline
		{
			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("PreDepth");
			m_preDepthPipeline = RHI::RenderPipeline::Create(pipelineInfo);
		}

		// Main pipeline
		{
			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("MeshIndirect");
			m_renderPipeline = RHI::RenderPipeline::Create(pipelineInfo);
		}

		// GBuffer
		{
			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("GBufferGeneration");
			pipelineInfo.depthCompareOperator = RHI::CompareOperator::Equal;
			pipelineInfo.depthMode = RHI::DepthMode::Read;

			m_gbufferPipeline = RHI::RenderPipeline::Create(pipelineInfo);
		}

		// Deferred Shading
		{
			m_deferredShadingPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("DeferredShading"));
		}
	}

	void SceneRendererNew::AddExternalResources(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		auto& bufferData = blackboard.Add<ExternalBuffersData>();
		auto& imageData = blackboard.Add<ExternalImagesData>();

		bufferData.indirectCommandsBuffer = renderGraph.AddExternalBuffer(m_indirectCommandsBuffer);
		bufferData.indirectCountsBuffer = renderGraph.AddExternalBuffer(m_indirectCountsBuffer);
		bufferData.drawToInstanceOffsetBuffer = renderGraph.AddExternalBuffer(m_drawToInstanceOffsetBuffer);
		bufferData.instanceOffsetToObjectIDBuffer = renderGraph.AddExternalBuffer(m_instanceOffsetToObjectIDBuffer);

		imageData.outputImage = renderGraph.AddExternalImage2D(m_outputImage);
	}

	void SceneRendererNew::AddSetupIndirectPasses(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		auto& bufferData = blackboard.Get<ExternalBuffersData>();

		renderGraph.AddPass("Clear counts buffers", [&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(bufferData.indirectCountsBuffer);
			builder.SetHasSideEffect();
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			RHI::ShaderDataBuffer pushConstantBuffer = ShaderMap::Get("ClearCountBuffer")->GetConstantsBuffer();
			pushConstantBuffer.SetMemberData("size", static_cast<uint32_t>(m_currentActiveCommandCount));

			const uint32_t dispatchCount = std::max(1u, static_cast<uint32_t>(m_currentActiveCommandCount / 256) + 1u);

			context.BindPipeline(m_clearIndirectCountsPipeline);
			context.SetBufferView(m_indirectCountsBuffer->GetView(), 0, 0);
			context.PushConstants(pushConstantBuffer.GetBuffer(), static_cast<uint32_t>(pushConstantBuffer.GetSize()));
			context.Dispatch(dispatchCount, 1, 1);
		});

		renderGraph.AddPass("Setup Indirect Args", [&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(bufferData.indirectCommandsBuffer);
			builder.WriteResource(bufferData.indirectCountsBuffer);
			builder.WriteResource(bufferData.drawToInstanceOffsetBuffer);
			builder.WriteResource(bufferData.instanceOffsetToObjectIDBuffer);

			builder.SetHasSideEffect();
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			RHI::ShaderDataBuffer pushConstantBuffer = ShaderMap::Get("IndirectSetup")->GetConstantsBuffer();
			pushConstantBuffer.SetMemberData("drawCallCount", static_cast<uint32_t>(m_currentActiveCommandCount));
			const uint32_t dispatchCount = std::max(1u, (uint32_t)(m_currentActiveCommandCount / 256) + 1u);

			context.BindPipeline(m_indirectSetupPipeline);

			context.SetBufferView(resources.GetBuffer(bufferData.indirectCountsBuffer)->GetView(), 0, 0);
			context.SetBufferView(resources.GetBuffer(bufferData.indirectCommandsBuffer)->GetView(), 0, 1);
			context.SetBufferView(resources.GetBuffer(bufferData.drawToInstanceOffsetBuffer)->GetView(), 0, 2);
			context.SetBufferView(resources.GetBuffer(bufferData.instanceOffsetToObjectIDBuffer)->GetView(), 0, 3);

			context.PushConstants(pushConstantBuffer.GetBuffer(), static_cast<uint32_t>(pushConstantBuffer.GetSize()));
			context.Dispatch(dispatchCount, 1, 1);
		});
	}

	void SceneRendererNew::AddPreDepthPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		blackboard.Add<PreDepthData>() = renderGraph.AddPass<PreDepthData>("Pre Depth Pass",
		[&](RenderGraph::Builder& builder, PreDepthData& data)
		{
			RenderGraphImageDesc desc{};
			desc.width = m_width;
			desc.height = m_height;
			desc.format = RHI::Format::D32_SFLOAT;
			desc.usage = RHI::ImageUsage::Attachment;
			desc.name = "PreDepth";
			data.depth = builder.CreateImage2D(desc);

			desc.format = RHI::Format::R16G16B16A16_SFLOAT;
			desc.name = "Normals";
			data.normals = builder.CreateImage2D(desc);
		},
		[=](const PreDepthData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Ref<RHI::ImageView> depthImageView = resources.GetImage2D(data.depth)->GetView();
			Ref<RHI::ImageView> normalsImageView = resources.GetImage2D(data.normals)->GetView();

			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { normalsImageView, depthImageView });

			context.BeginRendering(info);
			context.BindPipeline(m_preDepthPipeline);

			UpdateDescriptorTableForMeshRendering(*m_scene->GetRenderScene(), context);

			context.DrawIndirectCount(m_indirectCommandsBuffer, 0, m_indirectCountsBuffer, 0, m_currentActiveCommandCount, sizeof(IndirectGPUCommandNew));
			context.EndRendering();
		});
	}

	void SceneRendererNew::AddGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		blackboard.Add<GBufferData>() = renderGraph.AddPass<GBufferData>("GBuffer Pass",
		[&](RenderGraph::Builder& builder, GBufferData& data)
		{
			data.albedo = builder.CreateImage2D({ RHI::Format::R8G8B8A8_UNORM, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Albedo " });
			data.materialEmissive = builder.CreateImage2D({ RHI::Format::R16G16B16A16_SFLOAT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - MaterialEmissive" });
			data.normalEmissive = builder.CreateImage2D({ RHI::Format::R16G16B16A16_SFLOAT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - NormalEmissive" });

			builder.WriteResource(blackboard.Get<PreDepthData>().depth);
		},
		[=](const GBufferData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Ref<RHI::ImageView> albedoView = resources.GetImage2D(data.albedo)->GetView();
			Ref<RHI::ImageView> materialEmissiveView = resources.GetImage2D(data.materialEmissive)->GetView();
			Ref<RHI::ImageView> normalEmissiveView = resources.GetImage2D(data.normalEmissive)->GetView();

			Ref<RHI::ImageView> depthView = resources.GetImage2D(blackboard.Get<PreDepthData>().depth)->GetView();

			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { albedoView, materialEmissiveView, normalEmissiveView, depthView });
			info.renderingInfo.depthAttachmentInfo.clearMode = RHI::ClearMode::Load;

			context.BeginRendering(info);
			context.BindPipeline(m_gbufferPipeline);

			UpdateDescriptorTableForMeshRendering(*m_scene->GetRenderScene(), context);

			context.DrawIndirectCount(m_indirectCommandsBuffer, 0, m_indirectCountsBuffer, 0, m_currentActiveCommandCount, sizeof(IndirectGPUCommandNew));
			context.EndRendering();
		});
	}

	void SceneRendererNew::AddDeferredShadingPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		GBufferData gbufferData = blackboard.Get<GBufferData>();
		PreDepthData preDepthData = blackboard.Get<PreDepthData>();
		ExternalImagesData externalImagesData = blackboard.Get<ExternalImagesData>();

		renderGraph.AddPass("Deferred Shading Pass",
		[&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(externalImagesData.outputImage);

			builder.ReadResource(gbufferData.albedo);
			builder.ReadResource(gbufferData.materialEmissive);
			builder.ReadResource(gbufferData.normalEmissive);
			builder.ReadResource(preDepthData.depth);

			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto outputImage = resources.GetImage2D(externalImagesData.outputImage);

			context.BindPipeline(m_deferredShadingPipeline);
			context.SetImageView(resources.GetImage2D(gbufferData.albedo)->GetView(), 0, 0);
			context.SetImageView(resources.GetImage2D(gbufferData.materialEmissive)->GetView(), 0, 1);
			context.SetImageView(resources.GetImage2D(gbufferData.normalEmissive)->GetView(), 0, 2);
			context.SetImageView(resources.GetImage2D(preDepthData.depth)->GetView(), 0, 3);
			context.SetImageView(outputImage->GetView(), 0, 4);

			context.ClearImage(outputImage, { 0.1f, 0.1f, 0.1f, 1.f });
			context.Dispatch((m_width / 16) + 1, (m_width / 16) + 1, 1);
		});
	}
}
