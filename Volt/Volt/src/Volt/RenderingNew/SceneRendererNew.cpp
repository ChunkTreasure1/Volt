#include "vtpch.h"
#include "SceneRendererNew.h"

#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/RenderingNew/RenderScene.h"
#include "Volt/RenderingNew/RendererCommon.h"
#include "Volt/RenderingNew/RendererNew.h"

#include "Volt/RenderingNew/Shader/ShaderMap.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraph.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphExecutionThread.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "Volt/RenderingNew/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/RenderingNew/RenderingTechniques/PrefixSumTechnique.h"

#include "Volt/RenderingNew/DrawContext.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"

#include "Volt/Math/Math.h"

#include "Volt/Components/LightComponents.h"
#include "Volt/Components/RenderingComponents.h"
#include "Volt/Components/CoreComponents.h"

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
inline static constexpr uint32_t SAMPLERS_BINDING = 2;

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
			m_constantBufferSet->Add<CameraDataNew>(2, CAMERA_BUFFER_BINDING);
			m_constantBufferSet->Add<DirectionalLightData>(2, DIRECTIONAL_LIGHT_BINDING);
			m_constantBufferSet->Add<SamplersData>(2, SAMPLERS_BINDING);
		}

		// Storage buffers
		{
			m_indirectCommandsBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(IndirectGPUCommandNew), "Indirect Commands", RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::CPUToGPU));
			m_indirectCountsBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(2, sizeof(uint32_t), "Indirect Counts", RHI::BufferUsage::IndirectBuffer));
			m_drawToInstanceOffsetBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(uint32_t), "Draw To Instance Offset"));
			m_instanceOffsetToObjectIDBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(uint32_t), "Instance Offset to Object ID"));

			m_drawContextBuffer = GlobalResource<RHI::StorageBuffer>::Create(RHI::StorageBuffer::Create(1, sizeof(DrawContext), "Draw Context", RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU));
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
	}

	SceneRendererNew::~SceneRendererNew()
	{
		RenderGraphExecutionThread::WaitForFinishedExecution();
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

		UploadUniformBuffers(renderGraph, rgBlackboard, camera);
		AddExternalResources(renderGraph, rgBlackboard);
		AddSetupIndirectPasses(renderGraph, rgBlackboard);

		renderGraph.AddResourceTransition(rgBlackboard.Get<ExternalBuffersData>().indirectCountsBuffer, RHI::ResourceState::IndirectArgument);
		renderGraph.AddResourceTransition(rgBlackboard.Get<ExternalBuffersData>().indirectCommandsBuffer, RHI::ResourceState::IndirectArgument);

		AddPreDepthPass(renderGraph, rgBlackboard);

		//AddVisibilityBufferPass(renderGraph, rgBlackboard);
		//AddGenerateMaterialCountsPass(renderGraph, rgBlackboard);

		//PrefixSumTechnique prefixSum{ renderGraph, m_prefixSumPipeline };
		//prefixSum.Execute(rgBlackboard.Get<MaterialCountData>().materialCountBuffer, rgBlackboard.Get<MaterialCountData>().materialStartBuffer, m_materialsBuffer->GetSize());

		//AddCollectMaterialPixelsPass(renderGraph, rgBlackboard);
		//AddGenerateMaterialIndirectArgsPass(renderGraph, rgBlackboard);

		//// For every material -> run compute shading shader using indirect args
		//AddGenerateGBufferPass(renderGraph, rgBlackboard);

		//if (m_visibilityVisualization != VisibilityVisualization::None)
		//{
		//	AddVisibilityVisualizationPass(renderGraph, rgBlackboard);
		//}

		renderGraph.AddResourceTransition(rgBlackboard.Get<ExternalImagesData>().outputImage, RHI::ResourceState::PixelShaderRead);

		renderGraph.Compile();
		renderGraph.Execute();
	}

	void SceneRendererNew::Invalidate()
	{
		VT_PROFILE_FUNCTION();

		auto renderScene = m_scene->GetRenderScene();

		renderScene->PrepareForUpdate();
		const uint32_t renderObjectCount = renderScene->GetRenderObjectCount();

		if (m_indirectCommandsBuffer->GetResource()->GetSize() < renderObjectCount)
		{
			m_indirectCommandsBuffer->GetResource()->Resize(renderObjectCount);
			m_indirectCommandsBuffer->MarkAsDirty();
		}

		if (m_instanceOffsetToObjectIDBuffer->GetResource()->GetSize() < renderObjectCount)
		{
			m_instanceOffsetToObjectIDBuffer->GetResource()->Resize(renderObjectCount);
			m_instanceOffsetToObjectIDBuffer->MarkAsDirty();

			m_drawToInstanceOffsetBuffer->GetResource()->Resize(renderObjectCount);
			m_drawToInstanceOffsetBuffer->MarkAsDirty();
		}

		std::unordered_map<size_t, uint32_t> meshSubMeshIndexMap;

		IndirectGPUCommandNew* indirectCommands = m_indirectCommandsBuffer->GetResource()->Map<IndirectGPUCommandNew>();

		m_currentActiveCommandCount = 0;
		for (uint32_t index = 0; const auto & obj : *renderScene)
		{
			Entity entity{ obj.entity, m_scene };

			const auto& subMesh = obj.mesh->GetSubMeshes().at(obj.subMeshIndex);

			//const size_t meshSubMeshHash = Utility::HashMeshSubMesh(obj.mesh, subMesh);
			//if (meshSubMeshIndexMap.contains(meshSubMeshHash))
			//{
			//	const uint32_t cmdIndex = meshSubMeshIndexMap.at(meshSubMeshHash);
			//	indirectCommands[cmdIndex].command.instanceCount++;
			//}
			//else
			{
				//meshSubMeshIndexMap[meshSubMeshHash] = m_currentActiveCommandCount;

				indirectCommands[m_currentActiveCommandCount].command.vertexCount = subMesh.indexCount;
				indirectCommands[m_currentActiveCommandCount].command.instanceCount = 1;
				indirectCommands[m_currentActiveCommandCount].command.firstVertex = subMesh.indexStartOffset;
				indirectCommands[m_currentActiveCommandCount].command.firstInstance = 0;
				indirectCommands[m_currentActiveCommandCount].objectId = index;
				indirectCommands[m_currentActiveCommandCount].meshId = renderScene->GetMeshID(obj.mesh, obj.subMeshIndex);
				m_currentActiveCommandCount++;
			}

			index++;
		}

		m_indirectCommandsBuffer->GetResource()->Unmap();

		m_scene->GetRenderScene()->SetValid();
	}

	void SceneRendererNew::UpdateBuffers(Ref<Camera> camera)
	{
		UpdateCameraBuffer(camera);
		UpdateLightBuffers();
		UpdateSamplersBuffer();

		// Update draw context
		{
			DrawContext* drawContext = m_drawContextBuffer->GetResource()->Map<DrawContext>();

			drawContext->drawToInstanceOffset = m_drawToInstanceOffsetBuffer->GetResourceHandle();
			drawContext->instanceOffsetToObjectIDBuffer = m_instanceOffsetToObjectIDBuffer->GetResourceHandle();

			m_drawContextBuffer->GetResource()->Unmap();
		}
	}

	void SceneRendererNew::UpdateCameraBuffer(Ref<Camera> camera)
	{
		if (!camera)
		{
			return;
		}

		auto cameraBuffer = m_constantBufferSet->Get(2, CAMERA_BUFFER_BINDING, m_commandBuffer->GetCurrentIndex());

		CameraDataNew* cameraData = cameraBuffer->Map<CameraDataNew>();

		cameraData->projection = camera->GetProjection();
		cameraData->view = camera->GetView();
		cameraData->inverseView = glm::inverse(camera->GetView());
		cameraData->inverseProjection = glm::inverse(camera->GetProjection());
		cameraData->position = glm::vec4(camera->GetPosition(), 1.f);

		cameraBuffer->Unmap();
	}

	void SceneRendererNew::UpdateSamplersBuffer()
	{
		auto samplersBuffer = m_constantBufferSet->Get(2, SAMPLERS_BINDING, m_commandBuffer->GetCurrentIndex());

		*samplersBuffer->Map<SamplersData>() = RendererNew::GetSamplersData();
		samplersBuffer->Unmap();
	}

	void SceneRendererNew::UpdateLightBuffers()
	{
		// Directional Light
		{
			DirectionalLightData dirLightData{};

			m_scene->ForEachWithComponents<const DirectionalLightComponent, const TransformComponent>([&](const entt::entity id, const DirectionalLightComponent& lightComp, const TransformComponent& transComp)
			{
				if (!transComp.visible)
				{
					return;
				}

				Entity entity = { id, m_scene };

				const glm::vec3 dir = glm::rotate(entity.GetRotation(), { 0.f, 0.f, 1.f }) * -1.f;
				dirLightData.direction = glm::vec4{ dir.x, dir.y, dir.z, 0.f };
				dirLightData.color = lightComp.color;
				dirLightData.intensity = lightComp.intensity;
			});

			m_constantBufferSet->Get(2, DIRECTIONAL_LIGHT_BINDING, m_commandBuffer->GetCurrentIndex())->SetData<DirectionalLightData>(dirLightData);
		}
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

		//// Visibility Buffer pipeline
		//{
		//	RHI::RenderPipelineCreateInfo pipelineInfo{};
		//	pipelineInfo.shader = ShaderMap::Get("VisibilityBuffer");
		//	pipelineInfo.depthCompareOperator = RHI::CompareOperator::Equal;
		//	pipelineInfo.depthMode = RHI::DepthMode::Read;
		//	m_visibilityPipeline = RHI::RenderPipeline::Create(pipelineInfo);
		//}

		//// Visibility buffer compute
		//{
		//	m_visibilityVisualizationPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("VisibilityVisualization"));
		//	m_generateMaterialCountPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("GenerateMaterialCount"));
		//	m_collectMaterialPixelsPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("CollectMaterialPixels"));
		//	m_generateMaterialIndirectArgsPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("GenerateMaterialIndirectArgs"));
		//	m_generateGBufferPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("GenerateGBuffer"));
		//}

		//// Utility
		//{
		//	m_prefixSumPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("PrefixSum"));
		//}
	}

	void SceneRendererNew::UploadUniformBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera)
	{
		auto& buffersData = blackboard.Add<UniformBuffersData>();

		buffersData = renderGraph.AddPass<UniformBuffersData>("Create Uniform Buffers",
		[&](RenderGraph::Builder& builder, UniformBuffersData& data)
		{
			data.cameraDataBuffer = builder.CreateBuffer({ sizeof(CameraDataNew), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Camera Data" });
		},
		[=](const UniformBuffersData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			// Set camera data
			{
				auto cameraDataBuffer = resources.GetBufferRaw(data.cameraDataBuffer);
				CameraDataNew* cameraDataPtr = cameraDataBuffer->Map<CameraDataNew>();
			
				cameraDataPtr->projection = camera->GetProjection();
				cameraDataPtr->view = camera->GetView();
				cameraDataPtr->inverseView = glm::inverse(camera->GetView());
				cameraDataPtr->inverseProjection = glm::inverse(camera->GetProjection());
				cameraDataPtr->position = glm::vec4(camera->GetPosition(), 1.f);

				cameraDataBuffer->Unmap();
			}
		});
	}

	void SceneRendererNew::AddExternalResources(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		auto& bufferData = blackboard.Add<ExternalBuffersData>();
		auto& imageData = blackboard.Add<ExternalImagesData>();

		bufferData.indirectCommandsBuffer = renderGraph.AddExternalBuffer(m_indirectCommandsBuffer->GetResource(), false);
		bufferData.indirectCountsBuffer = renderGraph.AddExternalBuffer(m_indirectCountsBuffer->GetResource(), false);
		bufferData.instanceOffsetToObjectIDBuffer = renderGraph.AddExternalBuffer(m_instanceOffsetToObjectIDBuffer->GetResource(), false);
		bufferData.drawToInstanceOffsetBuffer = renderGraph.AddExternalBuffer(m_drawToInstanceOffsetBuffer->GetResource(), false);

		bufferData.drawContextBuffer = renderGraph.AddExternalBuffer(m_drawContextBuffer->GetResource(), false);

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
			const uint32_t dispatchCount = std::max(1u, static_cast<uint32_t>(m_currentActiveCommandCount / 256) + 1u);

			context.BindPipeline(m_clearIndirectCountsPipeline);
			context.SetConstant(resources.GetBuffer(bufferData.indirectCountsBuffer));
			context.SetConstant(static_cast<uint32_t>(m_currentActiveCommandCount));

			context.Dispatch(dispatchCount, 1, 1);
		});

		renderGraph.AddPass("Setup Indirect Args", [&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(bufferData.indirectCommandsBuffer);
			builder.WriteResource(bufferData.indirectCountsBuffer);
			builder.WriteResource(bufferData.instanceOffsetToObjectIDBuffer);

			builder.SetHasSideEffect();
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			const uint32_t dispatchCount = std::max(1u, (uint32_t)(m_currentActiveCommandCount / 256) + 1u);

			context.BindPipeline(m_indirectSetupPipeline);
			context.SetConstant(resources.GetBuffer(bufferData.indirectCountsBuffer));
			context.SetConstant(resources.GetBuffer(bufferData.indirectCommandsBuffer));
			context.SetConstant(resources.GetBuffer(bufferData.drawToInstanceOffsetBuffer));
			context.SetConstant(resources.GetBuffer(bufferData.instanceOffsetToObjectIDBuffer));
			context.SetConstant(static_cast<uint32_t>(m_currentActiveCommandCount));

			context.Dispatch(dispatchCount, 1, 1);
		});
	}

	void SceneRendererNew::AddPreDepthPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& externalBuffers = blackboard.Get<ExternalBuffersData>();
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();

		blackboard.Add<PreDepthData>() = renderGraph.AddPass<PreDepthData>("Pre Depth Pass",
		[&](RenderGraph::Builder& builder, PreDepthData& data)
		{
			RenderGraphImageDesc desc{};
			desc.width = m_width;
			desc.height = m_height;
			desc.format = RHI::PixelFormat::D32_SFLOAT;
			desc.usage = RHI::ImageUsage::Attachment;
			desc.name = "PreDepth";
			data.depth = builder.CreateImage2D(desc);

			desc.format = RHI::PixelFormat::R16G16B16A16_SFLOAT;
			desc.name = "Normals";
			data.normals = builder.CreateImage2D(desc);

			builder.ReadResource(externalBuffers.drawContextBuffer);
			builder.ReadResource(uniformBuffers.cameraDataBuffer);

			builder.SetHasSideEffect();
		},
		[=](const PreDepthData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Ref<RHI::ImageView> depthImageView = resources.GetImage2DView(data.depth);
			Ref<RHI::ImageView> normalsImageView = resources.GetImage2DView(data.normals);

			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { normalsImageView, depthImageView });

			context.BeginRendering(info);
			context.BindPipeline(m_preDepthPipeline);

			const auto gpuSceneHandle = m_scene->GetRenderScene()->GetGPUSceneBuffer().GetResourceHandle();
			const auto drawContextHandle = resources.GetBuffer(externalBuffers.drawContextBuffer);
			const auto cameraDataHandle = resources.GetBuffer(uniformBuffers.cameraDataBuffer);

			context.SetConstant(gpuSceneHandle);
			context.SetConstant(drawContextHandle);
			context.SetConstant(cameraDataHandle);

			context.DrawIndirectCount(m_indirectCommandsBuffer->GetResource(), 0, m_indirectCountsBuffer->GetResource(), 0, m_currentActiveCommandCount, sizeof(IndirectGPUCommandNew));
			context.EndRendering();
		});
	}

	void SceneRendererNew::AddVisibilityBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		/*const auto preDepthHandle = blackboard.Get<PreDepthData>().depth;

		blackboard.Add<VisibilityBufferData>() = renderGraph.AddPass<VisibilityBufferData>("Visibility Buffer",
		[&](RenderGraph::Builder& builder, VisibilityBufferData& data)
		{
			data.visibility = builder.CreateImage2D({ RHI::PixelFormat::R32G32_UINT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "VisiblityBuffer - Visibility" });
			builder.WriteResource(preDepthHandle);
			builder.SetHasSideEffect();
		},
		[=](const VisibilityBufferData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Ref<RHI::ImageView> visibilityView = resources.GetImage2D(data.visibility)->GetView();
			Ref<RHI::ImageView> depthView = resources.GetImage2D(preDepthHandle)->GetView();

			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { visibilityView, depthView });
			info.renderingInfo.depthAttachmentInfo.clearMode = RHI::ClearMode::Load;
			info.renderingInfo.colorAttachments.at(0).SetClearColor(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max());

			context.BeginRendering(info);
			context.BindPipeline(m_visibilityPipeline);

			UpdateDescriptorTableForMeshRendering(*m_scene->GetRenderScene(), context);

			context.DrawIndirectCount(m_indirectCommandsBuffer, 0, m_indirectCountsBuffer, 0, m_currentActiveCommandCount, sizeof(IndirectGPUCommandNew));
			context.EndRendering();
		});*/
	}

	void SceneRendererNew::AddVisibilityVisualizationPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		/*ExternalImagesData externalImagesData = blackboard.Get<ExternalImagesData>();
		VisibilityBufferData visBufferData = blackboard.Get<VisibilityBufferData>();

		renderGraph.AddPass("Visibility Visualization",
		[&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(externalImagesData.outputImage);

			builder.ReadResource(visBufferData.visibility);

			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto outputImage = resources.GetImage2D(externalImagesData.outputImage);

			context.BindPipeline(m_visibilityVisualizationPipeline);
			context.SetImageView(resources.GetImage2D(visBufferData.visibility)->GetView(), 0, 0);
			context.SetImageView(outputImage->GetView(), 0, 1);

			context.ClearImage(m_outputImage, { 0.1f, 0.1f, 0.1f, 1.f });
			context.PushConstants(m_visibilityVisualization);
			context.Dispatch(Math::DivideRoundUp(m_width, 16u), Math::DivideRoundUp(m_height, 16u), 1);
		});*/
	}

	void SceneRendererNew::AddGenerateMaterialCountsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		//ExternalBuffersData externalBuffersData = blackboard.Get<ExternalBuffersData>();
		//VisibilityBufferData visBufferData = blackboard.Get<VisibilityBufferData>();

		//blackboard.Add<MaterialCountData>() = renderGraph.AddPass<MaterialCountData>("Generate Material Count",
		//[&](RenderGraph::Builder& builder, MaterialCountData& data) 
		//{
		//	data.materialCountBuffer = builder.CreateBuffer({ m_materialsBuffer->GetSize() * sizeof(uint32_t), RHI::BufferUsage::StorageBuffer});
		//	data.materialStartBuffer = builder.CreateBuffer({ m_materialsBuffer->GetSize() * sizeof(uint32_t), RHI::BufferUsage::StorageBuffer });

		//	builder.ReadResource(visBufferData.visibility);
		//	builder.ReadResource(externalBuffersData.indirectDrawDataBuffer);

		//	builder.SetIsComputePass();
		//	builder.SetHasSideEffect();
		//}, 
		//[=](const MaterialCountData& data, RenderContext& context, const RenderGraphPassResources& resources) 
		//{
		//	Ref<RHI::ImageView> visBufferView = resources.GetImage2D(visBufferData.visibility)->GetView();
		//	Ref<RHI::BufferView> indirectDrawDataBuffer = resources.GetBuffer(externalBuffersData.indirectDrawDataBuffer)->GetView();

		//	Ref<RHI::StorageBuffer> materialCountBuffer = resources.GetBuffer(data.materialCountBuffer);
		//	
		//	context.ClearBuffer(materialCountBuffer, 0);

		//	context.BindPipeline(m_generateMaterialCountPipeline);
		//	context.SetImageView(visBufferView, 0, 0);
		//	context.SetBufferView(indirectDrawDataBuffer, 0, 1);
		//	context.SetBufferView(materialCountBuffer->GetView(), 0, 2);

		//	context.Dispatch(Math::DivideRoundUp(m_width, 16u), Math::DivideRoundUp(m_height, 16u), 1);
		//});
	}

	void SceneRendererNew::AddCollectMaterialPixelsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		//ExternalBuffersData externalBuffersData = blackboard.Get<ExternalBuffersData>();
		//VisibilityBufferData visBufferData = blackboard.Get<VisibilityBufferData>();
		//MaterialCountData matCountData = blackboard.Get<MaterialCountData>();

		//blackboard.Add<MaterialPixelsData>() = renderGraph.AddPass<MaterialPixelsData>("Collect Material Pixels",
		//[&](RenderGraph::Builder& builder, MaterialPixelsData& data)
		//{
		//	data.pixelCollectionBuffer = builder.CreateBuffer({ m_width * m_height * sizeof(glm::uvec2), RHI::BufferUsage::StorageBuffer });
		//	data.currentMaterialCountBuffer = builder.CreateBuffer({ m_materialsBuffer->GetSize() * sizeof(uint32_t), RHI::BufferUsage::StorageBuffer });

		//	builder.ReadResource(visBufferData.visibility);
		//	builder.ReadResource(externalBuffersData.indirectDrawDataBuffer);
		//	builder.ReadResource(matCountData.materialStartBuffer);

		//	builder.SetIsComputePass();
		//	builder.SetHasSideEffect();
		//},
		//[=](const MaterialPixelsData& data, RenderContext& context, const RenderGraphPassResources& resources)
		//{
		//	Ref<RHI::ImageView> visBufferView = resources.GetImage2D(visBufferData.visibility)->GetView();
		//	Ref<RHI::BufferView> indirectDrawDataBuffer = resources.GetBuffer(externalBuffersData.indirectDrawDataBuffer)->GetView();
		//	Ref<RHI::BufferView> matStartBuffer = resources.GetBuffer(matCountData.materialStartBuffer)->GetView();

		//	Ref<RHI::StorageBuffer> pixelCollectionBuffer = resources.GetBuffer(data.pixelCollectionBuffer);
		//	Ref<RHI::StorageBuffer> currentMatCountBuffer = resources.GetBuffer(data.currentMaterialCountBuffer);

		//	context.ClearBuffer(pixelCollectionBuffer, std::numeric_limits<uint32_t>::max());
		//	context.ClearBuffer(currentMatCountBuffer, 0);

		//	context.BindPipeline(m_collectMaterialPixelsPipeline);
		//	context.SetImageView("u_visibilityBuffer", visBufferView);
		//	context.SetBufferView("u_indirectDrawData", indirectDrawDataBuffer);
		//	context.SetBufferView("u_materialStartBuffer", matStartBuffer);
		//	context.SetBufferView("o_currentMaterialCountBuffer", currentMatCountBuffer->GetView());
		//	context.SetBufferView("o_pixelCollectionBuffer", pixelCollectionBuffer->GetView());

		//	context.Dispatch(Math::DivideRoundUp(m_width, 16u), Math::DivideRoundUp(m_height, 16u), 1);
		//});
	}

	void SceneRendererNew::AddGenerateMaterialIndirectArgsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		//MaterialCountData matCountData = blackboard.Get<MaterialCountData>();

		//blackboard.Add<MaterialIndirectArgsData>() = renderGraph.AddPass<MaterialIndirectArgsData>("Generate Material Indirect Args",
		//[&](RenderGraph::Builder& builder, MaterialIndirectArgsData& data) 
		//{
		//	data.materialIndirectArgsBuffer = builder.CreateBuffer({ m_materialsBuffer->GetSize() * sizeof(RHI::IndirectDispatchCommand), RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer });

		//	builder.ReadResource(matCountData.materialCountBuffer);
		//	builder.SetIsComputePass();
		//	builder.SetHasSideEffect();
		//}, 
		//[=](const MaterialIndirectArgsData& data, RenderContext& context, const RenderGraphPassResources& resources) 
		//{
		//	Ref<RHI::BufferView> matCountBuffer = resources.GetBuffer(matCountData.materialCountBuffer)->GetView();
		//	Ref<RHI::StorageBuffer> indirectArgsBuffer = resources.GetBuffer(data.materialIndirectArgsBuffer);

		//	context.ClearBuffer(indirectArgsBuffer, 0);

		//	const uint32_t materialCount = m_materialsBuffer->GetSize();

		//	context.BindPipeline(m_generateMaterialIndirectArgsPipeline);
		//	context.PushConstants(materialCount);
		//	context.SetBufferView("u_materialCounts", matCountBuffer);
		//	context.SetBufferView("u_indirectArgsBuffer", indirectArgsBuffer->GetView());
		//	context.Dispatch(Math::DivideRoundUp(m_materialsBuffer->GetSize(), 32u), 1, 1);
		//});
	}

	void SceneRendererNew::AddGenerateGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		//MaterialIndirectArgsData indirectArgsData = blackboard.Get<MaterialIndirectArgsData>();
		//VisibilityBufferData visBufferData = blackboard.Get<VisibilityBufferData>();
		//MaterialCountData matCountData = blackboard.Get<MaterialCountData>();
		//MaterialPixelsData matPixelsData = blackboard.Get<MaterialPixelsData>();

		//blackboard.Add<GBufferData>() = renderGraph.AddPass<GBufferData>("Generate GBuffer Data",
		//[&](RenderGraph::Builder& builder, GBufferData& data)
		//{
		//	data.albedo = builder.CreateImage2D({ RHI::PixelFormat::R16G16B16A16_SFLOAT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Albedo" });
		//	data.materialEmissive = builder.CreateImage2D({ RHI::PixelFormat::R16G16B16A16_SFLOAT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - MaterialEmissive" });
		//	data.normalEmissive = builder.CreateImage2D({ RHI::PixelFormat::R16G16B16A16_SFLOAT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - NormalEmissive" });

		//	builder.ReadResource(indirectArgsData.materialIndirectArgsBuffer, RHI::ResourceState::IndirectArgument);
		//	builder.ReadResource(visBufferData.visibility);
		//	builder.ReadResource(matCountData.materialCountBuffer);
		//	builder.ReadResource(matCountData.materialStartBuffer);
		//	builder.ReadResource(matPixelsData.pixelCollectionBuffer);

		//	builder.SetHasSideEffect();
		//	builder.SetIsComputePass();
		//},
		//[=](const GBufferData& data, RenderContext& context, const RenderGraphPassResources& resources)
		//{
		//	Ref<RHI::Image2D> albedoImage = resources.GetImage2D(data.albedo);
		//	Ref<RHI::Image2D> materialEmissiveImage = resources.GetImage2D(data.materialEmissive);
		//	Ref<RHI::Image2D> normalEmissiveImage = resources.GetImage2D(data.normalEmissive);
		//
		//	Ref<RHI::ImageView> visibilityBufferView = resources.GetImage2D(visBufferData.visibility)->GetView();
		//	Ref<RHI::BufferView> matCountBufferView = resources.GetBuffer(matCountData.materialCountBuffer)->GetView();
		//	Ref<RHI::BufferView> matStartBufferView = resources.GetBuffer(matCountData.materialStartBuffer)->GetView();
		//	Ref<RHI::BufferView> matPixelsBufferView = resources.GetBuffer(matPixelsData.pixelCollectionBuffer)->GetView();

		//	Ref<RHI::StorageBuffer> indirectArgsBuffer = resources.GetBuffer(indirectArgsData.materialIndirectArgsBuffer);
		//
		//	context.ClearImage(albedoImage, { 0.1f, 0.1f, 0.1f, 1.f });
		//	context.ClearImage(materialEmissiveImage, { 0.f, 0.f, 0.f, 0.f });
		//	context.ClearImage(normalEmissiveImage, { 0.f, 0.f, 0.f, 0.f });

		//	context.BindPipeline(m_generateGBufferPipeline);
		//	context.SetImageView("o_albedo", albedoImage->GetView());
		//	context.SetImageView("o_materialEmissive", materialEmissiveImage->GetView());
		//	context.SetImageView("o_normalEmissive", normalEmissiveImage->GetView());
		//	
		//	context.SetImageView("u_visibilityBuffer", visibilityBufferView);
		//	context.SetBufferView("u_materialCountBuffer", matCountBufferView);
		//	context.SetBufferView("u_materialStartBuffer", matStartBufferView);
		//	context.SetBufferView("u_pixelCollection", matPixelsBufferView);

		//	context.PushConstants(0);

		//	context.DispatchIndirect(indirectArgsBuffer, 0); // Should be offset with material ID
		//});
	}
}
