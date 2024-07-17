#include "vtpch.h"
#include "SceneRenderer.h"

#include "Volt/Core/Application.h"

#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/Rendering/RenderScene.h"
#include "Volt/Rendering/RendererCommon.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/Shader/ShaderMap.h"

#include "Volt/Rendering/RenderGraph/RenderGraph.h"
#include "Volt/Rendering/RenderGraph/RenderGraphUtils.h"
#include "Volt/Rendering/RenderGraph/RenderGraphBlackboard.h"
#include "Volt/Rendering/RenderGraph/RenderGraphExecutionThread.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphBufferResource.h"
#include "Volt/Rendering/RenderGraph/Resources/RenderGraphTextureResource.h"
#include "Volt/Rendering/RenderGraph/RenderContextUtils.h"

#include "Volt/Rendering/RenderingTechniques/PrefixSumTechnique.h"
#include "Volt/Rendering/RenderingTechniques/GTAOTechnique.h"
#include "Volt/Rendering/RenderingTechniques/DirectionalShadowTechnique.h"
#include "Volt/Rendering/RenderingTechniques/LightCullingTechnique.h"
#include "Volt/Rendering/RenderingTechniques/TAATechnique.h"
#include "Volt/Rendering/RenderingTechniques/VelocityTechnique.h"

#include "Volt/Rendering/RenderingUtils.h"
#include "Volt/Rendering/ShapeLibrary.h"
#include "Volt/Rendering/DrawContext.h"

#include "Volt/Project/ProjectManager.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"
#include "Volt/Asset/AssetManager.h"

#include "Volt/Math/Math.h"

#include "Volt/Components/LightComponents.h"
#include "Volt/Components/RenderingComponents.h"
#include "Volt/Components/CoreComponents.h"

#include "Volt/Utility/ShadowMappingUtility.h"
#include "Volt/Utility/Noise.h"
#include "Volt/Math/Math.h"

#include <VoltRHI/Images/Image2D.h>
#include <VoltRHI/Images/SamplerState.h>
#include <VoltRHI/Shader/Shader.h>
#include <VoltRHI/Pipelines/RenderPipeline.h>
#include <VoltRHI/Pipelines/ComputePipeline.h>

#include <VoltRHI/Buffers/CommandBuffer.h>
#include <VoltRHI/Buffers/UniformBuffer.h>
#include <VoltRHI/Buffers/StorageBuffer.h>

#include <VoltRHI/Descriptors/DescriptorTable.h>

namespace Volt
{
	SceneRenderer::SceneRenderer(const SceneRendererSpecification& specification)
		: m_scene(specification.scene)
	{
		m_commandBuffer = RHI::CommandBuffer::Create(Renderer::GetFramesInFlight(), RHI::QueueType::Graphics);

		CreateMainRenderTarget(specification.initialResolution.x, specification.initialResolution.y);

		m_sceneEnvironment.radianceMap = Renderer::GetDefaultResources().blackCubeTexture;
		m_sceneEnvironment.irradianceMap = Renderer::GetDefaultResources().blackCubeTexture;

		m_skyboxMesh = ShapeLibrary::GetCube();
	}

	SceneRenderer::~SceneRenderer()
	{
		RenderGraphExecutionThread::WaitForFinishedExecution();
		m_commandBuffer = nullptr;
	}

	void SceneRenderer::OnRenderEditor(Ref<Camera> camera)
	{
		OnRender(camera);
	}

	void SceneRenderer::Resize(const uint32_t width, const uint32_t height)
	{
		m_resizeWidth = width;
		m_resizeHeight = height;

		m_shouldResize = true;
	}

	RefPtr<RHI::Image2D> SceneRenderer::GetFinalImage()
	{
		return m_outputImage;
	}

	RefPtr<RHI::Image2D> SceneRenderer::GetObjectIDImage()
	{
		return m_objectIDImage;
	}

	void SceneRenderer::OnRender(Ref<Camera> camera)
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

			CreateMainRenderTarget(m_width, m_height);
			m_shouldResize = false;
		}

		RenderGraphBlackboard rgBlackboard{};
		RenderGraph renderGraph{ m_commandBuffer };

		renderGraph.SetTotalAllocatedSizeCallback([&](const uint64_t totalSize)
		{
			m_frameTotalGPUAllocation = totalSize;
		});

		m_scene->GetRenderScene()->Update(renderGraph);

		SetupFrameData(rgBlackboard, camera);
		AddExternalResources(renderGraph, rgBlackboard);

		UploadUniformBuffers(renderGraph, rgBlackboard, camera);
		UploadLightBuffers(renderGraph, rgBlackboard);

		AddPreDepthPass(renderGraph, rgBlackboard);
		AddObjectIDPass(renderGraph, rgBlackboard);

		GTAOSettings tempSettings{};
		tempSettings.radius = 50.f;
		tempSettings.radiusMultiplier = 1.457f;
		tempSettings.falloffRange = 0.615f;
		tempSettings.finalValuePower = 2.2f;

		GTAOTechnique gtaoTechnique{ 0 /*m_frameIndex*/, tempSettings };
		rgBlackboard.Add<GTAOOutput>() = gtaoTechnique.Execute(renderGraph, rgBlackboard);

		DirectionalShadowTechnique dirShadowTechnique{ renderGraph, rgBlackboard };
		rgBlackboard.Add<DirectionalShadowData>() = dirShadowTechnique.Execute(camera, m_scene->GetRenderScene(), m_directionalLightData);

		AddVisibilityBufferPass(renderGraph, rgBlackboard);
		AddGenerateMaterialCountsPass(renderGraph, rgBlackboard);

		LightCullingTechnique lightCulling{ renderGraph, rgBlackboard };
		rgBlackboard.Add<LightCullingData>() = lightCulling.Execute();

		PrefixSumTechnique prefixSum{ renderGraph };
		prefixSum.Execute(rgBlackboard.Get<MaterialCountData>().materialCountBuffer, rgBlackboard.Get<MaterialCountData>().materialStartBuffer, m_scene->GetRenderScene()->GetIndividualMaterialCount());

		AddCollectMaterialPixelsPass(renderGraph, rgBlackboard);
		AddGenerateMaterialIndirectArgsPass(renderGraph, rgBlackboard);

		////For every material -> run compute shading shader using indirect args
		auto& gbufferData = rgBlackboard.Add<GBufferData>();

		gbufferData.albedo = renderGraph.CreateImage2D({ RHI::PixelFormat::R8G8B8A8_UNORM, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Albedo" });
		gbufferData.normals = renderGraph.CreateImage2D({ RHI::PixelFormat::A2B10G10R10_UNORM_PACK32, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Normals" });
		gbufferData.material = renderGraph.CreateImage2D({ RHI::PixelFormat::R8G8_UNORM, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Material" });
		gbufferData.emissive = renderGraph.CreateImage2D({ RHI::PixelFormat::B10G11R11_UFLOAT_PACK32, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Emissive" });

		AddClearGBufferPass(renderGraph, rgBlackboard);

		renderGraph.BeginMarker("Materials");

		for (uint32_t matId = 0; matId < m_scene->GetRenderScene()->GetIndividualMaterialCount(); matId++)
		{
			AddGenerateGBufferPass(renderGraph, rgBlackboard, matId);
		}

		renderGraph.EndMarker();

		AddSkyboxPass(renderGraph, rgBlackboard);
		AddShadingPass(renderGraph, rgBlackboard);

		AddFinalCopyPass(renderGraph, rgBlackboard, rgBlackboard.Get<ShadingOutputData>().colorOutput);

		{
			RenderGraphBarrierInfo barrier{};
			barrier.dstStage = RHI::BarrierStage::PixelShader;
			barrier.dstAccess = RHI::BarrierAccess::ShaderRead;
			barrier.dstLayout = RHI::ImageLayout::ShaderRead;

			renderGraph.AddResourceBarrier(renderGraph.AddExternalImage2D(m_outputImage), barrier);
		}

		renderGraph.Compile();
		renderGraph.Execute();
	}

	void SceneRenderer::Invalidate()
	{
		VT_PROFILE_FUNCTION();

		auto renderScene = m_scene->GetRenderScene();
		renderScene->PrepareForUpdate();
		renderScene->SetValid();
	}

	const uint64_t SceneRenderer::GetFrameTotalGPUAllocationSize() const
	{
		return m_frameTotalGPUAllocation.load();
	}

	void SceneRenderer::BuildMeshPass(RenderGraph::Builder& builder, RenderGraphBlackboard& blackboard)
	{
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();
		GPUSceneData::SetupInputs(builder, blackboard.Get<GPUSceneData>());
		builder.ReadResource(uniformBuffers.viewDataBuffer);
	}

	void SceneRenderer::SetupMeshPassConstants(RenderContext& context, const RenderGraphPassResources& resources, const RenderGraphBlackboard& blackboard)
	{
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();

		GPUSceneData::SetupConstants(context, resources, blackboard.Get<GPUSceneData>());
		context.SetConstant("viewData"_sh, resources.GetUniformBuffer(uniformBuffers.viewDataBuffer));
	}

	void SceneRenderer::SetupFrameData(RenderGraphBlackboard& blackboard, Ref<Camera> camera)
	{
		// Render data
		{
			auto& data = blackboard.Add<RenderData>();
			data.camera = camera;
			data.renderSize = { m_width, m_height };
		}

		blackboard.Add<PreviousFrameData>() = m_previousFrameData;
	}

	void SceneRenderer::UploadUniformBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, Ref<Camera> camera)
	{
		auto& buffersData = blackboard.Add<UniformBuffersData>();

		// View data
		{
			const auto desc = RGUtils::CreateBufferDesc<ViewData>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "View Data");
			buffersData.viewDataBuffer = renderGraph.CreateUniformBuffer(desc);

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

			const auto& projection = camera->GetProjection();
			const auto projTranspose = glm::transpose(projection);

			const glm::vec4 frustumX = Math::NormalizePlane(projTranspose[3] + projTranspose[0]);
			const glm::vec4 frustumY = Math::NormalizePlane(projTranspose[3] + projTranspose[1]);

			viewData.cullingFrustum.x = frustumX.x;
			viewData.cullingFrustum.y = frustumX.z;
			viewData.cullingFrustum.z = frustumY.y;
			viewData.cullingFrustum.w = frustumY.z;

			// Light Culling
			viewData.tileCountX = Math::DivideRoundUp(m_width, LightCullingTechnique::TILE_SIZE);

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

		// Directional light
		{
			const auto desc = RGUtils::CreateBufferDesc<DirectionalLightData>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Directional Light Data");
			buffersData.directionalLightBuffer = renderGraph.CreateBuffer(desc);

			m_directionalLightData.intensity = 0.f;

			m_scene->ForEachWithComponents<const DirectionalLightComponent, const IDComponent, const TransformComponent>([&](entt::entity id, const DirectionalLightComponent& dirLightComp, const IDComponent& idComp, const TransformComponent& comp)
			{
				if (!comp.visible)
				{
					return;
				}

				auto entity = m_scene->GetEntityFromUUID(idComp.id);
				const glm::vec3 dir = glm::rotate(entity.GetRotation(), { 0.f, 0.f, 1.f }) * -1.f;

				m_directionalLightData.color = dirLightComp.color;
				m_directionalLightData.intensity = dirLightComp.intensity;
				m_directionalLightData.direction = { dir, 0.f };
				m_directionalLightData.castShadows = static_cast<uint32_t>(dirLightComp.castShadows);

				if (dirLightComp.castShadows)
				{
					const std::vector<float> cascades = { camera->GetFarPlane() / 50.f, camera->GetFarPlane() / 25.f, camera->GetFarPlane() / 10 };
					const auto lightMatrices = Utility::CalculateCascadeMatrices(camera, dir, cascades);

					for (size_t i = 0; i < lightMatrices.size(); i++)
					{
						m_directionalLightData.viewProjections[i] = lightMatrices.at(i);
					}

					for (size_t i = 0; i < cascades.size(); i++)
					{

						if (i < cascades.size())
						{
							m_directionalLightData.cascadeDistances[i] = cascades.at(i);
						}
						else
						{
							m_directionalLightData.cascadeDistances[i] = camera->GetFarPlane();
						}
					}
				}
			});

			renderGraph.AddMappedBufferUpload(buffersData.directionalLightBuffer, &m_directionalLightData, sizeof(DirectionalLightData), "Upload directional light data");
		}
	}

	void SceneRenderer::UploadLightBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
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

	void SceneRenderer::AddExternalResources(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		// Core images
		{
			auto& imageData = blackboard.Add<ExternalImagesData>();
			imageData.black1x1Cube = renderGraph.AddExternalImage2D(Renderer::GetDefaultResources().blackCubeTexture);
			imageData.BRDFLuT = renderGraph.AddExternalImage2D(Renderer::GetDefaultResources().BRDFLuT);
		}

		// GPU Scene
		{
			const auto gpuScene = m_scene->GetRenderScene()->GetGPUSceneBuffers();

			auto& bufferData = blackboard.Add<GPUSceneData>();
			bufferData.meshesBuffer = renderGraph.AddExternalBuffer(gpuScene.meshesBuffer);
			bufferData.materialsBuffer = renderGraph.AddExternalBuffer(gpuScene.materialsBuffer);
			bufferData.objectDrawDataBuffer = renderGraph.AddExternalBuffer(gpuScene.objectDrawDataBuffer);
			bufferData.bonesBuffer = renderGraph.AddExternalBuffer(gpuScene.bonesBuffer);
		}

		// Environment map
		{
			m_scene->ForEachWithComponents<SkylightComponent>([&](entt::entity id, SkylightComponent& skylightComp)
			{
				if (skylightComp.environmentHandle != skylightComp.lastEnvironmentHandle)
				{
					skylightComp.currentSceneEnvironment = Renderer::GenerateEnvironmentTextures(skylightComp.environmentHandle);
					skylightComp.lastEnvironmentHandle = skylightComp.environmentHandle;
				}

				m_sceneEnvironment = skylightComp.currentSceneEnvironment;
			});

			const auto& imageData = blackboard.Get<ExternalImagesData>();

			auto& environmentTexturesData = blackboard.Add<EnvironmentTexturesData>();
			environmentTexturesData.irradiance = m_sceneEnvironment.irradianceMap ? renderGraph.AddExternalImage2D(m_sceneEnvironment.irradianceMap) : imageData.black1x1Cube;
			environmentTexturesData.radiance = m_sceneEnvironment.radianceMap ? renderGraph.AddExternalImage2D(m_sceneEnvironment.radianceMap) : imageData.black1x1Cube;
		}
	}

	void SceneRenderer::AddPreDepthPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& gpuMeshes = m_scene->GetRenderScene()->GetGPUMeshes();
		const auto& objectDrawData = m_scene->GetRenderScene()->GetObjectDrawData();

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
			desc.name = "View Normals";
			data.normals = builder.CreateImage2D(desc);

			BuildMeshPass(builder, blackboard);
		},
		[=](const PreDepthData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { data.normals, data.depth });

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("PreDepthMeshShader");

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);
			context.BindPipeline(pipeline);

			SetupMeshPassConstants(context, resources, blackboard);

			for (uint32_t i = 0; const auto& objData : objectDrawData)
			{
				context.PushConstants(&i, sizeof(uint32_t));
				context.DispatchMeshTasks(Math::DivideRoundUp(gpuMeshes[objData.meshId].meshletCount, 32u), 1, 1);
				i++;
			}

			context.EndRendering();
		});
	}

	void SceneRenderer::AddObjectIDPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		struct Data
		{
			RenderGraphResourceHandle objectIdHandle;
		};

		const auto preDepthHandle = blackboard.Get<PreDepthData>().depth;
		const auto& gpuMeshes = m_scene->GetRenderScene()->GetGPUMeshes();
		const auto& objectDrawData = m_scene->GetRenderScene()->GetObjectDrawData();

		Data& data = renderGraph.AddPass<Data>("Object ID Pass",
		[&](RenderGraph::Builder& builder, Data& data)
		{
			data.objectIdHandle = builder.CreateImage2D({ RHI::PixelFormat::R32_UINT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "ObjectID" });
			builder.WriteResource(preDepthHandle);

			BuildMeshPass(builder, blackboard);
			builder.SetHasSideEffect();
		},
		[=](const Data& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { data.objectIdHandle, preDepthHandle });
			info.renderingInfo.depthAttachmentInfo.clearMode = RHI::ClearMode::Load;

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("ObjectIDMeshShader");
			pipelineInfo.depthCompareOperator = RHI::CompareOperator::Equal;
			pipelineInfo.depthMode = RHI::DepthMode::Read;

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);
			context.BindPipeline(pipeline);

			SetupMeshPassConstants(context, resources, blackboard);

			for (uint32_t i = 0; const auto & objData : objectDrawData)
			{
				context.PushConstants(&i, sizeof(uint32_t));
				context.DispatchMeshTasks(Math::DivideRoundUp(gpuMeshes[objData.meshId].meshletCount, 32u), 1, 1);
				i++;
			}
			
			context.EndRendering();
		});

		renderGraph.QueueImage2DExtraction(data.objectIdHandle, m_objectIDImage);
	}

	void SceneRenderer::AddVisibilityBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto preDepthHandle = blackboard.Get<PreDepthData>().depth;
		const auto& gpuMeshes = m_scene->GetRenderScene()->GetGPUMeshes();
		const auto& objectDrawData = m_scene->GetRenderScene()->GetObjectDrawData();

		blackboard.Add<VisibilityBufferData>() = renderGraph.AddPass<VisibilityBufferData>("Visibility Buffer",
		[&](RenderGraph::Builder& builder, VisibilityBufferData& data)
		{
			data.visibility = builder.CreateImage2D({ RHI::PixelFormat::R32G32_UINT, m_width, m_height, RHI::ImageUsage::AttachmentStorage, "VisibilityBuffer" });
			builder.WriteResource(preDepthHandle);

			BuildMeshPass(builder, blackboard);
			builder.SetHasSideEffect();
		},
		[=](const VisibilityBufferData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { data.visibility, preDepthHandle });
			info.renderingInfo.depthAttachmentInfo.clearMode = RHI::ClearMode::Load;
			info.renderingInfo.colorAttachments.At(0).SetClearColor(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max());

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("VisibilityBufferMeshShader");
			pipelineInfo.depthCompareOperator = RHI::CompareOperator::Equal;
			pipelineInfo.depthMode = RHI::DepthMode::Read;

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);
			context.BindPipeline(pipeline);

			SetupMeshPassConstants(context, resources, blackboard);

			for (uint32_t i = 0; const auto & objData : objectDrawData)
			{
				context.PushConstants(&i, sizeof(uint32_t));
				context.DispatchMeshTasks(Math::DivideRoundUp(gpuMeshes[objData.meshId].meshletCount, 32u), 1, 1);
				i++;
			}

			context.EndRendering();
		});
	}

	void SceneRenderer::AddClearGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& gbufferData = blackboard.Get<GBufferData>();
		
		renderGraph.AddPass("Clear GBuffer",
		[&](RenderGraph::Builder& builder) 
		{
			builder.WriteResource(gbufferData.albedo, RenderGraphResourceState::Clear);
			builder.WriteResource(gbufferData.normals, RenderGraphResourceState::Clear);
			builder.WriteResource(gbufferData.material, RenderGraphResourceState::Clear);
			builder.WriteResource(gbufferData.emissive, RenderGraphResourceState::Clear);
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources) 
		{
			context.ClearImage(gbufferData.albedo, { 0.1f, 0.1f, 0.1f, 0.f });
			context.ClearImage(gbufferData.normals, { 0.f, 0.f, 0.f, 0.f });
			context.ClearImage(gbufferData.material, { 0.f, 0.f, 0.f, 0.f });
			context.ClearImage(gbufferData.emissive, { 0.f, 0.f, 0.f, 0.f });
		});
	}

	void SceneRenderer::AddGenerateMaterialCountsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const VisibilityBufferData& visBufferData = blackboard.Get<VisibilityBufferData>();
		const GPUSceneData& gpuSceneData = blackboard.Get<GPUSceneData>();

		const auto& renderScene = m_scene->GetRenderScene();

		RenderGraphResourceHandle materialCountBuffer = 0;
		{
			const auto desc = RGUtils::CreateBufferDesc<uint32_t>(std::max(renderScene->GetIndividualMaterialCount(), 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Material Counts");
			materialCountBuffer = renderGraph.CreateBuffer(desc);
			renderGraph.AddClearBufferPass(materialCountBuffer, 0, "Clear Material Count");
		}

		blackboard.Add<MaterialCountData>() = renderGraph.AddPass<MaterialCountData>("Generate Material Count",
		[&](RenderGraph::Builder& builder, MaterialCountData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDesc<uint32_t>(std::max(renderScene->GetIndividualMaterialCount(), 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Material Starts");
				data.materialStartBuffer = builder.CreateBuffer(desc);
			}

			data.materialCountBuffer = materialCountBuffer;
			builder.WriteResource(data.materialCountBuffer);
			builder.ReadResource(visBufferData.visibility);

			GPUSceneData::SetupInputs(builder, gpuSceneData);

			builder.SetIsComputePass();
		},
		[=](const MaterialCountData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("GenerateMaterialCount");

			context.BindPipeline(pipeline);
			
			GPUSceneData::SetupConstants(context, resources, gpuSceneData);
			
			context.SetConstant("visibilityBuffer"_sh, resources.GetImage2D(visBufferData.visibility));
			context.SetConstant("materialCountsBuffer"_sh, resources.GetBuffer(data.materialCountBuffer));
			context.SetConstant("renderSize"_sh, glm::uvec2{ m_width, m_height });

			context.Dispatch(Math::DivideRoundUp(m_width, 8u), Math::DivideRoundUp(m_height, 8u), 1);
		});
	}

	void SceneRenderer::AddCollectMaterialPixelsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const VisibilityBufferData& visBufferData = blackboard.Get<VisibilityBufferData>();
		const MaterialCountData& matCountData = blackboard.Get<MaterialCountData>();
		const GPUSceneData& gpuSceneData = blackboard.Get<GPUSceneData>();

		RenderGraphResourceHandle currentMaterialCountBuffer = 0;

		{
			const auto desc = RGUtils::CreateBufferDescGPU<uint32_t>(std::max(m_scene->GetRenderScene()->GetIndividualMaterialCount(), 1u), "Current Material Count");
			currentMaterialCountBuffer = renderGraph.CreateBuffer(desc);
			renderGraph.AddClearBufferPass(currentMaterialCountBuffer, 0, "Clear Current Material Count");
		}

		blackboard.Add<MaterialPixelsData>() = renderGraph.AddPass<MaterialPixelsData>("Collect Material Pixels",
		[&](RenderGraph::Builder& builder, MaterialPixelsData& data)
		{
			{
				const auto desc = RGUtils::CreateBufferDescGPU<glm::uvec2>(m_width * m_height, "Pixel Collection");
				data.pixelCollectionBuffer = builder.CreateBuffer(desc);
			}

			data.currentMaterialCountBuffer = currentMaterialCountBuffer;

			builder.WriteResource(data.currentMaterialCountBuffer);

			GPUSceneData::SetupInputs(builder, gpuSceneData);

			builder.ReadResource(visBufferData.visibility);
			builder.ReadResource(matCountData.materialStartBuffer);

			builder.SetIsComputePass();
		},
		[=](const MaterialPixelsData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("CollectMaterialPixels");

			context.BindPipeline(pipeline);

			GPUSceneData::SetupConstants(context, resources, gpuSceneData);

			context.SetConstant("visibilityBuffer"_sh, resources.GetImage2D(visBufferData.visibility));
			context.SetConstant("materialStartBuffer"_sh, resources.GetBuffer(matCountData.materialStartBuffer));
			context.SetConstant("currentMaterialCountBuffer"_sh, resources.GetBuffer(data.currentMaterialCountBuffer));
			context.SetConstant("pixelCollectionBuffer"_sh, resources.GetBuffer(data.pixelCollectionBuffer));
			context.SetConstant("renderSize"_sh, glm::uvec2{ m_width, m_height });

			context.Dispatch(Math::DivideRoundUp(m_width, 8u), Math::DivideRoundUp(m_height, 8u), 1);
		});
	}

	void SceneRenderer::AddGenerateMaterialIndirectArgsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		MaterialCountData matCountData = blackboard.Get<MaterialCountData>();

		RenderGraphResourceHandle materialIndirectArgsBuffer = 0;
		{
			const auto desc = RGUtils::CreateBufferDesc<RHI::IndirectDispatchCommand>(std::max(m_scene->GetRenderScene()->GetIndividualMaterialCount(), 1u), RHI::BufferUsage::IndirectBuffer | RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Material Indirect Args");
			materialIndirectArgsBuffer = renderGraph.CreateBuffer(desc);
			renderGraph.AddClearBufferPass(materialIndirectArgsBuffer, 0, "Clear Material Indirect Args Buffer");
		}

		blackboard.Add<MaterialIndirectArgsData>() = renderGraph.AddPass<MaterialIndirectArgsData>("Generate Material Indirect Args",
		[&](RenderGraph::Builder& builder, MaterialIndirectArgsData& data)
		{
			data.materialIndirectArgsBuffer = materialIndirectArgsBuffer;

			builder.WriteResource(data.materialIndirectArgsBuffer);
			builder.ReadResource(matCountData.materialCountBuffer);
			builder.SetIsComputePass();
		},
		[=](const MaterialIndirectArgsData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			const uint32_t materialCount = m_scene->GetRenderScene()->GetIndividualMaterialCount();

			auto pipeline = ShaderMap::GetComputePipeline("GenerateMaterialIndirectArgs");

			context.BindPipeline(pipeline);
			context.SetConstant("materialCounts"_sh, resources.GetBuffer(matCountData.materialCountBuffer));
			context.SetConstant("indirectArgsBuffer"_sh, resources.GetBuffer(data.materialIndirectArgsBuffer));
			context.SetConstant("materialCount"_sh, materialCount);

			context.Dispatch(Math::DivideRoundUp(materialCount, 32u), 1, 1);
		});
	}

	void SceneRenderer::AddGenerateGBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, const uint32_t materialId)
	{
		const auto& indirectArgsData = blackboard.Get<MaterialIndirectArgsData>();
		const auto& visBufferData = blackboard.Get<VisibilityBufferData>();
		const auto& matCountData = blackboard.Get<MaterialCountData>();
		const auto& matPixelsData = blackboard.Get<MaterialPixelsData>();
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();
		const auto& gpuSceneData = blackboard.Get<GPUSceneData>();

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
			builder.ReadResource(uniformBuffers.viewDataBuffer);

			GPUSceneData::SetupInputs(builder, gpuSceneData);

			builder.WriteResource(gbufferData.albedo);
			builder.WriteResource(gbufferData.normals);
			builder.WriteResource(gbufferData.material);
			builder.WriteResource(gbufferData.emissive);

			builder.SetIsComputePass();
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto material = renderScene->GetMaterialFromID(materialId);
			auto pipeline = material->GetComputePipeline();

			if (!pipeline)
			{
				pipeline = ShaderMap::GetComputePipeline("OpaqueDefault");
			}

			context.BindPipeline(pipeline);

			GPUSceneData::SetupConstants(context, resources, gpuSceneData);

			context.SetConstant("visibilityBuffer"_sh, resources.GetImage2D(visBufferData.visibility));
			context.SetConstant("materialCountBuffer"_sh, resources.GetBuffer(matCountData.materialCountBuffer));
			context.SetConstant("materialStartBuffer"_sh, resources.GetBuffer(matCountData.materialStartBuffer));
			context.SetConstant("pixelCollection"_sh, resources.GetBuffer(matPixelsData.pixelCollectionBuffer));

			context.SetConstant("viewData"_sh, resources.GetUniformBuffer(uniformBuffers.viewDataBuffer));

			context.SetConstant("albedo"_sh, resources.GetImage2D(gbufferData.albedo));
			context.SetConstant("normals"_sh, resources.GetImage2D(gbufferData.normals));
			context.SetConstant("material"_sh, resources.GetImage2D(gbufferData.material));
			context.SetConstant("emissive"_sh, resources.GetImage2D(gbufferData.emissive));
			context.SetConstant("materialId"_sh, materialId);

			context.SetConstant("viewSize"_sh, glm::vec2(m_width, m_height));

			context.DispatchIndirect(indirectArgsData.materialIndirectArgsBuffer, sizeof(RHI::IndirectDispatchCommand) * materialId); // Should be offset with material ID
		});
	}

	void SceneRenderer::AddSkyboxPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& environmentTexturesData = blackboard.Get<EnvironmentTexturesData>();
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();

		RenderGraphResourceHandle meshVertexBufferHandle = renderGraph.AddExternalBuffer(m_skyboxMesh->GetVertexPositionsBuffer()->GetResource());
		RenderGraphResourceHandle indexBufferHandle = renderGraph.AddExternalBuffer(m_skyboxMesh->GetIndexBuffer()->GetResource());

		blackboard.Add<ShadingOutputData>() = renderGraph.AddPass<ShadingOutputData>("Skybox Pass",
		[&](RenderGraph::Builder& builder, ShadingOutputData& data)
		{
			{
				const auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::B10G11R11_UFLOAT_PACK32>(m_width, m_height, RHI::ImageUsage::AttachmentStorage, "Shading Output");
				data.colorOutput = builder.CreateImage2D(desc);
			}

			builder.ReadResource(environmentTexturesData.radiance);
			builder.ReadResource(meshVertexBufferHandle, RenderGraphResourceState::VertexBuffer);
			builder.ReadResource(indexBufferHandle, RenderGraphResourceState::IndexBuffer);
			builder.ReadResource(uniformBuffers.viewDataBuffer);
		},
		[=](const ShadingOutputData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { data.colorOutput });

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("Skybox");
			pipelineInfo.cullMode = RHI::CullMode::None;
			pipelineInfo.depthMode = RHI::DepthMode::None;

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);
			context.BindPipeline(pipeline);

			context.SetConstant("vertexPositions"_sh, resources.GetBuffer(meshVertexBufferHandle));
			context.SetConstant("viewData"_sh, resources.GetUniformBuffer(uniformBuffers.viewDataBuffer));
			context.SetConstant("environmentTexture"_sh, resources.GetImage2D(environmentTexturesData.radiance));
			context.SetConstant("linearSampler"_sh, Renderer::GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear>()->GetResourceHandle());
			context.SetConstant("lod"_sh, 0.f);
			context.SetConstant("intensity"_sh, 1.f);

			context.BindIndexBuffer(indexBufferHandle);
			context.DrawIndexed(static_cast<uint32_t>(m_skyboxMesh->GetIndexCount()), 1, 0, 0, 0);
			context.EndRendering();
		});
	}

	void SceneRenderer::AddShadingPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();
		const auto& externalImages = blackboard.Get<ExternalImagesData>();
		const auto& environmentTexturesData = blackboard.Get<EnvironmentTexturesData>();
		const auto& shadingOutputData = blackboard.Get<ShadingOutputData>();
		const auto& lightBuffers = blackboard.Get<LightBuffersData>();
		const auto& lightCullingData = blackboard.Get<LightCullingData>();
		const auto& gtaoOutput = blackboard.Get<GTAOOutput>();

		const auto& gbufferData = blackboard.Get<GBufferData>();
		const auto& preDepthData = blackboard.Get<PreDepthData>();
		const auto& dirShadowData = blackboard.Get<DirectionalShadowData>();

		renderGraph.AddPass("Shading Pass",
		[&](RenderGraph::Builder& builder)
		{
			builder.WriteResource(shadingOutputData.colorOutput);
			builder.ReadResource(gbufferData.albedo);
			builder.ReadResource(gbufferData.normals);
			builder.ReadResource(gbufferData.material);
			builder.ReadResource(gbufferData.emissive);
			builder.ReadResource(preDepthData.depth);

			// PBR Constants
			builder.ReadResource(uniformBuffers.viewDataBuffer);
			builder.ReadResource(uniformBuffers.directionalLightBuffer);
			builder.ReadResource(externalImages.BRDFLuT);
			builder.ReadResource(environmentTexturesData.irradiance);
			builder.ReadResource(environmentTexturesData.radiance);
			builder.ReadResource(lightBuffers.pointLightsBuffer);
			builder.ReadResource(lightBuffers.spotLightsBuffer);
			builder.ReadResource(lightCullingData.visiblePointLightsBuffer);
			builder.ReadResource(lightCullingData.visibleSpotLightsBuffer);
			builder.ReadResource(dirShadowData.shadowTexture);
			builder.ReadResource(gtaoOutput.outputImage);

			builder.SetIsComputePass();
			builder.SetHasSideEffect(); 
		},
		[=](RenderContext& context, const RenderGraphPassResources& resources)
		{
			auto pipeline = ShaderMap::GetComputePipeline("Shading");
			context.BindPipeline(pipeline);
			context.SetConstant("output"_sh, resources.GetImage2D(shadingOutputData.colorOutput));
			context.SetConstant("albedo"_sh, resources.GetImage2D(gbufferData.albedo));
			context.SetConstant("normals"_sh, resources.GetImage2D(gbufferData.normals));
			context.SetConstant("material"_sh, resources.GetImage2D(gbufferData.material));
			context.SetConstant("emissive"_sh, resources.GetImage2D(gbufferData.emissive));
			context.SetConstant("aoTexture"_sh, resources.GetImage2D(gtaoOutput.outputImage));
			context.SetConstant("depthTexture"_sh, resources.GetImage2D(preDepthData.depth));
			context.SetConstant("shadingMode"_sh, static_cast<uint32_t>(m_shadingMode));

			// PBR Constants
			context.SetConstant("pbrConstants.viewData"_sh, resources.GetUniformBuffer(uniformBuffers.viewDataBuffer));
			context.SetConstant("pbrConstants.directionalLight"_sh, resources.GetBuffer(uniformBuffers.directionalLightBuffer));
			context.SetConstant("pbrConstants.linearSampler"_sh, Renderer::GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear>()->GetResourceHandle());
			context.SetConstant("pbrConstants.pointLinearClampSampler"_sh, Renderer::GetSampler<RHI::TextureFilter::Nearest, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureWrap::Clamp>()->GetResourceHandle());
			context.SetConstant("pbrConstants.shadowSampler"_sh, Renderer::GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureWrap::Repeat, RHI::AnisotropyLevel::None, RHI::CompareOperator::LessEqual>()->GetResourceHandle());
			context.SetConstant("pbrConstants.BRDFLuT"_sh, resources.GetImage2D(externalImages.BRDFLuT));
			context.SetConstant("pbrConstants.environmentIrradiance"_sh, resources.GetImage2D(environmentTexturesData.irradiance));
			context.SetConstant("pbrConstants.environmentRadiance"_sh, resources.GetImage2D(environmentTexturesData.radiance));
			context.SetConstant("pbrConstants.pointLights"_sh, resources.GetBuffer(lightBuffers.pointLightsBuffer));
			context.SetConstant("pbrConstants.spotLights"_sh, resources.GetBuffer(lightBuffers.spotLightsBuffer));
			context.SetConstant("pbrConstants.visiblePointLights"_sh, resources.GetBuffer(lightCullingData.visiblePointLightsBuffer));
			context.SetConstant("pbrConstants.visibleSpotLights"_sh, resources.GetBuffer(lightCullingData.visibleSpotLightsBuffer));
			context.SetConstant("pbrConstants.directionalShadowMap"_sh, resources.GetImage2D(dirShadowData.shadowTexture));

			context.Dispatch(Math::DivideRoundUp(m_width, 8u), Math::DivideRoundUp(m_height, 8u), 1u);
		});
	}

	void SceneRenderer::AddFinalCopyPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, RenderGraphResourceHandle srcImage)
	{
		blackboard.Add<FinalCopyData>() = renderGraph.AddPass<FinalCopyData>("Final Copy",
		[&](RenderGraph::Builder& builder, FinalCopyData& data)
		{
			data.output = builder.AddExternalImage2D(GetFinalImage());
			builder.WriteResource(data.output);
			builder.ReadResource(srcImage);
			builder.SetHasSideEffect();
		},
		[=](const FinalCopyData& data, RenderContext& context, const RenderGraphPassResources& resources)
		{
			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { data.output });

			RHI::RenderPipelineCreateInfo pipelineInfo;
			pipelineInfo.shader = ShaderMap::Get("FinalCopy");
			pipelineInfo.depthMode = RHI::DepthMode::None;
			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);

			RCUtils::DrawFullscreenTriangle(context, pipeline, [&](RenderContext& context)
			{
				context.SetConstant("finalColor"_sh, resources.GetImage2D(srcImage));
			});

			context.EndRendering();
		});
	}

	void SceneRenderer::CreateMainRenderTarget(const uint32_t width, const uint32_t height)
	{
		RHI::ImageSpecification spec{};
		spec.width = width;
		spec.height = height;
		spec.usage = RHI::ImageUsage::AttachmentStorage;
		spec.generateMips = false;
		spec.format = RHI::PixelFormat::B10G11R11_UFLOAT_PACK32;
		spec.debugName = "Final Image";

		m_outputImage = RHI::Image2D::Create(spec);
	}
}
