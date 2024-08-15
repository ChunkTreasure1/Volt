#include "vtpch.h"
#include "SceneRenderer.h"

#include "Volt/Core/Application.h"

#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/Rendering/RenderScene.h"
#include "Volt/Rendering/RendererCommon.h"
#include "Volt/Rendering/Renderer.h"

#include "Volt/Rendering/RenderingTechniques/PrefixSumTechnique.h"
#include "Volt/Rendering/RenderingTechniques/GTAOTechnique.h"
#include "Volt/Rendering/RenderingTechniques/DirectionalShadowTechnique.h"
#include "Volt/Rendering/RenderingTechniques/LightCullingTechnique.h"
#include "Volt/Rendering/RenderingTechniques/TAATechnique.h"
#include "Volt/Rendering/RenderingTechniques/VelocityTechnique.h"
#include "Volt/Rendering/RenderingTechniques/CullingTechnique.h"

#include "Volt/Rendering/RenderingUtils.h"
#include "Volt/Rendering/ShapeLibrary.h"
#include "Volt/Rendering/DrawContext.h"

#include "Volt/Project/ProjectManager.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Scene/Entity.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Rendering/Material.h"

#include "Volt/Math/Math.h"

#include "Volt/Components/LightComponents.h"
#include "Volt/Components/RenderingComponents.h"
#include "Volt/Components/CoreComponents.h"

#include "Volt/Utility/ShadowMappingUtility.h"
#include "Volt/Utility/Noise.h"
#include "Volt/Math/Math.h"

#include <AssetSystem/AssetManager.h>

#include <RenderCore/RenderGraph/RenderGraph.h>
#include <RenderCore/RenderGraph/RenderGraphUtils.h>
#include <RenderCore/RenderGraph/RenderGraphBlackboard.h>
#include <RenderCore/RenderGraph/RenderGraphExecutionThread.h>
#include <RenderCore/RenderGraph/Resources/RenderGraphBufferResource.h>
#include <RenderCore/RenderGraph/Resources/RenderGraphTextureResource.h>
#include <RenderCore/RenderGraph/RenderContextUtils.h>
#include <RenderCore/RenderGraph/GPUReadbackBuffer.h>
#include <RenderCore/Shader/ShaderMap.h>

#include <RHIModule/Images/Image.h>
#include <RHIModule/Images/SamplerState.h>
#include <RHIModule/Shader/Shader.h>
#include <RHIModule/Pipelines/RenderPipeline.h>
#include <RHIModule/Pipelines/ComputePipeline.h>

#include <RHIModule/Buffers/CommandBuffer.h>
#include <RHIModule/Buffers/UniformBuffer.h>
#include <RHIModule/Buffers/StorageBuffer.h>

#include <RHIModule/Descriptors/DescriptorTable.h>

namespace Volt
{
	SceneRenderer::SceneRenderer(const SceneRendererSpecification& specification)
		: m_scene(specification.scene), m_commandBufferSet(Renderer::GetFramesInFlight())
	{
		CreateMainRenderTarget(specification.initialResolution.x, specification.initialResolution.y);

		m_sceneEnvironment.radianceMap = Renderer::GetDefaultResources().blackCubeTexture;
		m_sceneEnvironment.irradianceMap = Renderer::GetDefaultResources().blackCubeTexture;

		m_skyboxMesh = ShapeLibrary::GetCube();
	}

	SceneRenderer::~SceneRenderer()
	{
		RenderGraphExecutionThread::WaitForFinishedExecution();
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

	RefPtr<RHI::Image> SceneRenderer::GetFinalImage()
	{
		return m_outputImage;
	}

	RefPtr<RHI::Image> SceneRenderer::GetObjectIDImage()
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
		RenderGraph renderGraph{ m_commandBufferSet.IncrementAndGetCommandBuffer() };

		renderGraph.SetTotalAllocatedSizeCallback([&](const uint64_t totalSize)
		{
			m_frameTotalGPUAllocation = totalSize;
		});

		m_scene->GetRenderScene()->Update(renderGraph);

		SetupFrameData(rgBlackboard, camera);
		AddExternalResources(renderGraph, rgBlackboard);

		UploadUniformBuffers(renderGraph, rgBlackboard, camera);
		UploadLightBuffers(renderGraph, rgBlackboard);

		const auto renderScene = m_scene->GetRenderScene();
		const uint32_t drawCount = renderScene->GetDrawCount();

		if (drawCount > 0)
		{
			AddMainCullingPass(renderGraph, rgBlackboard);
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
			rgBlackboard.Add<DirectionalShadowData>() = dirShadowTechnique.Execute(camera, m_scene->GetRenderScene());

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

			gbufferData.albedo = renderGraph.CreateImage(RGUtils::CreateImage2DDesc<RHI::PixelFormat::R8G8B8A8_UNORM>(m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Albedo"));
			gbufferData.normals = renderGraph.CreateImage(RGUtils::CreateImage2DDesc<RHI::PixelFormat::A2B10G10R10_UNORM_PACK32>(m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Normals"));
			gbufferData.material = renderGraph.CreateImage(RGUtils::CreateImage2DDesc<RHI::PixelFormat::R8G8_UNORM>(m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Material"));
			gbufferData.emissive = renderGraph.CreateImage(RGUtils::CreateImage2DDesc<RHI::PixelFormat::B10G11R11_UFLOAT_PACK32>(m_width, m_height, RHI::ImageUsage::AttachmentStorage, "GBuffer - Emissive"));

			AddClearGBufferPass(renderGraph, rgBlackboard);
			RenderMaterials(renderGraph, rgBlackboard);

			AddSkyboxPass(renderGraph, rgBlackboard);
			AddShadingPass(renderGraph, rgBlackboard);

			if (m_visualizationMode == VisualizationMode::VisualizeMeshSDF)
			{
				AddVisualizeSDFPass(renderGraph, rgBlackboard, rgBlackboard.Get<ShadingOutputData>().colorOutput);
			}

			//AddVisualizeBricksPass(renderGraph, rgBlackboard, rgBlackboard.Get<ShadingOutputData>().colorOutput);

			AddFinalCopyPass(renderGraph, rgBlackboard, rgBlackboard.Get<ShadingOutputData>().colorOutput);
			AddFXAAPass(renderGraph, rgBlackboard, rgBlackboard.Get<FinalCopyData>().output);
		}
		else
		{
			RGUtils::ClearImage(renderGraph, renderGraph.AddExternalImage(m_outputImage), { 0.1f, 0.1f, 0.1f, 1.f });
		}

		{
			RenderGraphBarrierInfo barrier{};
			barrier.dstStage = RHI::BarrierStage::PixelShader;
			barrier.dstAccess = RHI::BarrierAccess::ShaderRead;
			barrier.dstLayout = RHI::ImageLayout::ShaderRead;

			renderGraph.AddResourceBarrier(renderGraph.AddExternalImage(m_outputImage), barrier);
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
		const auto& drawCullingData = blackboard.Get<DrawCullingData>();

		GPUSceneData::SetupInputs(builder, blackboard.Get<GPUSceneData>());

		builder.ReadResource(uniformBuffers.viewDataBuffer);
		builder.ReadResource(drawCullingData.countCommandBuffer, RenderGraphResourceState::IndirectArgument);
		builder.ReadResource(drawCullingData.taskCommandsBuffer);
	}

	void SceneRenderer::SetupMeshPassConstants(RenderContext& context, const RenderGraphBlackboard& blackboard)
	{
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();
		const auto& drawCullingData = blackboard.Get<DrawCullingData>();

		GPUSceneData::SetupConstants(context, blackboard.Get<GPUSceneData>());
		context.SetConstant("viewData"_sh, uniformBuffers.viewDataBuffer);
		context.SetConstant("taskCommands"_sh, drawCullingData.taskCommandsBuffer);
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
			viewData.cullingFrustum = camera->GetFrustumCullingInfo();

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

			blackboard.Add<ViewData>() = viewData;
		}

		// Directional light
		{
			const auto desc = RGUtils::CreateBufferDesc<DirectionalLightData>(1, RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::CPUToGPU, "Directional Light Data");
			buffersData.directionalLightBuffer = renderGraph.CreateUniformBuffer(desc);

			DirectionalLightInfo& lightInfo = blackboard.Add<DirectionalLightInfo>();
			DirectionalLightData& data = lightInfo.data;

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
				data.castShadows = static_cast<uint32_t>(dirLightComp.castShadows);

				if (dirLightComp.castShadows)
				{
					const Vector<float> cascades = { camera->GetFarPlane() / 50.f, camera->GetFarPlane() / 25.f, camera->GetFarPlane() / 10 };
					const auto lightMatrices = Utility::CalculateCascadeMatrices(camera, dir, cascades);

					for (size_t i = 0; i < lightMatrices.size(); i++)
					{
						const auto& matrices = lightMatrices.at(i);
						data.viewProjections[i] = matrices.projection * matrices.view;
						lightInfo.projectionBounds[i] = matrices.projectionBounds;
						lightInfo.views[i] = matrices.view;
					}

					for (size_t i = 0; i < cascades.size(); i++)
					{

						if (i < cascades.size())
						{
							data.cascadeDistances[i] = cascades.at(i);
						}
						else
						{
							data.cascadeDistances[i] = camera->GetFarPlane();
						}
					}
				}
			});

			renderGraph.AddMappedBufferUpload(buffersData.directionalLightBuffer, &data, sizeof(DirectionalLightData), "Upload directional light data");
		}
	}

	void SceneRenderer::UploadLightBuffers(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		auto& buffersData = blackboard.Add<LightBuffersData>();

		// Point lights
		{
			Vector<PointLightData> pointLights{};

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
			Vector<SpotLightData> spotLights{};

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
			imageData.black1x1Cube = renderGraph.AddExternalImage(Renderer::GetDefaultResources().blackCubeTexture);
			imageData.BRDFLuT = renderGraph.AddExternalImage(Renderer::GetDefaultResources().BRDFLuT);
		}

		// GPU Scene
		{
			const auto gpuScene = m_scene->GetRenderScene()->GetGPUSceneBuffers();

			auto& bufferData = blackboard.Add<GPUSceneData>();
			bufferData.meshesBuffer = renderGraph.AddExternalBuffer(gpuScene.meshesBuffer->GetResource());
			bufferData.sdfMeshesBuffer = renderGraph.AddExternalBuffer(gpuScene.sdfMeshesBuffer->GetResource());
			bufferData.materialsBuffer = renderGraph.AddExternalBuffer(gpuScene.materialsBuffer->GetResource());
			bufferData.primitiveDrawDataBuffer = renderGraph.AddExternalBuffer(gpuScene.primitiveDrawDataBuffer->GetResource());
			bufferData.sdfPrimitiveDrawDataBuffer = renderGraph.AddExternalBuffer(gpuScene.sdfPrimitiveDrawDataBuffer->GetResource());
			bufferData.bonesBuffer = renderGraph.AddExternalBuffer(gpuScene.bonesBuffer->GetResource());
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
			environmentTexturesData.irradiance = m_sceneEnvironment.irradianceMap ? renderGraph.AddExternalImage(m_sceneEnvironment.irradianceMap) : imageData.black1x1Cube;
			environmentTexturesData.radiance = m_sceneEnvironment.radianceMap ? renderGraph.AddExternalImage(m_sceneEnvironment.radianceMap) : imageData.black1x1Cube;
		}
	}

	void SceneRenderer::AddMainCullingPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& viewData = blackboard.Get<ViewData>();
		const auto renderScene = m_scene->GetRenderScene();

		CullingTechnique cullingTechnique{ renderGraph, blackboard };

		CullingTechnique::Info info{};
		info.viewMatrix = viewData.view;
		info.cullingFrustum = viewData.cullingFrustum;
		info.nearPlane = viewData.nearPlane;
		info.farPlane = viewData.farPlane;
		info.drawCommandCount = renderScene->GetDrawCount();
		info.meshletCount = renderScene->GetMeshletCount();

		blackboard.Add<DrawCullingData>() = cullingTechnique.Execute(info);
	}

	void SceneRenderer::AddPreDepthPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& drawCullingData = blackboard.Get<DrawCullingData>();

		blackboard.Add<PreDepthData>() = renderGraph.AddPass<PreDepthData>("Pre Depth Pass",
		[&](RenderGraph::Builder& builder, PreDepthData& data)
		{
			RenderGraphImageDesc desc{};
			desc.width = m_width;
			desc.height = m_height;
			desc.format = RHI::PixelFormat::D32_SFLOAT;
			desc.usage = RHI::ImageUsage::Attachment;
			desc.name = "PreDepth";
			data.depth = builder.CreateImage(desc);

			desc.format = RHI::PixelFormat::R16G16B16A16_SFLOAT;
			desc.name = "View Normals";
			data.normals = builder.CreateImage(desc);

			BuildMeshPass(builder, blackboard);

			builder.SetHasSideEffect();
		},
		[=](const PreDepthData& data, RenderContext& context)
		{
			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { data.normals, data.depth });

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("PreDepthMeshShader");

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);
			context.BindPipeline(pipeline);

			SetupMeshPassConstants(context, blackboard);

			context.DispatchMeshTasksIndirect(drawCullingData.countCommandBuffer, sizeof(uint32_t), 1, 0);
			context.EndRendering();
		});
	}

	void SceneRenderer::AddObjectIDPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		struct Data
		{
			RenderGraphImageHandle objectIdHandle;
		};

		const auto preDepthHandle = blackboard.Get<PreDepthData>().depth;
		const auto& drawCullingData = blackboard.Get<DrawCullingData>();

		Data& data = renderGraph.AddPass<Data>("Object ID Pass",
		[&](RenderGraph::Builder& builder, Data& data)
		{
			data.objectIdHandle = builder.CreateImage(RGUtils::CreateImage2DDesc<RHI::PixelFormat::R32_UINT>(m_width, m_height, RHI::ImageUsage::AttachmentStorage, "Entity ID"));
			builder.WriteResource(preDepthHandle);

			BuildMeshPass(builder, blackboard);
			builder.SetHasSideEffect();
		},
		[=](const Data& data, RenderContext& context)
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

			SetupMeshPassConstants(context, blackboard);

			context.DispatchMeshTasksIndirect(drawCullingData.countCommandBuffer, sizeof(uint32_t), 1, 0);
			context.EndRendering();
		});

		renderGraph.EnqueueImageExtraction(data.objectIdHandle, m_objectIDImage);
	}

	void SceneRenderer::AddVisibilityBufferPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto preDepthHandle = blackboard.Get<PreDepthData>().depth;
		const auto& drawCullingData = blackboard.Get<DrawCullingData>();

		blackboard.Add<VisibilityBufferData>() = renderGraph.AddPass<VisibilityBufferData>("Visibility Buffer",
		[&](RenderGraph::Builder& builder, VisibilityBufferData& data)
		{
			data.visibility = builder.CreateImage(RGUtils::CreateImage2DDesc<RHI::PixelFormat::R32G32_UINT>(m_width, m_height, RHI::ImageUsage::AttachmentStorage, "Visibility Buffer"));
			builder.WriteResource(preDepthHandle);

			BuildMeshPass(builder, blackboard);
			builder.SetHasSideEffect();
		},
		[=](const VisibilityBufferData& data, RenderContext& context)
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

			SetupMeshPassConstants(context, blackboard);

			context.DispatchMeshTasksIndirect(drawCullingData.countCommandBuffer, sizeof(uint32_t), 1, 0);
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
		[=](RenderContext& context)
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

		RenderGraphBufferHandle materialCountBuffer = RenderGraphNullHandle();
		{
			const auto desc = RGUtils::CreateBufferDesc<uint32_t>(std::max(renderScene->GetIndividualMaterialCount(), 1u), RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Material Counts");
			materialCountBuffer = renderGraph.CreateBuffer(desc);
			RGUtils::ClearBuffer(renderGraph, materialCountBuffer, 0, "Clear Material Count");
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
		[=](const MaterialCountData& data, RenderContext& context)
		{
			auto pipeline = ShaderMap::GetComputePipeline("GenerateMaterialCount");

			context.BindPipeline(pipeline);

			GPUSceneData::SetupConstants(context, gpuSceneData);

			context.SetConstant("visibilityBuffer"_sh, visBufferData.visibility);
			context.SetConstant("materialCountsBuffer"_sh, data.materialCountBuffer);
			context.SetConstant("renderSize"_sh, glm::uvec2{ m_width, m_height });

			context.Dispatch(Math::DivideRoundUp(m_width, 8u), Math::DivideRoundUp(m_height, 8u), 1);
		});
	}

	void SceneRenderer::AddCollectMaterialPixelsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const VisibilityBufferData& visBufferData = blackboard.Get<VisibilityBufferData>();
		const MaterialCountData& matCountData = blackboard.Get<MaterialCountData>();
		const GPUSceneData& gpuSceneData = blackboard.Get<GPUSceneData>();

		RenderGraphBufferHandle currentMaterialCountBuffer = RenderGraphNullHandle();

		{
			const auto desc = RGUtils::CreateBufferDescGPU<uint32_t>(std::max(m_scene->GetRenderScene()->GetIndividualMaterialCount(), 1u), "Current Material Count");
			currentMaterialCountBuffer = renderGraph.CreateBuffer(desc);
			RGUtils::ClearBuffer(renderGraph, currentMaterialCountBuffer, 0, "Clear Current Material Count");
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
		[=](const MaterialPixelsData& data, RenderContext& context)
		{
			auto pipeline = ShaderMap::GetComputePipeline("CollectMaterialPixels");

			context.BindPipeline(pipeline);

			GPUSceneData::SetupConstants(context, gpuSceneData);

			context.SetConstant("visibilityBuffer"_sh, visBufferData.visibility);
			context.SetConstant("materialStartBuffer"_sh, matCountData.materialStartBuffer);
			context.SetConstant("currentMaterialCountBuffer"_sh, data.currentMaterialCountBuffer);
			context.SetConstant("pixelCollectionBuffer"_sh, data.pixelCollectionBuffer);
			context.SetConstant("renderSize"_sh, glm::uvec2{ m_width, m_height });

			context.Dispatch(Math::DivideRoundUp(m_width, 8u), Math::DivideRoundUp(m_height, 8u), 1);
		});
	}

	void SceneRenderer::AddGenerateMaterialIndirectArgsPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		MaterialCountData matCountData = blackboard.Get<MaterialCountData>();

		RenderGraphBufferHandle materialIndirectArgsBuffer = RenderGraphNullHandle();
		{
			const auto desc = RGUtils::CreateBufferDesc<RHI::IndirectDispatchCommand>(std::max(m_scene->GetRenderScene()->GetIndividualMaterialCount(), 1u), RHI::BufferUsage::IndirectBuffer | RHI::BufferUsage::StorageBuffer, RHI::MemoryUsage::GPU, "Material Indirect Args");
			materialIndirectArgsBuffer = renderGraph.CreateBuffer(desc);
		}

		blackboard.Add<MaterialIndirectArgsData>() = renderGraph.AddPass<MaterialIndirectArgsData>("Generate Material Indirect Args",
		[&](RenderGraph::Builder& builder, MaterialIndirectArgsData& data)
		{
			data.materialIndirectArgsBuffer = materialIndirectArgsBuffer;

			builder.WriteResource(data.materialIndirectArgsBuffer);
			builder.ReadResource(matCountData.materialCountBuffer);
			builder.SetIsComputePass();
		},
		[=](const MaterialIndirectArgsData& data, RenderContext& context)
		{
			const uint32_t materialCount = m_scene->GetRenderScene()->GetIndividualMaterialCount();

			auto pipeline = ShaderMap::GetComputePipeline("GenerateMaterialIndirectArgs");

			context.BindPipeline(pipeline);
			context.SetConstant("materialCounts"_sh, matCountData.materialCountBuffer);
			context.SetConstant("indirectArgsBuffer"_sh, data.materialIndirectArgsBuffer);
			context.SetConstant("materialCount"_sh, materialCount);

			context.Dispatch(Math::DivideRoundUp(materialCount, 32u), 1, 1);
		});
	}

	void SceneRenderer::RenderMaterials(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		renderGraph.BeginMarker("Materials");

		for (uint32_t matId = 0; matId < m_scene->GetRenderScene()->GetIndividualMaterialCount(); matId++)
		{
			AddGenerateGBufferPass(renderGraph, blackboard, matId);
		}

		renderGraph.EndMarker();
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
		[=](RenderContext& context)
		{
			auto material = renderScene->GetMaterialFromID(materialId);
			auto pipeline = material->GetComputePipeline();

			if (!pipeline)
			{
				pipeline = ShaderMap::GetComputePipeline("OpaqueDefault");
			}

			context.BindPipeline(pipeline);

			GPUSceneData::SetupConstants(context, gpuSceneData);

			context.SetConstant("visibilityBuffer"_sh, visBufferData.visibility);
			context.SetConstant("materialCountBuffer"_sh, matCountData.materialCountBuffer);
			context.SetConstant("materialStartBuffer"_sh, matCountData.materialStartBuffer);
			context.SetConstant("pixelCollection"_sh, matPixelsData.pixelCollectionBuffer);

			context.SetConstant("viewData"_sh, uniformBuffers.viewDataBuffer);

			context.SetConstant("albedo"_sh, gbufferData.albedo);
			context.SetConstant("normals"_sh, gbufferData.normals);
			context.SetConstant("material"_sh, gbufferData.material);
			context.SetConstant("emissive"_sh, gbufferData.emissive);
			context.SetConstant("materialId"_sh, materialId);

			context.SetConstant("viewSize"_sh, glm::vec2(m_width, m_height));

			context.DispatchIndirect(indirectArgsData.materialIndirectArgsBuffer, sizeof(RHI::IndirectDispatchCommand) * materialId); // Should be offset with material ID
		});
	}

	void SceneRenderer::AddSkyboxPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard)
	{
		const auto& environmentTexturesData = blackboard.Get<EnvironmentTexturesData>();
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();

		RenderGraphBufferHandle meshVertexBufferHandle = renderGraph.AddExternalBuffer(m_skyboxMesh->GetVertexPositionsBuffer()->GetResource());
		RenderGraphBufferHandle indexBufferHandle = renderGraph.AddExternalBuffer(m_skyboxMesh->GetIndexBuffer()->GetResource());

		blackboard.Add<ShadingOutputData>() = renderGraph.AddPass<ShadingOutputData>("Skybox Pass",
		[&](RenderGraph::Builder& builder, ShadingOutputData& data)
		{
			{
				const auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::B10G11R11_UFLOAT_PACK32>(m_width, m_height, RHI::ImageUsage::AttachmentStorage, "Shading Output");
				data.colorOutput = builder.CreateImage(desc);
			}

			builder.ReadResource(environmentTexturesData.radiance);
			builder.ReadResource(meshVertexBufferHandle, RenderGraphResourceState::VertexBuffer);
			builder.ReadResource(indexBufferHandle, RenderGraphResourceState::IndexBuffer);
			builder.ReadResource(uniformBuffers.viewDataBuffer);
		},
		[=](const ShadingOutputData& data, RenderContext& context)
		{
			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { data.colorOutput });

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("Skybox");
			pipelineInfo.cullMode = RHI::CullMode::None;
			pipelineInfo.depthMode = RHI::DepthMode::None;

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);
			context.BindPipeline(pipeline);

			context.SetConstant("vertexPositions"_sh, meshVertexBufferHandle);
			context.SetConstant("viewData"_sh, uniformBuffers.viewDataBuffer);
			context.SetConstant("environmentTexture"_sh, environmentTexturesData.radiance);
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
		[=](RenderContext& context)
		{
			auto pipeline = ShaderMap::GetComputePipeline("Shading");
			context.BindPipeline(pipeline);
			context.SetConstant("output"_sh, shadingOutputData.colorOutput);
			context.SetConstant("albedo"_sh, gbufferData.albedo);
			context.SetConstant("normals"_sh, gbufferData.normals);
			context.SetConstant("material"_sh, gbufferData.material);
			context.SetConstant("emissive"_sh, gbufferData.emissive);
			context.SetConstant("aoTexture"_sh, gtaoOutput.outputImage);
			context.SetConstant("depthTexture"_sh, preDepthData.depth);
			context.SetConstant("shadingMode"_sh, static_cast<uint32_t>(m_shadingMode));
			context.SetConstant("visualizationMode"_sh, static_cast<uint32_t>(m_visualizationMode));

			// PBR Constants
			context.SetConstant("pbrConstants.viewData"_sh, uniformBuffers.viewDataBuffer);
			context.SetConstant("pbrConstants.directionalLight"_sh, uniformBuffers.directionalLightBuffer);
			context.SetConstant("pbrConstants.linearSampler"_sh, Renderer::GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear>()->GetResourceHandle());
			context.SetConstant("pbrConstants.pointLinearClampSampler"_sh, Renderer::GetSampler<RHI::TextureFilter::Nearest, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureWrap::Clamp>()->GetResourceHandle());
			context.SetConstant("pbrConstants.shadowSampler"_sh, Renderer::GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureWrap::Repeat, RHI::AnisotropyLevel::None, RHI::CompareOperator::LessEqual>()->GetResourceHandle());
			context.SetConstant("pbrConstants.BRDFLuT"_sh, externalImages.BRDFLuT);
			context.SetConstant("pbrConstants.environmentIrradiance"_sh, environmentTexturesData.irradiance);
			context.SetConstant("pbrConstants.environmentRadiance"_sh, environmentTexturesData.radiance);
			context.SetConstant("pbrConstants.pointLights"_sh, lightBuffers.pointLightsBuffer);
			context.SetConstant("pbrConstants.spotLights"_sh, lightBuffers.spotLightsBuffer);
			context.SetConstant("pbrConstants.visiblePointLights"_sh, lightCullingData.visiblePointLightsBuffer);
			context.SetConstant("pbrConstants.visibleSpotLights"_sh, lightCullingData.visibleSpotLightsBuffer);
			context.SetConstant("pbrConstants.directionalShadowMap"_sh, dirShadowData.shadowTexture);

			context.Dispatch(Math::DivideRoundUp(m_width, 8u), Math::DivideRoundUp(m_height, 8u), 1u);
		});
	}

	void SceneRenderer::AddFXAAPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, RenderGraphImageHandle srcImage)
	{
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();

		blackboard.Add<FXAAOutputData>() = renderGraph.AddPass<FXAAOutputData>("FXAA Pass",
		[&](RenderGraph::Builder& builder, FXAAOutputData& data)
		{
			data.output = builder.AddExternalImage(m_outputImage);

			builder.WriteResource(data.output);
			builder.ReadResource(srcImage);
			builder.ReadResource(uniformBuffers.viewDataBuffer);
		},
		[=](const FXAAOutputData& data, RenderContext& context)
		{
			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { data.output });

			RHI::RenderPipelineCreateInfo pipelineInfo;
			pipelineInfo.shader = ShaderMap::Get("FXAA");
			pipelineInfo.depthMode = RHI::DepthMode::None;
			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);

			RCUtils::DrawFullscreenTriangle(context, pipeline, [&](RenderContext& context)
			{
				context.SetConstant("sceneColor"_sh, srcImage);
				context.SetConstant("viewData"_sh, uniformBuffers.viewDataBuffer);
				context.SetConstant("linearSampler"_sh, Renderer::GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear>()->GetResourceHandle());
			});

			context.EndRendering();
		});
	}

	void SceneRenderer::AddFinalCopyPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, RenderGraphImageHandle srcImage)
	{
		blackboard.Add<FinalCopyData>() = renderGraph.AddPass<FinalCopyData>("Final Copy",
		[&](RenderGraph::Builder& builder, FinalCopyData& data)
		{
			{
				const auto desc = RGUtils::CreateImage2DDesc<RHI::PixelFormat::B10G11R11_UFLOAT_PACK32>(m_width, m_height, RHI::ImageUsage::AttachmentStorage, "Final Copy Output");
				data.output = builder.CreateImage(desc);
			}

			builder.WriteResource(data.output);
			builder.ReadResource(srcImage);
			builder.SetHasSideEffect();
		},
		[=](const FinalCopyData& data, RenderContext& context)
		{
			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { data.output });

			RHI::RenderPipelineCreateInfo pipelineInfo;
			pipelineInfo.shader = ShaderMap::Get("FinalCopy");
			pipelineInfo.depthMode = RHI::DepthMode::None;
			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);

			RCUtils::DrawFullscreenTriangle(context, pipeline, [&](RenderContext& context)
			{
				context.SetConstant("finalColor"_sh, srcImage);
			});

			context.EndRendering();
		});
	}

	void SceneRenderer::AddVisualizeSDFPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, RenderGraphImageHandle dstImage)
	{
		const auto& gpuSceneData = blackboard.Get<GPUSceneData>();
		const auto& uniformBuffers = blackboard.Get<UniformBuffersData>();

		const uint32_t sdfPrimitiveCount = m_scene->GetRenderScene()->GetSDFPrimitiveCount();

		renderGraph.AddPass("Visualize SDF",
		[&](RenderGraph::Builder& builder)
		{
			GPUSceneData::SetupInputs(builder, gpuSceneData);

			builder.WriteResource(dstImage);
			builder.ReadResource(uniformBuffers.viewDataBuffer);
			builder.SetHasSideEffect();
		},
		[=](RenderContext& context)
		{
			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { dstImage });
			info.renderingInfo.colorAttachments.At(0).clearMode = RHI::ClearMode::Load;

			RHI::RenderPipelineCreateInfo pipelineInfo;
			pipelineInfo.shader = ShaderMap::Get("TraceMeshSDFBrick");
			pipelineInfo.depthMode = RHI::DepthMode::None;
			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);

			RCUtils::DrawFullscreenTriangle(context, pipeline, [&](RenderContext& context)
			{
				GPUSceneData::SetupConstants(context, gpuSceneData);
				context.SetConstant("pointSampler"_sh, Renderer::GetSampler<RHI::TextureFilter::Linear, RHI::TextureFilter::Linear, RHI::TextureFilter::Linear>()->GetResourceHandle());
				context.SetConstant("viewData"_sh, uniformBuffers.viewDataBuffer);
				context.SetConstant("primitiveCount"_sh, sdfPrimitiveCount);
			});

			context.EndRendering();
		});
	}

	void SceneRenderer::AddVisualizeBricksPass(RenderGraph& renderGraph, RenderGraphBlackboard& blackboard, RenderGraphImageHandle dstImage)
	{
		auto cubeMesh = ShapeLibrary::GetCube();

		if (m_scene->GetRenderScene()->GetRenderObjectCount() < 1)
		{
			return;
		}

		const auto& mesh = m_scene->GetRenderScene()->GetRenderObjectAt(0).mesh;
		const auto& brickGrid = mesh->GetBrickGrid(0);

		const auto& viewData = blackboard.Get<ViewData>();

		struct PassData
		{
			RenderGraphImageHandle depthHandle;

			RenderGraphBufferHandle vertexBuffer;
			RenderGraphBufferHandle indexBuffer;
		};

		renderGraph.AddPass<PassData>("Visualize Bricks Pass",
		[&](RenderGraph::Builder& builder, PassData& data) 
		{
			RenderGraphImageDesc desc{};
			desc.width = m_width;
			desc.height = m_height;
			desc.format = RHI::PixelFormat::D32_SFLOAT;
			desc.usage = RHI::ImageUsage::Attachment;
			desc.name = "PreDepth";
			data.depthHandle = builder.CreateImage(desc);

			data.vertexBuffer = builder.AddExternalBuffer(cubeMesh->GetVertexPositionsBuffer()->GetResource());
			data.indexBuffer = builder.AddExternalBuffer(cubeMesh->GetIndexBuffer()->GetResource());

			builder.WriteResource(dstImage);

			builder.ReadResource(data.vertexBuffer, RenderGraphResourceState::VertexBuffer);
			builder.ReadResource(data.indexBuffer, RenderGraphResourceState::IndexBuffer);

			builder.SetHasSideEffect();
		},
		[=](const PassData& data, RenderContext& context) 
		{
			RenderingInfo info = context.CreateRenderingInfo(m_width, m_height, { dstImage, data.depthHandle });
			info.renderingInfo.colorAttachments.At(0).clearMode = RHI::ClearMode::Load;

			RHI::RenderPipelineCreateInfo pipelineInfo{};
			pipelineInfo.shader = ShaderMap::Get("VisualizeBricks");

			auto pipeline = ShaderMap::GetRenderPipeline(pipelineInfo);

			context.BeginRendering(info);
			context.BindPipeline(pipeline);

			context.BindVertexBuffers({ data.vertexBuffer }, 0);
			context.BindIndexBuffer(data.indexBuffer);

			context.SetConstant("viewProjection"_sh, viewData.viewProjection);

			for (const auto& brick : brickGrid)
			{
				if (!brick.hasData)
				{
					continue;
				}

				for (uint32_t index = 0; const auto& voxel : brick.data)
				{
					VT_UNUSED(voxel);

					if (voxel > 0.001f)
					{
						index++;
						continue;
					}

					struct PushData
					{
						glm::vec3 point;
						glm::vec3 scale;
						
						float data;
					} pushData;

					const auto voxelCoord = Math::Get3DCoordFrom1DIndex(index, 8u, 8u);

					pushData.point = brick.min + glm::vec3{ voxelCoord[0], voxelCoord[1], voxelCoord[2] } * 5.f;
					pushData.scale = 0.05f;
					pushData.data = voxel;

					context.PushConstants(&pushData, sizeof(PushData));
					context.DrawIndexed(36, 1, 0, 0, 0);

					index++;
				}
			}

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

		m_outputImage = RHI::Image::Create(spec);
	}
}
