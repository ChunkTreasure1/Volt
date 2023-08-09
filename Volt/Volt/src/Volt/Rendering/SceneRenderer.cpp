#include "vtpch.h"
#include "SceneRenderer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Asset/Animation/AnimationGraphAsset.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/ParticlePreset.h"

#include "Volt/Asset/Text/Font.h"
#include "Volt/Asset/Rendering/PostProcessingStack.h"
#include "Volt/Asset/Rendering/PostProcessingMaterial.h"

#include "Volt/Animation/AnimationController.h"

#include "Volt/Core/Application.h"
#include "Volt/Core/Graphics/SwapchainVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"
#include "Volt/Core/Graphics/GraphicsContextVolt.h"

#include "Volt/Components/Components.h"
#include "Volt/Components/LightComponents.h"
#include "Volt/Components/PostProcessingComponents.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"

#include "Volt/Rendering/Buffer/ShaderStorageBufferSet.h"
#include "Volt/Rendering/Buffer/ShaderStorageBuffer.h"
#include "Volt/Rendering/Buffer/UniformBufferSet.h"
#include "Volt/Rendering/Buffer/UniformBuffer.h"

#include "Volt/Rendering/Buffer/VertexBuffer.h"
#include "Volt/Rendering/Buffer/VertexBufferSet.h"
#include "Volt/Rendering/Buffer/IndexBuffer.h"

#include "Volt/Rendering/Texture/Image2D.h"
#include "Volt/Rendering/Texture/Image3D.h"
#include "Volt/Rendering/VulkanFramebuffer.h"
#include "Volt/Rendering/VulkanRenderPass.h"
#include "Volt/Rendering/CommandBuffer.h"
#include "Volt/Rendering/ComputePipeline.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/GlobalDescriptorSetManager.h"

#include "Volt/Rendering/RenderPipeline/ShaderRegistry.h"
#include "Volt/Rendering/GlobalDescriptorSet.h"
#include "Volt/Rendering/Texture/TextureTable.h"
#include "Volt/Rendering/MaterialTable.h"
#include "Volt/Rendering/MeshTable.h"
#include "Volt/Rendering/Shape.h"

#include "Volt/Rendering/Shader/ShaderUtility.h"

#include "Volt/Rendering/UIRenderer.h"
#include "Volt/Rendering/DebugRenderer.h"
#include "Volt/Rendering/RayTracedSceneRenderer.h"

#include "Volt/Rendering/FrameGraph/FrameGraph.h"

#include "Volt/Rendering/RenderTechniques/TechniqueStructures.h"
#include "Volt/Rendering/RenderTechniques/GTAOTechnique.h"
#include "Volt/Rendering/RenderTechniques/BloomTechnique.h"
#include "Volt/Rendering/RenderTechniques/OutlineTechnique.h"
#include "Volt/Rendering/RenderTechniques/TAATechnique.h"
#include "Volt/Rendering/RenderTechniques/FXAATechnique.h"
#include "Volt/Rendering/RenderTechniques/VolumetricFogTechnique.h"
#include "Volt/Rendering/RenderTechniques/SSRTechnique.h"

#include "Volt/Asset/Text/MSDFData.h"

#include "Volt/Particles/ParticleSystem.h"

#include "Volt/Math/Math.h"

#include "Volt/Utility/ImageUtility.h"
#include "Volt/Utility/ShadowMappingUtility.h"
#include "Volt/Utility/Noise.h"
#include "Volt/Platform/ThreadUtility.h"

namespace Volt
{
	namespace Utility
	{
		inline static Ref<RenderPipeline> CreateRenderPipeline(Ref<Shader> shader, std::vector<FramebufferAttachment> attachments, std::string_view name, CompareOperator depthCompare = CompareOperator::GreaterEqual)
		{
			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.shader = shader;
			pipelineSpec.framebufferAttachments = attachments;
			pipelineSpec.name = name;
			pipelineSpec.vertexLayout = Vertex::GetVertexLayout();
			pipelineSpec.depthCompareOperator = depthCompare;

			return RenderPipeline::Create(pipelineSpec);
		}

		inline static Ref<RenderPipeline> CreateFullscreenRenderPipeline(Ref<Shader> shader, std::vector<FramebufferAttachment> attachments, std::string_view name)
		{
			RenderPipelineSpecification pipelineSpec{};
			pipelineSpec.shader = shader;
			pipelineSpec.framebufferAttachments = attachments;
			pipelineSpec.name = name;

			return RenderPipeline::Create(pipelineSpec);
		}
	}

	SceneRenderer::SceneRenderer(const SceneRendererSpecification& specification, const SceneRendererSettings& initialSettings)
		: mySpecification(specification), mySettings(initialSettings)
	{
		myScene = mySpecification.scene;
		myOriginalSize = mySpecification.initialResolution;
		myScaledSize = (glm::uvec2)((glm::vec2)mySpecification.initialResolution * mySettings.renderScale);
		myRenderSize = myScaledSize;

		myTimestamps.resize(Renderer::GetFramesInFlightCount());
		myResizeLightCullingBuffers = std::vector<bool>(Renderer::GetFramesInFlightCount(), false);

		CreateBuffers();
		CreateGlobalDescriptorSets();
		CreateResources();
		UpdateGlobalDescriptorSets();

		CreateLineData();
		CreateBillboardData();
		CreateTextData();

		if (mySettings.enableRayTracing)
		{
			RayTracedSceneRendererSpecification spec{};
			spec.scene = myScene;

			myRayTracedSceneRenderer = CreateScope<RayTracedSceneRenderer>(spec);
		}

		myIndirectPasses.reserve(100);
	}

	SceneRenderer::~SceneRenderer()
	{
		//GraphicsContextVolt::GetDevice()->WaitForIdle();

		myShaderStorageBufferSet = nullptr;
		myUniformBufferSet = nullptr;
		myRayTracedSceneRenderer = nullptr;
		myGlobalDescriptorSets.clear();

		// Remove line data
		{
			delete[] myLineData.vertexBufferBase;
			myLineData.vertexBuffer = nullptr;
			myLineData.renderPipeline = nullptr;
		}

		// Remove billboard data
		{
			delete[] myBillboardData.vertexBufferBase;
			myBillboardData.vertexBuffer = nullptr;
			myBillboardData.renderPipeline = nullptr;
		}

		myTransientResourceSystem.Reset();
		myTransientResourceSystem.Clear();
	}

	void SceneRenderer::OnRenderEditor(Ref<Camera> camera)
	{
		OnRender(camera);
	}

	void SceneRenderer::OnRenderRuntime()
	{
		VT_PROFILE_FUNCTION();

		Ref<Camera> camera = nullptr;
		int32_t highestPrio = -1;

		myScene.lock()->GetRegistry().ForEach<CameraComponent>([&](Wire::EntityId, const CameraComponent& camComp)
		{
			if ((int32_t)camComp.priority > highestPrio)
			{
				highestPrio = (int32_t)camComp.priority;
				camera = camComp.camera;
			}
		});

		if (camera)
		{
			OnRender(camera);
		}
	}

	void SceneRenderer::Resize(uint32_t width, uint32_t height)
	{
		myShouldResize = true;
		myOriginalSize = glm::uvec2{ width, height };
		myScaledSize = (glm::uvec2)glm::max(glm::vec2((float)myOriginalSize.x, (float)myOriginalSize.y) * mySettings.renderScale, glm::vec2{ 1.f });
	}

	void SceneRenderer::SubmitOutlineMesh(Ref<Mesh> mesh, const glm::mat4& transform)
	{
		VT_PROFILE_FUNCTION();

		for (uint32_t i = 0; const auto & subMesh : mesh->GetSubMeshes())
		{
			auto& cmd = GetCPUData().outlineCommands.emplace_back();
			cmd.mesh = mesh;
			cmd.material = mesh->GetMaterial()->TryGetSubMaterialAt(subMesh.materialIndex);
			cmd.subMesh = subMesh;
			cmd.transform = transform * subMesh.transform;
			cmd.subMeshIndex = i;

			i++;
		}
	}

	void SceneRenderer::SubmitOutlineMesh(Ref<Mesh> mesh, uint32_t subMeshIndex, const glm::mat4& transform)
	{
		VT_PROFILE_FUNCTION();

		const auto& subMesh = mesh->GetSubMeshes().at(subMeshIndex);

		auto& cmd = GetCPUData().outlineCommands.emplace_back();
		cmd.mesh = mesh;
		cmd.material = mesh->GetMaterial()->TryGetSubMaterialAt(subMesh.materialIndex);
		cmd.subMesh = subMesh;
		cmd.transform = transform * subMesh.transform;
		cmd.subMeshIndex = subMeshIndex;
	}

	void SceneRenderer::ClearOutlineCommands()
	{
		GetCPUData().outlineCommands.clear();
	}

	Ref<Image2D> SceneRenderer::GetFinalImage()
	{
		return myOutputImage;
	}

	Ref<Image2D> SceneRenderer::GetIDImage()
	{
		return myIDImage;
	}

	void SceneRenderer::ApplySettings()
	{
		//GraphicsContextVolt::GetDevice()->WaitForIdle();

		myScaledSize = (glm::uvec2)glm::max(glm::vec2((float)myOriginalSize.x, (float)myOriginalSize.y) * mySettings.renderScale, glm::vec2{ 1.f });
		myShouldResize = true;

		myGTAOMainPassPipeline->GetSpecializationConstantBuffer().SetValue("u_quality", (uint32_t)mySettings.aoQuality);
		myGTAOMainPassPipeline->Invalidate();
	}

	void SceneRenderer::UpdateSettings(const SceneRendererSettings& settings)
	{
		mySettings.shadowResolution = settings.shadowResolution;
		mySettings.antiAliasing = settings.antiAliasing;
		mySettings.aoQuality = settings.aoQuality;
		mySettings.renderScale = settings.renderScale;

		mySettings.enableShadows = settings.enableShadows;
		mySettings.enableAO = settings.enableAO;
		mySettings.enableBloom = settings.enableBloom;
		mySettings.enableAntiAliasing = settings.enableAntiAliasing;
		mySettings.enableVolumetricFog = settings.enableVolumetricFog;
	}

	void SceneRenderer::SetRenderMode(RenderMode renderingMode)
	{
		if (myCurrentRenderMode == renderingMode)
		{
			return;
		}

		//GraphicsContextVolt::GetDevice()->WaitForIdle();

		myCurrentRenderMode = renderingMode;
		myDeferredShadingPipeline->GetSpecializationConstantBuffer().SetValue("u_renderMode", (uint32_t)myCurrentRenderMode);
		myDeferredShadingPipeline->Invalidate();
	}

	void SceneRenderer::UpdateRayTracingScene()
	{
		myRayTracedSceneRenderer->BuildBottomLevelAccelerationStructures();
		myRayTracedSceneRenderer->BuildTopLevelAccelerationStructures();
	}

	const std::vector<SceneRenderer::Timestamp> SceneRenderer::GetTimestamps()
	{
		return myTimestampsResult;
	}

	const SceneRendererStatistics& SceneRenderer::GetStatistics() const
	{
		return myStatistics;
	}

	void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, Ref<Material> material, const glm::mat4& transform, const std::vector<glm::mat4>& boneTransforms, float timeSinceCreation, float randomValue, uint32_t id)
	{
		for (uint32_t i = 0; const auto & subMesh : mesh->GetSubMeshes())
		{
			auto& cmd = GetCPUData().submitCommands.emplace_back();
			cmd.mesh = mesh;
			cmd.material = material->TryGetSubMaterialAt(subMesh.materialIndex);
			cmd.subMesh = subMesh;
			cmd.transform = transform * subMesh.transform;
			cmd.subMeshIndex = i;
			cmd.boneTransforms = boneTransforms;
			cmd.id = id;
			cmd.timeSinceCreation = timeSinceCreation;
			cmd.randomValue = randomValue;

			i++;
		}

		GetCPUData().animationBoneCount += (uint32_t)boneTransforms.size();
	}

	void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, Ref<Material> material, const glm::mat4& transform, float timeSinceCreation, float randomValue, uint32_t id)
	{
		for (uint32_t i = 0; const auto & subMesh : mesh->GetSubMeshes())
		{
			auto& cmd = GetCPUData().submitCommands.emplace_back();
			cmd.mesh = mesh;
			cmd.material = material->TryGetSubMaterialAt(subMesh.materialIndex);
			cmd.subMesh = subMesh;
			cmd.transform = transform * subMesh.transform;
			cmd.subMeshIndex = i;
			cmd.id = id;
			cmd.timeSinceCreation = timeSinceCreation;
			cmd.randomValue = randomValue;

			i++;
		}
	}

	void SceneRenderer::BeginTimestamp(const std::string& label)
	{
		return;

		//const uint32_t currentIndex = myCommandBuffer->GetCurrentIndex();
		//auto& timestamps = myTimestamps.at(currentIndex);

		//auto& timestamp = timestamps.emplace_back();
		//timestamp.label = label;
		//timestamp.id = myCommandBuffer->BeginTimestamp();
	}

	void SceneRenderer::EndTimestamp()
	{
		return;

		//const uint32_t currentIndex = myCommandBuffer->GetCurrentIndex();
		//auto& timestamps = myTimestamps.at(currentIndex);

		//if (timestamps.empty())
		//{
		//	return;
		//}

		//myCommandBuffer->EndTimestamp(timestamps.back().id);
	}

	void SceneRenderer::OnRender(Ref<Camera> camera)
	{
		VT_PROFILE_FUNCTION();

		GetCPUData().Clear();
		myStatistics.Reset();

		FetchCommands(camera);
		FetchParticles(camera);

		const uint32_t currentIndex = myCommandBuffer->GetLastIndex();

		EvaluateTimestamps();
		myRenderPipelineStatistics = myCommandBuffer->GetPipelineStatistics(currentIndex);
		myStatistics.pipelineStatistics = myRenderPipelineStatistics;

		myTimestamps.at(currentIndex).clear();

		myRenderSize = myScaledSize;
		myCurrentCamera = camera;

		ExecuteFrame();

		myCommandBuffer->Submit();

		if (mySettings.enableUI)
		{
			VT_PROFILE_SCOPE("Render UI");
			RenderUI();
		}
	}

	void SceneRenderer::CreateResources()
	{
		const uint32_t framesInFlight = Renderer::GetFramesInFlightCount();

		// Light culling
		{
			myLightCullPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("LightCull"), framesInFlight, true);
		}

		// Images
		{
			// Output
			{
				ImageSpecification specification{};
				specification.format = ImageFormat::RGBA16F;
				specification.usage = ImageUsage::AttachmentStorage;
				specification.width = myScaledSize.x;
				specification.height = myScaledSize.y;
				specification.isCubeMap = false;
				specification.layers = 1;
				specification.debugName = "Main Output";

				myOutputImage = Image2D::Create(specification);
				myHistoryColor = Image2D::Create(specification);

				specification.format = ImageFormat::R32F;
				myHistoryDepth = Image2D::Create(specification);
			}

			// ID
			if (mySettings.enableIDRendering)
			{
				ImageSpecification specification{};
				specification.format = ImageFormat::R32UI;
				specification.usage = ImageUsage::AttachmentStorage;
				specification.width = myScaledSize.x;
				specification.height = myScaledSize.y;
				specification.isCubeMap = false;
				specification.layers = 1;
				specification.debugName = "ID";

				myIDImage = Image2D::Create(specification);
			}

			// Volumetric fog
			{
				ImageSpecification specification{};
				specification.width = VOLUMETRIC_FOG_WIDTH;
				specification.height = VOLUMETRIC_FOG_HEIGHT;
				specification.depth = VOLUMETRIC_FOG_DEPTH;
				specification.debugName = "Volumetric Fog Injection";
				specification.usage = ImageUsage::Storage;
				specification.format = ImageFormat::RGBA32F;

				if (mySettings.enableVolumetricFog)
				{
					myVolumetricFogInjectImage = Image3D::Create(specification);

					specification.debugName = "Volumetric Fog Raymarch";
					myVolumetricFogRayMarchImage = Image3D::Create(specification);
				}
				else
				{
					specification.width = 1;
					specification.height = 1;
					specification.depth = 1;

					myVolumetricFogInjectImage = Image3D::Create(specification);

					specification.debugName = "Volumetric Fog Raymarch";
					myVolumetricFogRayMarchImage = Image3D::Create(specification);
				}
			}
		}

		// Render Pipelines
		{
			// Directional Shadow map
			{
				RenderPipelineSpecification pipelineSpec{};
				pipelineSpec.shader = ShaderRegistry::GetShader("DirectionalShadow");
				pipelineSpec.framebufferAttachments = { ImageFormat::DEPTH32F };
				pipelineSpec.name = "Directional Shadow";
				pipelineSpec.vertexLayout = Vertex::GetVertexLayout();
				pipelineSpec.depthCompareOperator = CompareOperator::LessEqual;

				myDirectionalShadowPipeline = RenderPipeline::Create(pipelineSpec);
			}

			// Spot Light Shadow map
			{
				RenderPipelineSpecification spec{};
				spec.shader = ShaderRegistry::GetShader("SpotlightShadow");
				spec.framebufferAttachments = { ImageFormat::DEPTH32F };
				spec.name = "Spotlight Shadow";
				spec.vertexLayout = Vertex::GetVertexLayout();
				spec.depthCompareOperator = CompareOperator::LessEqual;

				mySpotLightShadowPipeline = RenderPipeline::Create(spec);
			}

			// Point Light Shadow map
			{
				RenderPipelineSpecification spec{};
				spec.shader = ShaderRegistry::GetShader("PointLightShadow");
				spec.framebufferAttachments = { ImageFormat::DEPTH32F };
				spec.name = "Point Light Shadow";
				spec.vertexLayout = Vertex::GetVertexLayout();
				spec.depthCompareOperator = CompareOperator::LessEqual;

				myPointLightShadowPipeline = RenderPipeline::Create(spec);
			}

			// Deferred
			{
				myGBufferPipeline = Utility::CreateRenderPipeline(ShaderRegistry::GetShader("Illum"), { ImageFormat::RGBA, ImageFormat::RGBA16F, ImageFormat::RGBA16F, ImageFormat::DEPTH32F }, "Deferred");

				Ref<Shader> shadingShader = ShaderRegistry::GetShader("DeferredShading");
				ShaderDataBuffer renderModeBuffer = shadingShader->CreateSpecialiazationConstantsBuffer(ShaderStage::Compute);
				renderModeBuffer.SetValue("u_renderMode", 0);

				myDeferredShadingPipeline = ComputePipeline::Create(shadingShader, framesInFlight, true, renderModeBuffer);
			}

			myPreDepthPipeline = Utility::CreateRenderPipeline(ShaderRegistry::GetShader("PreDepth"), { ImageFormat::RGBA16F, ImageFormat::DEPTH32F }, "Pre Depth", CompareOperator::GreaterEqual);
			myIDPipeline = Utility::CreateRenderPipeline(ShaderRegistry::GetShader("ID"), { ImageFormat::R32UI, ImageFormat::DEPTH32F }, "ID");
			myOutlineGeometryPipeline = Utility::CreateRenderPipeline(ShaderRegistry::GetShader("OutlineGeometry"), { ImageFormat::RGBA16F, ImageFormat::DEPTH32F }, "Outline Geometry");

			myDepthReductionPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("DepthReduction"), framesInFlight, true);

			myPreethamPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("PreethamSky"), framesInFlight);
			myACESPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("ACES"), framesInFlight);
			myGammaCorrectionPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("GammaCorrection"), framesInFlight);
			myLuminosityPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("CalculateLuminosity"), framesInFlight);

			myBloomDownsamplePipeline = ComputePipeline::Create(ShaderRegistry::GetShader("BloomDownsample"), framesInFlight, true);
			myBloomUpsamplePipeline = ComputePipeline::Create(ShaderRegistry::GetShader("BloomUpsample"), framesInFlight, true);
			myBloomCompositePipeline = ComputePipeline::Create(ShaderRegistry::GetShader("BloomComposite"), framesInFlight);

			// GTAO
			{
				myGTAOPrefilterPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("GTAODepthPrefilter"), framesInFlight, true);

				Ref<Shader> mainPassShader = ShaderRegistry::GetShader("GTAOMainPass");
				ShaderDataBuffer qualityBuffer = mainPassShader->CreateSpecialiazationConstantsBuffer(ShaderStage::Compute);
				qualityBuffer.SetValue("u_quality", (uint32_t)mySettings.aoQuality);

				myGTAOMainPassPipeline = ComputePipeline::Create(mainPassShader, framesInFlight, true, qualityBuffer);
			}

			myVolumetricFogInjectPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("VolumetricFog_InjectLight"), framesInFlight, true);
			myVolumetricFogRayMarchPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("VolumetricFog_RayMarch"), framesInFlight, true);

			mySSSBlurPipeline[0] = ComputePipeline::Create(ShaderRegistry::GetShader("SSSBlur"), framesInFlight, true);
			mySSSBlurPipeline[1] = ComputePipeline::Create(ShaderRegistry::GetShader("SSSBlur"), framesInFlight, true);
			mySSSCompositePipeline = ComputePipeline::Create(ShaderRegistry::GetShader("SSSComposite"), framesInFlight);

			constexpr uint32_t GTAO_DENOISE_LEVELS = 5;
			for (uint32_t i = 0; i < GTAO_DENOISE_LEVELS; i++)
			{
				myGTAODenoisePipelines.push_back(ComputePipeline::Create(ShaderRegistry::GetShader("GTAODenoise"), framesInFlight, true));
			}

			myOutlineCompositePipeline = ComputePipeline::Create(ShaderRegistry::GetShader("OutlineComposite"), framesInFlight, true);

			myGenerateMotionVectorsPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("GenerateMotionVectors"), framesInFlight, true);
			myTAAPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("TAA"), framesInFlight, true);
			myFXAAPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("FXAA"), framesInFlight, true);
			myAACompositePipeline = ComputePipeline::Create(ShaderRegistry::GetShader("AAComposite"), framesInFlight, false);

			myLODSelectionPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("LODSelection"), framesInFlight, true);
		}

		// Materials
		{
			// Transparent Composite
			{
				RenderPipelineSpecification spec{};
				spec.shader = ShaderRegistry::GetShader("TransparentComposite");
				spec.framebufferAttachments = { { ImageFormat::RGBA16F, { 1.f }, TextureBlend::OneMinusSrcAlpha, "TransparentComposite", true } };
				spec.name = "TransparentComposite";
				spec.cullMode = CullMode::Back;

				myTransparentCompositeMaterial = Material::Create(spec);
			}

			// SSR
			{
				RenderPipelineSpecification spec{};
				spec.shader = ShaderRegistry::GetShader("SSR");
				spec.framebufferAttachments = { { ImageFormat::RGBA16F } };
				spec.name = "SSR";
				spec.cullMode = CullMode::Back;

				mySSRMaterial = Material::Create(spec);
			}

			// SSR Composite
			{
				RenderPipelineSpecification spec{};
				spec.shader = ShaderRegistry::GetShader("SSRComposite");
				spec.framebufferAttachments = { { ImageFormat::RGBA16F } };
				spec.name = "SSR Composite";
				spec.cullMode = CullMode::Back;

				mySSRCompositeMaterial = Material::Create(spec);
			}

			// Grid
			{
				RenderPipelineSpecification spec{};
				spec.shader = ShaderRegistry::GetShader("Grid");
				spec.framebufferAttachments =
				{
					{ ImageFormat::RGBA16F, { 1.f }, TextureBlend::Alpha, "Grid", true },
					{ ImageFormat::DEPTH32F, { 1.f }, TextureBlend::None, "Depth", true },
				};

				spec.name = "Grid";
				spec.cullMode = CullMode::Back;

				myGridMaterial = Material::Create(spec);
			}

			// Decal
			{
				myDecalMesh = AssetManager::GetAsset<Mesh>("Engine/Meshes/Primitives/SM_Cube.vtmesh");

				RenderPipelineSpecification spec{};
				spec.shader = ShaderRegistry::GetShader("Decal");
				spec.framebufferAttachments =
				{
					ImageFormat::RGBA,
					ImageFormat::RGBA16F,
					ImageFormat::RGBA16F,
					ImageFormat::DEPTH32F
				};

				spec.name = "Decal";
				spec.cullMode = CullMode::None;
				spec.depthMode = DepthMode::None;
				spec.vertexLayout = Vertex::GetVertexLayout();

				myDecalMaterial = Material::Create(spec);
			}

			// Default Particle
			{
				RenderPipelineSpecification spec{};
				spec.shader = ShaderRegistry::GetShader("Particle");
				spec.framebufferAttachments = { { ImageFormat::RGBA16F, { 1.f }, TextureBlend::Alpha }, ImageFormat::DEPTH32F };
				spec.name = "Default Particle Material";
				spec.cullMode = CullMode::Back;
				spec.topology = Topology::PointList;

				myDefaultParticleMaterial = Material::Create(spec);
			}

			// Jump Flood Init
			{
				RenderPipelineSpecification spec{};
				spec.shader = ShaderRegistry::GetShader("JumpFloodInit");
				spec.framebufferAttachments = { { ImageFormat::RGBA16F } };
				spec.name = "Jump Flood Init";

				myOutlineJumpFloodInitMaterial = Material::Create(spec);
			}

			// Jump Flood Pass
			{
				RenderPipelineSpecification spec{};
				spec.shader = ShaderRegistry::GetShader("JumpFloodPass");
				spec.framebufferAttachments = { { ImageFormat::RGBA16F } };
				spec.name = "Jump Flood Pass";

				myOutlineJumpFloodPassMaterial = Material::Create(spec);
			}

			// Skybox
			{
				mySkyboxMesh = Shape::CreateUnitCube();

				RenderPipelineSpecification pipelineSpec{};
				pipelineSpec.shader = ShaderRegistry::GetShader("Skybox");
				pipelineSpec.framebufferAttachments = { ImageFormat::RGBA16F };
				pipelineSpec.name = "Skybox";
				pipelineSpec.vertexLayout = Vertex::GetVertexLayout();
				pipelineSpec.cullMode = CullMode::Front;

				Ref<Material> material = Material::Create(pipelineSpec);
				mySkyboxMesh->SetMaterial(material);
			}
		}
	}

	void SceneRenderer::CreateBuffers()
	{
		//const uint32_t maxFramesInFlight = Application::Get().GetWindow().GetSwapchain().GetMaxFramesInFlight();
		//myCommandBuffer = CommandBuffer::Create(maxFramesInFlight, false);

		//myUniformBufferSet = UniformBufferSet::Create(maxFramesInFlight);
		myUniformBufferSet->Add<CameraData>(Sets::RENDERER_BUFFERS, Bindings::CAMERA_BUFFER);
		myUniformBufferSet->Add<DirectionalLight>(Sets::RENDERER_BUFFERS, Bindings::DIRECTIONAL_LIGHT);
		myUniformBufferSet->Add<SceneData>(Sets::RENDERER_BUFFERS, Bindings::SCENE_DATA);
		myUniformBufferSet->Add<RendererGPUData>(Sets::RENDERER_BUFFERS, Bindings::RENDERER_DATA);

		constexpr uint32_t START_OBJECT_COUNT = 1;
		constexpr uint32_t START_LIGHT_COUNT = 1;
		constexpr uint32_t START_PARTICLE_COUNT = 1;
		constexpr uint32_t START_VERTEX_COLOR_COUNT = 1;

		//myShaderStorageBufferSet = ShaderStorageBufferSet::Create(maxFramesInFlight);

		myShaderStorageBufferSet->Add<IndirectGPUCommand>(Sets::RENDERER_BUFFERS, Bindings::MAIN_INDIRECT_ARGS, START_OBJECT_COUNT, MemoryUsage::CPUToGPU | MemoryUsage::Indirect);
		myShaderStorageBufferSet->Add<ObjectData>(Sets::RENDERER_BUFFERS, Bindings::OBJECT_DATA, START_OBJECT_COUNT, MemoryUsage::CPUToGPU);

		myShaderStorageBufferSet->Add<glm::mat4>(Sets::RENDERER_BUFFERS, Bindings::ANIMATION_DATA, 8096, MemoryUsage::CPUToGPU);

		myShaderStorageBufferSet->Add<PointLight>(Sets::RENDERER_BUFFERS, Bindings::POINT_LIGHTS, START_LIGHT_COUNT, MemoryUsage::CPUToGPU);
		myShaderStorageBufferSet->Add<SpotLight>(Sets::RENDERER_BUFFERS, Bindings::SPOT_LIGHTS, START_LIGHT_COUNT, MemoryUsage::CPUToGPU);
		myShaderStorageBufferSet->Add<SphereLight>(Sets::RENDERER_BUFFERS, Bindings::SPHERE_LIGHTS, START_LIGHT_COUNT, MemoryUsage::CPUToGPU);
		myShaderStorageBufferSet->Add<RectangleLight>(Sets::RENDERER_BUFFERS, Bindings::RECTANGLE_LIGHTS, START_LIGHT_COUNT, MemoryUsage::CPUToGPU);

		myShaderStorageBufferSet->Add<ParticleRenderingInfo>(Sets::RENDERER_BUFFERS, Bindings::PARTICLE_INFO, START_PARTICLE_COUNT, MemoryUsage::CPUToGPU);
		myShaderStorageBufferSet->Add<uint32_t>(Sets::RENDERER_BUFFERS, Bindings::PAINTED_VERTEX_COLORS, START_VERTEX_COLOR_COUNT, MemoryUsage::CPUToGPU);

		// Light culling
		{
			constexpr uint32_t TILE_SIZE = 16;

			glm::uvec2 size = myRenderSize;
			size.x += TILE_SIZE - myRenderSize.x % TILE_SIZE;
			size.y += TILE_SIZE - myRenderSize.y % TILE_SIZE;

			myLightCullingWorkGroups = size / TILE_SIZE;

			myShaderStorageBufferSet->Add<int32_t>(Sets::RENDERER_BUFFERS, Bindings::VISIBLE_POINT_LIGHTS, MAX_POINT_LIGHTS * myLightCullingWorkGroups.x * myLightCullingWorkGroups.y);
			myShaderStorageBufferSet->Add<int32_t>(Sets::RENDERER_BUFFERS, Bindings::VISIBLE_SPOT_LIGHTS, MAX_SPOT_LIGHTS * myLightCullingWorkGroups.x * myLightCullingWorkGroups.y);
		}

	}

	void SceneRenderer::CreateLineData()
	{
		myLineData.vertexBufferBase = new LineVertex[LineData::MAX_VERTICES];
		myLineData.vertexBufferPtr = myLineData.vertexBufferBase;

		myLineData.vertexBuffer = VertexBufferSet::Create(Renderer::GetFramesInFlightCount(), myLineData.vertexBufferBase, LineData::MAX_VERTICES * sizeof(LineVertex));

		RenderPipelineSpecification pipelineSpec{};
		pipelineSpec.shader = ShaderRegistry::GetShader("Line");
		pipelineSpec.topology = Topology::LineList;
		pipelineSpec.cullMode = CullMode::Back;
		pipelineSpec.lineWidth = 2.f;
		pipelineSpec.vertexLayout = LineVertex::GetVertexLayout();
		pipelineSpec.name = "Line";

		myLineData.renderPipeline = RenderPipeline::Create(pipelineSpec);
	}

	void SceneRenderer::CreateBillboardData()
	{
		myBillboardData.vertexBufferBase = new BillboardVertex[BillboardData::MAX_BILLBOARDS];
		myBillboardData.vertexBufferPtr = myBillboardData.vertexBufferBase;

		myBillboardData.vertexBuffer = VertexBufferSet::Create(Renderer::GetFramesInFlightCount(), myBillboardData.vertexBufferBase, BillboardData::MAX_BILLBOARDS * sizeof(BillboardVertex));

		RenderPipelineSpecification pipelineSpec{};
		pipelineSpec.shader = ShaderRegistry::GetShader("Billboard");
		pipelineSpec.topology = Topology::PointList;
		pipelineSpec.cullMode = CullMode::Back;
		pipelineSpec.vertexLayout = BillboardVertex::GetVertexLayout();
		pipelineSpec.name = "Billboard";
		pipelineSpec.framebufferAttachments =
		{
			{ ImageFormat::RGBA16F, { 1.f }, TextureBlend::Alpha },
			{ ImageFormat::R32UI }
		};

		myBillboardData.renderPipeline = RenderPipeline::Create(pipelineSpec);
	}

	void SceneRenderer::CreateTextData()
	{
		auto& textData = myTextData;

		// Create pipeline
		{
			RenderPipelineSpecification spec{};
			spec.name = "Text";
			spec.shader = ShaderRegistry::GetShader("Text");
			spec.depthMode = DepthMode::ReadWrite;
			spec.cullMode = CullMode::Back;

			spec.framebufferAttachments =
			{
				{ ImageFormat::RGBA16F, { 1.f }, TextureBlend::Alpha },
				{ ImageFormat::DEPTH32F }
			};

			spec.vertexLayout = TextVertex::GetVertexLayout();
			spec.depthCompareOperator = CompareOperator::GreaterEqual;

			textData.renderPipeline = RenderPipeline::Create(spec);
		}

		textData.vertexBufferBase = new TextVertex[TextData::MAX_INDICES];
		textData.vertexBufferPtr = textData.vertexBufferBase;

		textData.vertexBuffer = VertexBufferSet::Create(Renderer::GetFramesInFlightCount(), textData.vertexBufferBase, sizeof(TextVertex) * TextData::MAX_VERTICES, true);

		// Index buffer
		{
			uint32_t* indices = new uint32_t[TextData::MAX_INDICES];
			uint32_t offset = 0;

			for (uint32_t indice = 0; indice < TextData::MAX_INDICES; indice += 6)
			{
				indices[indice + 0] = offset + 0;
				indices[indice + 1] = offset + 3;
				indices[indice + 2] = offset + 2;

				indices[indice + 3] = offset + 2;
				indices[indice + 4] = offset + 1;
				indices[indice + 5] = offset + 0;

				offset += 4;
			}

			textData.indexBuffer = IndexBuffer::Create(indices, TextData::MAX_INDICES);
			delete[] indices;
		}
	}

	void SceneRenderer::CreateGlobalDescriptorSets()
	{
		myGlobalDescriptorSets = GlobalDescriptorSetManager::CreateFullCopy();
	}

	void SceneRenderer::UpdateGlobalDescriptorSets()
	{
		for (uint32_t i = 0; i < Renderer::GetFramesInFlightCount(); i++)
		{
			auto rendererBufferDescriptor = myGlobalDescriptorSets[Sets::RENDERER_BUFFERS];
			rendererBufferDescriptor->GetOrAllocateDescriptorSet(i);

			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::MAIN_INDIRECT_ARGS, i, myShaderStorageBufferSet);

			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::ANIMATION_DATA, i, myShaderStorageBufferSet);
			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::PAINTED_VERTEX_COLORS, i, myShaderStorageBufferSet);
			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::CAMERA_BUFFER, i, myUniformBufferSet);
			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::SCENE_DATA, i, myUniformBufferSet);

			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::DIRECTIONAL_LIGHT, i, myUniformBufferSet);
			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::POINT_LIGHTS, i, myShaderStorageBufferSet);
			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::SPOT_LIGHTS, i, myShaderStorageBufferSet);
			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::SPHERE_LIGHTS, i, myShaderStorageBufferSet);
			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::RECTANGLE_LIGHTS, i, myShaderStorageBufferSet);

			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::OBJECT_DATA, i, myShaderStorageBufferSet);

			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::PARTICLE_INFO, i, myShaderStorageBufferSet);
			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::VISIBLE_POINT_LIGHTS, i, myShaderStorageBufferSet);
			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::RENDERER_DATA, i, myUniformBufferSet);
			UpdateGlobalDescriptorSet(rendererBufferDescriptor, Sets::RENDERER_BUFFERS, Bindings::VISIBLE_SPOT_LIGHTS, i, myShaderStorageBufferSet);
		}
	}

	void SceneRenderer::UpdateBuffers(Ref<Camera> camera)
	{
		const uint32_t currentIndex = myCommandBuffer->GetCurrentIndex();

		// Update camera data
		{
			CameraData camBuffer;

			camBuffer.proj = camera->GetProjection();
			camBuffer.view = camera->GetView();
			camBuffer.viewProjection = camera->GetProjection() * camera->GetView();

			const auto& projectionMatrix = camera->GetProjection();

			float depthLinearizeMul = (-projectionMatrix[3][2]);
			float depthLinearizeAdd = (projectionMatrix[2][2]);

			// correct the handedness issue
			if (depthLinearizeMul * depthLinearizeAdd < 0.f)
			{
				depthLinearizeAdd = -depthLinearizeAdd;
			}

			camBuffer.depthUnpackConsts = { depthLinearizeMul, depthLinearizeAdd };

			camBuffer.inverseProj = glm::inverse(camBuffer.proj);
			camBuffer.inverseView = glm::inverse(camBuffer.view);
			camBuffer.nonReversedProj = glm::perspective(glm::radians(camera->GetFieldOfView()), camera->GetAspectRatio(), camera->GetNearPlane(), camera->GetFarPlane());
			camBuffer.inverseNonReverseViewProj = glm::inverse(camBuffer.nonReversedProj * camera->GetView());

			const auto pos = camera->GetPosition();
			camBuffer.position = glm::vec4(pos.x, pos.y, pos.z, 0.f);
			camBuffer.nearPlane = camera->GetNearPlane();
			camBuffer.farPlane = camera->GetFarPlane();

			const float* P = glm::value_ptr(camBuffer.proj);
			const glm::vec4 projInfoPerspective = {
					 2.0f / (P[4 * 0 + 0]),                  // (x) * (R - L)/N
					 2.0f / (P[4 * 1 + 1]),                  // (y) * (T - B)/N
					-(1.0f - P[4 * 2 + 0]) / P[4 * 0 + 0],  // L/N
					-(1.0f + P[4 * 2 + 1]) / P[4 * 1 + 1],  // B/N
			};

			camBuffer.NDCToViewMul = { projInfoPerspective[0], projInfoPerspective[1] };
			camBuffer.NDCToViewAdd = { projInfoPerspective[2], projInfoPerspective[3] };

			myUniformBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::CAMERA_BUFFER, currentIndex)->SetData(&camBuffer, sizeof(CameraData));
		}

		// Update directional light
		{
			myUniformBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::DIRECTIONAL_LIGHT, currentIndex)->SetData(&GetGPUData().directionalLight, sizeof(DirectionalLight));
		}

		// Update Renderer Data
		{
			RendererGPUData data{};
			data.screenTilesCountX = myLightCullingWorkGroups.x;
			data.enableAO = static_cast<uint32_t>(mySettings.enableAO);

			myUniformBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::RENDERER_DATA, currentIndex)->SetData(&data, sizeof(RendererGPUData));
		}
	}

	void SceneRenderer::FetchCommands(Ref<Camera> camera)
	{
		VT_PROFILE_FUNCTION();

		FetchMeshCommands();
		FetchAnimatedMeshCommands();

		auto scenePtr = myScene.lock();
		auto& registry = scenePtr->GetRegistry();

		registry.ForEach<DirectionalLightComponent, TransformComponent>([&](Wire::EntityId id, const DirectionalLightComponent& dirLightComp, const TransformComponent& transformComp)
		{
			if (!transformComp.visible)
			{
				return;
			}

			auto lightEntity = Entity{ id, scenePtr.get() };

			DirectionalLight light{};
			light.colorIntensity = glm::vec4(dirLightComp.color.x, dirLightComp.color.y, dirLightComp.color.z, dirLightComp.intensity);

			const glm::vec3 dir = glm::rotate(lightEntity.GetRotation(), { 0.f, 0.f, 1.f }) * -1.f;
			light.direction = glm::vec4(dir.x, dir.y, dir.z, 1.f);
			light.castShadows = static_cast<uint32_t>(dirLightComp.castShadows);
			light.softShadows = static_cast<uint32_t>(dirLightComp.softShadows);
			light.lightSize = dirLightComp.lightSize;

			if (dirLightComp.castShadows)
			{
				const std::vector<float> cascades = { camera->GetFarPlane() / 50.f, camera->GetFarPlane() / 25.f, camera->GetFarPlane() / 10 };
				const auto lightMatrices = Utility::CalculateCascadeMatrices(camera, light.direction, cascades);

				for (size_t i = 0; i < lightMatrices.size(); i++)
				{
					light.viewProjections[i] = lightMatrices.at(i);
				}

				for (size_t i = 0; i < cascades.size() + 1; i++)
				{
					if (i < cascades.size())
					{
						light.cascadeDistances[i] = cascades.at(i);
					}
					else
					{
						light.cascadeDistances[i] = camera->GetFarPlane();
					}
				}
			}

			GetCPUData().directionalLight = light;
		});

		registry.ForEach<PointLightComponent, TransformComponent>([&](Wire::EntityId id, const PointLightComponent& pointLightComp, const TransformComponent& transformComp)
		{
			if (!transformComp.visible)
			{
				return;
			}

			auto lightEntity = Entity{ id, scenePtr.get() };
			const auto position = lightEntity.GetPosition();

			auto& pointLight = GetCPUData().pointLights.emplace_back();
			pointLight.color = glm::vec4{ pointLightComp.color, 1.f };
			pointLight.falloff = pointLightComp.falloff;
			pointLight.intensity = pointLightComp.intensity;
			pointLight.radius = pointLightComp.radius;
			pointLight.castShadows = (uint32_t)pointLightComp.castShadows;
			pointLight.position = glm::vec4{ position, 1.f };

			if (pointLightComp.castShadows)
			{
				const glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, 1.f, pointLightComp.radius);

				pointLight.viewProjectionMatrices[0] = projection * glm::lookAtLH(position, position + glm::vec3{ 1.f, 0.f, 0.f }, glm::vec3{ 0.f, 1.f, 0.f });
				pointLight.viewProjectionMatrices[1] = projection * glm::lookAtLH(position, position + glm::vec3{ -1.f, 0.f, 0.f }, glm::vec3{ 0.f, 1.f, 0.f });
				pointLight.viewProjectionMatrices[2] = projection * glm::lookAtLH(position, position + glm::vec3{ 0.f, 1.f, 0.f }, glm::vec3{ 0.f, 0.f, -1.f });
				pointLight.viewProjectionMatrices[3] = projection * glm::lookAtLH(position, position + glm::vec3{ 0.f, -1.f, 0.f }, glm::vec3{ 0.f, 0.f, 1.f });
				pointLight.viewProjectionMatrices[4] = projection * glm::lookAtLH(position, position + glm::vec3{ 0.f, 0.f, 1.f }, glm::vec3{ 0.f, 1.f, 0.f });
				pointLight.viewProjectionMatrices[5] = projection * glm::lookAtLH(position, position + glm::vec3{ 0.f, 0.f, -1.f }, glm::vec3{ 0.f, 1.f, 0.f });
			}
		});

		registry.ForEach<SpotLightComponent, TransformComponent>([&](Wire::EntityId id, const SpotLightComponent& spotLightComp, const TransformComponent& transformComp)
		{
			if (!transformComp.visible)
			{
				return;
			}

			auto lightEntity = Entity{ id, scenePtr.get() };

			auto& spotLight = GetCPUData().spotLights.emplace_back();
			spotLight.color = spotLightComp.color;
			spotLight.falloff = spotLightComp.falloff;
			spotLight.intensity = spotLightComp.intensity;
			spotLight.angleAttenuation = spotLightComp.angleAttenuation;
			spotLight.direction = lightEntity.GetForward() * -1.f;
			spotLight.range = spotLightComp.range;
			spotLight.angle = glm::radians(spotLightComp.angle);
			spotLight.castShadows = (uint32_t)spotLightComp.castShadows;

			if (spotLight.castShadows)
			{
				spotLight.projection = glm::perspective(spotLight.angle, 1.f, 0.1f, spotLight.range);
				spotLight.viewProjection = spotLight.projection * glm::lookAt(lightEntity.GetPosition(), lightEntity.GetPosition() + lightEntity.GetForward(), { 0.f, 1.f, 0.f });
			}

			spotLight.position = lightEntity.GetPosition();
		});

		// Sort and setup shadowed lights
		{
			auto& data = GetCPUData();

			std::sort(data.pointLights.begin(), data.pointLights.end(), [&](const auto& lhs, const auto& rhs)
			{
				const float distL = glm::distance2(glm::vec3{ lhs.position }, camera->GetPosition());
				const float distR = glm::distance2(glm::vec3{ rhs.position }, camera->GetPosition());

				return distL < distR;
			});

			std::sort(data.spotLights.begin(), data.spotLights.end(), [&](const auto& lhs, const auto& rhs)
			{
				const float distL = glm::distance2(glm::vec3{ lhs.position }, camera->GetPosition());
				const float distR = glm::distance2(glm::vec3{ rhs.position }, camera->GetPosition());

				return distL < distR;
			});

			for (auto& pointLight : data.pointLights)
			{
				if (data.pointLightShadowCount > MAX_POINT_LIGHT_SHADOWS - 1)
				{
					pointLight.castShadows = false;
				}
				else if (pointLight.castShadows)
				{
					pointLight.shadowMapIndex = data.pointLightShadowCount++;
				}
			}

			for (auto& spotLight : data.spotLights)
			{
				if (data.spotLightShadowCount > MAX_POINT_LIGHT_SHADOWS - 1)
				{
					spotLight.castShadows = false;
				}
				else if (spotLight.castShadows)
				{
					spotLight.shadowMapIndex = data.spotLightShadowCount++;
				}
			}
		}

		registry.ForEach<SphereLightComponent, TransformComponent>([&](Wire::EntityId id, const SphereLightComponent& sphereLightComp, const TransformComponent& transformComp)
		{
			if (!transformComp.visible)
			{
				return;
			}

			auto lightEntity = Entity{ id, scenePtr.get() };

			auto& sphereLight = GetCPUData().sphereLights.emplace_back();
			sphereLight.position = lightEntity.GetPosition();
			sphereLight.color = sphereLightComp.color;
			sphereLight.intensity = sphereLightComp.intensity;
			sphereLight.radius = sphereLightComp.radius;
		});

		registry.ForEach<RectangleLightComponent, TransformComponent>([&](Wire::EntityId id, const RectangleLightComponent& rectangleLightComp, const TransformComponent& transformComp)
		{
			if (!transformComp.visible)
			{
				return;
			}

			auto lightEntity = Entity{ id, scenePtr.get() };
			const glm::vec3 dir = glm::rotate(lightEntity.GetRotation(), { 0.f, 0.f, 1.f });
			const glm::vec3 left = glm::rotate(lightEntity.GetRotation(), { -1.f, 0.f, 0.f });
			const glm::vec3 up = glm::rotate(lightEntity.GetRotation(), { 0.f, 1.f, 0.f });

			const auto scale = lightEntity.GetScale();

			auto& rectangleLight = GetCPUData().rectangleLights.emplace_back();
			rectangleLight.position = lightEntity.GetPosition();
			rectangleLight.color = rectangleLightComp.color;
			rectangleLight.intensity = rectangleLightComp.intensity;
			rectangleLight.width = rectangleLightComp.width * scale.x;
			rectangleLight.height = rectangleLightComp.height * scale.y;
			rectangleLight.direction = dir;
			rectangleLight.up = up;
			rectangleLight.left = left;
		});

		registry.ForEach<SkylightComponent, TransformComponent>([&](Wire::EntityId id, SkylightComponent& comp, const TransformComponent& transformComp)
		{
			if (!transformComp.visible)
			{
				myEnvironmentSettings.intensity = 0.f;
				return;
			}

			if (comp.environmentHandle != Asset::Null())
			{
				if (comp.environmentHandle != comp.lastEnvironmentHandle)
				{
					comp.lastEnvironmentHandle = comp.environmentHandle;
					comp.currentSceneEnvironment = Renderer::GenerateEnvironmentMap(comp.environmentHandle);

					myEnvironmentSettings.irradianceMap = comp.currentSceneEnvironment.irradianceMap;
					myEnvironmentSettings.radianceMap = comp.currentSceneEnvironment.radianceMap;
				}

				myEnvironmentSettings.irradianceMap = comp.currentSceneEnvironment.irradianceMap;
				myEnvironmentSettings.radianceMap = comp.currentSceneEnvironment.radianceMap;
			}
			else
			{
				myEnvironmentSettings.irradianceMap = nullptr;
				myEnvironmentSettings.radianceMap = nullptr;
			}

			myEnvironmentSettings.azimuth = comp.azimuth;
			myEnvironmentSettings.inclination = comp.inclination;
			myEnvironmentSettings.turbidity = comp.turbidity;
			myEnvironmentSettings.intensity = comp.intensity;
		});

		registry.ForEach<DecalComponent, TransformComponent, EntityDataComponent>([&](Wire::EntityId id, const DecalComponent& decalComp, const TransformComponent& transComp, const EntityDataComponent& dataComp)
		{
			if (!transComp.visible)
			{
				return;
			}

			Ref<Material> material = AssetManager::QueueAsset<Material>(decalComp.materialHandle);
			if (!material || !material->IsValid())
			{
				return;
			}

			Entity entity{ id, scenePtr.get() };

			auto& newCmd = GetCPUData().decalCommands.emplace_back();
			newCmd.material = material;
			newCmd.transform = entity.GetTransform();
			newCmd.randomValue = dataComp.randomValue;
			newCmd.timeSinceCreation = dataComp.timeSinceCreation;
		});

		registry.ForEach<PostProcessingStackComponent>([&](Wire::EntityId id, const PostProcessingStackComponent& comp)
		{
			Ref<PostProcessingStack> stack = AssetManager::GetAsset<PostProcessingStack>(comp.postProcessingStack);
			if (!stack || !stack->IsValid())
			{
				return;
			}

			GetCPUData().postProcessingStack = stack;
		});

		registry.ForEach<GTAOEffectComponent>([&](Wire::EntityId id, const GTAOEffectComponent& comp)
		{
			myGTAOSettings.radius = comp.radius;
			myGTAOSettings.radiusMultiplier = comp.radiusMultiplier;
			myGTAOSettings.falloffRange = comp.falloffRange;
			myGTAOSettings.finalValuePower = comp.finalValuePower;
		});

		registry.ForEach<GlobalFogComponent>([&](Wire::EntityId	id, const GlobalFogComponent& comp)
		{
			myVolumetricFogSettings.anisotropy = comp.anisotropy;
			myVolumetricFogSettings.density = comp.density;
			myVolumetricFogSettings.globalColor = comp.globalColor;
			myVolumetricFogSettings.globalDensity = comp.globalDensity;
		});

		registry.ForEach<TextRendererComponent>([&](Wire::EntityId id, const TextRendererComponent& comp)
		{
			Ref<Font> fontAsset = AssetManager::GetAsset<Font>(comp.fontHandle);
			if (!fontAsset || !fontAsset->IsValid())
			{
				return;
			}

			Entity entity{ id, scenePtr.get() };

			TextCommand textCmd{};
			textCmd.text = comp.text;
			textCmd.font = fontAsset;
			textCmd.transform = entity.GetTransform();
			textCmd.maxWidth = comp.maxWidth;
			textCmd.color = comp.color;

			SubmitString(textCmd);
		});

		if (mySettings.enableDebugRenderer)
		{
			DebugRenderer::Execute(GetCPUData());
		}
	}

	void SceneRenderer::FetchMeshCommands()
	{
		VT_PROFILE_FUNCTION();

		auto scenePtr = myScene.lock();
		auto& registry = scenePtr->GetRegistry();
		auto& data = GetCPUData();

		const auto& meshComponentView = (myShouldHideMeshes) ? std::vector<Wire::EntityId>() : registry.GetSingleComponentView<MeshComponent>();
		data.submitCommands.reserve(meshComponentView.size());

		const uint32_t hardwareConcurrency = std::thread::hardware_concurrency() * 2;
		const uint32_t threadCount = std::max(std::min(static_cast<uint32_t>(meshComponentView.size()), hardwareConcurrency), 1u);
		const uint32_t meshesPerThread = static_cast<uint32_t>(meshComponentView.size()) / threadCount;

		std::vector<uint32_t> perThreadStartIndices;
		std::vector<std::vector<SubmitCommand>> perThreadSubmitCommands;
		std::atomic_uint32_t totalVertexColorCount = 0;
		std::atomic_uint64_t totalTriangleCount = 0;

		for (uint32_t currentMeshCount = 0, i = 0; i < threadCount; i++)
		{
			perThreadStartIndices.emplace_back(currentMeshCount);

			auto& cmdVector = perThreadSubmitCommands.emplace_back();
			cmdVector.reserve(meshesPerThread);

			currentMeshCount += meshesPerThread;
		}

		auto submitFunc = [&](uint32_t currentIndex, uint32_t threadIndex)
		{
			Wire::EntityId id = meshComponentView.at(currentIndex);
			Entity entity{ id, scenePtr.get() };

			const MeshComponent& meshComp = entity.GetComponent<MeshComponent>();
			const TransformComponent& transComp = entity.GetComponent<TransformComponent>();
			const EntityDataComponent& dataComp = entity.GetComponent<EntityDataComponent>();

			if (meshComp.handle == Asset::Null() || !transComp.visible)
			{
				return;
			}

			Ref<Mesh> mesh = AssetManager::QueueAsset<Mesh>(meshComp.handle);
			if (!mesh || !mesh->IsValid())
			{
				return;
			}

			AssetHandle materialHandle = Asset::Null();
			if (meshComp.overrideMaterial != Asset::Null())
			{
				materialHandle = meshComp.overrideMaterial;
			}
			else
			{
				materialHandle = mesh->GetMaterial()->handle;
			}

			glm::mat4 transform = scenePtr->GetWorldSpaceTransform(entity);

			auto& submitCommands = perThreadSubmitCommands.at(threadIndex);

			const bool hasVertexPaintedComponent = entity.HasComponent<VertexPaintedComponent>();

			totalTriangleCount += mesh->GetIndexCount() / 3;

			for (uint32_t i = 0; const auto & subMesh : mesh->GetSubMeshes())
			{
				auto& cmd = submitCommands.emplace_back();
				cmd.mesh = mesh;

				Ref<Material> material = AssetManager::GetAsset<Material>(materialHandle);
				if (material && material->IsValid())
				{
					cmd.material = (meshComp.subMaterialIndex != -1) ? material->TryGetSubMaterialAt(meshComp.subMaterialIndex) : material->TryGetSubMaterialAt(subMesh.materialIndex);
				}
				else
				{
					cmd.material = Renderer::GetDefaultData().defaultMaterial;
				}

				cmd.subMesh = subMesh;
				cmd.transform = transform * subMesh.transform;
				cmd.subMeshIndex = i;
				cmd.id = id;
				cmd.timeSinceCreation = dataComp.timeSinceCreation;
				cmd.randomValue = dataComp.randomValue;

				if (hasVertexPaintedComponent)
				{
					cmd.vertexColors = entity.GetComponent<VertexPaintedComponent>().vertexColors;
					totalVertexColorCount += static_cast<uint32_t>(entity.GetComponent<VertexPaintedComponent>().vertexColors.size());
				}

				i++;
			}
		};

		std::vector<std::future<void>> threadFutures;
		auto& threadPool = Application::GetThreadPool();

		for (uint32_t i = 0; i < threadCount; i++)
		{
			const uint32_t startIndex = perThreadStartIndices.at(i);
			uint32_t meshCount = meshesPerThread;

			if (i == threadCount - 1)
			{
				meshCount = static_cast<uint32_t>(meshComponentView.size()) - startIndex;
			}

			threadFutures.emplace_back(threadPool.SubmitTask([=]()
			{
				for (uint32_t j = 0; j < meshCount; j++)
				{
					submitFunc(j + startIndex, i);
				}
			}));
		}

		// Wait for threads

		{
			VT_PROFILE_SCOPE("Wait for threads");
			for (auto& future : threadFutures)
			{
				future.get();
			}
		}

		{
			VT_PROFILE_SCOPE("Collect commands");
			for (const auto& cmds : perThreadSubmitCommands)
			{
				data.submitCommands.insert(data.submitCommands.end(), cmds.begin(), cmds.end());
			}
		}

		data.vertexColorCount += totalVertexColorCount;
		myStatistics.triangleCount += totalTriangleCount;
	}

	void SceneRenderer::FetchAnimatedMeshCommands()
	{
		VT_PROFILE_FUNCTION();

		auto scenePtr = myScene.lock();
		auto& registry = scenePtr->GetRegistry();
		registry.ForEach<AnimationControllerComponent, TransformComponent, EntityDataComponent>([&](Wire::EntityId id, AnimationControllerComponent& animComp, const TransformComponent& transformComp, const EntityDataComponent& dataComp)
		{
			if (!transformComp.visible)
			{
				return;
			}

			if (animComp.controller)
			{
				const auto characterHandle = animComp.controller->GetGraph()->GetCharacterHandle();

				const auto character = AssetManager::QueueAsset<AnimatedCharacter>(animComp.controller->GetGraph()->GetCharacterHandle());
				if (!character || !character->IsValid())
				{
					return;
				}

				auto entity = Entity(id, scenePtr.get());

				const auto anim = animComp.controller->Sample();

				if (animComp.applyRootMotion)
				{
					const auto rootMotion = animComp.controller->GetRootMotion();
					entity.SetLocalPosition(entity.GetLocalPosition() + rootMotion.position);
				}

				auto skin = character->GetSkin();
				if (animComp.overrideSkin != Asset::Null())
				{
					Ref<Mesh> overrideSkin = AssetManager::GetAsset<Mesh>(animComp.overrideSkin);
					if (overrideSkin && overrideSkin->IsValid())
					{
						skin = overrideSkin;
					}
				}

				if (!skin || !skin->IsValid())
				{
					return;
				}

				Ref<Material> material = skin->GetMaterial();
				if (animComp.overrideMaterial != Asset::Null())
				{
					Ref<Material> overrideMaterial = AssetManager::GetAsset<Material>(animComp.overrideMaterial);
					if (overrideMaterial && overrideMaterial->IsValid())
					{
						material = overrideMaterial;
					}
				}

				myStatistics.triangleCount += skin->GetIndexCount() / 3;

				const glm::mat4 transform = scenePtr->GetWorldSpaceTransform(entity);
				SubmitMesh(skin, material, transform, anim, dataComp.timeSinceCreation, dataComp.randomValue, id);
			}
			else
			{
				auto animGraph = AssetManager::QueueAsset<AnimationGraphAsset>(animComp.animationGraph);

				if (!animGraph || !animGraph->IsValid())
				{
					return;
				}

				auto character = AssetManager::QueueAsset<AnimatedCharacter>(animGraph->GetCharacterHandle());
				if (!character || !character->IsValid())
				{
					return;
				}

				auto skin = character->GetSkin();
				if (!skin || !skin->IsValid())
				{
					return;
				}

				auto entity = Entity(id, scenePtr.get());
				myStatistics.triangleCount += skin->GetIndexCount() / 3;

				Ref<Material> material = skin->GetMaterial();
				if (animComp.overrideMaterial != Asset::Null())
				{
					Ref<Material> overrideMaterial = AssetManager::GetAsset<Material>(animComp.overrideMaterial);
					if (overrideMaterial && overrideMaterial->IsValid())
					{
						material = overrideMaterial;
					}
				}

				const glm::mat4 transform = scenePtr->GetWorldSpaceTransform(entity);
				SubmitMesh(skin, material, transform, dataComp.timeSinceCreation, dataComp.randomValue, id);
			}
		});

		registry.ForEach<AnimatedCharacterComponent, TransformComponent, EntityDataComponent>([&](Wire::EntityId id, const AnimatedCharacterComponent& charComp, const TransformComponent& transformComp, const EntityDataComponent& dataComp)
		{
			if (!transformComp.visible)
			{
				return;
			}

			Ref<AnimatedCharacter> character = AssetManager::QueueAsset<AnimatedCharacter>(charComp.animatedCharacter);

			if (!character || !character->IsValid())
			{
				return;
			}

			if (!character->GetSkin() || !character->GetSkeleton())
			{
				return;
			}

			Volt::Entity ent{ id, scenePtr.get() };
			std::vector<glm::mat4> anim{};

			if (charComp.selectedFrame != -1)
			{
				anim = character->SampleAnimation(charComp.currentAnimation, (uint32_t)charComp.selectedFrame);
			}
			else if (charComp.isPlaying)
			{
				anim = character->SampleAnimation(charComp.currentAnimation, charComp.currentStartTime, charComp.isLooping);
			}

			if (!anim.empty())
			{
				for (const auto& [attachmentId, attachedEntities] : charComp.attachedEntities)
				{
					auto attachment = character->GetJointAttachmentFromID(attachmentId);
					if (attachment.jointIndex == -1)
					{
						continue;
					}

					const glm::mat4 offsetTransform = glm::translate({ 1.f }, attachment.positionOffset) * glm::mat4_cast(attachment.rotationOffset);

					glm::vec3 t, s;
					glm::quat q;
					Math::Decompose(anim[attachment.jointIndex] * offsetTransform * glm::inverse(character->GetSkeleton()->GetInverseBindPose().at(attachment.jointIndex)), t, q, s);

					for (auto attachedEntity : attachedEntities)
					{
						attachedEntity.SetLocalPosition(t);
						attachedEntity.SetLocalRotation(q);
					}
				}
			}

			SubmitMesh(character->GetSkin(), character->GetSkin()->GetMaterial(), ent.GetTransform(), anim, dataComp.timeSinceCreation, dataComp.randomValue, id);
		});
	}

	void SceneRenderer::FetchParticles(Ref<Camera> camera)
	{
		VT_PROFILE_FUNCTION();

		auto scenePtr = myScene.lock();

		const auto& particleStorage = scenePtr->GetParticleSystem().GetParticleStorage();
		for (const auto& [entityId, storage] : particleStorage)
		{
			const auto preset = AssetManager::GetAsset<ParticlePreset>(storage.preset);
			if (!preset || !preset->IsValid())
			{
				continue;
			}

			const auto material = AssetManager::GetAsset<Material>(preset->material);

			Entity entity{ entityId, scenePtr.get() };

			if (preset->type == ParticlePreset::eType::PARTICLE)
			{
				auto& particleBatch = GetCPUData().particleBatches.emplace_back();
				particleBatch.particles.reserve(storage.numberOfAliveParticles);
				particleBatch.material = (material && material->IsValid()) ? material : myDefaultParticleMaterial;
				particleBatch.emitterPosition = entity.GetPosition();

				GetCPUData().particleCount += storage.numberOfAliveParticles;

				for (int32_t i = 0; i < storage.numberOfAliveParticles; i++)
				{
					const auto& particle = storage.particles.at(i);

					if (particle.dead)
					{
						continue;
					}

					auto& particleToRender = particleBatch.particles.emplace_back();
					particleToRender.position = particle.position;
					particleToRender.scale = particle.size;
					particleToRender.color = particle.color;
					particleToRender.randomValue = particle.randomValue;
					particleToRender.timeSinceSpawn = particle.timeSinceSpawn;

					auto texture = AssetManager::GetAsset<Volt::Texture2D>(preset->texture);

					if (texture && texture->IsValid())
					{
						particleToRender.albedoIndex = Renderer::GetBindlessData().textureTable->GetBindingFromTexture(texture->GetImage());
					}
					else
					{
						particleToRender.albedoIndex = 0;
					}
				}
			}
			else if (preset->type == ParticlePreset::eType::MESH)
			{
				const auto mesh = AssetManager::GetAsset<Mesh>(preset->mesh);

				if (!mesh || !mesh->IsValid())
				{
					return;
				}

				for (int32_t i = 0; i < storage.numberOfAliveParticles; i++)
				{
					const auto& particle = storage.particles.at(i);
					const auto particleTransform = glm::translate(glm::mat4(1.f), particle.position) * glm::mat4_cast(glm::quat(particle.rotation)) * glm::scale(glm::mat4(1.f), particle.size);

					if (material && material->IsValid())
					{
						SubmitMesh(mesh, material, particleTransform, 0.f, 0.f, 0);
					}
					else
					{
						SubmitMesh(mesh, mesh->GetMaterial(), particleTransform, 0.f, 0.f, 0);
					}
				}
			}
		}
	}

	void SceneRenderer::DispatchText(Ref<CommandBuffer> cmdBuffer)
	{
		VT_PROFILE_FUNCTION();

		auto& textData = myTextData;

		if (textData.indexCount == 0)
		{
			return;
		}

		const uint32_t currentIndex = cmdBuffer->GetCurrentIndex();
		auto vertexBuffer = textData.vertexBuffer->Get(currentIndex);

		uint32_t dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(textData.vertexBufferPtr) - reinterpret_cast<uint8_t*>(textData.vertexBufferBase));
		vertexBuffer->SetDataMapped(cmdBuffer->GetCurrentCommandBuffer(), textData.vertexBufferBase, dataSize);

		const glm::mat4 viewProjection = myCurrentCamera->GetProjection() * myCurrentCamera->GetView();
		Renderer::DrawIndexedVertexBuffer(cmdBuffer, textData.indexCount, vertexBuffer, textData.indexBuffer, textData.renderPipeline, &viewProjection, sizeof(glm::mat4));
	}

	inline static bool NextLine(int32_t aIndex, const std::vector<int32_t>& aLines)
	{
		for (int32_t line : aLines)
		{
			if (line == aIndex)
			{
				return true;
			}
		}

		return false;
	}

	void SceneRenderer::SubmitString(const TextCommand& cmd)
	{
		VT_PROFILE_FUNCTION();

		auto& textData = myTextData;

		std::u32string utf32string = Utils::To_UTF32(cmd.text);
		Ref<Texture2D> fontAtlas = cmd.font->GetAtlas();

		if (!fontAtlas)
		{
			return;
		}

		uint32_t textureIndex = Renderer::GetBindlessData().textureTable->GetBindingFromTexture(fontAtlas->GetImage());
		if (textureIndex == 0)
		{
			textureIndex = Renderer::GetBindlessData().textureTable->AddTexture(fontAtlas->GetImage());
		}

		auto& fontGeom = cmd.font->GetMSDFData()->fontGeometry;
		const auto& metrics = fontGeom.getMetrics();

		std::vector<int32_t> nextLines;

		// Find new lines
		{
			double x = 0.0;
			double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
			double y = -fsScale * metrics.ascenderY;

			int32_t lastSpace = -1;

			for (int32_t i = 0; i < utf32string.size(); i++)
			{
				char32_t character = utf32string[i];
				if (character == '\n')
				{
					x = 0.0;
					y -= fsScale * metrics.lineHeight;
					continue;
				}

				auto glyph = fontGeom.getGlyph(character);
				if (!glyph)
				{
					glyph = fontGeom.getGlyph('?');
				}

				if (!glyph)
				{
					continue;
				}

				if (character != ' ')
				{
					// Calculate geometry
					double pl, pb, pr, pt;
					glyph->getQuadPlaneBounds(pl, pb, pr, pt);
					glm::vec2 quadMin((float)pl, (float)pb);
					glm::vec2 quadMax((float)pl, (float)pb);

					quadMin *= (float)fsScale;
					quadMax *= (float)fsScale;
					quadMin += glm::vec2((float)x, (float)y);
					quadMax += glm::vec2((float)x, (float)y);

					if (quadMax.x > cmd.maxWidth && lastSpace != -1)
					{
						i = lastSpace;
						nextLines.emplace_back(lastSpace);
						lastSpace = -1;
						x = 0.0;
						y -= fsScale * metrics.lineHeight;
					}
				}
				else
				{
					lastSpace = i;
				}

				double advance = glyph->getAdvance();
				fontGeom.getAdvance(advance, character, utf32string[i + 1]);
				x += fsScale * advance;
			}
		}

		// Setup vertices
		{
			double x = 0.0;
			double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
			double y = 0.0;

			for (int32_t i = 0; i < utf32string.size(); i++)
			{
				char32_t character = utf32string[i];
				if (character == '\n' || NextLine(i, nextLines))
				{
					x = 0.0;
					y -= fsScale * metrics.lineHeight;
					continue;
				}

				auto glyph = fontGeom.getGlyph(character);

				if (!glyph)
				{
					glyph = fontGeom.getGlyph('?');
				}

				if (!glyph)
				{
					continue;
				}

				double l, b, r, t;
				glyph->getQuadAtlasBounds(l, b, r, t);

				double pl, pb, pr, pt;
				glyph->getQuadPlaneBounds(pl, pb, pr, pt);

				pl *= fsScale, pb *= fsScale, pr *= fsScale, pt *= fsScale;
				pl += x, pb += y, pr += x, pt += y;

				double texelWidth = 1.0 / fontAtlas->GetWidth();
				double texelHeight = 1.0 / fontAtlas->GetHeight();

				l *= texelWidth, b *= texelHeight, r *= texelWidth, t *= texelHeight;

				textData.vertexBufferPtr->position = cmd.transform * glm::vec4{ (float)pl, (float)pb, 0.f, 1.f };
				textData.vertexBufferPtr->color = cmd.color;
				textData.vertexBufferPtr->texCoords = { (float)l, (float)b };
				textData.vertexBufferPtr->textureIndex = (uint32_t)textureIndex;
				textData.vertexBufferPtr++;

				textData.vertexBufferPtr->position = cmd.transform * glm::vec4{ (float)pr, (float)pb, 0.f, 1.f };
				textData.vertexBufferPtr->color = cmd.color;
				textData.vertexBufferPtr->texCoords = { (float)r, (float)b };
				textData.vertexBufferPtr->textureIndex = (uint32_t)textureIndex;
				textData.vertexBufferPtr++;

				textData.vertexBufferPtr->position = cmd.transform * glm::vec4{ (float)pr, (float)pt, 0.f, 1.f };
				textData.vertexBufferPtr->color = cmd.color;
				textData.vertexBufferPtr->texCoords = { (float)r, (float)t };
				textData.vertexBufferPtr->textureIndex = (uint32_t)textureIndex;
				textData.vertexBufferPtr++;

				textData.vertexBufferPtr->position = cmd.transform * glm::vec4{ (float)pl, (float)pt, 0.f, 1.f };
				textData.vertexBufferPtr->color = cmd.color;
				textData.vertexBufferPtr->texCoords = { (float)l, (float)t };
				textData.vertexBufferPtr->textureIndex = (uint32_t)textureIndex;
				textData.vertexBufferPtr++;

				textData.indexCount += 6;

				double advance = glyph->getAdvance();
				fontGeom.getAdvance(advance, character, utf32string[i + 1]);
				x += fsScale * advance;
			}
		}
	}

	void SceneRenderer::ResizeBuffersIfNeeded()
	{
		const auto& data = GetGPUData();
		const uint32_t currentIndex = myCommandBuffer->GetCurrentIndex();

		///// Objects /////
		{
			auto argsSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::MAIN_INDIRECT_ARGS, currentIndex);
			auto objectSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::OBJECT_DATA, currentIndex);
			auto animSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::ANIMATION_DATA, currentIndex);
			auto colorSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::PAINTED_VERTEX_COLORS, currentIndex);

			if (argsSSBO->GetElementCount() < data.submitCommands.size())
			{
				argsSSBO->ResizeWithElementCount((uint32_t)data.submitCommands.size());
				UpdateGlobalDescriptorSet(myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS), Sets::RENDERER_BUFFERS, Bindings::MAIN_INDIRECT_ARGS, currentIndex, myShaderStorageBufferSet);
			}

			if (objectSSBO->GetElementCount() < data.submitCommands.size())
			{
				objectSSBO->ResizeWithElementCount((uint32_t)data.submitCommands.size());
				UpdateGlobalDescriptorSet(myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS), Sets::RENDERER_BUFFERS, Bindings::OBJECT_DATA, currentIndex, myShaderStorageBufferSet);
			}

			if (animSSBO->GetElementCount() < data.animationBoneCount)
			{
				animSSBO->ResizeWithElementCount(data.animationBoneCount);
				UpdateGlobalDescriptorSet(myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS), Sets::RENDERER_BUFFERS, Bindings::ANIMATION_DATA, currentIndex, myShaderStorageBufferSet);
			}

			if (colorSSBO->GetElementCount() < data.vertexColorCount)
			{
				colorSSBO->ResizeWithElementCount(data.vertexColorCount);
				UpdateGlobalDescriptorSet(myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS), Sets::RENDERER_BUFFERS, Bindings::PAINTED_VERTEX_COLORS, currentIndex, myShaderStorageBufferSet);
			}

			for (const auto& pass : myIndirectPasses)
			{
				auto countSSBO = pass.drawCountIDStorageBuffer->Get(Sets::DRAW_BUFFERS, Bindings::INDIRECT_COUNTS, currentIndex);
				auto drawToObjectSSBO = pass.drawCountIDStorageBuffer->Get(Sets::DRAW_BUFFERS, Bindings::DRAW_TO_OBJECT_ID, currentIndex);

				if (countSSBO->GetElementCount() < data.indirectBatches.size())
				{
					countSSBO->ResizeWithElementCount((uint32_t)data.indirectBatches.size());
					UpdateGlobalDescriptorSet(pass.drawBuffersSet, Sets::DRAW_BUFFERS, Bindings::INDIRECT_COUNTS, currentIndex, pass.drawCountIDStorageBuffer);
				}

				if (drawToObjectSSBO->GetElementCount() < data.submitCommands.size())
				{
					drawToObjectSSBO->ResizeWithElementCount((uint32_t)data.submitCommands.size());
					UpdateGlobalDescriptorSet(pass.drawBuffersSet, Sets::DRAW_BUFFERS, Bindings::DRAW_TO_OBJECT_ID, currentIndex, pass.drawCountIDStorageBuffer);
				}
			}
		}

		///// Lights /////
		{
			auto pointLightSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::POINT_LIGHTS, currentIndex);
			auto spotLightSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::SPOT_LIGHTS, currentIndex);
			auto sphereLightsSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::SPHERE_LIGHTS, currentIndex);
			auto rectangleLightsSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::RECTANGLE_LIGHTS, currentIndex);

			if (pointLightSSBO->GetElementCount() < data.pointLights.size())
			{
				pointLightSSBO->ResizeWithElementCount((uint32_t)data.pointLights.size());
				UpdateGlobalDescriptorSet(myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS), Sets::RENDERER_BUFFERS, Bindings::POINT_LIGHTS, currentIndex, myShaderStorageBufferSet);
			}

			if (spotLightSSBO->GetElementCount() < data.spotLights.size())
			{
				spotLightSSBO->ResizeWithElementCount((uint32_t)data.spotLights.size());
				UpdateGlobalDescriptorSet(myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS), Sets::RENDERER_BUFFERS, Bindings::SPOT_LIGHTS, currentIndex, myShaderStorageBufferSet);
			}

			if (sphereLightsSSBO->GetElementCount() < data.sphereLights.size())
			{
				sphereLightsSSBO->ResizeWithElementCount((uint32_t)data.sphereLights.size());
				UpdateGlobalDescriptorSet(myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS), Sets::RENDERER_BUFFERS, Bindings::SPHERE_LIGHTS, currentIndex, myShaderStorageBufferSet);
			}

			if (rectangleLightsSSBO->GetElementCount() < data.rectangleLights.size())
			{
				rectangleLightsSSBO->ResizeWithElementCount((uint32_t)data.rectangleLights.size());
				UpdateGlobalDescriptorSet(myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS), Sets::RENDERER_BUFFERS, Bindings::RECTANGLE_LIGHTS, currentIndex, myShaderStorageBufferSet);
			}
		}

		// Other
		{
			auto particleSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::PARTICLE_INFO, currentIndex);

			if (particleSSBO->GetElementCount() < data.particleCount)
			{
				particleSSBO->ResizeWithElementCount((uint32_t)data.particleCount);
				UpdateGlobalDescriptorSet(myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS), Sets::RENDERER_BUFFERS, Bindings::PARTICLE_INFO, currentIndex, myShaderStorageBufferSet);
			}

			if (myResizeLightCullingBuffers.at(currentIndex))
			{
				myResizeLightCullingBuffers[currentIndex] = false;

				auto visiblePointLightsSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::VISIBLE_POINT_LIGHTS, currentIndex);
				auto visibleSpotLightsSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::VISIBLE_SPOT_LIGHTS, currentIndex);

				visiblePointLightsSSBO->ResizeWithElementCount(MAX_POINT_LIGHTS * myLightCullingWorkGroups.x * myLightCullingWorkGroups.y);
				UpdateGlobalDescriptorSet(myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS), Sets::RENDERER_BUFFERS, Bindings::VISIBLE_POINT_LIGHTS, currentIndex, myShaderStorageBufferSet);

				visibleSpotLightsSSBO->ResizeWithElementCount(MAX_SPOT_LIGHTS * myLightCullingWorkGroups.x * myLightCullingWorkGroups.y);
				UpdateGlobalDescriptorSet(myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS), Sets::RENDERER_BUFFERS, Bindings::VISIBLE_SPOT_LIGHTS, currentIndex, myShaderStorageBufferSet);
			}
		}
	}

	void SceneRenderer::SortSubmitCommands()
	{
		VT_PROFILE_FUNCTION();

		std::sort(std::execution::par, GetGPUData().submitCommands.begin(), GetGPUData().submitCommands.end(), [](const auto& lhs, const auto& rhs)
		{
			if (lhs.material < rhs.material)
			{
				return true;
			}
			else if (lhs.material > rhs.material)
			{
				return false;
			}

			if (lhs.subMesh < rhs.subMesh)
			{
				return true;
			}
			else if (lhs.subMesh > rhs.subMesh)
			{
				return false;
			}

			return false;
		});
	}

	void SceneRenderer::PrepareForIndirectDraw()
	{
		VT_PROFILE_FUNCTION();
		auto& data = GetGPUData();
		if (data.submitCommands.empty())
		{
			return;
		}

		data.indirectBatches.clear();

		IndirectBatch& firstBatch = data.indirectBatches.emplace_back();
		firstBatch.material = data.submitCommands.at(0).material;
		firstBatch.subMesh = data.submitCommands.at(0).subMesh;
		firstBatch.mesh = data.submitCommands.at(0).mesh;
		firstBatch.first = 0;
		firstBatch.count = 1;
		firstBatch.id = 0;

		for (size_t i = 1; i < data.submitCommands.size(); i++)
		{
			const bool sameSubMesh = (data.submitCommands[i].subMesh == data.indirectBatches.back().subMesh);
			const bool sameMaterial = (data.submitCommands[i].material == data.indirectBatches.back().material);
			const bool sameMesh = (data.submitCommands[i].mesh == data.indirectBatches.back().mesh);

			if (sameMaterial && sameSubMesh && sameMesh)
			{
				data.indirectBatches.back().count++;
			}
			else
			{
				IndirectBatch& newBatch = data.indirectBatches.emplace_back();
				newBatch.material = data.submitCommands.at(i).material;
				newBatch.subMesh = data.submitCommands.at(i).subMesh;
				newBatch.mesh = data.submitCommands.at(i).mesh;
				newBatch.first = (uint32_t)i;
				newBatch.count = 1;
				newBatch.id = uint32_t(data.indirectBatches.size() - 1);
			}

			data.submitCommands[i].batchId = data.indirectBatches.back().id;
		}
	}

	void SceneRenderer::PrepareLineBuffer()
	{
		VT_PROFILE_FUNCTION();

		{
			VT_PROFILE_SCOPE("Collect data");

			for (const auto& cmd : GetGPUData().lineCommands)
			{
				myLineData.vertexBufferPtr->position = { cmd.startPosition.x, cmd.startPosition.y, cmd.startPosition.z, 1.f };
				myLineData.vertexBufferPtr->color = cmd.color;
				myLineData.vertexBufferPtr++;

				myLineData.vertexBufferPtr->position = { cmd.endPosition.x, cmd.endPosition.y, cmd.endPosition.z, 1.f };
				myLineData.vertexBufferPtr->color = cmd.color;
				myLineData.vertexBufferPtr++;

				myLineData.vertexCount += 2;
			}
		}

		if (myLineData.vertexCount > 0)
		{
			myLineData.vertexBuffer->Get(myCommandBuffer->GetCurrentIndex())->SetData(myCommandBuffer->GetCurrentCommandBuffer(), myLineData.vertexBufferBase, sizeof(LineVertex) * myLineData.vertexCount);
		}
	}

	void SceneRenderer::PrepareBillboardBuffer()
	{
		VT_PROFILE_FUNCTION();

		{
			VT_PROFILE_SCOPE("Collect data");

			for (const auto& cmd : GetGPUData().billboardCommands)
			{
				myBillboardData.vertexBufferPtr->postition = { cmd.position.x, cmd.position.y, cmd.position.z, 1.f };
				myBillboardData.vertexBufferPtr->color = cmd.color;
				myBillboardData.vertexBufferPtr->scale = cmd.scale;
				myBillboardData.vertexBufferPtr->id = cmd.id;
				myBillboardData.vertexBufferPtr->textureIndex = cmd.texture ? Renderer::GetBindlessData().textureTable->GetBindingFromTexture(cmd.texture->GetImage()) : 0;
				myBillboardData.vertexBufferPtr++;

				myBillboardData.vertexCount++;
			}
		}

		if (myBillboardData.vertexCount > 0)
		{
			myBillboardData.vertexBuffer->Get(myCommandBuffer->GetCurrentIndex())->SetData(myCommandBuffer->GetCurrentCommandBuffer(), myBillboardData.vertexBufferBase, sizeof(BillboardVertex) * myBillboardData.vertexCount);
		}
	}

	void SceneRenderer::UploadIndirectDrawCommands()
	{
		VT_PROFILE_FUNCTION();

		auto& data = GetGPUData();
		if (data.submitCommands.empty())
		{
			return;
		}

		const uint32_t currentIndex = myCommandBuffer->GetCurrentIndex();

		// Fill indirect commands
		{
			auto argsSSBO = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::MAIN_INDIRECT_ARGS, currentIndex);
			auto* drawCommands = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::MAIN_INDIRECT_ARGS, currentIndex)->Map<IndirectGPUCommand>();

			for (size_t i = 0; i < data.submitCommands.size(); i++)
			{
				auto& cmd = data.submitCommands.at(i);

				drawCommands[i].command.indexCount = cmd.subMesh.indexCount;
				drawCommands[i].command.firstIndex = cmd.subMesh.indexStartOffset + cmd.mesh->GetIndexStartOffset();
				drawCommands[i].command.vertexOffset = cmd.subMesh.vertexStartOffset + cmd.mesh->GetVertexStartOffset();
				drawCommands[i].command.firstInstance = data.indirectBatches.at(cmd.batchId).first;
				drawCommands[i].command.instanceCount = 1;
				drawCommands[i].batchId = cmd.batchId;
				drawCommands[i].objectId = (uint32_t)i;
			}

			myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::MAIN_INDIRECT_ARGS, currentIndex)->Unmap();
		}
	}

	void SceneRenderer::UploadObjectData()
	{
		VT_PROFILE_FUNCTION();

		auto& data = GetGPUData();
		const uint32_t currentIndex = myCommandBuffer->GetCurrentIndex();
		std::unordered_map<uint32_t, uint32_t> boneOffsets{};
		std::unordered_map<uint32_t, uint32_t> colorOffsets{};

		// Animation data
		{
			VT_PROFILE_SCOPE("Upload Animation Data");

			uint32_t currentBoneOffset = 0;

			auto ssbo = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::ANIMATION_DATA, currentIndex);
			glm::mat4* animationData = ssbo->Map<glm::mat4>();

			for (uint32_t i = 0; const auto & cmd : data.submitCommands)
			{
				if (!cmd.boneTransforms.empty())
				{
					memcpy_s(&animationData[currentBoneOffset], ssbo->GetElementCount() * sizeof(glm::mat4), cmd.boneTransforms.data(), sizeof(glm::mat4) * cmd.boneTransforms.size());

					boneOffsets[i] = currentBoneOffset;
					currentBoneOffset += (uint32_t)cmd.boneTransforms.size();
				}

				i++;
			}

			ssbo->Unmap();
		}

		// Vertex colors
		{
			VT_PROFILE_SCOPE("Upload Vertex Color Data");

			uint32_t currentColorOffset = 0;

			auto ssbo = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::PAINTED_VERTEX_COLORS, currentIndex);
			uint32_t* colorData = ssbo->Map<uint32_t>();

			for (uint32_t i = 0; const auto & cmd : data.submitCommands)
			{
				if (!cmd.vertexColors.empty())
				{
					memcpy_s(&colorData[currentColorOffset], ssbo->GetElementCount() * sizeof(uint32_t), cmd.vertexColors.data(), sizeof(uint32_t) * cmd.vertexColors.size());

					colorOffsets[i] = currentColorOffset;
					currentColorOffset += static_cast<uint32_t>(cmd.vertexColors.size());
				}

				i++;
			}

			ssbo->Unmap();
		}

		// Object data
		{
			VT_PROFILE_SCOPE("Upload Object Data");
			auto ssbo = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::OBJECT_DATA, currentIndex);

			auto* objectDatas = ssbo->Map<ObjectData>();

			for (uint32_t i = 0; const auto & cmd : data.submitCommands)
			{
				objectDatas[i].transform = cmd.transform;
				objectDatas[i].materialIndex = Renderer::GetBindlessData().materialTable->GetIndexFromMaterial(cmd.material.get());
				objectDatas[i].meshIndex = Renderer::GetBindlessData().meshTable->GetMeshIndex(cmd.mesh.get(), cmd.subMeshIndex);
				objectDatas[i].isAnimated = !cmd.boneTransforms.empty();
				objectDatas[i].id = cmd.id;
				objectDatas[i].timeSinceCreation = cmd.timeSinceCreation;
				objectDatas[i].randomValue = cmd.randomValue;

				if (boneOffsets.contains(i))
				{
					objectDatas[i].boneOffset = boneOffsets.at(i);
				}

				if (colorOffsets.contains(i))
				{
					objectDatas[i].colorOffset = static_cast<int32_t>(colorOffsets.at(i));
				}
				else
				{
					objectDatas[i].colorOffset = -1;
				}

				Volt::BoundingSphere boundingSphere = cmd.mesh->GetBoundingSphere();
				const glm::vec3 globalScale = { glm::length(cmd.transform[0]), glm::length(cmd.transform[1]), glm::length(cmd.transform[2]) };
				float maxScale = glm::max(glm::max(globalScale.x, globalScale.y), globalScale.z);
				const glm::vec3 globalCenter = cmd.transform * glm::vec4(boundingSphere.center, 1.f);

				if (!cmd.boneTransforms.empty())
				{
					maxScale *= 100.f;
				}

				objectDatas[i].boundingSphereRadius = boundingSphere.radius * maxScale;
				objectDatas[i].boundingSphereCenter = globalCenter;

				i++;
			}

			ssbo->Unmap();
		}
	}

	void SceneRenderer::UploadPerFrameData()
	{
		VT_PROFILE_FUNCTION();

		auto& data = GetGPUData();
		const uint32_t currentIndex = myCommandBuffer->GetCurrentIndex();

		// Point lights
		if (!data.pointLights.empty())
		{
			auto ssbo = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::POINT_LIGHTS, currentIndex);

			auto* pointLights = ssbo->Map<PointLight>();
			memcpy_s(pointLights, sizeof(PointLight) * MAX_POINT_LIGHTS, data.pointLights.data(), std::min((uint32_t)data.pointLights.size(), MAX_POINT_LIGHTS) * sizeof(PointLight));
			ssbo->Unmap();
		}

		// Spot lights
		if (!data.spotLights.empty())
		{
			auto ssbo = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::SPOT_LIGHTS, currentIndex);

			auto* spotLights = ssbo->Map<SpotLight>();
			memcpy_s(spotLights, sizeof(SpotLight) * MAX_SPOT_LIGHTS, data.spotLights.data(), std::min((uint32_t)data.spotLights.size(), MAX_SPOT_LIGHTS) * sizeof(SpotLight));
			ssbo->Unmap();
		}

		// Sphere lights
		if (!data.sphereLights.empty())
		{
			auto ssbo = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::SPHERE_LIGHTS, currentIndex);

			auto* sphereLights = ssbo->Map<SphereLight>();
			memcpy_s(sphereLights, sizeof(SphereLight) * MAX_SPHERE_LIGHTS, data.sphereLights.data(), std::min((uint32_t)data.sphereLights.size(), MAX_SPHERE_LIGHTS) * sizeof(SphereLight));
			ssbo->Unmap();
		}

		// Rectangle lights
		if (!data.rectangleLights.empty())
		{
			auto ssbo = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::RECTANGLE_LIGHTS, currentIndex);

			auto* rectangleLights = ssbo->Map<RectangleLight>();
			memcpy_s(rectangleLights, sizeof(RectangleLight) * MAX_RECTANGLE_LIGHTS, data.rectangleLights.data(), std::min((uint32_t)data.rectangleLights.size(), MAX_RECTANGLE_LIGHTS) * sizeof(RectangleLight));
			ssbo->Unmap();
		}

		// Scene data
		{
			auto ubo = myUniformBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::SCENE_DATA, currentIndex);

			auto* sceneData = ubo->Map<SceneData>();
			sceneData->pointLightCount = (uint32_t)data.pointLights.size();
			sceneData->spotLightCount = (uint32_t)data.spotLights.size();
			sceneData->sphereLightCount = (uint32_t)data.sphereLights.size();
			sceneData->rectangleLightCount = (uint32_t)data.rectangleLights.size();
			sceneData->ambianceMultiplier = myEnvironmentSettings.intensity;

			ubo->Unmap();
		}
	}

	void SceneRenderer::UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, Ref<UniformBufferSet> uniformBufferSet)
	{
		//auto uniformBuffer = uniformBufferSet->Get(set, binding, index);

		//VkDescriptorBufferInfo descriptorInfo{};
		//descriptorInfo.buffer = uniformBuffer->GetHandle();
		//descriptorInfo.offset = 0;
		//descriptorInfo.range = uniformBuffer->GetSize();

		//auto writeDescriptor = globalDescriptorSet->GetWriteDescriptor(index, binding);
		//writeDescriptor.pBufferInfo = &descriptorInfo;

		//auto device = GraphicsContextVolt::GetDevice();
		//vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
	}

	void SceneRenderer::UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, Ref<ShaderStorageBufferSet> storageBufferSet)
	{
		//auto storageBuffer = storageBufferSet->Get(set, binding, index);

		//VkDescriptorBufferInfo descriptorInfo{};
		//descriptorInfo.buffer = storageBuffer->GetHandle();
		//descriptorInfo.offset = 0;
		//descriptorInfo.range = storageBuffer->GetSize();

		//auto writeDescriptor = globalDescriptorSet->GetWriteDescriptor(index, binding);
		//writeDescriptor.pBufferInfo = &descriptorInfo;

		//auto device = GraphicsContextVolt::GetDevice();
		//vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
	}

	void SceneRenderer::UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, Weak<Image2D> image)
	{
		UpdateGlobalDescriptorSet(globalDescriptorSet, set, binding, index, image, image.lock()->GetLayout());
	}

	void SceneRenderer::UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, Weak<Image2D> image, VkImageLayout targetLayout)
	{
		//auto imagePtr = image.lock();

		VkDescriptorImageInfo descriptorInfo{};
		//descriptorInfo.imageLayout = targetLayout;
		//descriptorInfo.imageView = imagePtr->GetView();
		//descriptorInfo.sampler = nullptr;

		//auto writeDescriptor = globalDescriptorSet->GetWriteDescriptor(index, binding);
		//writeDescriptor.pImageInfo = &descriptorInfo;

		//auto device = GraphicsContextVolt::GetDevice();
		//vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
	}

	void SceneRenderer::UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, Weak<Image3D> image)
	{
		//auto imagePtr = image.lock();

		//VkDescriptorImageInfo descriptorInfo{};
		//descriptorInfo.imageLayout = imagePtr->GetLayout();
		//descriptorInfo.imageView = imagePtr->GetView();
		//descriptorInfo.sampler = nullptr;

		//auto writeDescriptor = globalDescriptorSet->GetWriteDescriptor(index, binding);
		//writeDescriptor.pImageInfo = &descriptorInfo;

		//auto device = GraphicsContextVolt::GetDevice();
		//vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
	}

	void SceneRenderer::UpdateGlobalDescriptorSet(Ref<GlobalDescriptorSet> globalDescriptorSet, uint32_t set, uint32_t binding, uint32_t index, std::vector<Weak<Image2D>> images, VkImageLayout targetLayout)
	{
		//std::vector<VkDescriptorImageInfo> descriptorInfos{};

		//for (const auto& image : images)
		//{
		//	auto imagePtr = image.lock();
		//	auto& descriptorInfo = descriptorInfos.emplace_back();

		//	descriptorInfo.imageLayout = targetLayout;
		//	descriptorInfo.imageView = imagePtr->GetView();
		//	descriptorInfo.sampler = nullptr;
		//}

		//auto writeDescriptor = globalDescriptorSet->GetWriteDescriptor(index, binding);
		//writeDescriptor.pImageInfo = descriptorInfos.data();

		//auto device = GraphicsContextVolt::GetDevice();
		//vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
	}

	void SceneRenderer::SortAndUploadParticleData(Ref<Camera> camera)
	{
		VT_PROFILE_FUNCTION();

		auto& data = GetGPUData();
		const uint32_t currentIndex = myCommandBuffer->GetCurrentIndex();
		const auto& camPos = camera->GetPosition();

		{
			VT_PROFILE_SCOPE("Sort Systems");
			std::sort(data.particleBatches.begin(), data.particleBatches.end(), [&](const auto& lhs, const auto& rhs)
			{
				return glm::distance2(camPos, lhs.emitterPosition) < glm::distance2(camPos, lhs.emitterPosition);
			});
		}

		{
			VT_PROFILE_SCOPE("Sort Particles");

			for (auto& batch : data.particleBatches)
			{
				std::sort(std::execution::par, batch.particles.begin(), batch.particles.end(), [&](const auto& lhs, const auto& rhs)
				{
					return glm::distance2(camPos, lhs.position) > glm::distance2(camPos, rhs.position);
				});
			}
		}

		{
			VT_PROFILE_SCOPE("Upload Particles");

			auto* particleData = myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::PARTICLE_INFO, currentIndex)->Map<ParticleRenderingInfo>();
			uint32_t startOffset = 0;

			for (auto& batch : data.particleBatches)
			{
				batch.startOffset = startOffset;
				memcpy_s(&particleData[startOffset], sizeof(ParticleRenderingInfo) * MAX_PARTICLES - sizeof(ParticleRenderingInfo) * startOffset, batch.particles.data(), sizeof(ParticleRenderingInfo) * batch.particles.size());
				startOffset += (uint32_t)batch.particles.size();
			}

			myShaderStorageBufferSet->Get(Sets::RENDERER_BUFFERS, Bindings::PARTICLE_INFO, currentIndex)->Unmap();
		}
	}

	void SceneRenderer::ExecuteFrame()
	{
		VT_PROFILE_SCOPE("Build Command Buffer");

		myCommandBuffer->Begin();

		if (myShouldResize)
		{
			myShouldResize = false;
			myOutputImage->Invalidate(myRenderSize.x, myRenderSize.y);

			myHistoryDepth->Invalidate(myRenderSize.x, myRenderSize.y);
			myHistoryColor->Invalidate(myRenderSize.x, myRenderSize.y);

			if (myIDImage)
			{
				myIDImage->Invalidate(myRenderSize.x, myRenderSize.y);
			}

			// Resize light culling buffer
			{
				constexpr uint32_t TILE_SIZE = 16;

				glm::uvec2 size = myRenderSize;
				size.x += TILE_SIZE - myRenderSize.x % TILE_SIZE;
				size.y += TILE_SIZE - myRenderSize.y % TILE_SIZE;

				myLightCullingWorkGroups = size / TILE_SIZE;

				for (auto&& value : myResizeLightCullingBuffers)
				{
					value = true;
				}
			}

			myTransientResourceSystem.Reset();
			myTransientResourceSystem.Clear();
		}

		if (!mySpecification.debugName.empty())
		{
			Renderer::BeginSection(myCommandBuffer, mySpecification.debugName, TO_NORMALIZEDRGB(255, 153, 0));
		}

		myTransientResourceSystem.Reset();
		myCurrentIndirectPass = 0;

		if (mySettings.enableAntiAliasing && mySettings.antiAliasing == AntiAliasingSetting::TAA)
		{
			myPreviousJitter = myLastJitter;
			myCurrentCamera->SetSubpixelOffset(Noise::GetTAAJitter(myFrameIndex, myRenderSize));
			myLastJitter = myCurrentCamera->GetSubpixelOffset();
		}
		else
		{
			myCurrentCamera->SetSubpixelOffset({ 0.f });
		}

		SortSubmitCommands();
		PrepareForIndirectDraw();

		ResizeBuffersIfNeeded();

		PrepareLineBuffer();
		PrepareBillboardBuffer();

		UploadIndirectDrawCommands();
		UploadObjectData();
		UploadPerFrameData();

		SortAndUploadParticleData(myCurrentCamera);

		if (myCurrentCamera)
		{
			UpdateBuffers(myCurrentCamera);
		}

		//PerformLODSelection(myCommandBuffer);

		Renderer::Begin(myCommandBuffer);
		FrameGraph frameGraph{ myTransientResourceSystem, myCommandBuffer, myCommandBufferCache, Application::GetRenderThreadPool() };
		AddPreDepthPass(frameGraph);

		const uint64_t frameIndex = (mySettings.enableAntiAliasing && mySettings.antiAliasing == AntiAliasingSetting::TAA) ? myFrameIndex % 256 : 0;

		GTAOTechnique gtaoTechnique{ myCurrentCamera, myRenderSize, frameIndex, myGTAOSettings };
		gtaoTechnique.AddPrefilterDepthPass(frameGraph, myGTAOPrefilterPipeline, frameGraph.GetBlackboard().Get<PreDepthData>().preDepth);
		gtaoTechnique.AddMainPass(frameGraph, myGTAOMainPassPipeline, frameGraph.GetBlackboard().Get<PreDepthData>().viewNormals);
		gtaoTechnique.AddDenoisePass(frameGraph, myGTAODenoisePipelines);

		if (mySettings.enableIDRendering)
		{
			AddIDPass(frameGraph);
		}

		AddLightCullPass(frameGraph);
		AddDirectionalShadowPass(frameGraph);

		if (GetGPUData().spotLightShadowCount > 0)
		{
			AddSpotlightShadowPasses(frameGraph);
		}

		if (GetGPUData().pointLightShadowCount > 0)
		{
			AddPointLightShadowPasses(frameGraph);
		}

		if (!myEnvironmentSettings.radianceMap)
		{
			AddPreethamSkyPass(frameGraph);
		}

		AddGBufferPass(frameGraph);
		AddDecalPass(frameGraph);

		AddSkyboxPass(frameGraph);

		UpdatePBRDescriptors(frameGraph);

		VolumetricFogTechnique fogTechnique{ myRenderSize, myGlobalDescriptorSets, myVolumetricFogSettings, static_cast<bool>(GetGPUData().directionalLight.castShadows) };
		fogTechnique.AddInjectionPass(frameGraph, myVolumetricFogInjectPipeline, myVolumetricFogInjectImage);
		fogTechnique.AddRayMarchPass(frameGraph, myVolumetricFogRayMarchPipeline, myVolumetricFogRayMarchImage);

		AddDeferredShadingPass(frameGraph);

		if (myCurrentRenderMode == RenderMode::Default)
		{
			AddForwardOpaquePass(frameGraph);

			//AddForwardSSSPass(frameGraph);
			//AddSSSBlurPass(frameGraph);
			//AddSSSCompositePass(frameGraph);

			AddForwardTransparentPass(frameGraph);
			AddTransparentCompositePass(frameGraph);

			if (!GetGPUData().particleBatches.empty())
			{
				AddParticlesPass(frameGraph);
			}

			if (myBillboardData.vertexCount > 0)
			{
				AddBillboardPass(frameGraph);
			}

			//SSRTechnique ssrTechnique{ myGlobalDescriptorSets, myRenderSize };
			//ssrTechnique.AddSSRPass(frameGraph, mySSRMaterial);
			//ssrTechnique.AddSSRCompositePass(frameGraph, mySSRCompositeMaterial);

			AddLuminosityPass(frameGraph);

			if (mySettings.enableBloom)
			{
				BloomTechnique bloomTechnique{ myRenderSize };
				bloomTechnique.AddBloomDownsamplePass(frameGraph, myBloomDownsamplePipeline);
				bloomTechnique.AddBloomUpsamplePass(frameGraph, myBloomUpsamplePipeline);
				bloomTechnique.AddBloomCompositePass(frameGraph, myBloomCompositePipeline);
			}

			if (GetGPUData().postProcessingStack && mySettings.enablePostProcessing)
			{
				AddPostProcessingStackPasses(frameGraph);
			}

			if (mySettings.enableOutline)
			{
				OutlineTechnique outlineTechnique{ myRenderSize, myGlobalDescriptorSets };
				outlineTechnique.AddOutlineGeometryPass(frameGraph, myOutlineGeometryPipeline, GetGPUData().outlineCommands);
				outlineTechnique.AddJumpFloodInitPass(frameGraph, myOutlineJumpFloodInitMaterial);
				outlineTechnique.AddJumpFloodPass(frameGraph, myOutlineJumpFloodPassMaterial);
				outlineTechnique.AddOutlineCompositePass(frameGraph, myOutlineCompositePipeline);
			}

			if (mySettings.enableAntiAliasing)
			{
				switch (mySettings.antiAliasing)
				{
					case AntiAliasingSetting::FXAA:
					{
						FXAATechnique fxaaTechnique{ myRenderSize };
						fxaaTechnique.AddFXAAPass(frameGraph, myFXAAPipeline);
						fxaaTechnique.AddFXAAApplyPass(frameGraph, myAACompositePipeline);

						break;
					}

					case AntiAliasingSetting::TAA:
					{
						glm::mat4 viewProj = myCurrentCamera->GetProjection() * myCurrentCamera->GetView();
						glm::mat4 reprojectionMatrix = myPreviousViewProjection * glm::inverse(viewProj);
						myPreviousViewProjection = viewProj;

						glm::vec2 jitterDelta = myPreviousJitter - myCurrentCamera->GetSubpixelOffset();

						TAATechnique taaTechnique{ myCurrentCamera, reprojectionMatrix, myRenderSize, myFrameIndex, jitterDelta };
						taaTechnique.AddGenerateMotionVectorsPass(frameGraph, myGenerateMotionVectorsPipeline, myGlobalDescriptorSets[Sets::RENDERER_BUFFERS], frameGraph.GetBlackboard().Get<PreDepthData>().preDepth);
						taaTechnique.AddTAAPass(frameGraph, myTAAPipeline, myHistoryColor, frameGraph.GetBlackboard().Get<PreDepthData>().preDepth);
						taaTechnique.AddTAAApplyPass(frameGraph, myAACompositePipeline);

						AddCopyHistoryPass(frameGraph);

						break;
					}
				}
			}

			AddACESPass(frameGraph);
			AddGammaCorrectionPass(frameGraph);
		}

		if (mySettings.enableGrid)
		{
			AddGridPass(frameGraph);
		}

		frameGraph.Compile();
		frameGraph.Execute();

		myOutputImage->TransitionToLayout(myCommandBuffer->GetCurrentCommandBuffer(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		if (myIDImage)
		{
			myIDImage->TransitionToLayout(myCommandBuffer->GetCurrentCommandBuffer(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		Renderer::End();

		if (!mySpecification.debugName.empty())
		{
			Renderer::EndSection(myCommandBuffer);
		}

		myCommandBuffer->End();

		// Reset text
		{
			myTextData.vertexBufferPtr = myTextData.vertexBufferBase;
			myTextData.indexCount = 0;
		}

		myFrameIndex++;
	}

	void SceneRenderer::EvaluateTimestamps()
	{
		std::vector<Timestamp> result{};

		Timestamp& totalTime = result.emplace_back();
		totalTime.label = mySpecification.debugName.empty() ? "SceneRenderer" : mySpecification.debugName;
		totalTime.time = myCommandBuffer->GetExecutionTime(myCommandBuffer->GetLastIndex(), 0);

		for (const auto& timestampInfo : myTimestamps.at(myCommandBuffer->GetLastIndex()))
		{
			auto& timestamp = result.emplace_back();
			timestamp.label = timestampInfo.label;
			timestamp.time = myCommandBuffer->GetExecutionTime(myCommandBuffer->GetLastIndex(), timestampInfo.id);
		}

		myTimestampsResult = result;
	}

	const IndirectPass& SceneRenderer::GetOrCreateIndirectPass(Ref<CommandBuffer> commandBuffer, const IndirectPassParams& params)
	{
		if (myCurrentIndirectPass + 1 < static_cast<uint32_t>(myIndirectPasses.size()))
		{
			std::scoped_lock lock{ myRenderingMutex };

			const auto& pass = myIndirectPasses.at(myCurrentIndirectPass);
			myCurrentIndirectPass++;

			ClearCountBuffer(pass, commandBuffer);
			CullRenderCommands(pass, params, commandBuffer);
			return pass;
		}

		uint32_t passIndex = 0;

		{
			std::scoped_lock lock{ myRenderingMutex };
			myIndirectPasses.emplace_back();
			passIndex = static_cast<uint32_t>(myIndirectPasses.size() - 1);
			myCurrentIndirectPass++;
		}
		auto& newPass = myIndirectPasses.at(passIndex);

		//const uint32_t maxFramesInFlight = Application::Get().GetWindow().GetSwapchain().GetMaxFramesInFlight();

		//Ref<ShaderStorageBufferSet> countStorageBufferSet = ShaderStorageBufferSet::Create(maxFramesInFlight);
		//countStorageBufferSet->Add<uint32_t>(Sets::DRAW_BUFFERS, Bindings::INDIRECT_COUNTS, std::max(static_cast<uint32_t>(GetGPUData().indirectBatches.size()), 1u), MemoryUsage::Indirect);
		//countStorageBufferSet->Add<uint32_t>(Sets::DRAW_BUFFERS, Bindings::DRAW_TO_OBJECT_ID, std::max(static_cast<uint32_t>(GetGPUData().submitCommands.size()), 1u), MemoryUsage::Indirect);

		//newPass.drawArgsStorageBuffer = myShaderStorageBufferSet;
		//newPass.drawCountIDStorageBuffer = countStorageBufferSet;
		//newPass.drawBuffersSet = GlobalDescriptorSetManager::CreateCopyOfType(Sets::DRAW_BUFFERS);

		//const uint32_t framesInFlight = Renderer::GetFramesInFlightCount();

		//newPass.indirectCullPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("IndirectCull"), framesInFlight, true);
		//newPass.clearCountBufferPipeline = ComputePipeline::Create(ShaderRegistry::GetShader("ClearCountBuffer"), framesInFlight, true);

		//for (uint32_t i = 0; i < maxFramesInFlight; i++)
		//{
		//	newPass.drawBuffersSet->GetOrAllocateDescriptorSet(i);
		//	UpdateGlobalDescriptorSet(newPass.drawBuffersSet, Sets::DRAW_BUFFERS, Bindings::INDIRECT_COUNTS, i, newPass.drawCountIDStorageBuffer);
		//	UpdateGlobalDescriptorSet(newPass.drawBuffersSet, Sets::DRAW_BUFFERS, Bindings::DRAW_TO_OBJECT_ID, i, newPass.drawCountIDStorageBuffer);
		//}

		ClearCountBuffer(newPass, commandBuffer);
		CullRenderCommands(newPass, params, commandBuffer);
		return newPass;
	}

	void SceneRenderer::AddDirectionalShadowPass(FrameGraph& frameGraph)
	{
		frameGraph.GetBlackboard().Add<DirectionalShadowData>() = frameGraph.AddRenderPass<DirectionalShadowData>("Directional Shadow Pass",
		[&](FrameGraph::Builder& builder, DirectionalShadowData& data)
		{
			const auto size = SceneRendererSettings::GetResolutionFromShadowSetting(mySettings.shadowResolution);
			data.size = size;

			FrameGraphTextureSpecification spec{};
			spec.format = ImageFormat::DEPTH32F;
			spec.width = size.x;
			spec.height = size.y;
			spec.usage = ImageUsage::Attachment;
			spec.layers = 5;
			spec.isCubeMap = false;
			spec.name = "Directional Shadow";
			spec.clearColor = { 1.f };

			data.shadowMap = builder.CreateTexture(spec);
		},

		[=, &batches = GetGPUData().indirectBatches](const DirectionalShadowData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Directional Shadow");

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(50, 168, 105);
			renderPassInfo.name = "Directional Shadow";
			renderPassInfo.overridePipeline = myDirectionalShadowPipeline;

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(data.shadowMap)
				});

			renderingInfo.layers = 5;
			renderingInfo.width = data.size.x;
			renderingInfo.height = data.size.y;

			IndirectPassParams indirectParams{};
			indirectParams.materialFlags = MaterialFlag::CastShadows;
			indirectParams.cullMode = DrawCullMode::CameraFrustum;

			const auto& indirectPass = GetOrCreateIndirectPass(commandBuffer, indirectParams);

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawIndirectBatches(commandBuffer, indirectPass, myGlobalDescriptorSets, batches);
			Renderer::EndFrameGraphPass(commandBuffer);

			EndTimestamp();
		});
	}

	void SceneRenderer::AddSpotlightShadowPasses(FrameGraph& frameGraph)
	{
		constexpr uint32_t SPOTLIGHT_SHADOW_SIZE = 512;

		frameGraph.GetBlackboard().Add<SpotLightShadowData>() = frameGraph.AddRenderPass<SpotLightShadowData>("Spot light Shadow Pass",
			[&](FrameGraph::Builder& builder, SpotLightShadowData& data)
		{
			for (uint32_t lightIndex = 0; const auto & spotlight : GetGPUData().spotLights)
			{
				if (spotlight.castShadows)
				{
					data.spotLightShadows.emplace_back(builder.CreateTexture({ ImageFormat::DEPTH32F, { SPOTLIGHT_SHADOW_SIZE, SPOTLIGHT_SHADOW_SIZE }, { 1.f }, "Spotlight shadow map" }));
					data.spotLightIndices.emplace_back(lightIndex);
				}

				lightIndex++;
			}
		},
			[=, &batches = GetGPUData().indirectBatches, &spotLights = GetGPUData().spotLights](const SpotLightShadowData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Spot Light Shadow");

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(50, 168, 105);
			renderPassInfo.name = "Spot Light Shadow Pass";
			renderPassInfo.overridePipeline = mySpotLightShadowPipeline;

			for (uint32_t index = 0; const auto & handle : data.spotLightShadows)
			{
				FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
					{
						resources.GetImageResource(handle)
					});

				renderingInfo.width = SPOTLIGHT_SHADOW_SIZE;
				renderingInfo.height = SPOTLIGHT_SHADOW_SIZE;

				const SpotLight& spotLight = spotLights.at(index);

				PushConstantDrawData pushcConstantData{ &data.spotLightIndices.at(index), sizeof(uint32_t) };

				IndirectPassParams indirectParams{};
				indirectParams.materialFlags = MaterialFlag::CastShadows;
				indirectParams.cullMode = DrawCullMode::Frustum;
				indirectParams.projection = spotLight.projection;

				const auto& indirectPass = GetOrCreateIndirectPass(commandBuffer, indirectParams);

				Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
				Renderer::DrawIndirectBatches(commandBuffer, indirectPass, myGlobalDescriptorSets, batches, pushcConstantData);
				Renderer::EndFrameGraphPass(commandBuffer);

				index++;
			}

			EndTimestamp();
		});
	}

	void SceneRenderer::AddPointLightShadowPasses(FrameGraph& frameGraph)
	{
		constexpr uint32_t POINTLIGHT_SHADOW_SIZE = 16;

		frameGraph.GetBlackboard().Add<PointLightShadowData>() = frameGraph.AddRenderPass<PointLightShadowData>("Point light Shadow Pass",
			[&](FrameGraph::Builder& builder, PointLightShadowData& data)
		{
			for (uint32_t lightIndex = 0; const auto & pointLight : GetGPUData().pointLights)
			{
				if (pointLight.castShadows)
				{
					FrameGraphTextureSpecification spec;
					spec.format = ImageFormat::DEPTH32F;
					spec.width = POINTLIGHT_SHADOW_SIZE;
					spec.height = POINTLIGHT_SHADOW_SIZE;
					spec.usage = ImageUsage::Attachment;
					spec.layers = 6;
					spec.isCubeMap = true;
					spec.name = "Point Light Shadow";
					spec.clearColor = { 1.f };

					data.pointLightShadows.emplace_back(builder.CreateTexture(spec));
					data.pointLightIndices.emplace_back(lightIndex);
				}

				lightIndex++;
			}
		},

			[=, &batches = GetGPUData().indirectBatches](const PointLightShadowData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			//BeginTimestamp("Point Light Shadows");
			//Renderer::BeginSection(commandBuffer, "Point Light Shadows", { 1.f });

			//FrameGraphRenderPassInfo renderPassInfo{};
			//renderPassInfo.color = TO_NORMALIZEDRGB(50, 168, 105);
			//renderPassInfo.name = "Point Light Shadow Pass";
			//renderPassInfo.overridePipeline = myPointLightShadowPipeline;

			//for (uint32_t index = 0; const auto & handle : data.pointLightShadows)
			//{
			//	FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
			//		{
			//			resources.GetImageResource(handle)
			//		});

			//	renderingInfo.width = POINTLIGHT_SHADOW_SIZE;
			//	renderingInfo.height = POINTLIGHT_SHADOW_SIZE;
			//	renderingInfo.layers = 6;

			//	PushConstantDrawData pushConstantData{ &data.pointLightIndices.at(index), sizeof(uint32_t) };

			//	const auto& pointLight = GetGPUData().pointLights.at(data.pointLightIndices.at(index));

			//	const glm::vec3 max = { pointLight.position.x + pointLight.radius, pointLight.position.y + pointLight.radius, pointLight.position.z + pointLight.radius };
			//	const glm::vec3 min = { pointLight.position.x - pointLight.radius, pointLight.position.y - pointLight.radius, pointLight.position.z - pointLight.radius };

			//	IndirectPassParams params;
			//	params.cullMode = DrawCullMode::AABB;
			//	params.aabbMin = min;
			//	params.aabbMax = max;
			//	params.materialFlags = MaterialFlag::CastShadows;

			//	const auto& indirectPass = GetOrCreateIndirectPass(commandBuffer, params);

			//	Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			//	Renderer::DrawIndirectBatches(commandBuffer, indirectPass, myGlobalDescriptorSets, batches, pushConstantData);
			//	Renderer::EndFrameGraphPass(commandBuffer);

			//	index++;
			//}

			//Renderer::EndSection(commandBuffer);
			//EndTimestamp();
		});
	}

	void SceneRenderer::UpdatePBRDescriptors(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& dirShadowData = frameGraph.GetBlackboard().Get<DirectionalShadowData>();
		const auto& gtaoData = frameGraph.GetBlackboard().Get<GTAOOutput>();

		frameGraph.AddRenderPass("Update Shadow Descriptors",
			[&](FrameGraph::Builder& builder)
		{
			if (GetGPUData().directionalLight.castShadows)
			{
				builder.ReadResource(dirShadowData.shadowMap);
			}

			if (GetGPUData().spotLightShadowCount > 0)
			{
				const auto& spotlightShadowData = frameGraph.GetBlackboard().Get<SpotLightShadowData>();

				for (const auto& lightHandle : spotlightShadowData.spotLightShadows)
				{
					builder.ReadResource(lightHandle);
				}
			}

			if (GetGPUData().pointLightShadowCount > 0)
			{
				const auto& pointLightShadowData = frameGraph.GetBlackboard().Get<PointLightShadowData>();

				for (const auto& lightHandle : pointLightShadowData.pointLightShadows)
				{
					builder.ReadResource(lightHandle);
				}
			}

			if (myEnvironmentSettings.radianceMap)
			{
				builder.ReadResource(skyboxData.radianceImage);
				builder.ReadResource(skyboxData.irradianceImage);
			}
			else
			{
				const auto& preethamData = frameGraph.GetBlackboard().Get<PreethamSkyData>();
				builder.ReadResource(preethamData.skybox);
			}

			if (mySettings.enableAO)
			{
				builder.ReadResource(gtaoData.outputImage);
			}

			builder.SetHasSideEffect();
		},

		[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			const uint32_t index = commandBuffer->GetCurrentIndex();
			auto resourcesDescriptor = myGlobalDescriptorSets[Sets::PBR_RESOURCES];
			resourcesDescriptor->GetOrAllocateDescriptorSet(index);

			// Dir shadow
			if (GetGPUData().directionalLight.castShadows)
			{
				const auto& dirShadowResource = resources.GetImageResource(dirShadowData.shadowMap);
				UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::DIR_SHADOWMAP, index, dirShadowResource.image, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
			}
			else
			{
				UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::DIR_SHADOWMAP, index, Renderer::GetDefaultData().whiteTexture->GetImage(), VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
			}

			// Spot Light shadows
			{
				std::vector<FrameGraphResourceHandle> shadowResources{};
				if (GetGPUData().spotLightShadowCount > 0)
				{
					const auto& spotlightShadowData = frameGraph.GetBlackboard().Get<SpotLightShadowData>();
					shadowResources = spotlightShadowData.spotLightShadows;
				}

				std::vector<Weak<Image2D>> images{};
				images.resize(MAX_SPOT_LIGHT_SHADOWS);

				for (uint32_t i = 0; i < MAX_SPOT_LIGHT_SHADOWS; i++)
				{
					if (i < (uint32_t)shadowResources.size())
					{
						const auto& shadowResource = resources.GetImageResource(shadowResources.at(i));
						images[i] = shadowResource.image;
					}
					else
					{
						images[i] = Renderer::GetDefaultData().whiteTexture->GetImage();
					}
				}
				UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::SPOT_SHADOWMAPS, index, images, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
			}

			// Point Light Shadows
			{
				std::vector<FrameGraphResourceHandle> shadowResources{};
				if (GetGPUData().pointLightShadowCount > 0)
				{
					const auto& pointLightShadowData = frameGraph.GetBlackboard().Get<PointLightShadowData>();
					shadowResources = pointLightShadowData.pointLightShadows;
				}

				std::vector<Weak<Image2D>> images{};
				images.resize(MAX_POINT_LIGHT_SHADOWS);

				for (uint32_t i = 0; i < MAX_POINT_LIGHT_SHADOWS; i++)
				{
					if (i < (uint32_t)shadowResources.size())
					{
						const auto& shadowResource = resources.GetImageResource(shadowResources.at(i));
						images[i] = shadowResource.image;
					}
					else
					{
						images[i] = Renderer::GetDefaultData().whiteTexture->GetImage();
					}
				}
				UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::POINT_SHADOWMAPS, index, images, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
			}

			// Update PBR descriptor sets
			{
				if (myEnvironmentSettings.radianceMap)
				{
					const auto& radianceResource = resources.GetImageResource(skyboxData.radianceImage);
					const auto& irradianceResource = resources.GetImageResource(skyboxData.irradianceImage);

					UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::IRRADIANCE, index, irradianceResource.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::RADIANCE, index, radianceResource.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				}
				else
				{
					const auto& preethamData = frameGraph.GetBlackboard().Get<PreethamSkyData>();
					const auto& preethamResource = resources.GetImageResource(preethamData.skybox);

					UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::IRRADIANCE, index, preethamResource.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::RADIANCE, index, preethamResource.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				}

				UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::BRDF, index, Renderer::GetDefaultData().brdfLut, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}

			if (mySettings.enableAO)
			{
				const auto& gtaoResource = resources.GetImageResource(gtaoData.outputImage);
				UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::AO_TEXTURE, index, gtaoResource.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		});
	}

	void SceneRenderer::AddIDPass(FrameGraph& frameGraph)
	{
		frameGraph.GetBlackboard().Add<IDData>() = frameGraph.AddRenderPass<IDData>("ID Pass",
		[&](FrameGraph::Builder& builder, IDData& data)
		{
			data.id = builder.AddExternalTexture(myIDImage, "ID");
			data.depth = builder.CreateTexture({ ImageFormat::DEPTH32F, myRenderSize, { 0.f }, "ID Depth" });

			builder.WriteResource(data.id);
			builder.SetHasSideEffect();
		},

		[=, &batches = GetGPUData().indirectBatches](const IDData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("ID");

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(45, 53, 110);
			renderPassInfo.name = "ID";
			renderPassInfo.overridePipeline = myIDPipeline;

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(data.id),
					resources.GetImageResource(data.depth)
				});

			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			const auto& indirectPass = GetOrCreateIndirectPass(commandBuffer);

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawIndirectBatches(commandBuffer, indirectPass, myGlobalDescriptorSets, batches);
			Renderer::EndFrameGraphPass(commandBuffer);

			EndTimestamp();
		});
	}

	void SceneRenderer::AddPreDepthPass(FrameGraph& frameGraph)
	{
		frameGraph.GetBlackboard().Add<PreDepthData>() = frameGraph.AddRenderPass<PreDepthData>("Pre Depth Pass",
			[&](FrameGraph::Builder& builder, PreDepthData& data)
		{
			data.preDepth = builder.CreateTexture({ ImageFormat::DEPTH32F, myRenderSize, { 0.f }, "Pre Depth" });
			data.viewNormals = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, { 0.f, 0.f, 0.f, 1.f }, "View Normals" });
		},

			[=, &batches = GetGPUData().indirectBatches](const PreDepthData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Pre Depth");

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(45, 53, 110);
			renderPassInfo.name = "Pre Depth";
			renderPassInfo.overridePipeline = myPreDepthPipeline;

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(data.viewNormals),
					resources.GetImageResource(data.preDepth)
				});

			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			IndirectPassParams indirectParams{};
			indirectParams.materialFlags = MaterialFlag::CastAO | MaterialFlag::Opaque | MaterialFlag::Deferred;

			const auto& indirectPass = GetOrCreateIndirectPass(commandBuffer, indirectParams);

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawIndirectBatches(commandBuffer, indirectPass, myGlobalDescriptorSets, batches);
			Renderer::EndFrameGraphPass(commandBuffer);

			EndTimestamp();
		});
	}

	void SceneRenderer::AddPreethamSkyPass(FrameGraph& frameGraph)
	{
		constexpr uint32_t cubeMapSize = 1024;
		constexpr uint32_t groupSize = 32;

		frameGraph.GetBlackboard().Add<PreethamSkyData>() = frameGraph.AddRenderPass<PreethamSkyData>("",
			[&](FrameGraph::Builder& builder, PreethamSkyData& data)
		{
			FrameGraphTextureSpecification spec{};
			spec.format = ImageFormat::RGBA16F;
			spec.width = cubeMapSize;
			spec.height = cubeMapSize;
			spec.usage = ImageUsage::Storage;
			spec.layers = 6;
			spec.isCubeMap = true;
			spec.mips = Utility::CalculateMipCount(cubeMapSize, cubeMapSize);

			data.skybox = builder.CreateTexture(spec);

			builder.SetIsComputePass();
		},

			[=](const PreethamSkyData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Preetham Sky");

			struct PreethamData
			{
				float turbidity;
				float azimuth;
				float inclination;
			} pushData{ myEnvironmentSettings.turbidity, myEnvironmentSettings.azimuth, myEnvironmentSettings.inclination };

			const uint32_t currentIndex = commandBuffer->GetCurrentIndex();
			const auto outputImage = resources.GetImageResource(data.skybox);

			myPreethamPipeline->Clear(currentIndex);
			myPreethamPipeline->SetImage(outputImage.image.lock(), 0, 0, ImageAccess::Write);
			myPreethamPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
			myPreethamPipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &pushData, sizeof(PreethamData));
			myPreethamPipeline->Dispatch(commandBuffer->GetCurrentCommandBuffer(), cubeMapSize / groupSize, cubeMapSize / groupSize, 6, currentIndex);

			myPreethamPipeline->ClearAllResources();
			EndTimestamp();
		});
	}

	void SceneRenderer::AddSkyboxPass(FrameGraph& frameGraph)
	{
		frameGraph.GetBlackboard().Add<SkyboxData>() = frameGraph.AddRenderPass<SkyboxData>("Skybox Pass",
			[&](FrameGraph::Builder& builder, SkyboxData& data)
		{
			data.outputImage = builder.AddExternalTexture(myOutputImage, "Main Output");

			if (myEnvironmentSettings.radianceMap)
			{
				data.radianceImage = builder.AddExternalTexture(myEnvironmentSettings.radianceMap, "Radiance Map");
				data.irradianceImage = builder.AddExternalTexture(myEnvironmentSettings.irradianceMap, "Irradiance Map");

				builder.ReadResource(data.radianceImage);
				builder.ReadResource(data.irradianceImage);
			}
			else
			{
				const auto& preethamSkyData = frameGraph.GetBlackboard().Get<PreethamSkyData>();
				builder.ReadResource(preethamSkyData.skybox);
			}

			builder.WriteResource(data.outputImage);
		},

			[=](const SkyboxData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			if (!mySettings.enableSkybox)
			{
				return;
			}

			BeginTimestamp("Skybox Pass");

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(235, 192, 52);
			renderPassInfo.name = "Skybox Pass";

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(data.outputImage),
				});

			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			if (myEnvironmentSettings.radianceMap)
			{
				const auto& envResource = resources.GetImageResource(data.radianceImage);
				mySkyboxMesh->GetMaterial()->GetSubMaterialAt(0)->Set(0, envResource.image.lock());
			}
			else
			{
				const auto& preethamSkyData = frameGraph.GetBlackboard().Get<PreethamSkyData>();
				const auto& preethamResource = resources.GetImageResource(preethamSkyData.skybox);
				mySkyboxMesh->GetMaterial()->GetSubMaterialAt(0)->Set(0, preethamResource.image.lock());
			}

			mySkyboxMesh->GetMaterial()->GetSubMaterialAt(0)->SetValue("intensity", myEnvironmentSettings.intensity);
			mySkyboxMesh->GetMaterial()->GetSubMaterialAt(0)->SetValue("lod", 0.f);

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawMesh(commandBuffer, mySkyboxMesh, myGlobalDescriptorSets);
			Renderer::EndFrameGraphPass(commandBuffer);

			EndTimestamp();
		});
	}

	void SceneRenderer::AddGBufferPass(FrameGraph& frameGraph)
	{
		frameGraph.GetBlackboard().Add<GBufferData>() = frameGraph.AddRenderPass<GBufferData>("GBuffer Pass",
		[&](FrameGraph::Builder& builder, GBufferData& data)
		{
			data.albedo = builder.CreateTexture({ ImageFormat::RGBA, myRenderSize, { 0.f }, "Albedo", ImageUsage::AttachmentStorage });
			data.materialEmissive = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, { 0.f }, "Material Emissive", ImageUsage::AttachmentStorage });
			data.normalEmissive = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, { 0.f }, "Normal Emissive", ImageUsage::AttachmentStorage });
			data.depth = builder.CreateTexture({ ImageFormat::DEPTH32F, myRenderSize, { 0.f }, "Depth" });
		},

		[=, &batches = GetGPUData().indirectBatches](const GBufferData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("GBuffer Pass");

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(45, 53, 110);
			renderPassInfo.name = "GBuffer Pass";
			renderPassInfo.overridePipeline = myGBufferPipeline;
			renderPassInfo.materialFlags = MaterialFlag::Deferred;

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(data.albedo),
					resources.GetImageResource(data.materialEmissive),
					resources.GetImageResource(data.normalEmissive),
					resources.GetImageResource(data.depth)
				});

			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			IndirectPassParams indirectPassParams{};
			indirectPassParams.materialFlags = MaterialFlag::Deferred;
			const auto& indirectPass = GetOrCreateIndirectPass(commandBuffer);

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawIndirectBatches(commandBuffer, indirectPass, myGlobalDescriptorSets, batches);
			Renderer::EndFrameGraphPass(commandBuffer);

			EndTimestamp();
		});
	}

	void SceneRenderer::AddDeferredShadingPass(FrameGraph& frameGraph)
	{
		const auto& gbufferData = frameGraph.GetBlackboard().Get<GBufferData>();
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& dirShadowData = frameGraph.GetBlackboard().Get<DirectionalShadowData>();
		const auto& volumetricFogData = frameGraph.GetBlackboard().Get<VolumetricFogData>();
		const auto& gtaoData = frameGraph.GetBlackboard().Get<GTAOOutput>();

		frameGraph.GetBlackboard().Add<DeferredShadingData>() = frameGraph.AddRenderPass<DeferredShadingData>("Deferred Shading",
			[&](FrameGraph::Builder& builder, DeferredShadingData& data)
		{
			builder.WriteResource(skyboxData.outputImage);
			builder.ReadResource(gbufferData.albedo);
			builder.ReadResource(gbufferData.normalEmissive);
			builder.ReadResource(gbufferData.materialEmissive);
			builder.ReadResource(gbufferData.depth);

			if (myEnvironmentSettings.radianceMap)
			{
				builder.ReadResource(skyboxData.radianceImage);
				builder.ReadResource(skyboxData.irradianceImage);
			}
			else
			{
				const auto& preethamData = frameGraph.GetBlackboard().Get<PreethamSkyData>();
				builder.ReadResource(preethamData.skybox);
			}

			if (GetGPUData().directionalLight.castShadows)
			{
				builder.ReadResource(dirShadowData.shadowMap);
			}

			if (GetGPUData().spotLightShadowCount > 0)
			{
				const auto& spotlightShadowData = frameGraph.GetBlackboard().Get<SpotLightShadowData>();

				for (const auto& lightHandle : spotlightShadowData.spotLightShadows)
				{
					builder.ReadResource(lightHandle);
				}
			}

			if (GetGPUData().pointLightShadowCount > 0)
			{
				const auto& pointLightShadowData = frameGraph.GetBlackboard().Get<PointLightShadowData>();

				for (const auto& lightHandle : pointLightShadowData.pointLightShadows)
				{
					builder.ReadResource(lightHandle);
				}
			}

			if (mySettings.enableVolumetricFog)
			{
				builder.ReadResource(volumetricFogData.rayMarched);
			}

			if (mySettings.enableAO)
			{
				builder.ReadResource(gtaoData.outputImage);
			}

			builder.SetHasSideEffect();
			builder.SetIsComputePass();
		},

			[=](const DeferredShadingData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Deferred Shading Pass");

			auto shadingPipeline = myDeferredShadingPipeline;

			const auto& albedoResource = resources.GetImageResource(gbufferData.albedo);
			const auto& materialEmissiveResource = resources.GetImageResource(gbufferData.materialEmissive);
			const auto& normalEmissiveResource = resources.GetImageResource(gbufferData.normalEmissive);
			const auto& depthResource = resources.GetImageResource(gbufferData.depth);

			const auto& targetResource = resources.GetImageResource(skyboxData.outputImage);

			const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

			shadingPipeline->SetImage(albedoResource.image.lock(), Sets::OTHER, 0, ImageAccess::Read);
			shadingPipeline->SetImage(materialEmissiveResource.image.lock(), Sets::OTHER, 1, ImageAccess::Read);
			shadingPipeline->SetImage(normalEmissiveResource.image.lock(), Sets::OTHER, 2, ImageAccess::Read);
			shadingPipeline->SetImage(depthResource.image.lock(), Sets::OTHER, 3, ImageAccess::Read);
			shadingPipeline->SetImage(targetResource.image.lock(), Sets::OTHER, 4, ImageAccess::Write);

			shadingPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
			shadingPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myGlobalDescriptorSets[Sets::RENDERER_BUFFERS]->GetOrAllocateDescriptorSet(currentIndex), Sets::RENDERER_BUFFERS);
			shadingPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myGlobalDescriptorSets[Sets::PBR_RESOURCES]->GetOrAllocateDescriptorSet(currentIndex), Sets::PBR_RESOURCES);

			struct PushConstant
			{
				uint32_t debugMode = 0;
			} pushConstants;

			pushConstants.debugMode = 0;

			//shadingPipeline->PushConstants(myCommandBuffer->GetCurrentCommandBuffer(), &pushConstants, sizeof(PushConstant));

			constexpr uint32_t threadCount = 8;

			const uint32_t dispatchX = std::max(1u, (targetResource.image.lock()->GetWidth() / threadCount) + 1);
			const uint32_t dispatchY = std::max(1u, (targetResource.image.lock()->GetHeight() / threadCount) + 1);
			Renderer::DispatchComputePipeline(commandBuffer, shadingPipeline, dispatchX, dispatchY, 1);

			shadingPipeline->ClearAllResources();

			EndTimestamp();
		});
	}

	void SceneRenderer::AddDecalPass(FrameGraph& frameGraph)
	{
		const auto& gbufferData = frameGraph.GetBlackboard().Get<GBufferData>();

		frameGraph.AddRenderPass("Decal Pass",
		[&](FrameGraph::Builder& builder)
		{
			builder.WriteResource(gbufferData.albedo);
			builder.WriteResource(gbufferData.materialEmissive);
			builder.WriteResource(gbufferData.normalEmissive);
			builder.ReadResource(gbufferData.depth);
		},

		[=, &commands = GetGPUData().decalCommands](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Decal Pass");

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(45, 53, 110);
			renderPassInfo.name = "Decal Pass";

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(gbufferData.albedo),
					resources.GetImageResource(gbufferData.materialEmissive),
					resources.GetImageResource(gbufferData.normalEmissive)
				});

			renderingInfo.colorAttachmentInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			renderingInfo.colorAttachmentInfo[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			renderingInfo.colorAttachmentInfo[2].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			const auto& depthResource = resources.GetImageResource(gbufferData.depth);

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);

			for (const auto& decalCmd : commands)
			{
				decalCmd.material->GetSubMaterialAt(0)->Set(0, depthResource.image.lock());
				decalCmd.material->GetSubMaterialAt(0)->SetValue("targetSize", myRenderSize);
				decalCmd.material->GetSubMaterialAt(0)->SetValue("transform", decalCmd.transform);
				decalCmd.material->GetSubMaterialAt(0)->SetValue("materialIndex", Renderer::GetBindlessData().materialTable->GetIndexFromMaterial(decalCmd.material->TryGetSubMaterialAt(0).get()));
				decalCmd.material->GetSubMaterialAt(0)->SetValue("randomValue", decalCmd.randomValue);
				decalCmd.material->GetSubMaterialAt(0)->SetValue("timeSinceCreation", decalCmd.timeSinceCreation);
				Renderer::DrawMesh(commandBuffer, myDecalMesh, decalCmd.material, myGlobalDescriptorSets);
			}

			Renderer::EndFrameGraphPass(commandBuffer);

			EndTimestamp();
		});
	}

	void SceneRenderer::AddForwardOpaquePass(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& dirShadowData = frameGraph.GetBlackboard().Get<DirectionalShadowData>();

		const auto& volumetricFogData = frameGraph.GetBlackboard().Get<VolumetricFogData>();
		const auto& gtaoData = frameGraph.GetBlackboard().Get<GTAOOutput>();
		const auto& gbufferData = frameGraph.GetBlackboard().Get<GBufferData>();

		frameGraph.GetBlackboard().Add<ForwardData>() = frameGraph.AddRenderPass<ForwardData>("Forward Pass",
		[&](FrameGraph::Builder& builder, ForwardData& data)
		{
			builder.WriteResource(skyboxData.outputImage);
			builder.WriteResource(gbufferData.depth);

			if (myEnvironmentSettings.radianceMap)
			{
				builder.ReadResource(skyboxData.radianceImage);
				builder.ReadResource(skyboxData.irradianceImage);
			}
			else
			{
				const auto& preethamData = frameGraph.GetBlackboard().Get<PreethamSkyData>();
				builder.ReadResource(preethamData.skybox);
			}

			if (GetGPUData().directionalLight.castShadows)
			{
				builder.ReadResource(dirShadowData.shadowMap);
			}

			if (GetGPUData().spotLightShadowCount > 0)
			{
				const auto& spotlightShadowData = frameGraph.GetBlackboard().Get<SpotLightShadowData>();

				for (const auto& lightHandle : spotlightShadowData.spotLightShadows)
				{
					builder.ReadResource(lightHandle);
				}
			}

			if (GetGPUData().pointLightShadowCount > 0)
			{
				const auto& pointLightShadowData = frameGraph.GetBlackboard().Get<PointLightShadowData>();

				for (const auto& lightHandle : pointLightShadowData.pointLightShadows)
				{
					builder.ReadResource(lightHandle);
				}
			}

			if (mySettings.enableVolumetricFog)
			{
				builder.ReadResource(volumetricFogData.rayMarched);
			}

			if (mySettings.enableAO)
			{
				builder.ReadResource(gtaoData.outputImage);
			}
		},

		[=, &batches = GetGPUData().indirectBatches](const ForwardData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Forward Pass");

			const uint32_t index = commandBuffer->GetCurrentIndex();

			// Update needed descriptors
			{
				auto resourcesDescriptor = myGlobalDescriptorSets[Sets::PBR_RESOURCES];
				resourcesDescriptor->GetOrAllocateDescriptorSet(index);

				if (mySettings.enableVolumetricFog)
				{
					const auto& fogResource = resources.GetImageResource(volumetricFogData.rayMarched);
					UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::VOLUMETRIC_FOG_TEXTURE, index, fogResource.image3D.lock());
				}
				else
				{
					UpdateGlobalDescriptorSet(resourcesDescriptor, Sets::PBR_RESOURCES, Bindings::VOLUMETRIC_FOG_TEXTURE, index, Renderer::GetDefaultData().black3DImage);
				}
			}

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(115, 237, 190);
			renderPassInfo.name = "Forward Pass";
			renderPassInfo.materialFlags = MaterialFlag::Opaque;

			IndirectPassParams passParams{};
			passParams.materialFlags = MaterialFlag::Opaque;

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(skyboxData.outputImage),
					resources.GetImageResource(gbufferData.depth)
				});

			renderingInfo.depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

			if (mySettings.enableSkybox)
			{
				renderingInfo.colorAttachmentInfo.front().loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			}
			else
			{
				renderingInfo.colorAttachmentInfo.front().clearValue.color = { 0.07f, 0.07f, 0.07f, 1.f };
			}

			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			const auto& indirectPass = GetOrCreateIndirectPass(commandBuffer, passParams);

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawIndirectBatches(commandBuffer, indirectPass, myGlobalDescriptorSets, batches);

			// Lines
			if (myLineData.vertexCount > 0)
			{
				Renderer::DrawVertexBuffer(commandBuffer, myLineData.vertexCount, myLineData.vertexBuffer->Get(index), myLineData.renderPipeline, myGlobalDescriptorSets);
				myLineData.vertexCount = 0;
				myLineData.vertexBufferPtr = myLineData.vertexBufferBase;
			}

			// Text
			{
				DispatchText(commandBuffer);
			}

			Renderer::EndFrameGraphPass(commandBuffer);

			EndTimestamp();
		});
	}

	void SceneRenderer::AddForwardSSSPass(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& gbufferData = frameGraph.GetBlackboard().Get<GBufferData>();
		const auto& dirShadowData = frameGraph.GetBlackboard().Get<DirectionalShadowData>();

		const auto& volumetricFogData = frameGraph.GetBlackboard().Get<VolumetricFogData>();
		const auto& gtaoData = frameGraph.GetBlackboard().Get<GTAOOutput>();

		frameGraph.GetBlackboard().Add<ForwardSSSData>() = frameGraph.AddRenderPass<ForwardSSSData>("Forward SSS Pass",
			[&](FrameGraph::Builder& builder, ForwardSSSData& data)
		{
			data.diffuse = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, { 0.f }, "SSS Diffuse", ImageUsage::AttachmentStorage });

			builder.WriteResource(skyboxData.outputImage);
			builder.WriteResource(gbufferData.depth);

			if (myEnvironmentSettings.radianceMap)
			{
				builder.ReadResource(skyboxData.radianceImage);
				builder.ReadResource(skyboxData.irradianceImage);
			}
			else
			{
				const auto& preethamData = frameGraph.GetBlackboard().Get<PreethamSkyData>();
				builder.ReadResource(preethamData.skybox);
			}

			if (GetGPUData().directionalLight.castShadows)
			{
				builder.ReadResource(dirShadowData.shadowMap);
			}

			if (GetGPUData().spotLightShadowCount > 0)
			{
				const auto& spotlightShadowData = frameGraph.GetBlackboard().Get<SpotLightShadowData>();

				for (const auto& lightHandle : spotlightShadowData.spotLightShadows)
				{
					builder.ReadResource(lightHandle);
				}
			}

			if (GetGPUData().pointLightShadowCount > 0)
			{
				const auto& pointLightShadowData = frameGraph.GetBlackboard().Get<PointLightShadowData>();

				for (const auto& lightHandle : pointLightShadowData.pointLightShadows)
				{
					builder.ReadResource(lightHandle);
				}
			}

			if (mySettings.enableVolumetricFog)
			{
				builder.ReadResource(volumetricFogData.rayMarched);
			}

			if (mySettings.enableAO)
			{
				builder.ReadResource(gtaoData.outputImage);
			}
		},

			[=, &batches = GetGPUData().indirectBatches](const ForwardSSSData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Forward SSS Pass");

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(115, 237, 190);
			renderPassInfo.name = "Forward SSS Pass";
			renderPassInfo.materialFlags = MaterialFlag::SSS;

			IndirectPassParams passParams{};
			passParams.materialFlags = MaterialFlag::SSS;

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(data.diffuse),
					resources.GetImageResource(skyboxData.outputImage),
					resources.GetImageResource(gbufferData.depth)
				});

			renderingInfo.colorAttachmentInfo.back().loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			renderingInfo.depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			const auto& indirectPass = GetOrCreateIndirectPass(commandBuffer, passParams);

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawIndirectBatches(commandBuffer, indirectPass, myGlobalDescriptorSets, batches);
			Renderer::EndFrameGraphPass(commandBuffer);

			EndTimestamp();
		});
	}

	void SceneRenderer::AddSSSBlurPass(FrameGraph& frameGraph)
	{
		const auto& forwardSSSData = frameGraph.GetBlackboard().Get<ForwardSSSData>();
		const auto& depthData = frameGraph.GetBlackboard().Get<PreDepthData>();

		frameGraph.GetBlackboard().Add<SSSBlurData>() = frameGraph.AddRenderPass<SSSBlurData>("SSS Blur",
			[&](FrameGraph::Builder& builder, SSSBlurData& data)
		{
			data.blurOne = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, { 0.f }, "SSS Blur One", ImageUsage::AttachmentStorage });
			data.blurTwo = forwardSSSData.diffuse;

			builder.WriteResource(data.blurTwo);
			builder.ReadResource(depthData.preDepth);

			builder.SetIsComputePass();
		},

		[=](const SSSBlurData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("SSS Blur Pass");

			const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

			struct PushConstants
			{
				glm::vec2 texelSize;
				glm::vec2 dir;
				float sssWidth;
			} pushConstants;

			pushConstants.texelSize.x = 1.f / myRenderSize.x;
			pushConstants.texelSize.y = 1.f / myRenderSize.y;

			constexpr float sssWidth = 10.f;

			pushConstants.dir = { 1.f, 0.f };
			pushConstants.sssWidth = sssWidth * 1.f / std::tanf(glm::radians(myCurrentCamera->GetFieldOfView()) * 0.05f) * static_cast<float>(myRenderSize.x) / static_cast<float>(myRenderSize.y);

			VkImageSubresourceRange subresourceRange{};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.layerCount = 1;
			subresourceRange.levelCount = 1;

			// Blur 0
			{
				const auto& diffuseInputImage = resources.GetImageResource(forwardSSSData.diffuse);
				const auto& depthInputImage = resources.GetImageResource(depthData.preDepth);
				const auto& outputBlurImage = resources.GetImageResource(data.blurOne);

				auto diffuseImage = diffuseInputImage.image.lock();
				Utility::TransitionImageLayout(commandBuffer->GetCurrentCommandBuffer(), diffuseImage->GetHandle(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);

				diffuseImage->TransitionToLayout(commandBuffer->GetCurrentCommandBuffer(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				mySSSBlurPipeline[0]->SetImage(diffuseInputImage.image.lock(), Sets::OTHER, 0, ImageAccess::Read);
				mySSSBlurPipeline[0]->SetImage(depthInputImage.image.lock(), Sets::OTHER, 1, ImageAccess::Read);
				mySSSBlurPipeline[0]->SetImage(outputBlurImage.image.lock(), Sets::OTHER, 2, ImageAccess::Write);

				mySSSBlurPipeline[0]->Bind(commandBuffer->GetCurrentCommandBuffer());
				mySSSBlurPipeline[0]->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &pushConstants, sizeof(PushConstants));
				mySSSBlurPipeline[0]->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS)->GetOrAllocateDescriptorSet(currentIndex), Sets::RENDERER_BUFFERS);

				constexpr uint32_t threadCount = 16;

				const uint32_t dispatchX = std::max(1u, (outputBlurImage.image.lock()->GetWidth() / threadCount) + 1);
				const uint32_t dispatchY = std::max(1u, (outputBlurImage.image.lock()->GetHeight() / threadCount) + 1);

				ImageBarrierInfo imageBarrier{};
				imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
				imageBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
				imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
				imageBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
				imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageBarrier.subresourceRange.baseArrayLayer = 0;
				imageBarrier.subresourceRange.baseMipLevel = 0;
				imageBarrier.subresourceRange.layerCount = 1;
				imageBarrier.subresourceRange.levelCount = 1;

				mySSSBlurPipeline[0]->InsertImageBarrier(Sets::OTHER, 2, imageBarrier);

				Renderer::DispatchComputePipeline(commandBuffer, mySSSBlurPipeline[0], dispatchX, dispatchY, 1);
			}

			// Blur 1
			{
				pushConstants.dir = { 0.f, 1.f };

				const auto& diffuseInputImage = resources.GetImageResource(data.blurOne);
				const auto& depthInputImage = resources.GetImageResource(depthData.preDepth);
				const auto& outputBlurImage = resources.GetImageResource(data.blurTwo);

				auto outputImage = outputBlurImage.image.lock();
				Utility::TransitionImageLayout(commandBuffer->GetCurrentCommandBuffer(), outputImage->GetHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, subresourceRange);

				mySSSBlurPipeline[1]->SetImage(diffuseInputImage.image.lock(), Sets::OTHER, 0, ImageAccess::Read);
				mySSSBlurPipeline[1]->SetImage(depthInputImage.image.lock(), Sets::OTHER, 1, ImageAccess::Read);
				mySSSBlurPipeline[1]->SetImage(outputBlurImage.image.lock(), Sets::OTHER, 2, ImageAccess::Write);

				mySSSBlurPipeline[1]->Bind(commandBuffer->GetCurrentCommandBuffer());
				mySSSBlurPipeline[1]->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &pushConstants, sizeof(PushConstants));
				mySSSBlurPipeline[1]->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS)->GetOrAllocateDescriptorSet(currentIndex), Sets::RENDERER_BUFFERS);

				constexpr uint32_t threadCount = 16;

				const uint32_t dispatchX = std::max(1u, (outputBlurImage.image.lock()->GetWidth() / threadCount) + 1);
				const uint32_t dispatchY = std::max(1u, (outputBlurImage.image.lock()->GetHeight() / threadCount) + 1);

				Renderer::DispatchComputePipeline(commandBuffer, mySSSBlurPipeline[1], dispatchX, dispatchY, 1);
			}

			EndTimestamp();
		});
	}

	void SceneRenderer::AddSSSCompositePass(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& blurData = frameGraph.GetBlackboard().Get<SSSBlurData>();

		frameGraph.AddRenderPass("SSS Composite Pass",
			[&](FrameGraph::Builder& builder)
		{
			builder.WriteResource(skyboxData.outputImage);
			builder.ReadResource(blurData.blurTwo);

			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},

		[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("SSS Composite Pass");

			const auto& outputImageResource = resources.GetImageResource(skyboxData.outputImage);
			const auto& sssBlurResource = resources.GetImageResource(blurData.blurTwo);

			mySSSCompositePipeline->SetImage(sssBlurResource.image.lock(), Sets::OTHER, 0, ImageAccess::Read);
			mySSSCompositePipeline->SetImage(outputImageResource.image.lock(), Sets::OTHER, 1, ImageAccess::Write);
			mySSSCompositePipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			constexpr uint32_t threadCount = 16;

			const uint32_t dispatchX = std::max(1u, (outputImageResource.image.lock()->GetWidth() / threadCount) + 1);
			const uint32_t dispatchY = std::max(1u, (outputImageResource.image.lock()->GetHeight() / threadCount) + 1);

			Renderer::DispatchComputePipeline(commandBuffer, mySSSCompositePipeline, dispatchX, dispatchY, 1);
			EndTimestamp();
		});
	}

	void SceneRenderer::AddForwardTransparentPass(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& gbufferData = frameGraph.GetBlackboard().Get<GBufferData>();
		const auto& dirShadowData = frameGraph.GetBlackboard().Get<DirectionalShadowData>();

		const auto& volumetricFogData = frameGraph.GetBlackboard().Get<VolumetricFogData>();
		const auto& gtaoData = frameGraph.GetBlackboard().Get<GTAOOutput>();

		frameGraph.GetBlackboard().Add<ForwardTransparentData>() = frameGraph.AddRenderPass<ForwardTransparentData>("Forward Transparent Pass",
		[&](FrameGraph::Builder& builder, ForwardTransparentData& data)
		{
			data.accumulation = builder.CreateTexture({ ImageFormat::RGBA16F, myRenderSize, { 0.f }, "Accumulation" });
			data.revealage = builder.CreateTexture({ ImageFormat::R8U, myRenderSize, { 1.f }, "Revealage" });

			builder.WriteResource(gbufferData.depth);

			if (myEnvironmentSettings.radianceMap)
			{
				builder.ReadResource(skyboxData.radianceImage);
				builder.ReadResource(skyboxData.irradianceImage);
			}
			else
			{
				const auto& preethamData = frameGraph.GetBlackboard().Get<PreethamSkyData>();
				builder.ReadResource(preethamData.skybox);
			}

			if (GetGPUData().directionalLight.castShadows)
			{
				builder.ReadResource(dirShadowData.shadowMap);
			}

			if (GetGPUData().spotLightShadowCount > 0)
			{
				const auto& spotlightShadowData = frameGraph.GetBlackboard().Get<SpotLightShadowData>();

				for (const auto& lightHandle : spotlightShadowData.spotLightShadows)
				{
					builder.ReadResource(lightHandle);
				}
			}

			if (GetGPUData().pointLightShadowCount > 0)
			{
				const auto& pointLightShadowData = frameGraph.GetBlackboard().Get<PointLightShadowData>();

				for (const auto& lightHandle : pointLightShadowData.pointLightShadows)
				{
					builder.ReadResource(lightHandle);
				}
			}

			if (mySettings.enableVolumetricFog)
			{
				builder.ReadResource(volumetricFogData.rayMarched);
			}

			if (mySettings.enableAO)
			{
				builder.ReadResource(gtaoData.outputImage);
			}
		},

		[=, &batches = GetGPUData().indirectBatches](const ForwardTransparentData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Forward Transparent Pass");

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(115, 237, 190);
			renderPassInfo.name = "Forward Transparent Pass";
			renderPassInfo.materialFlags = MaterialFlag::Transparent;

			IndirectPassParams passParams{};
			passParams.materialFlags = MaterialFlag::Transparent;

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(data.accumulation),
					resources.GetImageResource(data.revealage),
					resources.GetImageResource(gbufferData.depth)
				});

			renderingInfo.depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			const auto& indirectPass = GetOrCreateIndirectPass(commandBuffer, passParams);

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawIndirectBatches(commandBuffer, indirectPass, myGlobalDescriptorSets, batches);
			Renderer::EndFrameGraphPass(commandBuffer);

			EndTimestamp();
		});
	}

	void SceneRenderer::AddTransparentCompositePass(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& forwardTransparentData = frameGraph.GetBlackboard().Get<ForwardTransparentData>();

		frameGraph.AddRenderPass("Transparent Composite",

		[&](FrameGraph::Builder& builder)
		{
			builder.ReadResource(forwardTransparentData.accumulation);
			builder.ReadResource(forwardTransparentData.revealage);
			builder.WriteResource(skyboxData.outputImage);
		},

		[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Transparent Composite");
			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(115, 237, 190);
			renderPassInfo.name = "Transparent Composite Pass";

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(skyboxData.outputImage)
				});

			renderingInfo.colorAttachmentInfo.front().loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			const auto& accumResource = resources.GetImageResource(forwardTransparentData.accumulation);
			const auto& revealResource = resources.GetImageResource(forwardTransparentData.revealage);

			myTransparentCompositeMaterial->GetSubMaterialAt(0)->Set(0, accumResource.image.lock());
			myTransparentCompositeMaterial->GetSubMaterialAt(0)->Set(1, revealResource.image.lock());

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawFullscreenTriangleWithMaterial(commandBuffer, myTransparentCompositeMaterial, myGlobalDescriptorSets);
			Renderer::EndFrameGraphPass(commandBuffer);
			EndTimestamp();
		});
	}

	void SceneRenderer::AddParticlesPass(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& gbufferData = frameGraph.GetBlackboard().Get<GBufferData>();

		frameGraph.AddRenderPass("Particle Pass",
			[&](FrameGraph::Builder& builder)
		{
			builder.WriteResource(skyboxData.outputImage);
			builder.WriteResource(gbufferData.depth);
		},

			[=, &batches = GetGPUData().particleBatches](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Particle Pass");

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(115, 237, 190);
			renderPassInfo.name = "Particle Pass";

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources({ resources.GetImageResource(skyboxData.outputImage), resources.GetImageResource(gbufferData.depth) });
			renderingInfo.colorAttachmentInfo.front().loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // #TODO_Ivar: Improve
			renderingInfo.depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawParticleBatches(commandBuffer, myShaderStorageBufferSet, myGlobalDescriptorSets, batches);
			Renderer::EndFrameGraphPass(commandBuffer);
			EndTimestamp();
		});
	}

	void SceneRenderer::AddBillboardPass(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& idData = frameGraph.GetBlackboard().Get<IDData>();

		frameGraph.AddRenderPass("Billboard Pass",
			[&](FrameGraph::Builder& builder)
		{
			builder.WriteResource(skyboxData.outputImage);
			builder.WriteResource(idData.id);
		},

			[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Billboard Pass");

			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(115, 237, 190);
			renderPassInfo.name = "Billboard Pass";

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources({ resources.GetImageResource(skyboxData.outputImage), resources.GetImageResource(idData.id) });
			renderingInfo.colorAttachmentInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // #TODO_Ivar: Improve
			renderingInfo.colorAttachmentInfo[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // #TODO_Ivar: Improve
			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);

			Renderer::DrawVertexBuffer(commandBuffer, myBillboardData.vertexCount, myBillboardData.vertexBuffer->Get(commandBuffer->GetCurrentIndex()), myBillboardData.renderPipeline, myGlobalDescriptorSets);
			myBillboardData.vertexBufferPtr = myBillboardData.vertexBufferBase;
			myBillboardData.vertexCount = 0;

			Renderer::EndFrameGraphPass(commandBuffer);
			EndTimestamp();
		});
	}

	void SceneRenderer::AddLuminosityPass(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();

		frameGraph.GetBlackboard().Add<LuminosityData>() = frameGraph.AddRenderPass<LuminosityData>("Luminosity Pass",
		[&](FrameGraph::Builder& builder, LuminosityData& data)
		{
			FrameGraphTextureSpecification spec{};
			spec.format = ImageFormat::RGBA16F;
			spec.width = myRenderSize.x;
			spec.height = myRenderSize.y;
			spec.usage = ImageUsage::AttachmentStorage;
			spec.mips = BLOOM_MIP_COUNT;
			spec.name = "Luminosity";

			data.luminosityImage = builder.CreateTexture(spec);
			builder.ReadResource(skyboxData.outputImage);
			builder.SetIsComputePass();
		},

		[=](const LuminosityData& data, FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Luminosity Pass");

			const auto& srcColorResource = resources.GetImageResource(skyboxData.outputImage);
			const auto& targetResource = resources.GetImageResource(data.luminosityImage);

			myLuminosityPipeline->SetImage(targetResource.image.lock(), Sets::OTHER, 0, ImageAccess::Write);
			myLuminosityPipeline->SetImage(srcColorResource.image.lock(), Sets::OTHER, 1, ImageAccess::Read);
			myLuminosityPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			constexpr uint32_t threadCount = 8;

			const uint32_t dispatchX = std::max(1u, (targetResource.image.lock()->GetWidth() / threadCount) + 1);
			const uint32_t dispatchY = std::max(1u, (targetResource.image.lock()->GetHeight() / threadCount) + 1);

			Renderer::DispatchComputePipeline(commandBuffer, myLuminosityPipeline, dispatchX, dispatchY, 1);

			myLuminosityPipeline->ClearAllResources();
			EndTimestamp();
		});
	}

	void SceneRenderer::AddPostProcessingStackPasses(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();

		for (const auto& effect : GetGPUData().postProcessingStack->GetAllEffects())
		{
			Ref<PostProcessingMaterial> postMaterial = AssetManager::GetAsset<PostProcessingMaterial>(effect.materialHandle);
			if (!postMaterial || !postMaterial->IsValid())
			{
				continue;
			}

			const std::string name = postMaterial->GetName();

			frameGraph.AddRenderPass(name,
			[&](FrameGraph::Builder& builder)
			{
				builder.WriteResource(skyboxData.outputImage);

				builder.SetIsComputePass();
				builder.SetHasSideEffect();
			},

			[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
			{
				BeginTimestamp(name);

				const auto& outputImageResource = resources.GetImageResource(skyboxData.outputImage);
				postMaterial->Render(commandBuffer, outputImageResource.image.lock());
				EndTimestamp();
			});
		}
	}

	void SceneRenderer::AddACESPass(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();

		frameGraph.AddRenderPass("ACES Pass",
			[&](FrameGraph::Builder& builder)
		{
			builder.WriteResource(skyboxData.outputImage);

			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},

		[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("ACES Pass");

			const auto& outputImageResource = resources.GetImageResource(skyboxData.outputImage);

			myACESPipeline->SetImage(outputImageResource.image.lock(), Sets::OTHER, 0, ImageAccess::Write);
			myACESPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			constexpr uint32_t threadCount = 8;

			const uint32_t dispatchX = std::max(1u, (outputImageResource.image.lock()->GetWidth() / threadCount) + 1);
			const uint32_t dispatchY = std::max(1u, (outputImageResource.image.lock()->GetHeight() / threadCount) + 1);

			Renderer::DispatchComputePipeline(commandBuffer, myACESPipeline, dispatchX, dispatchY, 1);
			EndTimestamp();
		});
	}

	void SceneRenderer::AddGammaCorrectionPass(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();

		frameGraph.AddRenderPass("Gamma Correction Pass",
			[&](FrameGraph::Builder& builder)
		{
			builder.WriteResource(skyboxData.outputImage);

			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},

		[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Gamma Correction Pass");

			const auto& outputImageResource = resources.GetImageResource(skyboxData.outputImage);

			myGammaCorrectionPipeline->SetImage(outputImageResource.image.lock(), Sets::OTHER, 0, ImageAccess::Write);
			myGammaCorrectionPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

			constexpr uint32_t threadCount = 8;

			const uint32_t dispatchX = std::max(1u, (outputImageResource.image.lock()->GetWidth() / threadCount) + 1);
			const uint32_t dispatchY = std::max(1u, (outputImageResource.image.lock()->GetHeight() / threadCount) + 1);

			Renderer::DispatchComputePipeline(commandBuffer, myGammaCorrectionPipeline, dispatchX, dispatchY, 1);
			EndTimestamp();
		});
	}

	void SceneRenderer::ClearCountBuffer(const IndirectPass& pass, Ref<CommandBuffer> commandBuffer)
	{
		{
			BufferBarrierInfo barrierInfo{};
			barrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			barrierInfo.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;

			barrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			barrierInfo.dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;

			pass.clearCountBufferPipeline->InsertStorageBufferBarrier(pass.drawCountIDStorageBuffer, Sets::DRAW_BUFFERS, Bindings::INDIRECT_COUNTS, barrierInfo);
		}

		pass.clearCountBufferPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());

		const uint32_t dispatchCount = std::max(1u, (uint32_t)(GetGPUData().submitCommands.size() / 256) + 1u);
		const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

		const uint32_t currentSize = (uint32_t)pass.drawCountIDStorageBuffer->Get(Sets::DRAW_BUFFERS, Bindings::INDIRECT_COUNTS, commandBuffer->GetCurrentIndex())->GetElementCount();

		pass.clearCountBufferPipeline->Clear(currentIndex);
		pass.clearCountBufferPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), pass.drawBuffersSet->GetOrAllocateDescriptorSet(currentIndex), Sets::DRAW_BUFFERS);
		pass.clearCountBufferPipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &currentSize, sizeof(uint32_t));
		pass.clearCountBufferPipeline->Dispatch(commandBuffer->GetCurrentCommandBuffer(), dispatchCount, 1, 1, currentIndex);
		pass.clearCountBufferPipeline->InsertBarriers(commandBuffer->GetCurrentCommandBuffer(), currentIndex);
	}

	void SceneRenderer::CullRenderCommands(const IndirectPass& pass, const IndirectPassParams& params, Ref<CommandBuffer> commandBuffer)
	{
		VT_PROFILE_FUNCTION();

		struct CullData
		{
			float P00, P11, zNear, zFar;

			float frustum0;
			float frustum1;
			float frustum2;
			float frustum3;

			uint32_t drawCallCount;
			uint32_t materialFlags;
			uint32_t cullMode;
			uint32_t enableLodSelection;

			glm::vec4 aabbMin = 0.f;
			glm::vec4 aabbMax = 0.f;

		} indirectCullData{};

		indirectCullData.drawCallCount = (uint32_t)GetGPUData().submitCommands.size();

		const auto projection = params.cullMode == DrawCullMode::Frustum ? params.projection : myCurrentCamera->GetProjection();
		const glm::mat4 projTranspose = glm::transpose(projection);

		indirectCullData.P00 = projection[0][0];
		indirectCullData.P11 = projection[1][1];


		if (params.cullMode == DrawCullMode::Frustum)
		{
			const auto row4 = projection[3];

			indirectCullData.zNear = row4.z / (row4.w - 1.f);
			indirectCullData.zFar = row4.z / (row4.w + 1.f);
		}
		else
		{
			indirectCullData.zNear = myCurrentCamera->GetNearPlane();
			indirectCullData.zFar = myCurrentCamera->GetFarPlane();
		}

		const glm::vec4 frustumX = Math::NormalizePlane(projTranspose[3] + projTranspose[0]);
		const glm::vec4 frustumY = Math::NormalizePlane(projTranspose[3] + projTranspose[1]);

		indirectCullData.frustum0 = frustumX.x;
		indirectCullData.frustum1 = frustumX.z;
		indirectCullData.frustum2 = frustumY.y;
		indirectCullData.frustum3 = frustumY.z;

		indirectCullData.materialFlags = static_cast<uint32_t>(params.materialFlags);
		indirectCullData.cullMode = static_cast<uint32_t>(params.cullMode);
		indirectCullData.aabbMin = glm::vec4{ params.aabbMin, 0.f };
		indirectCullData.aabbMax = glm::vec4{ params.aabbMax, 0.f };

		{
			BufferBarrierInfo barrierInfo{};
			barrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			barrierInfo.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;

			barrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
			barrierInfo.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;

			pass.indirectCullPipeline->InsertStorageBufferBarrier(pass.drawCountIDStorageBuffer, Sets::DRAW_BUFFERS, Bindings::INDIRECT_COUNTS, barrierInfo);
		}

		{
			BufferBarrierInfo barrierInfo{};
			barrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			barrierInfo.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;

			barrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
			barrierInfo.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

			pass.indirectCullPipeline->InsertStorageBufferBarrier(pass.drawCountIDStorageBuffer, Sets::DRAW_BUFFERS, Bindings::DRAW_TO_OBJECT_ID, barrierInfo);
		}

		pass.indirectCullPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
		pass.indirectCullPipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &indirectCullData, sizeof(CullData));

		const uint32_t dispatchCount = std::max(1u, (uint32_t)(GetGPUData().submitCommands.size() / 256) + 1u);
		const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

		pass.indirectCullPipeline->Clear(currentIndex);
		pass.indirectCullPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), pass.drawBuffersSet->GetOrAllocateDescriptorSet(currentIndex), Sets::DRAW_BUFFERS);
		pass.indirectCullPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS)->GetOrAllocateDescriptorSet(currentIndex), Sets::RENDERER_BUFFERS);
		pass.indirectCullPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), Renderer::GetBindlessData().globalDescriptorSets[Sets::MAINBUFFERS]->GetOrAllocateDescriptorSet(currentIndex), Sets::MAINBUFFERS);

		pass.indirectCullPipeline->Dispatch(commandBuffer->GetCurrentCommandBuffer(), dispatchCount, 1, 1, currentIndex);
		pass.indirectCullPipeline->InsertBarriers(commandBuffer->GetCurrentCommandBuffer(), currentIndex);
	}

	void SceneRenderer::PerformLODSelection(Ref<CommandBuffer> commandBuffer)
	{
		VT_PROFILE_FUNCTION();

		struct SelectionData
		{
			uint32_t drawCallCount = 0;
			glm::vec3 cameraPosition = 0.f;

			float lodBase = 0.f;
			float lodStep = 0.f;
		} selectionData;

		selectionData.lodBase = 10.f;
		selectionData.lodStep = 1.5f;
		selectionData.drawCallCount = (uint32_t)GetGPUData().submitCommands.size();
		selectionData.cameraPosition = myCurrentCamera->GetPosition();

		{
			BufferBarrierInfo barrierInfo{};
			barrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			barrierInfo.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;

			barrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
			barrierInfo.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT;

			myLODSelectionPipeline->InsertStorageBufferBarrier(myShaderStorageBufferSet, Sets::RENDERER_BUFFERS, Bindings::MAIN_INDIRECT_ARGS, barrierInfo);
		}

		myLODSelectionPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
		myLODSelectionPipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &selectionData, sizeof(SelectionData));

		const uint32_t dispatchCount = std::max(1u, (uint32_t)(GetGPUData().submitCommands.size() / 256) + 1u);
		const uint32_t currentIndex = commandBuffer->GetCurrentIndex();

		myLODSelectionPipeline->Clear(currentIndex);
		myLODSelectionPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS)->GetOrAllocateDescriptorSet(currentIndex), Sets::RENDERER_BUFFERS);
		//myLODSelectionPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), Renderer::GetBindlessData().globalDescriptorSets[Sets::MAINBUFFERS]->GetOrAllocateDescriptorSet(Application::Get().GetWindow().GetSwapchain().GetCurrentFrame()), Sets::MAINBUFFERS);

		myLODSelectionPipeline->Dispatch(commandBuffer->GetCurrentCommandBuffer(), dispatchCount, 1, 1, currentIndex);
		myLODSelectionPipeline->InsertBarriers(commandBuffer->GetCurrentCommandBuffer(), currentIndex);
	}

	void SceneRenderer::AddLightCullPass(FrameGraph& frameGraph)
	{
		VT_PROFILE_FUNCTION();

		const auto& preDepthData = frameGraph.GetBlackboard().Get<PreDepthData>();

		frameGraph.AddRenderPass("Light Cull",
			[&](FrameGraph::Builder& builder)
		{
			builder.ReadResource(preDepthData.preDepth);
			builder.SetIsComputePass();
			builder.SetHasSideEffect();
		},

		[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			struct LightCullData
			{
				glm::uvec2	targetSize;
				glm::vec2 depthUnpackConsts;
			} cullData;

			cullData.targetSize = myRenderSize;

			const auto projectionMatrix = glm::perspective(glm::radians(myCurrentCamera->GetFieldOfView()), myCurrentCamera->GetAspectRatio(), myCurrentCamera->GetNearPlane(), myCurrentCamera->GetFarPlane());

			float depthLinearizeMul = (-projectionMatrix[3][2]);
			float depthLinearizeAdd = (projectionMatrix[2][2]);

			// correct the handedness issue
			if (depthLinearizeMul * depthLinearizeAdd < 0.f)
			{
				depthLinearizeAdd = -depthLinearizeAdd;
			}

			cullData.depthUnpackConsts = { depthLinearizeMul, depthLinearizeAdd };

			BeginTimestamp("Cull Lights");

			const auto& depthResource = resources.GetImageResource(preDepthData.preDepth);

			myLightCullPipeline->Bind(commandBuffer->GetCurrentCommandBuffer());
			myLightCullPipeline->PushConstants(commandBuffer->GetCurrentCommandBuffer(), &cullData, sizeof(LightCullData));

			myLightCullPipeline->SetImage(depthResource.image.lock(), Sets::OTHER, 0, ImageAccess::Read);
			myLightCullPipeline->BindDescriptorSet(commandBuffer->GetCurrentCommandBuffer(), myGlobalDescriptorSets.at(Sets::RENDERER_BUFFERS)->GetOrAllocateDescriptorSet(commandBuffer->GetCurrentIndex()), Sets::RENDERER_BUFFERS);

			{
				BufferBarrierInfo barrierInfo{};
				barrierInfo.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
				barrierInfo.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;

				barrierInfo.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
				barrierInfo.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

				myLightCullPipeline->InsertStorageBufferBarrier(myShaderStorageBufferSet, Sets::RENDERER_BUFFERS, Bindings::VISIBLE_POINT_LIGHTS, barrierInfo);
			}

			Renderer::DispatchComputePipeline(commandBuffer, myLightCullPipeline, myLightCullingWorkGroups.x, myLightCullingWorkGroups.y, 1);
			EndTimestamp();
		});
	}

	void SceneRenderer::AddGridPass(FrameGraph& frameGraph)
	{
		const auto& skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();
		const auto& gbufferData = frameGraph.GetBlackboard().Get<GBufferData>();

		frameGraph.AddRenderPass("Grid",

		[&](FrameGraph::Builder& builder)
		{
			builder.WriteResource(skyboxData.outputImage);
			builder.WriteResource(gbufferData.depth);
			builder.SetHasSideEffect();
		},

		[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			BeginTimestamp("Grid");
			FrameGraphRenderPassInfo renderPassInfo{};
			renderPassInfo.color = TO_NORMALIZEDRGB(115, 237, 190);
			renderPassInfo.name = "Grid Pass";

			FrameGraphRenderingInfo renderingInfo = FrameGraph::CreateRenderingInfoFromResources(
				{
					resources.GetImageResource(skyboxData.outputImage),
					resources.GetImageResource(gbufferData.depth)
				});

			renderingInfo.colorAttachmentInfo[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			renderingInfo.depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			renderingInfo.width = myRenderSize.x;
			renderingInfo.height = myRenderSize.y;

			Renderer::BeginFrameGraphPass(commandBuffer, renderPassInfo, renderingInfo);
			Renderer::DrawFullscreenQuadWithMaterial(commandBuffer, myGridMaterial, myGlobalDescriptorSets);
			Renderer::EndFrameGraphPass(commandBuffer);
			EndTimestamp();
		});
	}

	void SceneRenderer::AddCopyHistoryPass(FrameGraph& frameGraph)
	{
		const auto motionVectorData = frameGraph.GetBlackboard().Get<MotionVectorData>();
		const auto skyboxData = frameGraph.GetBlackboard().Get<SkyboxData>();

		frameGraph.AddRenderPass("Copy History",
			[&](FrameGraph::Builder& builder)
		{
			builder.ReadResource(motionVectorData.currentDepth);
			builder.ReadResource(skyboxData.outputImage);

			builder.SetHasSideEffect();
		},

			[=](FrameGraphRenderPassResources& resources, Ref<CommandBuffer> commandBuffer)
		{
			const auto& colorHistoryResource = resources.GetImageResource(skyboxData.outputImage);
			const auto& motionVectorResource = resources.GetImageResource(motionVectorData.currentDepth);

			myHistoryColor->CopyFromImage(commandBuffer->GetCurrentCommandBuffer(), colorHistoryResource.image.lock());
			myHistoryDepth->CopyFromImage(commandBuffer->GetCurrentCommandBuffer(), motionVectorResource.image.lock());
		});
	}

	void SceneRenderer::RenderUI()
	{
		VT_PROFILE_FUNCTION();

		auto scenePtr = myScene.lock();

		const auto halfSize = myScaledSize / 2u;

		UIRenderer::Begin(myOutputImage);
		UIRenderer::SetProjection(glm::ortho(-(float)halfSize.x, (float)halfSize.x, -(float)halfSize.y, (float)halfSize.y, 0.1f, 100.f));
		UIRenderer::SetView({ 1.f });

		scenePtr->GetRegistry().ForEachSafe<MonoScriptComponent, TransformComponent>([&](Wire::EntityId id, const MonoScriptComponent& scriptComp, const TransformComponent& transComp)
		{
			if (!transComp.visible)
			{
				return;
			}

			for (auto scriptId : scriptComp.scriptIds)
			{
				MonoScriptEngine::OnRenderUIInstance(scriptId);
			}
		});

		UIRenderer::DispatchSprites();
		UIRenderer::DispatchText();
		UIRenderer::End();
	}
}
