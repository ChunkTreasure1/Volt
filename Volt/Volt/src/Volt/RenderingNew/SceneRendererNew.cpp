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
#include "Volt/RenderingNew/RenderingTechniques/GTAOTechnique.h"

#include "Volt/RenderingNew/DrawContext.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Core/Profiling.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"

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

		// Create render target
		{
			RHI::ImageSpecification spec{};
			spec.width = specification.initialResolution.x;
			spec.height = specification.initialResolution.y;
			spec.usage = RHI::ImageUsage::AttachmentStorage;
			spec.generateMips = false;
			spec.format = RHI::PixelFormat::R16G16B16A16_SFLOAT;

			m_outputImage = RHI::Image2D::Create(spec);
		}

		m_sceneEnvironment.radianceMap = RendererNew::GetDefaultResources().blackCubeTexture;
		m_sceneEnvironment.irradianceMap = RendererNew::GetDefaultResources().blackCubeTexture;
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

		renderGraph.SetTotalAllocatedSizeCallback([&](const uint64_t totalSize)
		{
			m_frameTotalGPUAllocation = totalSize;
		});

		AddExternalResources(renderGraph, rgBlackboard);
		SetupDrawContext(renderGraph, rgBlackboard);


		UploadUniformBuffers(renderGraph, rgBlackboard, camera);
		UploadLightBuffers(renderGraph, rgBlackboard);

		if (m_scene->GetRenderScene()->GetRenderObjectCount() > 0)
		{
			renderGraph.BeginMarker("Culling", { 0.f, 1.f, 0.f, 1.f });

			AddCullObjectsPass(renderGraph, rgBlackboard, camera);
			AddCullMeshletsPass(renderGraph, rgBlackboard, camera);
			AddCullPrimitivesPass(renderGraph, rgBlackboard, camera);

			renderGraph.EndMarker();
		
			//AddStatsReadbackPass(renderGraph, rgBlackboard);

			AddPreDepthPass(renderGraph, rgBlackboard);

			GTAOSettings tempSettings{};
			tempSettings.radius = 0.5f;
			tempSettings.radiusMultiplier = 1.457f;
			tempSettings.falloffRange = 0.615f;
			tempSettings.finalValuePower = 2.2f;

			GTAOTechnique gtaoTechnique{ 0 /*m_frameIndex*/, tempSettings };
			gtaoTechnique.AddGTAOPasses(renderGraph, rgBlackboard, camera, { m_width, m_height });

			AddVisibilityBufferPass(renderGraph, rgBlackboard);
			AddGenerateMaterialCountsPass(renderGraph, rgBlackboard);

			PrefixSumTechnique prefixSum{ renderGraph };
			prefixSum.Execute(rgBlackboard.Get<MaterialCountData>().materialCountBuffer, rgBlackboard.Get<MaterialCountData>().materialStartBuffer, m_scene->GetRenderScene()->GetIndividualMaterialCount());
			
			AddCollectMaterialPixelsPass(renderGraph, rgBlackboard);
			AddGenerateMaterialIndirectArgsPass(renderGraph, rgBlackboard);

			////For every material -> run compute shading shader using indirect args
			auto& gbufferData = rgBlackboard.Add<GBufferData>();

			gbufferData.albedo = renderGraph.CreateImage2D({ RHI::PixelFormat::R16G16B16A16_SFLOAT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Albedo" });
			gbufferData.materialEmissive = renderGraph.CreateImage2D({ RHI::PixelFormat::R16G16B16A16_SFLOAT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - MaterialEmissive" });
			gbufferData.normalEmissive = renderGraph.CreateImage2D({ RHI::PixelFormat::R16G16B16A16_SFLOAT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - NormalEmissive" });

			renderGraph.BeginMarker("Materials");

			for (uint32_t matId = 0; matId < m_scene->GetRenderScene()->GetIndividualMaterialCount(); matId++)
			{
				AddGenerateGBufferPass(renderGraph, rgBlackboard, matId == 0, matId);
			}

			renderGraph.EndMarker();

			AddShadingPass(renderGraph, rgBlackboard);
		}

		{
			RenderGraphBarrierInfo barrier{};
			barrier.dstStage = RHI::BarrierStage::PixelShader;
			barrier.dstAccess = RHI::BarrierAccess::ShaderRead;
			barrier.dstLayout = RHI::ImageLayout::ShaderRead;

			renderGraph.AddResourceBarrier(rgBlackboard.Get<ExternalImagesData>().outputImage, barrier);
		}

		renderGraph.Compile();
		renderGraph.Execute();

		m_frameIndex++;
	}

	void SceneRendererNew::Invalidate()
	{
		VT_PROFILE_FUNCTION();

		auto renderScene = m_scene->GetRenderScene();
		renderScene->PrepareForUpdate();
		renderScene->SetValid();
	}

	const uint64_t SceneRendererNew::GetFrameTotalGPUAllocationSize() const
	{
		return m_frameTotalGPUAllocation.load();
	}

	void SceneRendererNew::BuildMeshPass(RenderGraph::Builder& builder, RenderGraphBlackboard& blackboard)
	{
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();
		const auto& cullPrimitivesData = blackboard.Get<CullPrimitivesData>();
		const auto& externalBuffers = blackboard.Get<ExternalBuffersData>();

		builder.ReadResource(uniformBuffers.viewDataBuffer);
		builder.ReadResource(externalBuffers.gpuSceneBuffer);
		builder.ReadResource(cullPrimitivesData.indexBuffer, RenderGraphResourceState::IndexBuffer);
		builder.ReadResource(cullPrimitivesData.drawCommand, RenderGraphResourceState::IndirectArgument);
	}

	void SceneRendererNew::RenderMeshes(RenderContext& context, const RenderGraphPassResources& resources, const RenderGraphBlackboard blackboard)
	{
		const auto& externalBuffers = blackboard.Get<ExternalBuffersData>();
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();
		const auto& cullPrimitivesData = blackboard.Get<CullPrimitivesData>();

		const auto gpuSceneHandle = resources.GetBuffer(externalBuffers.gpuSceneBuffer);
		const auto viewDataHandle = resources.GetBuffer(uniformBuffers.viewDataBuffer);

		const auto indirectCommands = resources.GetBufferRaw(cullPrimitivesData.drawCommand);
		const auto indexBuffer = resources.GetBufferRaw(cullPrimitivesData.indexBuffer);

		context.BindIndexBuffer(indexBuffer);
		context.SetConstant("gpuScene", gpuSceneHandle);
		context.SetConstant("viewData", viewDataHandle);

		context.DrawIndexedIndirect(indirectCommands, 0, 1, sizeof(RHI::IndirectIndexedCommand));
	}

	void SceneRendererNew::UploadUniformBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera)
	{
		auto& buffersData = blackboard.Add<UniformBuffersData>();

		{
			const auto desc = RGUtils::CreateBufferDesc<ViewData>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "View Data");
			buffersData.viewDataBuffer = renderGraph.CreateBuffer(desc);

			ViewData viewData{};

			// Camera
			viewData.projection = camera->GetProjection();
			viewData.view = camera->GetView();
			viewData.inverseView = glm::inverse(viewData.view);
			viewData.inverseProjection = glm::inverse(viewData.projection);
			viewData.viewProjection = viewData.projection * viewData.view;
			viewData.inverseViewProjection = glm::inverse(viewData.viewProjection);
			viewData.cameraPosition = glm::vec4(camera->GetPosition(), 1.f);
			viewData.nearPlane = camera->GetNearPlane();
			viewData.farPlane = camera->GetFarPlane();
			
			float depthLinearizeMul = (-viewData.projection[3][2]);
			float depthLinearizeAdd = (viewData.projection[2][2]);

			if (depthLinearizeMul * depthLinearizeAdd < 0.f)
			{
				depthLinearizeAdd = -depthLinearizeAdd;
			}

			viewData.depthUnpackConsts = { depthLinearizeMul, depthLinearizeAdd };

			// Render Target
			viewData.renderSize = { m_width, m_height };
			viewData.invRenderSize = { 1.f / static_cast<float>(m_width), 1.f / static_cast<float>(m_height) };

			m_scene->ForEachWithComponents<const PointLightComponent, const IDComponent, const TransformComponent>([&](entt::entity entityId, const PointLightComponent& comp, const IDComponent& idComp, const TransformComponent& transComp)
			{
				if (!transComp.visible)
				{
					return;
				}

				viewData.pointLightCount++;
			});

			m_scene->ForEachWithComponents<const SpotLightComponent, const IDComponent, const TransformComponent>([&](entt::entity entityId, const SpotLightComponent& comp, const IDComponent& idComp, const TransformComponent& transComp)
			{
				if (!transComp.visible)
				{
					return;
				}

				viewData.spotLightCount++;
			});

			renderGraph.AddMappedBufferUpload(buffersData.viewDataBuffer, &viewData, sizeof(ViewData), "Upload view data");
		}

		{
			const auto desc = RGUtils::CreateBufferDesc<DirectionalLightData>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Directional Light Data");
			buffersData.directionalLightBuffer = renderGraph.CreateBuffer(desc);

			DirectionalLightData data{};
			data.intensity = 0.f;

			m_scene->ForEachWithComponents<const DirectionalLightComponent, const IDComponent, const TransformComponent>([&](entt::entity id, const DirectionalLightComponent& dirLightComp, const IDComponent& idComp, const TransformComponent& comp) 
			{
				if (!comp.visible)
				{
					return;
				}

				auto entity = m_scene->GetEntityFromUUID(idComp.id);
				const glm::vec3 dir = glm::rotate(entity.GetRotation(), { 0.f, 0.f, 1.f }) * -1.f;
				
				data.color = dirLightComp.color;
				data.intensity = dirLightComp.intensity;
				data.direction = { dir, 0.f };
			});

			renderGraph.AddMappedBufferUpload(buffersData.directionalLightBuffer, &data, sizeof(DirectionalLightData), "Upload directional light data");
		}
	}

	void SceneRendererNew::UploadLightBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		auto& buffersData = blackboard.Add<LightBuffersData>();

		// Point lights
		{
			std::vector<PointLightData> pointLights{};

			m_scene->ForEachWithComponents<const PointLightComponent, const IDComponent, const TransformComponent>([&](entt::entity entityId, const PointLightComponent& comp, const IDComponent& idComp, const TransformComponent& transComp)
			{
				if (!transComp.visible)
				{
					return;
				}

				auto entity = m_scene->GetEntityFromUUID(idComp.id);
				
				auto& data = pointLights.emplace_back();
				data.position = entity.GetPosition();
				data.radius = comp.radius;
				data.color = comp.color;
				data.intensity = comp.intensity;
				data.falloff = comp.falloff;
			});

			const auto desc = RGUtils::CreateBufferDesc<PointLightData>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Point light Data");
			buffersData.pointLightsBuffer = renderGraph.CreateBuffer(desc);
			
			if (!pointLights.empty())
			{
				renderGraph.AddMappedBufferUpload(buffersData.pointLightsBuffer, pointLights.data(), sizeof(PointLightData) * pointLights.size(), "Upload point light data");
			}
		}

		// Spot lights
		{
			std::vector<SpotLightData> spotLights{};

			m_scene->ForEachWithComponents<const SpotLightComponent, const IDComponent, const TransformComponent>([&](entt::entity entityId, const SpotLightComponent& comp, const IDComponent& idComp, const TransformComponent& transComp)
			{
				if (!transComp.visible)
				{
					return;
				}

				auto entity = m_scene->GetEntityFromUUID(idComp.id);
				
				auto& data = spotLights.emplace_back();
				data.position = entity.GetPosition();
				data.color = comp.color;
				data.falloff = comp.falloff;
				data.intensity = comp.intensity;
				data.angleAttenuation = comp.angleAttenuation;
				data.direction = entity.GetForward() * -1.f;
				data.range = comp.range;
				data.angle = glm::radians(comp.angle);
			});

			const auto desc = RGUtils::CreateBufferDesc<SpotLightData>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Spot light Data");
			buffersData.spotLightsBuffer = renderGraph.CreateBuffer(desc);

			if (!spotLights.empty())
			{
				renderGraph.AddMappedBufferUpload(buffersData.spotLightsBuffer, spotLights.data(), sizeof(SpotLightData) * spotLights.size(), "Upload spot light data");
			}
		}
	}

	void SceneRendererNew::SetupDrawContext(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		//const auto& renderScene = m_scene->GetRenderScene();
		auto& bufferData = blackboard.Get<ExternalBuffersData>();

		{
			const auto desc = RGUtils::CreateBufferDesc<DrawContext>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Draw Context");
			bufferData.drawContextBuffer = renderGraph.CreateBuffer(desc);
		}

		// Upload draw context
		{
			//DrawContext context{};
			//context.instanceOffsetToObjectIDBuffer = renderGraph.GetBuffer(bufferData.instanceOffsetToObjectIDBuffer);
			//context.drawIndexToObjectId = renderGraph.GetBuffer(bufferData.drawIndexToObjectId);
			//context.drawIndexToMeshletId = renderGraph.GetBuffer(bufferData.drawIndexToMeshletId);

			//renderGraph.AddMappedBufferUpload(bufferData.drawContextBuffer, &context, sizeof(DrawContext), "Upload Draw Context");
		}
	}

	void SceneRendererNew::AddExternalResources(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		auto& imageData = blackboard.Add<ExternalImagesData>();
		imageData.outputImage = renderGraph.AddExternalImage2D(m_outputImage);
		imageData.black1x1Cube = renderGraph.AddExternalImage2D(RendererNew::GetDefaultResources().blackCubeTexture);
		imageData.BRDFLuT = renderGraph.AddExternalImage2D(RendererNew::GetDefaultResources().BRDFLuT);

		auto& bufferData = blackboard.Add<ExternalBuffersData>();
		bufferData.objectDrawDataBuffer = renderGraph.AddExternalBuffer(m_scene->GetRenderScene()->GetObjectDrawDataBuffer().GetResource(), false);
		bufferData.gpuMeshesBuffer = renderGraph.AddExternalBuffer(m_scene->GetRenderScene()->GetGPUMeshesBuffer().GetResource(), false);
		bufferData.gpuMeshletsBuffer = renderGraph.AddExternalBuffer(m_scene->GetRenderScene()->GetGPUMeshletsBuffer().GetResource(), false);
		bufferData.gpuSceneBuffer = renderGraph.AddExternalBuffer(m_scene->GetRenderScene()->GetGPUSceneBuffer().GetResource(), false);
	}

	void SceneRendererNew::AddCullObjectsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera)
	{
		auto renderScene = m_scene->GetRenderScene();

		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();
		const auto& externalBuffers = blackboard.Get<ExternalBuffersData>();

		blackboard.Add<CullObjectsData>() = renderGraph.AddPass<CullObjectsData>("Cull Objects",
		[&](RenderGraph::Builder& builder, CullObjectsData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<glm::uvec2>(std::max(renderScene->GetMeshletCount(), 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Meshlet To Object ID And Offset");
				data.meshletToObjectIdAndOffset = builder.CreateBuffer(desc);
			}

			{
				const auto desc = RGUtils::CreateBufferDesc<uint32_t>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Meshlet Count");
				data.meshletCount = builder.CreateBuffer(desc);
			}

			{
				const auto desc = RGUtils::CreateBufferDesc<uint32_t>(3, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::TransferSrc, RHI::MemoryUsage::GPU, "Statistics");
				data.statisticsBuffer = builder.CreateBuffer(desc);
			}

			builder.ReadResource(uniformBuffers.viewDataBuffer);
			builder.ReadResource(externalBuffers.objectDrawDataBuffer);
			builder.ReadResource(externalBuffers.gpuMeshesBuffer);
			builder.SetIsComputePass();
		},
		[=](const CullObjectsData& data, RenderContext& context, const RenderGraphPassResources& resources) 
		{
			const uint32_t commandCount = renderScene->GetRenderObjectCount();
			const uint32_t dispatchCount = Math::DivideRoundUp(commandCount, 256u);

			context.ClearBuffer(resources.GetBufferRaw(data.meshletCount), 0);
			context.ClearBuffer(resources.GetBufferRaw(data.statisticsBuffer), 0);

			auto pipeline = ShaderMap::GetComputePipeline("CullObjects");

			context.BindPipeline(pipeline);
			context.SetConstant("meshletCount", resources.GetBuffer(data.meshletCount));
			context.SetConstant("meshletToObjectIdAndOffset", resources.GetBuffer(data.meshletToObjectIdAndOffset));
			context.SetConstant("statisticsBuffer", resources.GetBuffer(data.statisticsBuffer));
			context.SetConstant("objectDrawDataBuffer", resources.GetBuffer(externalBuffers.objectDrawDataBuffer));
			context.SetConstant("meshBuffer", resources.GetBuffer(externalBuffers.gpuMeshesBuffer));
			context.SetConstant("viewData", resources.GetBuffer(uniformBuffers.viewDataBuffer));
			context.SetConstant("objectCount", commandCount);

			const auto projection = camera->GetProjection();
			const glm::mat4 projTranspose = glm::transpose(projection);

			const glm::vec4 frustumX = Math::NormalizePlane(projTranspose[3] + projTranspose[0]);
			const glm::vec4 frustumY = Math::NormalizePlane(projTranspose[3] + projTranspose[1]);

			context.SetConstant("frustum0", frustumX.x);
			context.SetConstant("frustum1", frustumX.z);
			context.SetConstant("frustum2", frustumY.y);
			context.SetConstant("frustum3", frustumY.z);

			context.Dispatch(dispatchCount, 1, 1);
		});
	}

	RenderGraphResourceHandle SceneRendererNew::AddGenerateIndirectArgsPass(RenderGraph& renderGraph, RenderGraphResourceHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName)
	{
		struct Output
		{
			RenderGraphResourceHandle argsBufferHandle = 0;
		};

		RenderGraphResourceHandle outHandle = 0;

		renderGraph.AddPass<Output>("Generate Indirect Args",
		[&](RenderGraph::Builder& builder, Output& data)
		{
			const auto argsDesc = RGUtils::CreateBufferDesc<uint32_t>(3, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, argsBufferName);
			data.argsBufferHandle = builder.CreateBuffer(argsDesc);
			outHandle = data.argsBufferHandle;

			builder.ReadResource(countBuffer);
			builder.SetIsComputePass();
		},
		[=](const Output& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("GenerateIndirectArgs");

			context.BindPipeline(pipeline);
			context.SetConstant("indirectArgs", resources.GetBuffer(data.argsBufferHandle));
			context.SetConstant("countBuffer", resources.GetBuffer(countBuffer));
			context.SetConstant("threadGroupSize", groupSize);

			context.Dispatch(1, 1, 1);
		});

		return outHandle;
	}

	RenderGraphResourceHandle SceneRendererNew::AddGenerateIndirectArgsPassWrapped(RenderGraph& renderGraph, RenderGraphResourceHandle countBuffer, uint32_t groupSize, std::string_view argsBufferName)
	{
		struct Output
		{
			RenderGraphResourceHandle argsBufferHandle = 0;
		};

		RenderGraphResourceHandle outHandle = 0;

		renderGraph.AddPass<Output>("Generate Indirect Args",
		[&](RenderGraph::Builder& builder, Output& data)
		{
			const auto argsDesc = RGUtils::CreateBufferDesc<uint32_t>(3, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, argsBufferName);
			data.argsBufferHandle = builder.CreateBuffer(argsDesc);
			outHandle = data.argsBufferHandle;

			builder.ReadResource(countBuffer);
			builder.SetIsComputePass();
		},
		[=](const Output& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("GenerateIndirectArgsWrapped");

			context.BindPipeline(pipeline);
			context.SetConstant("indirectArgs", resources.GetBuffer(data.argsBufferHandle));
			context.SetConstant("countBuffer", resources.GetBuffer(countBuffer));
			context.SetConstant("groupSize", groupSize);

			context.Dispatch(1, 1, 1);
		});

		return outHandle;
	}

	void SceneRendererNew::AddCullMeshletsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera)
	{
		auto renderScene = m_scene->GetRenderScene();

		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();
		const auto& cullObjectsData = blackboard.Get<CullObjectsData>();
		const auto& externalBuffers = blackboard.Get<ExternalBuffersData>();

		RenderGraphResourceHandle argsBufferHandle = AddGenerateIndirectArgsPass(renderGraph, cullObjectsData.meshletCount, 256, "Cull Meshlets Indirect Args");

		blackboard.Add<CullMeshletsData>() = renderGraph.AddPass<CullMeshletsData>("Cull Meshlets",
		[&](RenderGraph::Builder& builder, CullMeshletsData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<uint32_t>(std::max(renderScene->GetMeshletCount(), 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Surviving Meshlets");
				data.survivingMeshlets = builder.CreateBuffer(desc);
			}

			{
				const auto desc = RGUtils::CreateBufferDesc<uint32_t>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Surviving Meshlets Count");
				data.survivingMeshletCount = builder.CreateBuffer(desc);
			}

			builder.ReadResource(uniformBuffers.viewDataBuffer);
			builder.ReadResource(cullObjectsData.meshletCount);
			builder.ReadResource(cullObjectsData.meshletToObjectIdAndOffset);
			builder.ReadResource(externalBuffers.gpuMeshletsBuffer);
			builder.ReadResource(externalBuffers.objectDrawDataBuffer);
			builder.ReadResource(argsBufferHandle, RenderGraphResourceState::IndirectArgument);
			
			builder.SetIsComputePass();
		},
		[=](const CullMeshletsData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			context.ClearBuffer(resources.GetBufferRaw(data.survivingMeshletCount), 0);

			auto pipeline = ShaderMap::GetComputePipeline("CullMeshlets");

			context.BindPipeline(pipeline);
			context.SetConstant("survivingMeshlets", resources.GetBuffer(data.survivingMeshlets));
			context.SetConstant("survivingMeshletCount", resources.GetBuffer(data.survivingMeshletCount));
			context.SetConstant("meshletCount", resources.GetBuffer(cullObjectsData.meshletCount));
			context.SetConstant("meshletToObjectIdAndOffset", resources.GetBuffer(cullObjectsData.meshletToObjectIdAndOffset));
			context.SetConstant("gpuMeshlets", resources.GetBuffer(externalBuffers.gpuMeshletsBuffer));
			context.SetConstant("objectDrawDataBuffer", resources.GetBuffer(externalBuffers.objectDrawDataBuffer));
			context.SetConstant("viewData", resources.GetBuffer(uniformBuffers.viewDataBuffer));

			const auto projection = camera->GetProjection();
			const glm::mat4 projTranspose = glm::transpose(projection);

			const glm::vec4 frustumX = Math::NormalizePlane(projTranspose[3] + projTranspose[0]);
			const glm::vec4 frustumY = Math::NormalizePlane(projTranspose[3] + projTranspose[1]);

			context.SetConstant("frustum0", frustumX.x);
			context.SetConstant("frustum1", frustumX.z);
			context.SetConstant("frustum2", frustumY.y);
			context.SetConstant("frustum3", frustumY.z);

			auto argsBuffer = resources.GetBufferRaw(argsBufferHandle);
			context.DispatchIndirect(argsBuffer, 0);
		});
	}

	void SceneRendererNew::AddCullPrimitivesPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera)
	{
		auto renderScene = m_scene->GetRenderScene();

		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();
		const auto& cullMeshletsData = blackboard.Get<CullMeshletsData>();
		const auto& externalBuffers = blackboard.Get<ExternalBuffersData>();

		RenderGraphResourceHandle argsBufferHandle = AddGenerateIndirectArgsPassWrapped(renderGraph, cullMeshletsData.survivingMeshletCount, 1, "Cull Primitives Indirect Args");

		blackboard.Add<CullPrimitivesData>() = renderGraph.AddPass<CullPrimitivesData>("Cull Primitives",
		[&](RenderGraph::Builder& builder, CullPrimitivesData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<uint32_t>(std::max(renderScene->GetIndexCount(), 1u), RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndexBuffer, RHI::MemoryUsage::GPU, "Meshlet Index Buffer");
				data.indexBuffer = builder.CreateBuffer(desc);
			}

			{
				const auto desc = RGUtils::CreateBufferDesc<RHI::IndirectIndexedCommand>(1, RHI::BufferUsage::StorageBuffer | RHI::BufferUsage::IndirectBuffer, RHI::MemoryUsage::GPU, "Meshlet Draw Command");
				data.drawCommand = builder.CreateBuffer(desc);
			}

			builder.ReadResource(uniformBuffers.viewDataBuffer);
			builder.ReadResource(cullMeshletsData.survivingMeshlets);
			builder.ReadResource(cullMeshletsData.survivingMeshletCount);
			builder.ReadResource(externalBuffers.gpuMeshletsBuffer);
			builder.ReadResource(externalBuffers.gpuMeshesBuffer);
			builder.ReadResource(externalBuffers.objectDrawDataBuffer);
			builder.ReadResource(argsBufferHandle, RenderGraphResourceState::IndirectArgument);
		
			builder.SetIsComputePass();
		},
		[=](const CullPrimitivesData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			{
				RHI::IndirectIndexedCommand command{};
				command.indexCount = 0;
				command.firstInstance = 0;
				command.firstIndex = 0;
				command.vertexOffset = 0;
				command.instanceCount = 1;
				context.UploadBufferData(resources.GetBufferRaw(data.drawCommand), &command, sizeof(RHI::IndirectIndexedCommand));
			}

			context.ClearBuffer(resources.GetBufferRaw(data.indexBuffer), 0);

			auto pipeline = ShaderMap::GetComputePipeline("CullPrimitives");

			context.BindPipeline(pipeline);
			context.SetConstant("indexBuffer", resources.GetBuffer(data.indexBuffer));
			context.SetConstant("drawCommand", resources.GetBuffer(data.drawCommand));
			context.SetConstant("survivingMeshlets", resources.GetBuffer(cullMeshletsData.survivingMeshlets));
			context.SetConstant("survivingMeshletCount", resources.GetBuffer(cullMeshletsData.survivingMeshletCount));
			context.SetConstant("gpuMeshlets", resources.GetBuffer(externalBuffers.gpuMeshletsBuffer));
			context.SetConstant("gpuMeshes", resources.GetBuffer(externalBuffers.gpuMeshesBuffer));
			context.SetConstant("objectDrawDataBuffer", resources.GetBuffer(externalBuffers.objectDrawDataBuffer));
			context.SetConstant("viewData", resources.GetBuffer(uniformBuffers.viewDataBuffer));
			context.SetConstant("renderSize", glm::vec2(m_width, m_height));

			auto argsBuffer = resources.GetBufferRaw(argsBufferHandle);
			context.DispatchIndirect(argsBuffer, 0);
		});
	}

	void SceneRendererNew::AddStatsReadbackPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& objectCullData = blackboard.Get<CullObjectsData>();

		renderGraph.AddPass("Statistics Readback",
		[&](RenderGraph::Builder& builder)
		{
			builder.ReadResource(objectCullData.statisticsBuffer);
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			context.Flush();
			Ref<RHI::StorageBuffer> testBuffer = context.GetReadbackBuffer(resources.GetBufferRaw(objectCullData.statisticsBuffer));
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
		},
		[=](const PreDepthData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Weak<RHI::ImageView> depthImageView = resources.GetImage2DView(data.depth);
			Weak<RHI::ImageView> normalsImageView = resources.GetImage2DView(data.normals);

			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { normalsImageView, depthImageView });

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("PreDepth");

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);
			context.BindPipeline(pipeline);

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
			data.visibility = builder.CreateImage2D({ RHI::PixelFormat::R32_UINT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "VisibilityBuffer - Visibility" });
			builder.WriteResource(preDepthHandle);

			BuildMeshPass(builder, blackboard);
		},
		[=](const VisibilityBufferData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Ref<RHI::ImageView> visibilityView = resources.GetImage2DView(data.visibility);
			Ref<RHI::ImageView> depthView = resources.GetImage2DView(preDepthHandle);

			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { visibilityView, depthView });
			info.renderingInfo.depthAttachmentInfo.clearMode = RHI::ClearMode::Load;
			info.renderingInfo.colorAttachments.At(0).SetClearColor(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max());

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("VisibilityBuffer");
			pipelineInfo.depthCompareOperator = RHI::CompareOperator::Equal;
			pipelineInfo.depthMode = RHI::DepthMode::Read;

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);
			context.BindPipeline(pipeline);

			RenderMeshes(context, resources, blackboard);

			context.EndRendering();
		});
	}

	void SceneRendererNew::AddGenerateMaterialCountsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const ExternalBuffersData& externalBuffersData = blackboard.Get<ExternalBuffersData>();
		const VisibilityBufferData& visBufferData = blackboard.Get<VisibilityBufferData>();

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
			builder.ReadResource(externalBuffersData.gpuMeshletsBuffer);

			builder.SetIsComputePass();
		},
		[=](const MaterialCountData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Ref<RHI::StorageBuffer> materialCountBuffer = resources.GetBufferRaw(data.materialCountBuffer);
			context.ClearBuffer(materialCountBuffer, 0);

			auto pipeline = ShaderMap::GetComputePipeline("GenerateMaterialCount");

			context.BindPipeline(pipeline);
			context.SetConstant("visibilityBuffer", resources.GetImage2D(visBufferData.visibility));
			context.SetConstant("objectDrawData", resources.GetBuffer(externalBuffersData.objectDrawDataBuffer));
			context.SetConstant("meshletsBuffer", resources.GetBuffer(externalBuffersData.gpuMeshletsBuffer));
			context.SetConstant("materialCountsBuffer", resources.GetBuffer(data.materialCountBuffer));

			context.Dispatch(Math::DivideRoundUp(m_width, 8u), Math::DivideRoundUp(m_height, 8u), 1);
		});
	}

	void SceneRendererNew::AddCollectMaterialPixelsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const ExternalBuffersData& externalBuffersData = blackboard.Get<ExternalBuffersData>();
		const VisibilityBufferData& visBufferData = blackboard.Get<VisibilityBufferData>();
		const MaterialCountData& matCountData = blackboard.Get<MaterialCountData>();

		blackboard.Add<MaterialPixelsData>() = renderGraph.AddPass<MaterialPixelsData>("Collect Material Pixels",
		[&](RenderGraph::Builder& builder, MaterialPixelsData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDescGPU<glm::uvec2>(m_width * m_height, "Pixel Collection");
				data.pixelCollectionBuffer = builder.CreateBuffer(desc);
			}

			{
				const auto desc = RGUtils::CreateBufferDescGPU<uint32_t>(std::max(m_scene->GetRenderScene()->GetIndividualMaterialCount(), 1u), "Current Material Count");
				data.currentMaterialCountBuffer = builder.CreateBuffer(desc);
			}

			builder.ReadResource(visBufferData.visibility);
			builder.ReadResource(externalBuffersData.objectDrawDataBuffer);
			builder.ReadResource(externalBuffersData.gpuMeshletsBuffer);
			builder.ReadResource(matCountData.materialStartBuffer);

			builder.SetIsComputePass();
		},
		[=](const MaterialPixelsData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Weak<RHI::StorageBuffer> pixelCollectionBuffer = resources.GetBufferRaw(data.pixelCollectionBuffer);
			Weak<RHI::StorageBuffer> currentMatCountBuffer = resources.GetBufferRaw(data.currentMaterialCountBuffer);

			context.ClearBuffer(pixelCollectionBuffer, std::numeric_limits<uint32_t>::max());
			context.ClearBuffer(currentMatCountBuffer, 0);

			auto pipeline = ShaderMap::GetComputePipeline("CollectMaterialPixels");

			context.BindPipeline(pipeline);
			context.SetConstant("visibilityBuffer", resources.GetImage2D(visBufferData.visibility));
			context.SetConstant("objectDrawData", resources.GetBuffer(externalBuffersData.objectDrawDataBuffer));
			context.SetConstant("meshletsBuffer", resources.GetBuffer(externalBuffersData.gpuMeshletsBuffer));
			context.SetConstant("materialStartBuffer", resources.GetBuffer(matCountData.materialStartBuffer));
			context.SetConstant("currentMaterialCountBuffer", resources.GetBuffer(data.currentMaterialCountBuffer));
			context.SetConstant("pixelCollectionBuffer", resources.GetBuffer(data.pixelCollectionBuffer));

			context.Dispatch(Math::DivideRoundUp(m_width, 8u), Math::DivideRoundUp(m_height, 8u), 1);
		});
	}

	void SceneRendererNew::AddGenerateMaterialIndirectArgsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		MaterialCountData matCountData = blackboard.Get<MaterialCountData>();

		blackboard.Add<MaterialIndirectArgsData>() = renderGraph.AddPass<MaterialIndirectArgsData>("Generate Material Indirect Args",
		[&](RenderGraph::Builder& builder, MaterialIndirectArgsData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<RHI::IndirectDispatchCommand>(std::max(m_scene->GetRenderScene()->GetIndividualMaterialCount(), 1u), RHI::BufferUsage::IndirectBuffer | RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Material Indirect Args");
				data.materialIndirectArgsBuffer = builder.CreateBuffer(desc);
			}

			builder.ReadResource(matCountData.materialCountBuffer);
			builder.SetIsComputePass();
		},
		[=](const MaterialIndirectArgsData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Weak<RHI::StorageBuffer> indirectArgsBuffer = resources.GetBufferRaw(data.materialIndirectArgsBuffer);

			context.ClearBuffer(indirectArgsBuffer, 0);
			const uint32_t materialCount = m_scene->GetRenderScene()->GetIndividualMaterialCount();

			auto pipeline = ShaderMap::GetComputePipeline("GenerateMaterialIndirectArgs");

			context.BindPipeline(pipeline);
			context.SetConstant("materialCounts", resources.GetBuffer(matCountData.materialCountBuffer));
			context.SetConstant("indirectArgsBuffer", resources.GetBuffer(data.materialIndirectArgsBuffer));
			context.SetConstant("materialCount", materialCount);

			context.Dispatch(Math::DivideRoundUp(materialCount, 32u), 1, 1);
		});
	}

	void SceneRendererNew::AddGenerateGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, bool first, const uint32_t materialId)
	{
		const auto& indirectArgsData = blackboard.Get<MaterialIndirectArgsData>();
		const auto& visBufferData = blackboard.Get<VisibilityBufferData>();
		const auto& matCountData = blackboard.Get<MaterialCountData>();
		const auto& matPixelsData = blackboard.Get<MaterialPixelsData>();
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();
		const auto& externalBuffersData = blackboard.Get<ExternalBuffersData>();

		const auto& gbufferData = blackboard.Get<GBufferData>();

		const std::string passName = std::format("Generate GBuffer Data: Material {0}", materialId);

		const auto& renderScene = m_scene->GetRenderScene();

		renderGraph.AddPass(passName,
		[&](RenderGraph::Builder& builder)
		{
			builder.ReadResource(indirectArgsData.materialIndirectArgsBuffer, RenderGraphResourceState::IndirectArgument);
			builder.ReadResource(visBufferData.visibility);
			builder.ReadResource(matCountData.materialCountBuffer);
			builder.ReadResource(matCountData.materialStartBuffer);
			builder.ReadResource(matPixelsData.pixelCollectionBuffer);
			builder.ReadResource(externalBuffersData.gpuSceneBuffer);
			builder.ReadResource(uniformBuffers.viewDataBuffer);

			builder.WriteResource(gbufferData.albedo);
			builder.WriteResource(gbufferData.materialEmissive);
			builder.WriteResource(gbufferData.normalEmissive);

			builder.SetIsComputePass();
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			Weak<RHI::Image2D> albedoImage = resources.GetImage2DRaw(gbufferData.albedo);
			Weak<RHI::Image2D> materialEmissiveImage = resources.GetImage2DRaw(gbufferData.materialEmissive);
			Weak<RHI::Image2D> normalEmissiveImage = resources.GetImage2DRaw(gbufferData.normalEmissive);

			Weak<RHI::StorageBuffer> indirectArgsBuffer = resources.GetBufferRaw(indirectArgsData.materialIndirectArgsBuffer);

			if (first)
			{
				context.ClearImage(albedoImage, { 0.1f, 0.1f, 0.1f, 0.f });
				context.ClearImage(materialEmissiveImage, { 0.f, 0.f, 0.f, 0.f });
				context.ClearImage(normalEmissiveImage, { 0.f, 0.f, 0.f, 0.f });
			}

			auto material = renderScene->GetMaterialFromID(materialId);
			auto pipeline = material->GetComputePipeline();

			if (!pipeline)
			{
				pipeline = ShaderMap::GetComputePipeline("GenerateGBuffer");
			}

			context.BindPipeline(pipeline);

			context.SetConstant("visibilityBuffer", resources.GetImage2D(visBufferData.visibility));
			context.SetConstant("materialCountBuffer", resources.GetBuffer(matCountData.materialCountBuffer));
			context.SetConstant("materialStartBuffer", resources.GetBuffer(matCountData.materialStartBuffer));
			context.SetConstant("pixelCollection", resources.GetBuffer(matPixelsData.pixelCollectionBuffer));

			context.SetConstant("gpuScene", resources.GetBuffer(externalBuffersData.gpuSceneBuffer));
			context.SetConstant("viewData", resources.GetBuffer(uniformBuffers.viewDataBuffer));

			context.SetConstant("albedo", resources.GetImage2D(gbufferData.albedo));
			context.SetConstant("materialEmissive", resources.GetImage2D(gbufferData.materialEmissive));
			context.SetConstant("normalEmissive", resources.GetImage2D(gbufferData.normalEmissive));
			context.SetConstant("materialId", materialId);

			context.SetConstant("viewSize", glm::vec2(m_width, m_height));

			context.DispatchIndirect(indirectArgsBuffer, sizeof(RHI::IndirectDispatchCommand) * materialId); // Should be offset with material ID
		});
	}

	void SceneRendererNew::AddShadingPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();
		const auto& externalImages = blackboard.Get<ExternalImagesData>();
		const auto& lightBuffers = blackboard.Get<LightBuffersData>();

		const auto& gbufferData = blackboard.Get<GBufferData>();
		const auto& preDepthData = blackboard.Get<PreDepthData>();

		m_scene->ForEachWithComponents<SkylightComponent>([&](entt::entity id, SkylightComponent& skylightComp)
		{
			if (skylightComp.environmentHandle != skylightComp.lastEnvironmentHandle)
			{
				skylightComp.currentSceneEnvironment = RendererNew::GenerateEnvironmentTextures(skylightComp.environmentHandle);
				skylightComp.lastEnvironmentHandle = skylightComp.environmentHandle;
			}

			m_sceneEnvironment = skylightComp.currentSceneEnvironment;
		});

		RenderGraphResourceHandle irradianceHandle = m_sceneEnvironment.irradianceMap ? renderGraph.AddExternalImage2D(m_sceneEnvironment.irradianceMap) : externalImages.black1x1Cube;
		RenderGraphResourceHandle radianceHandle = m_sceneEnvironment.radianceMap ? renderGraph.AddExternalImage2D(m_sceneEnvironment.radianceMap) : externalImages.black1x1Cube;

		blackboard.Add<FinalColorData>() = renderGraph.AddPass<FinalColorData>("Shading Pass",
		[&](RenderGraph::Builder& builder, FinalColorData& data) 
		{
			data.finalColorOutput = renderGraph.AddExternalImage2D(m_outputImage);

			builder.WriteResource(data.finalColorOutput);
			builder.ReadResource(gbufferData.albedo);
			builder.ReadResource(gbufferData.materialEmissive);
			builder.ReadResource(gbufferData.normalEmissive);
			builder.ReadResource(preDepthData.depth);

			// PBR Constants
			builder.ReadResource(uniformBuffers.viewDataBuffer);
			builder.ReadResource(uniformBuffers.directionalLightBuffer);
			builder.ReadResource(externalImages.BRDFLuT);
			builder.ReadResource(irradianceHandle);
			builder.ReadResource(radianceHandle);
			builder.ReadResource(lightBuffers.pointLightsBuffer);
			builder.ReadResource(lightBuffers.spotLightsBuffer);

			builder.SetHasSideEffect();
			builder.SetIsComputePass();
		},
		[=](const FinalColorData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			Weak<RHI::Image2D> outputImage = resources.GetImage2DRaw(data.finalColorOutput);

			context.ClearImage(outputImage, { 0.1f, 0.1f, 0.1f, 0.f });

			auto pipeline = ShaderMap::GetComputePipeline("Shading");
			context.BindPipeline(pipeline);
			context.SetConstant("output", resources.GetImage2D(data.finalColorOutput));
			context.SetConstant("albedo", resources.GetImage2D(gbufferData.albedo));
			context.SetConstant("materialEmissive", resources.GetImage2D(gbufferData.materialEmissive));
			context.SetConstant("normalEmissive", resources.GetImage2D(gbufferData.normalEmissive));
			context.SetConstant("depthTexture", resources.GetImage2D(preDepthData.depth));
			
			// PBR Constants
			context.SetConstant("pbrConstants.viewData", resources.GetBuffer(uniformBuffers.viewDataBuffer));
			context.SetConstant("pbrConstants.DirectionalLight", resources.GetBuffer(uniformBuffers.directionalLightBuffer));
			context.SetConstant("pbrConstants.linearSampler", RendererNew::GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear>()->GetResourceHandle());
			context.SetConstant("pbrConstants.pointLinearClampSampler", RendererNew::GetSampler<RHI::TextureFilter::Nearest, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureWrap::Clamp>()->GetResourceHandle());
			context.SetConstant("pbrConstants.BRDFLuT", resources.GetImage2D(externalImages.BRDFLuT));
			context.SetConstant("pbrConstants.environmentIrradiance", resources.GetImage2D(irradianceHandle));
			context.SetConstant("pbrConstants.environmentRadiance", resources.GetImage2D(radianceHandle));
			context.SetConstant("pbrConstants.pointLights", resources.GetBuffer(lightBuffers.pointLightsBuffer));
			context.SetConstant("pbrConstants.spotLights", resources.GetBuffer(lightBuffers.spotLightsBuffer));

			context.Dispatch(Math::DivideRoundUp(m_width, 8u), Math::DivideRoundUp(m_height, 8u), 1u);
		});
	}
}
