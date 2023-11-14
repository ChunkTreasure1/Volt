#include "vtpch.h"
#include "SceneRendererNew.h"

#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/RenderingNew/RenderScene.h"
#include "Volt/RenderingNew/RendererCommon.h"
#include "Volt/RenderingNew/RendererNew.h"

#include "Volt/RenderingNew/Shader/ShaderMap.h"

#include "Volt/RenderingNew/RenderGraph/RenderGraph.h"
#include "Volt/RenderingNew/RenderGraph/RenderGraphUtils.h"
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

		RenderGraphBlackboard rgBlackboard{};
		RenderGraph renderGraph{ m_commandBuffer };

		AddExternalResources(renderGraph, rgBlackboard);
		SetupDrawContext(renderGraph, rgBlackboard);

		UploadUniformBuffers(renderGraph, rgBlackboard, camera);
		AddSetupIndirectPasses(renderGraph, rgBlackboard);

		AddPreDepthPass(renderGraph, rgBlackboard);

		AddVisibilityBufferPass(renderGraph, rgBlackboard);
		AddGenerateMaterialCountsPass(renderGraph, rgBlackboard);

		PrefixSumTechnique prefixSum{ renderGraph, m_prefixSumPipeline };
		prefixSum.Execute(rgBlackboard.Get<MaterialCountData>().materialCountBuffer, rgBlackboard.Get<MaterialCountData>().materialStartBuffer, m_scene->GetRenderScene()->GetIndividualMaterialCount());

		AddCollectMaterialPixelsPass(renderGraph, rgBlackboard);
		AddGenerateMaterialIndirectArgsPass(renderGraph, rgBlackboard);

		 //For every material -> run compute shading shader using indirect args
		AddGenerateGBufferPass(renderGraph, rgBlackboard);

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
		renderScene->SetValid();
	}

	void SceneRendererNew::UploadIndirectCommands(RenderGraph& renderGraph, RenderGraphResourceHandle bufferHandle)
	{
		const auto& renderScene = m_scene->GetRenderScene();

		renderGraph.AddMappedBufferUpload(bufferHandle, renderScene->GetMeshCommands().data(), renderScene->GetMeshCommandCount() * sizeof(IndirectGPUCommandNew), "Upload Indirect Commands");
	}

	void SceneRendererNew::BuildMeshPass(RenderGraph::Builder& builder, RenderGraphBlackboard& blackboard)
	{
		const auto& externalBuffers = blackboard.Get<ExternalBuffersData>();
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();

		builder.ReadResource(externalBuffers.drawContextBuffer);
		builder.ReadResource(uniformBuffers.cameraDataBuffer);

		builder.ReadResource(externalBuffers.indirectCommandsBuffer, RHI::ResourceState::IndirectArgument);
		builder.ReadResource(externalBuffers.indirectCountsBuffer, RHI::ResourceState::IndirectArgument);
	}

	void SceneRendererNew::RenderMeshes(RenderContext& context, const RenderGraphPassResources& resources, const RenderGraphBlackboard blackboard)
	{
		const auto& externalBuffers = blackboard.Get<ExternalBuffersData>();
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();

		const auto gpuSceneHandle = m_scene->GetRenderScene()->GetGPUSceneBuffer().GetResourceHandle();
		const auto drawContextHandle = resources.GetBuffer(externalBuffers.drawContextBuffer);
		const auto cameraDataHandle = resources.GetBuffer(uniformBuffers.cameraDataBuffer);

		const auto indirectCommands = resources.GetBufferRaw(externalBuffers.indirectCommandsBuffer);
		const auto indirectCounts = resources.GetBufferRaw(externalBuffers.indirectCountsBuffer);

		context.SetConstant(gpuSceneHandle);
		context.SetConstant(drawContextHandle);
		context.SetConstant(cameraDataHandle);

		context.DrawIndirectCount(indirectCommands, 0, indirectCounts, 0, m_scene->GetRenderScene()->GetMeshCommandCount(), sizeof(IndirectGPUCommandNew));
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

		// Visibility Buffer pipeline
		{
			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("VisibilityBuffer");
			pipelineInfo.depthCompareOperator = RHI::CompareOperator::Equal;
			pipelineInfo.depthMode = RHI::DepthMode::Read;
			m_visibilityPipeline = RHI::RenderPipeline::Create(pipelineInfo);
		}

		// Visibility buffer compute
		{
			//	m_visibilityVisualizationPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("VisibilityVisualization"));
			m_generateMaterialCountPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("GenerateMaterialCount"));
			m_collectMaterialPixelsPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("CollectMaterialPixels"));
			m_generateMaterialIndirectArgsPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("GenerateMaterialIndirectArgs"));
			m_generateGBufferPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("GenerateGBuffer"));
		}

		// Utility
		{
			m_prefixSumPipeline = RHI::ComputePipeline::Create(ShaderMap::Get("PrefixSum"));
		}
	}

	void SceneRendererNew::UploadUniformBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera)
	{
		auto& buffersData = blackboard.Add<UniformBuffersData>();

		{
			const auto desc = RGUtils::CreateBufferDesc<CameraDataNew>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Camera Data");
			buffersData.cameraDataBuffer = renderGraph.CreateBuffer(desc);

			// Upload
			CameraDataNew camData{};
			camData.projection = camera->GetProjection();
			camData.view = camera->GetView();
			camData.inverseView = glm::inverse(camData.view);
			camData.inverseProjection = glm::inverse(camData.projection);
			camData.position = glm::vec4(camera->GetPosition(), 1.f);

			renderGraph.AddMappedBufferUpload(buffersData.cameraDataBuffer, &camData, sizeof(CameraDataNew), "Upload Camera Data");
		}
	}

	void SceneRendererNew::SetupDrawContext(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& renderScene = m_scene->GetRenderScene();
		auto& bufferData = blackboard.Get<ExternalBuffersData>();

		{
			const auto desc = RGUtils::CreateBufferDesc<IndirectGPUCommandNew>(std::max(renderScene->GetRenderObjectCount(), 1u), RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::CPUToGPU, "Indirect Commands");
			bufferData.indirectCommandsBuffer = renderGraph.CreateBuffer(desc);
		}

		{
			const auto desc = RGUtils::CreateBufferDesc<uint32_t>(2, RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, "Indirect Counts");
			bufferData.indirectCountsBuffer = renderGraph.CreateBuffer(desc);
		}

		{
			const auto desc = RGUtils::CreateBufferDesc<uint32_t>(std::max(renderScene->GetRenderObjectCount(), 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Draw To Instance Offset");
			bufferData.drawToInstanceOffsetBuffer = renderGraph.CreateBuffer(desc);
		}

		{
			const auto desc = RGUtils::CreateBufferDesc<uint32_t>(std::max(renderScene->GetRenderObjectCount(), 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Instance Offset To Object ID");
			bufferData.instanceOffsetToObjectIDBuffer = renderGraph.CreateBuffer(desc);
		}

		{
			const auto desc = RGUtils::CreateBufferDesc<DrawContext>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Draw Context");
			bufferData.drawContextBuffer = renderGraph.CreateBuffer(desc);
		}

		UploadIndirectCommands(renderGraph, bufferData.indirectCommandsBuffer);

		// Upload draw context
		{
			DrawContext context{};
			context.drawToInstanceOffset = renderGraph.GetBuffer(bufferData.drawToInstanceOffsetBuffer);
			context.instanceOffsetToObjectIDBuffer = renderGraph.GetBuffer(bufferData.instanceOffsetToObjectIDBuffer);

			renderGraph.AddMappedBufferUpload(bufferData.drawContextBuffer, &context, sizeof(DrawContext), "Upload Draw Context");
		}
	}

	void SceneRendererNew::AddExternalResources(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		auto& imageData = blackboard.Add<ExternalImagesData>();
		imageData.outputImage = renderGraph.AddExternalImage2D(m_outputImage);

		auto& bufferData = blackboard.Add<ExternalBuffersData>();
		bufferData.objectDrawDataBuffer = renderGraph.AddExternalBuffer(m_scene->GetRenderScene()->GetObjectDrawDataBuffer().GetResource(), false);
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
			const uint32_t commandCount = m_scene->GetRenderScene()->GetMeshCommandCount();
			const uint32_t dispatchCount = Math::DivideRoundUp(commandCount, 256u);

			context.BindPipeline(m_clearIndirectCountsPipeline);
			context.SetConstant(resources.GetBuffer(bufferData.indirectCountsBuffer));
			context.SetConstant(commandCount);

			context.Dispatch(dispatchCount, 1, 1);
		});

		renderGraph.AddPass("Setup Indirect Args", [&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(bufferData.indirectCommandsBuffer);
			builder.WriteResource(bufferData.indirectCountsBuffer);
			builder.WriteResource(bufferData.instanceOffsetToObjectIDBuffer);
			builder.WriteResource(bufferData.drawToInstanceOffsetBuffer);

			builder.SetHasSideEffect();
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			const uint32_t commandCount = m_scene->GetRenderScene()->GetMeshCommandCount();
			const uint32_t dispatchCount = Math::DivideRoundUp(commandCount, 256u);

			context.BindPipeline(m_indirectSetupPipeline);
			context.SetConstant(resources.GetBuffer(bufferData.indirectCountsBuffer));
			context.SetConstant(resources.GetBuffer(bufferData.indirectCommandsBuffer));
			context.SetConstant(resources.GetBuffer(bufferData.drawToInstanceOffsetBuffer));
			context.SetConstant(resources.GetBuffer(bufferData.instanceOffsetToObjectIDBuffer));
			context.SetConstant(commandCount);

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
			desc.format = RHI::PixelFormat::D32_SFLOAT;
			desc.usage = RHI::ImageUsage::Attachment;
			desc.name = "PreDepth";
			data.depth = builder.CreateImage2D(desc);

			desc.format = RHI::PixelFormat::R16G16B16A16_SFLOAT;
			desc.name = "Normals";
			data.normals = builder.CreateImage2D(desc);

			BuildMeshPass(builder, blackboard);

			builder.SetHasSideEffect();
		},
		[=](const PreDepthData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Weak<RHI::ImageView> depthImageView = resources.GetImage2DView(data.depth);
			Weak<RHI::ImageView> normalsImageView = resources.GetImage2DView(data.normals);

			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { normalsImageView, depthImageView });

			context.BeginRendering(info);
			context.BindPipeline(m_preDepthPipeline);

			RenderMeshes(context, resources, blackboard);

			context.EndRendering();
		});
	}

	void SceneRendererNew::AddVisibilityBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto preDepthHandle = blackboard.Get<PreDepthData>().depth;

		blackboard.Add<VisibilityBufferData>() = renderGraph.AddPass<VisibilityBufferData>("Visibility Buffer",
		[&](RenderGraph::Builder& builder, VisibilityBufferData& data)
		{
			data.visibility = builder.CreateImage2D({ RHI::PixelFormat::R32G32_UINT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "VisiblityBuffer - Visibility" });
			builder.WriteResource(preDepthHandle);

			BuildMeshPass(builder, blackboard);

			builder.SetHasSideEffect();
		},
		[=](const VisibilityBufferData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Ref<RHI::ImageView> visibilityView = resources.GetImage2DView(data.visibility);
			Ref<RHI::ImageView> depthView = resources.GetImage2DView(preDepthHandle);

			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { visibilityView, depthView });
			info.renderingInfo.depthAttachmentInfo.clearMode = RHI::ClearMode::Load;
			info.renderingInfo.colorAttachments.At(0).SetClearColor(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max());

			context.BeginRendering(info);
			context.BindPipeline(m_visibilityPipeline);

			RenderMeshes(context, resources, blackboard);

			context.EndRendering();
		});
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
		ExternalBuffersData externalBuffersData = blackboard.Get<ExternalBuffersData>();
		VisibilityBufferData visBufferData = blackboard.Get<VisibilityBufferData>();

		const auto& renderScene = m_scene->GetRenderScene();

		blackboard.Add<MaterialCountData>() = renderGraph.AddPass<MaterialCountData>("Generate Material Count",
		[&](RenderGraph::Builder& builder, MaterialCountData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<uint32_t>(std::max(renderScene->GetIndividualMaterialCount(), 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Material Counts");
				data.materialCountBuffer = builder.CreateBuffer(desc);
			}

			{
				const auto desc = RGUtils::CreateBufferDesc<uint32_t>(std::max(renderScene->GetIndividualMaterialCount(), 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Material Starts");
				data.materialStartBuffer = builder.CreateBuffer(desc);
			}

			builder.ReadResource(visBufferData.visibility);
			builder.ReadResource(externalBuffersData.objectDrawDataBuffer);

			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},
		[=](const MaterialCountData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Ref<RHI::StorageBuffer> materialCountBuffer = resources.GetBufferRaw(data.materialCountBuffer);
			context.ClearBuffer(materialCountBuffer, 0);

			context.BindPipeline(m_generateMaterialCountPipeline);
			context.SetConstant(resources.GetImage2D(visBufferData.visibility));
			context.SetConstant(m_scene->GetRenderScene()->GetObjectDrawDataBuffer().GetResourceHandle());
			context.SetConstant(resources.GetBuffer(data.materialCountBuffer));

			context.Dispatch(Math::DivideRoundUp(m_width, 16u), Math::DivideRoundUp(m_height, 16u), 1);
		});
	}

	void SceneRendererNew::AddCollectMaterialPixelsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		ExternalBuffersData externalBuffersData = blackboard.Get<ExternalBuffersData>();
		VisibilityBufferData visBufferData = blackboard.Get<VisibilityBufferData>();
		MaterialCountData matCountData = blackboard.Get<MaterialCountData>();

		blackboard.Add<MaterialPixelsData>() = renderGraph.AddPass<MaterialPixelsData>("Collect Material Pixels",
		[&](RenderGraph::Builder& builder, MaterialPixelsData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDescGPU<glm::uvec2>(m_width * m_height, "Pixel Collection");
				data.pixelCollectionBuffer = builder.CreateBuffer(desc);
			}

			{
				const auto desc = RGUtils::CreateBufferDescGPU<uint32_t>(m_scene->GetRenderScene()->GetIndividualMaterialCount(), "Current Material Count");
				data.currentMaterialCountBuffer = builder.CreateBuffer(desc);
			}

			builder.ReadResource(visBufferData.visibility);
			builder.ReadResource(externalBuffersData.objectDrawDataBuffer);
			builder.ReadResource(matCountData.materialStartBuffer);

			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},
		[=](const MaterialPixelsData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Weak<RHI::StorageBuffer> pixelCollectionBuffer = resources.GetBufferRaw(data.pixelCollectionBuffer);
			Weak<RHI::StorageBuffer> currentMatCountBuffer = resources.GetBufferRaw(data.currentMaterialCountBuffer);

			context.ClearBuffer(pixelCollectionBuffer, std::numeric_limits<uint32_t>::max());
			context.ClearBuffer(currentMatCountBuffer, 0);

			context.BindPipeline(m_collectMaterialPixelsPipeline);
			context.SetConstant(resources.GetImage2D(visBufferData.visibility));
			context.SetConstant(resources.GetBuffer(externalBuffersData.objectDrawDataBuffer));
			context.SetConstant(resources.GetBuffer(matCountData.materialStartBuffer));
			context.SetConstant(resources.GetBuffer(data.currentMaterialCountBuffer));
			context.SetConstant(resources.GetBuffer(data.pixelCollectionBuffer));

			context.Dispatch(Math::DivideRoundUp(m_width, 16u), Math::DivideRoundUp(m_height, 16u), 1);
		});
	}

	void SceneRendererNew::AddGenerateMaterialIndirectArgsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		MaterialCountData matCountData = blackboard.Get<MaterialCountData>();

		blackboard.Add<MaterialIndirectArgsData>() = renderGraph.AddPass<MaterialIndirectArgsData>("Generate Material Indirect Args",
		[&](RenderGraph::Builder& builder, MaterialIndirectArgsData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<RHI::IndirectDispatchCommand>(m_scene->GetRenderScene()->GetIndividualMaterialCount(), RHI::BufferUsage::IndirectBuffer | RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Material Indirect Args");
				data.materialIndirectArgsBuffer = builder.CreateBuffer(desc);
			}

			builder.ReadResource(matCountData.materialCountBuffer);
			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},
		[=](const MaterialIndirectArgsData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Weak<RHI::StorageBuffer> indirectArgsBuffer = resources.GetBufferRaw(data.materialIndirectArgsBuffer);

			context.ClearBuffer(indirectArgsBuffer, 0);

			const uint32_t materialCount = m_scene->GetRenderScene()->GetIndividualMaterialCount();

			context.BindPipeline(m_generateMaterialIndirectArgsPipeline);
			context.SetConstant(resources.GetBuffer(matCountData.materialCountBuffer));
			context.SetConstant(resources.GetBuffer(data.materialIndirectArgsBuffer));
			context.SetConstant(materialCount);

			context.Dispatch(Math::DivideRoundUp(materialCount, 32u), 1, 1);
		});
	}

	void SceneRendererNew::AddGenerateGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		MaterialIndirectArgsData indirectArgsData = blackboard.Get<MaterialIndirectArgsData>();
		VisibilityBufferData visBufferData = blackboard.Get<VisibilityBufferData>();
		MaterialCountData matCountData = blackboard.Get<MaterialCountData>();
		MaterialPixelsData matPixelsData = blackboard.Get<MaterialPixelsData>();

		blackboard.Add<GBufferData>() = renderGraph.AddPass<GBufferData>("Generate GBuffer Data",
		[&](RenderGraph::Builder& builder, GBufferData& data)
		{
			data.albedo = renderGraph.AddExternalImage2D(m_outputImage);  //builder.CreateImage2D({ RHI::PixelFormat::R16G16B16A16_SFLOAT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Albedo" });
			data.materialEmissive = builder.CreateImage2D({ RHI::PixelFormat::R16G16B16A16_SFLOAT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - MaterialEmissive" });
			data.normalEmissive = builder.CreateImage2D({ RHI::PixelFormat::R16G16B16A16_SFLOAT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - NormalEmissive" });

			builder.ReadResource(indirectArgsData.materialIndirectArgsBuffer, RHI::ResourceState::IndirectArgument);
			builder.ReadResource(visBufferData.visibility);
			builder.ReadResource(matCountData.materialCountBuffer);
			builder.ReadResource(matCountData.materialStartBuffer);
			builder.ReadResource(matPixelsData.pixelCollectionBuffer);

			builder.ReadResource(data.albedo);

			builder.SetHasSideEffect();
			builder.SetIsComputePass();
		},
		[=](const GBufferData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Weak<RHI::Image2D> albedoImage = resources.GetImage2DRaw(data.albedo);
			Weak<RHI::Image2D> materialEmissiveImage = resources.GetImage2DRaw(data.materialEmissive);
			Weak<RHI::Image2D> normalEmissiveImage = resources.GetImage2DRaw(data.normalEmissive);
		
			Weak<RHI::StorageBuffer> indirectArgsBuffer = resources.GetBufferRaw(indirectArgsData.materialIndirectArgsBuffer);
		
			context.ClearImage(albedoImage, { 0.1f, 0.1f, 0.1f, 1.f });
			context.ClearImage(materialEmissiveImage, { 0.f, 0.f, 0.f, 0.f });
			context.ClearImage(normalEmissiveImage, { 0.f, 0.f, 0.f, 0.f });

			context.BindPipeline(m_generateGBufferPipeline);
			
			context.SetConstant(resources.GetImage2D(visBufferData.visibility));
			context.SetConstant(resources.GetBuffer(matCountData.materialCountBuffer));
			context.SetConstant(resources.GetBuffer(matCountData.materialStartBuffer));
			context.SetConstant(resources.GetBuffer(matPixelsData.pixelCollectionBuffer));

			context.SetConstant(resources.GetImage2D(data.albedo));
			context.SetConstant(resources.GetImage2D(data.materialEmissive));
			context.SetConstant(resources.GetImage2D(data.normalEmissive));
			context.SetConstant(0); // Material ID

			context.DispatchIndirect(indirectArgsBuffer, 0); // Should be offset with material ID
		});
	}
}
