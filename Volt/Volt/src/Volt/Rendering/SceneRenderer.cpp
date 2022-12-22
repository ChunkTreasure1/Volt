#include "vtpch.h"
#include "SceneRenderer.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Animation/AnimationTreeController.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Mesh/SubMaterial.h"
#include "Volt/Asset/Video/Video.h"
#include "Volt/Asset/Text/Font.h"

#include "Volt/Animation/AnimationManager.h"
#include "Volt/Animation/AnimationStateMachine.h"
#include "Volt/Asset/Animation/Skeleton.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/Framebuffer.h"
#include "Volt/Rendering/RendererStructs.h"
#include "Volt/Rendering/ComputePipeline.h"
#include "Volt/Rendering/Shader/ShaderRegistry.h"
#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Rendering/Camera/Camera.h"
#include "Volt/Rendering/Texture/Texture2D.h"
#include "Volt/Rendering/Buffer/ConstantBuffer.h"
#include "Volt/Rendering/Buffer/StructuredBuffer.h"
#include "Volt/Rendering/Shape.h"

#include "Volt/Core/Profiling.h"
#include "Volt/Scene/Scene.h"

#include "Volt/Components/Components.h"
#include "Volt/Components/LightComponents.h"
#include "Volt/Components/PostProcessComponents.h"

#include "Volt/Scene/Entity.h"
#include "Volt/Particles/ParticleSystem.h"

#include "Volt/Utility/Math.h"
#include "Volt/Utility/Noise.h"

#include <GEM/gem.h>
#include <imgui.h>

#include <ranges>
#include <future>

namespace Volt
{
	SceneRenderer::SceneRenderer(Ref<Scene> aScene, const std::string& context, bool isLowRes)
		: myScene(aScene), myContext(context)
	{
		CreatePasses(isLowRes);

		myHighlightedAnimatedMeshes.reserve(20);
		myHighlightedMeshes.reserve(20);
	}

	void SceneRenderer::OnRenderEditor(Ref<Camera> aCamera)
	{
		OnRender(aCamera);
	}

	void SceneRenderer::OnRender(Ref<Camera> aCamera)
	{
		VT_PROFILE_FUNCTION();

		// Handle resize
		if (myShouldResize)
		{
			VT_PROFILE_SCOPE("Resize");

			myShouldResize = false;

			const uint32_t width = myResizeSize.x;
			const uint32_t height = myResizeSize.y;

			const uint32_t quarterWidth = width / 4;
			const uint32_t quarterHeight = height / 4;

			myPreDepthPass.framebuffer->Resize(width, height);
			mySkyboxPass.framebuffer->Resize(width, height);
			myDeferredPass.framebuffer->Resize(width, height);
			myShadingPass.framebuffer->Resize(width, height);
			myForwardPass.framebuffer->Resize(width, height);
			myFXAAPass.framebuffer->Resize(width, height);
			myBloomCompositePass.framebuffer->Resize(width, height);
			myGammaCorrectionPass.framebuffer->Resize(width, height);
			myVignettePass.framebuffer->Resize(width, height);
			myDecalPass.framebuffer->Resize(width, height);
			myHeightFogPass.framebuffer->Resize(width, height);

			myReinterleavingPass.framebuffer->Resize(width, height);
			myHBAOBlurPass[0].framebuffer->Resize(width, height);
			myHBAOBlurPass[1].framebuffer->Resize(width, height);
			myAOCompositePass.framebuffer->Resize(width, height);

			myDeinterleavingPass[0].framebuffer->Resize(quarterWidth, quarterHeight);
			myDeinterleavingPass[1].framebuffer->Resize(quarterWidth, quarterHeight);
			myHBAOPass.framebuffer->Resize(quarterWidth, quarterHeight);

			// Outline
			{
				myHighlightedGeometryPass.framebuffer->Resize(width, height);
				myJumpFloodInitPass.framebuffer->Resize(width, height);
				myJumpFloodCompositePass.framebuffer->Resize(width, height);

				for (uint32_t i = 0; i < 2; i++)
				{
					myJumpFloodPass[i].framebuffer->Resize(width, height);
				}
			}

			gem::vec2ui mipSize = { width, height };

			for (auto& pass : myBloomDownsamplePasses)
			{
				mipSize /= 2;
				mipSize = gem::max(mipSize, 1u);

				pass.framebuffer->Resize(mipSize.x, mipSize.y);
			}

			mipSize = { width, height };

			for (auto& myBloomUpsamplePass : std::ranges::reverse_view(myBloomUpsamplePasses))
			{
				mipSize /= 2;
				mipSize = gem::max(mipSize, 1u);

				myBloomUpsamplePass.framebuffer->Resize(mipSize.x, mipSize.y);
			}
		}

		auto& registry = myScene->GetRegistry();

		CollectStaticMeshCommands();
		CollectAnimatedMeshCommands();

		registry.ForEach<AnimationControllerComponent, TransformComponent, EntityDataComponent>([&](Wire::EntityId id, AnimationControllerComponent& animaitonTree, const TransformComponent& transformComp, const EntityDataComponent& dataComp)
			{
				if (animaitonTree.AnimTreeControl != nullptr)
				{
					animaitonTree.AnimTreeControl->Render(id, transformComp, dataComp, myScene);
				}
			});

		registry.ForEach<PointLightComponent, TransformComponent>([&](Wire::EntityId id, const PointLightComponent& pointLightComp, const TransformComponent& transformComp)
			{
				if (transformComp.visible)
				{
					PointLight light{};
					light.color = pointLightComp.color;
					light.falloff = pointLightComp.falloff;
					light.farPlane = pointLightComp.farPlane;
					light.intensity = pointLightComp.intensity;
					light.radius = pointLightComp.radius;
					light.castShadows = (uint32_t)pointLightComp.castShadows;

					auto lightEntity = Entity{ id, myScene.get() };
					auto trs = myScene->GetWorldSpaceTRS(lightEntity);
					light.position = trs.position;

					if (pointLightComp.castShadows)
					{
						const gem::mat4 projection = gem::perspective(gem::radians(90.f), 1.f, 1.f, pointLightComp.farPlane);

						light.viewProjectionMatrices[0] = projection * gem::lookAtLH(trs.position, trs.position + gem::vec3{ 1.f, 0.f, 0.f }, gem::vec3{ 0.f, 1.f, 0.f });
						light.viewProjectionMatrices[1] = projection * gem::lookAtLH(trs.position, trs.position + gem::vec3{ -1.f, 0.f, 0.f }, gem::vec3{ 0.f, 1.f, 0.f });
						light.viewProjectionMatrices[2] = projection * gem::lookAtLH(trs.position, trs.position + gem::vec3{ 0.f, 1.f, 0.f }, gem::vec3{ 0.f, 0.f, -1.f });
						light.viewProjectionMatrices[3] = projection * gem::lookAtLH(trs.position, trs.position + gem::vec3{ 0.f, -1.f, 0.f }, gem::vec3{ 0.f, 0.f, 1.f });
						light.viewProjectionMatrices[4] = projection * gem::lookAtLH(trs.position, trs.position + gem::vec3{ 0.f, 0.f, 1.f }, gem::vec3{ 0.f, 1.f, 0.f });
						light.viewProjectionMatrices[5] = projection * gem::lookAtLH(trs.position, trs.position + gem::vec3{ 0.f, 0.f, -1.f }, gem::vec3{ 0.f, 1.f, 0.f });
					}

					Renderer::SubmitLight(light);
				}
			});

		Ref<Camera> dirLightCamera = nullptr; // We probably don't want to heap allocate every frame

		registry.ForEach<DirectionalLightComponent, TransformComponent>([&](Wire::EntityId, const DirectionalLightComponent& dirLightComp, const TransformComponent& transformComp)
			{
				if (transformComp.visible)
				{
					DirectionalLight light{};
					light.colorIntensity = gem::vec4(dirLightComp.color.x, dirLightComp.color.y, dirLightComp.color.z, dirLightComp.intensity);

					const gem::vec3 dir = gem::normalize(gem::mat3(transformComp.GetTransform()) * gem::vec3(1.f)) * -1.f;
					light.direction = gem::vec4(dir.x, dir.y, dir.z, 1.f);
					light.castShadows = static_cast<uint32_t>(dirLightComp.castShadows);
					light.shadowBias = dirLightComp.shadowBias;

					if (dirLightComp.castShadows)
					{
						constexpr float size = 4000.f;
						const gem::vec3 camPos = { aCamera->GetPosition().x, 0.f, aCamera->GetPosition().z };

						const gem::mat4 view = gem::lookAtLH((dir * size) + camPos, gem::vec3{ 0.f } + camPos, { 0.f, 1.f, 0.f });
						const gem::mat4 projection = gem::ortho(-size, size, -size, size, 1.f, 100000.f);

						light.viewProjection = projection * view;

						dirLightCamera = CreateRef<Camera>();
						dirLightCamera->SetView(view);
						dirLightCamera->SetProjection(projection);
					}

					Renderer::SubmitLight(light);
				}
			});

		SceneEnvironment sceneEnvironment{ Renderer::GetDefaultData().blackCubeImage, Renderer::GetDefaultData().blackCubeImage };

		registry.ForEach<SkylightComponent>([&](Wire::EntityId, SkylightComponent& skylightComp)
			{
				if (skylightComp.environmentHandle != Asset::Null())
				{
					if (skylightComp.environmentHandle != skylightComp.lastEnvironmentHandle)
					{
						skylightComp.lastEnvironmentHandle = skylightComp.environmentHandle;
						skylightComp.currentSceneEnvironment = Renderer::GenerateEnvironmentMap(skylightComp.environmentHandle);
					}

					auto asset = AssetManager::GetAsset<Texture2D>(skylightComp.environmentHandle);
					if (asset && asset->IsValid())
					{
						sceneEnvironment = skylightComp.currentSceneEnvironment;
					}

					sceneEnvironment.lod = skylightComp.lod;
					sceneEnvironment.intensity = skylightComp.intensity;
				}

				Renderer::SetAmbianceMultiplier(skylightComp.intensity);
			});

		registry.ForEach<TextRendererComponent>([&](Wire::EntityId id, const TextRendererComponent& textComp)
			{
				Ref<Font> fontAsset = AssetManager::GetAsset<Font>(textComp.fontHandle);
				if (fontAsset && fontAsset->IsValid())
				{
					Renderer::SubmitString(textComp.text, fontAsset, myScene->GetWorldSpaceTransform(Entity{ id, myScene.get() }), textComp.maxWidth);
				}
			});

		registry.ForEach<VideoPlayerComponent>([&](Wire::EntityId id, const VideoPlayerComponent& videoPlayer)
			{
				if (videoPlayer.videoHandle != Asset::Null())
				{
					Ref<Video> video = AssetManager::GetAsset<Video>(videoPlayer.videoHandle);
					if (video && video->IsValid())
					{
						Renderer::SubmitSprite(video->GetTexture(), myScene->GetWorldSpaceTransform(Entity{ id, myScene.get() }), id);
					}
				}
			});

		registry.ForEach<DecalComponent, TransformComponent>([&](Wire::EntityId id, const DecalComponent& decalComponent, const TransformComponent& transComp)
			{
				if (transComp.visible)
				{
					Ref<Material> material = AssetManager::GetAsset<Material>(decalComponent.materialHandle);
					if (material && material->IsValid())
					{
						Renderer::SubmitDecal(material, myScene->GetWorldSpaceTransform(Entity{ id, myScene.get() }), id);
					}
				}
			});

		ResetPostProcess();

		registry.ForEach<HeightFogComponent>([&](Wire::EntityId id, const HeightFogComponent& comp)
			{
				myHeightFogData.fogColor = comp.color;
				myHeightFogData.fogMinY = comp.minY;
				myHeightFogData.fogMaxY = comp.maxY;
				myHeightFogData.strength = comp.strength;
			});

		registry.ForEach<HBAOComponent>([&](Wire::EntityId id, const HBAOComponent& comp)
			{
				myHBAOSettings.radius = comp.radius;
				myHBAOSettings.intensity = comp.intensity;
				myHBAOSettings.bias = comp.bias;
				myHBAOSettings.enabled = true;
			});

		registry.ForEach<BloomComponent>([&](Wire::EntityId id, const BloomComponent& comp)
			{
				myBloomSettings.enabled = true;
			});

		registry.ForEach<FXAAComponent>([&](Wire::EntityId id, const FXAAComponent& comp)
			{
				myFXAASettings.enabled = true;
			});

		registry.ForEach<VignetteComponent>([&](Wire::EntityId id, const VignetteComponent& comp)
			{
				myVignetteSettings.enabled = true;
				myVignetteSettings.color = comp.color;
				myVignetteSettings.sharpness = comp.sharpness;
				myVignetteSettings.width = comp.width;
			});

#ifdef VT_THREADED_RENDERING
		Renderer::Begin(Context::Deferred, "Test");
		Renderer::SetFullResolution({ (float)myResizeSize.x, (float)myResizeSize.y });

		// Skybox
		{
			Renderer::BeginPass(mySkyboxPass, aCamera);
			Renderer::BindTexturesToStage(ShaderStage::Pixel, { sceneEnvironment.radianceMap->GetSRV().Get() }, 11);
			Renderer::DrawMesh(mySkyboxMesh, { 1.f });
			Renderer::EndPass();
		}

		// Deferred
		{
			Renderer::BeginPass(myDeferredPass, aCamera);
			Renderer::DispatchRenderCommandsInstanced();
			Renderer::EndPass();
		}

		ShadingPass(sceneEnvironment);

		Renderer::End();
		Renderer::SyncAndWait();
#else

		Renderer::SetDepthState(DepthState::ReadWrite);
		Renderer::ResetStatistics();
		Renderer::Begin(myContext);
		Renderer::SetFullResolution({ (float)myResizeSize.x, (float)myResizeSize.y });

		// Directional Shadow Pass
		{
			VT_PROFILE_SCOPE("SceneRenderer::DirectionalLightShadow");

			if (dirLightCamera)
			{
				Renderer::BeginPass(myDirectionalShadowPass, dirLightCamera, true, true);
				Renderer::DispatchRenderCommandsInstanced();
				Renderer::EndPass();
			}
		}

		// Point Light Shadow Pass
		{
			//VT_PROFILE_SCOPE("SceneRenderer::PointLightShadow");
			//Renderer::BeginPass(myPointLightPass, aCamera, true, true);

			//uint32_t lightIndex = 0;
			//registry.ForEach<PointLightComponent, TransformComponent>([&](Wire::EntityId id, const PointLightComponent& pointLightComp, const TransformComponent& transformComp)
			//	{
			//		if (transformComp.visible)
			//		{
			//			if (pointLightComp.castShadows)
			//			{
			//				const uint32_t data[4] = { lightIndex, 0, 0, 0 };

			//				myPointLightBuffer->SetData(data, sizeof(uint32_t) * 4);
			//				myPointLightBuffer->Bind(13);
			//				Renderer::DispatchRenderCommandsInstanced();
			//			}

			//			lightIndex++;
			//		}
			//	});

			//Renderer::EndPass();
		}

		// Pre depth
		{
			Renderer::BeginPass(myPreDepthPass, aCamera, true, false, true);
			Renderer::DispatchRenderCommandsInstanced();
			Renderer::EndPass();
		}

		// Test Voxel
		{
			//Renderer::BeginPass(myVoxelPass, aCamera);

			//VoxelSceneData data{};
			//myVoxelBuffer->SetData(&data, sizeof(VoxelSceneData));
			//myVoxelBuffer->Bind(13);

			//auto context = GraphicsContext::GetContext();
			//context->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, 1, myVoxelResultBuffer->GetUAV().GetAddressOf(), nullptr);
			//context->PSSetShaderResources(14, 1, myDirectionalShadowPass.framebuffer->GetDepthAttachment()->GetSRV().GetAddressOf());

			//Renderer::DispatchRenderCommandsInstanced();
			//Renderer::EndPass();
		}

		// Deferred Pass
		{
			VT_PROFILE_SCOPE("SceneRenderer::Deferred");

			Renderer::BeginPass(myDeferredPass, aCamera);
			Renderer::DispatchRenderCommandsInstanced();
			Renderer::EndPass();
		}

		// Decals
		{
			VT_PROFILE_SCOPE("SceneRenderer::Decals");
			auto context = GraphicsContext::GetImmediateContext();

			Renderer::BeginSection("Decals");

			context->PSSetShaderResources(11, 1, myDeferredPass.framebuffer->GetColorAttachment(5)->GetSRV().GetAddressOf());
			context->PSSetShaderResources(12, 1, myDeferredPass.framebuffer->GetColorAttachment(3)->GetSRV().GetAddressOf());

			myDecalPass.framebuffer->Clear();
			myDecalPass.framebuffer->Bind();
			Renderer::DispatchDecalsWithShader(myDecalPass.overrideShader);
			myDecalPass.framebuffer->Unbind();

			ID3D11ShaderResourceView* nullSRV[12] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
			context->PSSetShaderResources(0, 12, nullSRV);


			Renderer::EndSection("Decals");
		}

		// Skybox
		{
			Renderer::SetDepthState(DepthState::None);
			Renderer::SetRasterizerState(RasterizerState::CullFront);
			Renderer::BeginPass(mySkyboxPass, aCamera);

			auto context = GraphicsContext::GetImmediateContext();
			context->PSSetShaderResources(11, 1, sceneEnvironment.radianceMap->GetSRV().GetAddressOf());

			mySkyboxData.intensity = sceneEnvironment.intensity;
			mySkyboxData.textureLod = sceneEnvironment.lod;
			mySkyboxBuffer->SetData(&mySkyboxData, sizeof(SkyboxData));
			mySkyboxBuffer->Bind(13);
			Renderer::DrawMesh(mySkyboxMesh, gem::mat4{ 1.f });

			Renderer::EndPass();
			Renderer::SetRasterizerState(RasterizerState::CullBack);
			Renderer::SetDepthState(DepthState::ReadWrite);
		}

		ShadingPass(sceneEnvironment);

		// Forward Pass
		{
			VT_PROFILE_SCOPE("SceneRenderer::Forward");

			Renderer::SetDepthState(DepthState::ReadWrite);
			Renderer::BeginPass(myForwardPass, aCamera);

			auto context = GraphicsContext::GetImmediateContext();

			context->PSSetShaderResources(11, 1, Renderer::GetDefaultData().brdfLut->GetSRV().GetAddressOf());
			context->PSSetShaderResources(12, 1, sceneEnvironment.irradianceMap->GetSRV().GetAddressOf());
			context->PSSetShaderResources(13, 1, sceneEnvironment.radianceMap->GetSRV().GetAddressOf());
			context->PSSetShaderResources(14, 1, myDirectionalShadowPass.framebuffer->GetDepthAttachment()->GetSRV().GetAddressOf());
			context->PSSetShaderResources(15, 1, myPointLightPass.framebuffer->GetDepthAttachment()->GetSRV().GetAddressOf());

			context->PSSetShaderResources(20, 1, myPreDepthPass.framebuffer->GetDepthAttachment()->GetSRV().GetAddressOf());

			Renderer::DispatchRenderCommandsInstanced();
			Renderer::DispatchBillboardsWithShader(myBillboardShader);

			Renderer::SetDepthState(DepthState::Read);
			myScene->myParticleSystem->RenderParticles();
			Renderer::DispatchText();
			Renderer::DispatchSpritesWithShader(ShaderRegistry::Get("Quad"));
			Renderer::EndPass();
		}

		// Forward callbacks
		{
			VT_PROFILE_SCOPE("SceneRenderer::ForwardCallbacks");
			for (const auto& callback : myForwardRenderCallbacks)
			{
				callback(myScene, aCamera);
			}
		}

		PostProcessPasses(aCamera);

		for (const auto& callback : myExternalPassRenderCallbacks)
		{
			callback(myScene, aCamera);
		}

		Renderer::End();
		Renderer::SetDepthState(DepthState::ReadWrite);
#endif
		}

	void SceneRenderer::OnRenderRuntime()
	{
		Ref<Camera> camera = nullptr;
		uint32_t lowestPrio = UINT_MAX;

		myScene->GetRegistry().ForEach<CameraComponent>([&](Wire::EntityId, const CameraComponent& camComp)
			{
				if (camComp.priority < lowestPrio)
				{
					lowestPrio = camComp.priority;
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
		myResizeSize = { width, height };
	}

	void SceneRenderer::AddForwardCallback(std::function<void(Ref<Scene>, Ref<Camera>)>&& callback)
	{
		myForwardRenderCallbacks.emplace_back(callback);
	}

	void SceneRenderer::AddExternalPassCallback(std::function<void(Ref<Scene>, Ref<Camera>)>&& callback)
	{
		myExternalPassRenderCallbacks.emplace_back(callback);
	}

	void SceneRenderer::UpdateVignetteSettings()
	{
		myVignetteMaterial->GetSubMaterials().at(0)->SetParameter("width", myVignetteSettings.width);
		myVignetteMaterial->GetSubMaterials().at(0)->SetParameter("sharpness", myVignetteSettings.sharpness);
		myVignetteMaterial->GetSubMaterials().at(0)->SetParameter("colorTint", myVignetteSettings.color);
	}

	Ref<Framebuffer> SceneRenderer::GetFinalFramebuffer()
	{
		return myShadingPass.framebuffer;
	}

	Ref<Framebuffer> SceneRenderer::GetFinalObjectFramebuffer()
	{
		return myForwardPass.framebuffer;
	}

	Ref<Framebuffer> SceneRenderer::GetSelectionFramebuffer()
	{
		return myForwardPass.framebuffer;
	}

	void SceneRenderer::CreatePasses(bool isLowRes)
	{
		// Pre depth
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA16F, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None, "View Normals" },
				{ ImageFormat::R32Typeless, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None, "Pre Depth" }
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			myPreDepthPass.framebuffer = Framebuffer::Create(spec);
			myPreDepthPass.overrideShader = ShaderRegistry::Get("PreDepth");
			myPreDepthPass.debugName = "Pre Depth";
		}

		// GBuffer
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA, { 0.1f, 0.1f, 0.1f, 0.f }, TextureBlend::Alpha, "Albedo" }, // Albedo
				{ ImageFormat::RGBA16F, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None, "Material" }, // Material (Metallic, Roughness, R, Emissive G)
				{ ImageFormat::RGBA16F, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None, "Normal AO" }, // Normal Emissive B
				{ ImageFormat::RGBA16F, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None, "Vertex Normal" }, // Vertex normal
				{ ImageFormat::RGBA16F, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None, "View Normal" }, // View normal
				{ ImageFormat::RGBA32F, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None, "World Position" }, // World position
				{ ImageFormat::R32UI, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None, "ID" }, // ID
				{ ImageFormat::R32Typeless, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None, "Main Depth" }
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			myDeferredPass.framebuffer = Framebuffer::Create(spec);
			myDeferredPass.exclusiveShaderHash = ShaderRegistry::Get("Deferred")->GetHash();
			myDeferredPass.debugName = "Deferred";

			myFramebuffers.emplace(0, std::make_pair("GBuffer", myDeferredPass.framebuffer));
		}

		// Decal
		{
			FramebufferSpecification spec{};
			spec.attachments =
			{
				{ ImageFormat::RGBA, { 0.1f, 0.1f, 0.1f, 0.f }, TextureBlend::None }, // Albedo
				{ ImageFormat::RGBA, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None }, // Material
				{ ImageFormat::RGBA16F, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None }, // NormalAO,
				{ ImageFormat::R32UI, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None }, // ID
				{ ImageFormat::DEPTH32F }
			};

			spec.existingImages =
			{
				{ 0, myDeferredPass.framebuffer->GetColorAttachment(0) },
				{ 1, myDeferredPass.framebuffer->GetColorAttachment(1) },
				{ 2, myDeferredPass.framebuffer->GetColorAttachment(2) },
				{ 3, myDeferredPass.framebuffer->GetColorAttachment(6) }
			};

			spec.existingDepth = myDeferredPass.framebuffer->GetDepthAttachment();

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			myDecalPass.framebuffer = Framebuffer::Create(spec);
			myDecalPass.debugName = "Decals";
			myDecalPass.overrideShader = ShaderRegistry::Get("Decal");
		}

		// Skybox
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA32F, { 0.05f, 0.05f, 0.05f, 1.f }, TextureBlend::None, "Main Color" },
				{ ImageFormat::RGBA32F, { 0.f, 0.f, 0.f, 1.f }, TextureBlend::None, "Luminance" }
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			mySkyboxPass.framebuffer = Framebuffer::Create(spec);
			mySkyboxPass.debugName = "Skybox";
			mySkyboxPass.overrideShader = ShaderRegistry::Get("Skybox");
			mySkyboxPass.cullState = CullState::CullFront;
			mySkyboxPass.depthState = DepthState::None;

			mySkyboxMesh = Shape::CreateUnitCube();
			mySkyboxBuffer = ConstantBuffer::Create(&mySkyboxData, sizeof(SkyboxData), ShaderStage::Pixel);
		}

		// Shading
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA32F, { 0.05f, 0.05f, 0.05f, 1.f }, TextureBlend::Alpha, "Main Color" },
				{ ImageFormat::RGBA32F, { 0.f, 0.f, 0.f, 1.f }, TextureBlend::None, "Luminance" }
			};

			spec.existingImages =
			{
				{ 0, mySkyboxPass.framebuffer->GetColorAttachment(0) },
				{ 0, mySkyboxPass.framebuffer->GetColorAttachment(1) }
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			myShadingPass.framebuffer = Framebuffer::Create(spec);
			myShadingPass.debugName = "Shading";
			myShadingPass.depthState = DepthState::None;

			myFramebuffers.emplace(1, std::make_pair("Deferred Shading", myShadingPass.framebuffer));
		}

		// Forward
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA32F }, // Color
				{ ImageFormat::RGBA32F }, // Lumincance
				{ ImageFormat::RGBA16F }, // View normals
				{ ImageFormat::R32UI }, // ID
				{ ImageFormat::DEPTH32F, { 0.f, 0.f, 0.f, 0.f }, TextureBlend::None, "Main Depth" }
			};

			spec.existingImages =
			{
				{ 0, myShadingPass.framebuffer->GetColorAttachment(0) },
				{ 1, myShadingPass.framebuffer->GetColorAttachment(1) },
				{ 2, myDeferredPass.framebuffer->GetColorAttachment(4) },
				{ 3, myDeferredPass.framebuffer->GetColorAttachment(6) },
			};

			spec.existingDepth = myDeferredPass.framebuffer->GetDepthAttachment();

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			myForwardPass.framebuffer = Framebuffer::Create(spec);
			myForwardPass.excludedShaderHashes.emplace_back(myDeferredPass.exclusiveShaderHash);
			myForwardPass.debugName = "Forward";

			myFramebuffers.emplace(2, std::make_pair("Forward", myForwardPass.framebuffer));
		}

		// Directional Shadow
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::R32Typeless, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::None, "Directional Shadow Depth"}
			};

			spec.width = isLowRes ? 512 : 4096;
			spec.height = isLowRes ? 512 : 4096;

			myDirectionalShadowPass.framebuffer = Framebuffer::Create(spec);
			myDirectionalShadowPass.overrideShader = ShaderRegistry::Get("DirectionalShadow");
			myDirectionalShadowPass.debugName = "Directional Shadow";
		}

		// Point Light Shadow
		{
			myPointLightBuffer = ConstantBuffer::Create(nullptr, sizeof(uint32_t) * 4, ShaderStage::Geometry);

			FramebufferSpecification spec{};

			FramebufferAttachment attachment{};
			attachment.format = ImageFormat::R32Typeless;
			attachment.isCubeMap = true;

			spec.attachments.emplace_back(attachment);
			spec.width = 512;
			spec.height = 512;

			myPointLightPass.framebuffer = Framebuffer::Create(spec);
			myPointLightPass.overrideShader = ShaderRegistry::Get("PointLightShadow");
			myPointLightPass.debugName = "Point Light Shadow";
		}

		// Outline
		{
			// Hightlighted Geometry
			{
				FramebufferSpecification spec{};
				spec.attachments =
				{
					{ ImageFormat::RGBA32F, gem::vec4{ 0.f, 0.f, 0.f, 0.f }},
					ImageFormat::DEPTH32F
				};

				spec.width = isLowRes ? 640 : 1920;
				spec.height = isLowRes ? 360 : 1080;

				myHighlightedGeometryPass.framebuffer = Framebuffer::Create(spec);
				myHighlightedGeometryPass.overrideShader = ShaderRegistry::Get("SelectedGeometry");
				myHighlightedGeometryPass.debugName = "Highlighted Geometry";
			}

			// Jump Flood Init
			{
				FramebufferSpecification spec{};
				spec.attachments =
				{
					{ ImageFormat::RGBA32F, gem::vec4{ 1.f, 1.f, 1.f, 0.f }},
				};

				spec.width = isLowRes ? 640 : 1920;
				spec.height = isLowRes ? 360 : 1080;

				myJumpFloodInitPass.framebuffer = Framebuffer::Create(spec);
				myJumpFloodInitPass.overrideShader = ShaderRegistry::Get("JumpFloodInit");
				myJumpFloodInitPass.debugName = "Highlighted Jump Flood Init";
			}

			// Jump Flood Passes
			{
				myJumpFloodBuffer = ConstantBuffer::Create(nullptr, sizeof(gem::vec2) + sizeof(int32_t) * 2, ShaderStage::Vertex | ShaderStage::Pixel);

				for (uint32_t i = 0; i < 2; i++)
				{
					FramebufferSpecification spec{};
					spec.attachments =
					{
						ImageFormat::RGBA32F
					};

					spec.width = isLowRes ? 640 : 1920;
					spec.height = isLowRes ? 360 : 1080;

					myJumpFloodPass[i].framebuffer = Framebuffer::Create(spec);
					myJumpFloodPass[i].overrideShader = ShaderRegistry::Get("JumpFloodPass");
					myJumpFloodPass[i].debugName = "Highlighted Jump Flood Pass" + std::to_string(i);
				}

				myFramebuffers.emplace(3, std::make_pair("Jump Flood", myJumpFloodPass[0].framebuffer));
			}

			// Jump Flood Composite
			{
				FramebufferSpecification spec{};
				spec.attachments =
				{
					{ ImageFormat::RGBA32F },
				};

				spec.existingImages =
				{
					{ 0, myForwardPass.framebuffer->GetColorAttachment(0) },
				};

				spec.existingDepth = myForwardPass.framebuffer->GetDepthAttachment();

				spec.width = isLowRes ? 640 : 1920;
				spec.height = isLowRes ? 360 : 1080;

				myJumpFloodCompositePass.framebuffer = Framebuffer::Create(spec);
				myJumpFloodCompositePass.overrideShader = ShaderRegistry::Get("JumpFloodComposite");
				myJumpFloodCompositePass.debugName = "Highlighted Jump Flood Composite";
				myJumpFloodCompositePass.depthState = DepthState::None;
			}
		}

		// HBAO
		{
			// Deinterleaving
			{
				FramebufferSpecification spec{};

				spec.attachments =
				{
					{ ImageFormat::R16F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::None, "Deinterleaving0" },
					{ ImageFormat::R16F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::None, "Deinterleaving1" },
					{ ImageFormat::R16F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::None, "Deinterleaving2" },
					{ ImageFormat::R16F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::None, "Deinterleaving3" },
					{ ImageFormat::R16F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::None, "Deinterleaving4" },
					{ ImageFormat::R16F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::None, "Deinterleaving5" },
					{ ImageFormat::R16F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::None, "Deinterleaving6" },
					{ ImageFormat::R16F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::None, "Deinterleaving7" },
				};

				spec.width = isLowRes ? 640 / 4 : 1920 / 4;
				spec.height = isLowRes ? 360 : 1080 / 4;

				for (size_t i = 0; i < 2; i++)
				{
					myDeinterleavingPass[i].framebuffer = Framebuffer::Create(spec);
					myDeinterleavingPass[i].overrideShader = ShaderRegistry::Get("Deinterleaving");
					myDeinterleavingPass[i].debugName = "Deinterleaving" + std::to_string(i);
				}
			}

			// Main HBAO
			{
				FramebufferSpecification spec{};
				spec.attachments = { 16, { ImageFormat::RG16F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::None, "AO", true } };

				spec.width = isLowRes ? 640 : 1920;
				spec.height = isLowRes ? 360 : 1080;

				myHBAOPass.framebuffer = Framebuffer::Create(spec);
				myHBAOPass.overrideShader = ShaderRegistry::Get("HBAO");
				myHBAOPass.debugName = "HBAO";

				myHBAOPipeline = ComputePipeline::Create(ShaderRegistry::Get("HBAO"));

				for (int32_t i = 0; i < 16; i++)
				{
					myHBAOData.float2Offsets[i] = gem::vec4((float)(i % 4) + 0.5f, (float)(i / 4) + 0.5f, 0.f, 1.f);
				}

				memcpy_s(myHBAOData.jitters, sizeof(gem::vec4) * 16, Noise::HBAOJitter().data(), sizeof(gem::vec4) * 16);
			}

			// Reinterleaving
			{
				FramebufferSpecification spec{};
				spec.attachments =
				{
					{ ImageFormat::RG16F, { 0.5f, 0.1f, 0.1f, 1.f }, TextureBlend::None, "Reinterleaving" },
				};

				spec.width = isLowRes ? 640 : 1920;
				spec.height = isLowRes ? 360 : 1080;

				myReinterleavingPass.framebuffer = Framebuffer::Create(spec);
				myReinterleavingPass.overrideShader = ShaderRegistry::Get("Reinterleaving");
				myReinterleavingPass.debugName = "Reinterleaving";
			}

			// Blur
			{
				FramebufferSpecification spec{};
				spec.attachments =
				{
					{ ImageFormat::RG16F, { 0.5f, 0.1f, 0.1f, 1.f }, TextureBlend::None, "HBAOBlur" },
				};

				spec.width = isLowRes ? 640 : 1920;
				spec.height = isLowRes ? 360 : 1080;

				for (uint32_t i = 0; i < 2; i++)
				{
					spec.attachments.back().debugName = std::format("HBAOBlur{}", i);
					myHBAOBlurPass[i].framebuffer = Framebuffer::Create(spec);
					myHBAOBlurPass[i].overrideShader = ShaderRegistry::Get("HBAOBlur");
					myHBAOBlurPass[i].debugName = std::format("HBAO Blur {}", i);
				}
			}

			myHBAOBuffer = ConstantBuffer::Create(&myHBAOData, sizeof(HBAOData), ShaderStage::Pixel | ShaderStage::Compute);
		}

		// AO Composite
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA32F, { 0.05f, 0.05f, 0.05f, 1.f }, TextureBlend::ZeroSrcColor, "AO Composite" },
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			spec.existingImages =
			{
				{ 0, myForwardPass.framebuffer->GetColorAttachment(0) }
			};

			myAOCompositePass.framebuffer = Framebuffer::Create(spec);
			myAOCompositePass.debugName = "AO Composite";
			myAOCompositePass.overrideShader = ShaderRegistry::Get("AOComposite");
		}

		// Bloom
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA32F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::Alpha, "Bloom" }
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			myBloomCompositePass.framebuffer = Framebuffer::Create(spec);

			spec.attachments[0].storageCompatible = true;

			for (uint32_t i = 0; i < myBloomMipCount; i++)
			{
				spec.width /= 2;
				spec.height /= 2;

				myBloomDownsamplePasses.emplace_back(Framebuffer::Create(spec));
			}

			spec.attachments[0].blending = TextureBlend::Add;
			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			for (uint32_t i = 0; i < myBloomMipCount; i++)
			{
				spec.width /= 2;
				spec.height /= 2;

				spec.existingImages =
				{
					{ 0, myBloomDownsamplePasses[i].framebuffer->GetColorAttachment(0) }
				};

				myBloomUpsamplePasses.emplace_back(Framebuffer::Create(spec));
			}

			std::reverse(myBloomUpsamplePasses.begin(), myBloomUpsamplePasses.end());

			myBloomDownsamplePipeline = ComputePipeline::Create(ShaderRegistry::Get("BloomDownsample"));
			myBloomUpsamplePipeline = ComputePipeline::Create(ShaderRegistry::Get("BloomUpsample"));
			myBloomUpsampleBuffer = ConstantBuffer::Create(nullptr, sizeof(gem::vec4), ShaderStage::Pixel);
		}

		// Height Fog
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA32F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::Alpha, "Height Fog" }
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			spec.existingImages =
			{
				{ 0, myBloomCompositePass.framebuffer->GetColorAttachment(0) }
			};

			myHeightFogPass.framebuffer = Framebuffer::Create(spec);
			myHeightFogPass.overrideShader = ShaderRegistry::Get("HeightFog");
			myHeightFogPass.debugName = "Height Fog";

			myHeightFogBuffer = ConstantBuffer::Create(&myHeightFogData, sizeof(HeightFogData), ShaderStage::Pixel);
		}

		// Voxel
		{
			myVoxelPass.overrideShader = ShaderRegistry::Get("Voxelization");
			myVoxelPass.debugName = "Voxel Pass";
			myVoxelBuffer = ConstantBuffer::Create(nullptr, sizeof(VoxelSceneData), ShaderStage::Geometry | ShaderStage::Pixel);
			myVoxelResultBuffer = StructuredBuffer::Create(sizeof(VoxelType), 100000, ShaderStage::Pixel, true);
		}

		// FXAA pass
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA32F, { 0.05f, 0.05f, 0.05f, 1.f }, TextureBlend::Alpha, "FXAA Output" },
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			myFXAAPass.framebuffer = Framebuffer::Create(spec);
			myFXAAPass.debugName = "FXAA";
		}

		// Vignette
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA32F, { 1.f, 1.f, 1.f, 1.f }, TextureBlend::Alpha, "Vignette" },
			};

			spec.existingImages =
			{
				{ 0, myFXAAPass.framebuffer->GetColorAttachment(0) }
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			myVignettePass.framebuffer = Framebuffer::Create(spec);
			myVignettePass.debugName = "Vignette";
			myVignettePass.overrideShader = ShaderRegistry::Get("Vignette");

			myVignetteMaterial = CreateRef<Material>();
			myVignetteMaterial->CreateSubMaterial(ShaderRegistry::Get("Vignette"));
			UpdateVignetteSettings();
		}

		// Gamma Correction
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA32F, { 0.05f, 0.05f, 0.05f, 1.f }, TextureBlend::Alpha, "Final Color" },
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			myGammaCorrectionPass.framebuffer = Framebuffer::Create(spec);
			myGammaCorrectionPass.debugName = "Gamma Correction";
			myGammaCorrectionPass.overrideShader = ShaderRegistry::Get("GammaCorrection");
		}

		myBillboardShader = ShaderRegistry::Get("EntityGizmo");
	}

	void SceneRenderer::ResetPostProcess()
	{
		myHeightFogData.strength = 0.f;
		myHBAOSettings.enabled = false;
		myBloomSettings.enabled = false;
		myFXAASettings.enabled = false;
		myVignetteSettings.enabled = false;
	}

	void SceneRenderer::PostProcessPasses(Ref<Camera> aCamera)
	{
		RenderCommand::BeginAnnotation("Post Process");

		Ref<Framebuffer> lastFramebuffer = myForwardPass.framebuffer;

		if (myHBAOSettings.enabled)
		{
			HBAOPass(aCamera);

			// AO Composite
			{
				auto context = GraphicsContext::GetImmediateContext();
				Renderer::BeginFullscreenPass(myAOCompositePass, aCamera);

				context->PSSetShaderResources(0, 1, myHBAOBlurPass[1].framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());

				Renderer::DrawFullscreenTriangleWithShader(myAOCompositePass.overrideShader);

				ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
				context->PSSetShaderResources(0, 1, nullSRV);

				Renderer::EndFullscreenPass();
			}

			lastFramebuffer = myAOCompositePass.framebuffer;
		}

		// Outline
		{
			// Highlighted Geometry
			{
				Renderer::BeginPass(myHighlightedGeometryPass, aCamera);

				for (const auto& [handle, id] : myHighlightedMeshes)
				{
					auto mesh = AssetManager::GetAsset<Mesh>(handle);
					if (mesh && mesh->IsValid())
					{
						Renderer::DrawMesh(mesh, myScene->GetWorldSpaceTransform(Entity{ id, myScene.get() }, myScene->IsPlaying()));
					}
				}

				for (const auto& [handle, id, animCharComp] : myHighlightedAnimatedMeshes)
				{
					auto character = AssetManager::GetAsset<AnimatedCharacter>(handle);
					if (character && character->IsValid())
					{
						const gem::mat4 transform = myScene->GetWorldSpaceTransform(Entity(id, myScene.get()), myScene->IsPlaying());

						Renderer::DrawMesh(character->GetSkin(), character->SampleAnimation(animCharComp.currentAnimation, animCharComp.currentStartTime, animCharComp.isLooping), transform);
					}
				}

				Renderer::EndPass();
			}

			auto context = GraphicsContext::GetImmediateContext(); // #TODO: Find better way to bind textures here

			// Jump Flood Init
			{
				Renderer::BeginPass(myJumpFloodInitPass, aCamera);

				context->PSSetShaderResources(0, 1, myHighlightedGeometryPass.framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());
				Renderer::DrawFullscreenTriangleWithShader(myJumpFloodInitPass.overrideShader);

				Renderer::EndPass();
			}

			// Jump Flood Pass
			{
				int32_t steps = 2;
				int32_t step = (int32_t)std::round(std::pow(steps - 1, 2));
				int32_t index = 0;

				struct FloodPassData
				{
					gem::vec2 texelSize = 0.f;
					int32_t step = 0;
					int32_t padding = 0;
				} floodPassData;

				auto framebuffer = myJumpFloodPass[0].framebuffer;
				floodPassData.texelSize = { 1.f / (float)framebuffer->GetWidth(), 1.f / (float)framebuffer->GetHeight() };
				floodPassData.step = step;

				while (step != 0)
				{
					Renderer::BeginPass(myJumpFloodPass[index], aCamera);
					myJumpFloodBuffer->SetData(&floodPassData, sizeof(FloodPassData));
					myJumpFloodBuffer->Bind(13);

					if (index == 0)
					{
						context->PSSetShaderResources(0, 1, myJumpFloodInitPass.framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());
					}
					else
					{
						context->PSSetShaderResources(0, 1, myJumpFloodPass[0].framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());
					}

					Renderer::DrawFullscreenQuadWithShader(myJumpFloodPass[index].overrideShader);
					Renderer::EndPass();

					ID3D11ShaderResourceView* nullSRV = nullptr;
					context->PSSetShaderResources(0, 1, &nullSRV);

					index = (index + 1) % 2;
					step /= 2;

					floodPassData.step = step;
				}
			}

			// Jump Flood Composite
			{
				Renderer::BeginPass(myJumpFloodCompositePass, aCamera);

				const gem::vec4 color = { 1.f, 0.f, 0.f, 1.f };

				myJumpFloodBuffer->SetData(&color, sizeof(gem::vec4));
				myJumpFloodBuffer->Bind(13);

				context->PSSetShaderResources(0, 1, myJumpFloodPass[0].framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());
				Renderer::DrawFullscreenQuadWithShader(myJumpFloodCompositePass.overrideShader);
				Renderer::EndPass();
			}
		}

		if (myBloomSettings.enabled)
		{
			BloomPass(myForwardPass.framebuffer->GetColorAttachment(1));
			lastFramebuffer = myBloomCompositePass.framebuffer;
		}

		// Height Fog
		{
			auto context = GraphicsContext::GetImmediateContext();

			myHeightFogPass.framebuffer->SetColorAttachment(lastFramebuffer->GetColorAttachment(0), 0);
			Renderer::BeginFullscreenPass(myHeightFogPass, aCamera);

			context->PSSetShaderResources(0, 1, myPreDepthPass.framebuffer->GetDepthAttachment()->GetSRV().GetAddressOf());

			myHeightFogBuffer->SetData(&myHeightFogData, sizeof(HeightFogData));
			myHeightFogBuffer->Bind(13);
			Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("HeightFog"));

			ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
			context->PSSetShaderResources(0, 2, nullSRV);
			Renderer::EndFullscreenPass();

			lastFramebuffer = myHeightFogPass.framebuffer;
		}

		// FXAA
		if (myFXAASettings.enabled)
		{
			auto context = GraphicsContext::GetImmediateContext();

			Renderer::BeginFullscreenPass(myFXAAPass, aCamera);

			context->PSSetShaderResources(0, 1, lastFramebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());

			Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("FXAA"));

			ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
			context->PSSetShaderResources(0, 1, nullSRV);

			Renderer::EndFullscreenPass();

			lastFramebuffer = myFXAAPass.framebuffer;
		}

		// Vignette

		myVignettePass.framebuffer->SetColorAttachment(lastFramebuffer->GetColorAttachment(0), 0);

		if (myVignetteSettings.enabled)
		{
			UpdateVignetteSettings();
			Renderer::BeginFullscreenPass(myVignettePass, aCamera);
			Renderer::DrawFullscreenTriangleWithMaterial(myVignetteMaterial);
			Renderer::EndFullscreenPass();
		}

		// Gamma Correction
		{
			auto context = GraphicsContext::GetImmediateContext();
			Renderer::BeginFullscreenPass(myGammaCorrectionPass, aCamera);

			context->PSSetShaderResources(0, 1, myVignettePass.framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());

			Renderer::DrawFullscreenTriangleWithShader(myGammaCorrectionPass.overrideShader);

			ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
			context->PSSetShaderResources(0, 1, nullSRV);

			Renderer::EndFullscreenPass();
		}

		RenderCommand::EndAnnotation();
	}

	void SceneRenderer::ShadingPass(const SceneEnvironment& sceneEnv)
	{
		VT_PROFILE_FUNCTION();

		Renderer::BeginFullscreenPass(myShadingPass, nullptr);

		std::vector<ID3D11ShaderResourceView*> mainSRVs =
		{
			myDeferredPass.framebuffer->GetColorAttachment(0)->GetSRV().Get(),
			myDeferredPass.framebuffer->GetColorAttachment(1)->GetSRV().Get(),
			myDeferredPass.framebuffer->GetColorAttachment(2)->GetSRV().Get(),
			myDeferredPass.framebuffer->GetColorAttachment(3)->GetSRV().Get(),
			myDeferredPass.framebuffer->GetColorAttachment(5)->GetSRV().Get(),
			nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			Renderer::GetDefaultData().brdfLut->GetSRV().Get(),
			sceneEnv.irradianceMap->GetSRV().Get(),
			sceneEnv.radianceMap->GetSRV().Get(),
			myDirectionalShadowPass.framebuffer->GetDepthAttachment()->GetSRV().Get(),
			myPointLightPass.framebuffer->GetDepthAttachment()->GetSRV().Get()
		};

		Renderer::BindTexturesToStage(ShaderStage::Pixel, mainSRVs, 0);
		Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("Shading"));
		Renderer::ClearTexturesAtStage(ShaderStage::Pixel, 0, 16);
		Renderer::EndFullscreenPass();
	}

	void SceneRenderer::BloomPass(Ref<Image2D> sourceImage)
	{
		VT_PROFILE_FUNCTION();

		Renderer::BeginSection("Bloom");

		Ref<Image2D> lastSource = sourceImage;
		auto context = GraphicsContext::GetImmediateContext();

		// Downsample
		{

			for (auto& pass : myBloomDownsamplePasses)
			{
				context->PSSetShaderResources(0, 1, lastSource->GetSRV().GetAddressOf());
				pass.framebuffer->Clear();
				pass.framebuffer->Bind();
				Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("BloomDownsamplePS"));
				pass.framebuffer->Unbind();
				lastSource = pass.framebuffer->GetColorAttachment(0);
			}
		}

		// Upsample
		{
			struct BloomUpsampleData
			{
				float filterRadius;
				gem::vec3 padding;
			} data{};

			data.filterRadius = 0.005f;
			myBloomUpsampleBuffer->SetData(&data, sizeof(BloomUpsampleData));
			myBloomUpsampleBuffer->Bind(13);

			for (uint32_t i = 0; i < (uint32_t)myBloomUpsamplePasses.size() - 1; i++)
			{
				auto currMip = myBloomUpsamplePasses[i].framebuffer;
				auto nextMip = myBloomUpsamplePasses[i + 1].framebuffer;

				context->PSSetShaderResources(0, 1, currMip->GetColorAttachment(0)->GetSRV().GetAddressOf());
				nextMip->Bind();
				Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("BloomUpsamplePS"));
				nextMip->Unbind();
			}
		}

		// Composite
		{
			context->PSSetShaderResources(0, 1, myAOCompositePass.framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());
			context->PSSetShaderResources(1, 1, myBloomUpsamplePasses.back().framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());

			myBloomCompositePass.framebuffer->Clear();
			myBloomCompositePass.framebuffer->Bind();
			Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("BloomComposite"));
			myBloomCompositePass.framebuffer->Unbind();

			ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
			context->PSSetShaderResources(0, 2, nullSRV);
		}

		Renderer::EndSection("Bloom");
	}

	void SceneRenderer::HBAOPass(Ref<Camera> aCamera)
	{
		RenderCommand::BeginAnnotation("HBAO");
		auto context = GraphicsContext::GetImmediateContext();

		const gem::mat4& projectionMat = aCamera->GetProjection();
		const float width = (float)myHBAOPass.framebuffer->GetWidth();
		const float height = (float)myHBAOPass.framebuffer->GetHeight();

		const float meters2Viewspace = 100.f;
		const float R = myHBAOSettings.radius * meters2Viewspace;
		const float R2 = R * R;

		myHBAOData.negInvR2 = -1.f / R2;
		myHBAOData.inverseQuarterSize = gem::vec2{ 1.f / (float)width, 1.f / (float)height };
		myHBAOData.radiusToScreen = R * 0.5f * (float)width / (tanf(gem::radians(aCamera->GetFieldOfView()) * 0.5f) * 2.f);

		const gem::vec4 projInfoPerspective =
		{
			 2.f * (1.f / (float)fabs(projectionMat[0][0])),
			-2.f * (1.f / (float)fabs(projectionMat[1][1])),
			-1.f * (1.f / (float)fabs(projectionMat[0][0])),
			 1.f * (1.f / (float)fabs(projectionMat[1][1]))
		};

		myHBAOData.perspectiveInfo = projInfoPerspective;
		myHBAOData.powExponent = gem::max(myHBAOSettings.intensity, 0.f);
		myHBAOData.NdotVBias = gem::min(gem::max(0.f, myHBAOSettings.bias), 1.f);
		myHBAOData.multiplier = 1.f / (1.f - myHBAOData.NdotVBias);
		myHBAOData.sharpness = 1.f;

		// Deinterleaving
		for (int32_t i = 0; i < 2; i++)
		{
			Renderer::BeginFullscreenPass(myDeinterleavingPass[i], aCamera);

			myHBAOData.uvOffsetIndex = i;

			myHBAOBuffer->SetData(&myHBAOData, sizeof(HBAOData));
			myHBAOBuffer->Bind(13);

			context->PSSetShaderResources(0, 1, myPreDepthPass.framebuffer->GetDepthAttachment()->GetSRV().GetAddressOf());
			Renderer::DrawFullscreenTriangleWithShader(myDeinterleavingPass[i].overrideShader);

			ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
			context->PSSetShaderResources(0, 1, nullSRV);

			Renderer::EndFullscreenPass();
		}

		// Main Pass
		{
			Renderer::BeginSection("HBAO Main");

			for (uint32_t i = 0; i < 16; i++)
			{
				myHBAOPipeline->SetTarget(myHBAOPass.framebuffer->GetColorAttachment(i), i);
			}

			myHBAOPipeline->SetImage(myPreDepthPass.framebuffer->GetColorAttachment(0), 0);

			for (uint32_t i = 0; i < 2; i++)
			{
				for (uint32_t j = 0; j < 8; j++)
				{
					myHBAOPipeline->SetImage(myDeinterleavingPass[i].framebuffer->GetColorAttachment(j), i * 8 + j + 1);
				}
			}

			constexpr uint32_t WORK_GROUP_SIZE = 16;
			gem::vec2ui size = { myHBAOPass.framebuffer->GetWidth(), myHBAOPass.framebuffer->GetHeight() };
			size = { size.x + WORK_GROUP_SIZE - size.x % WORK_GROUP_SIZE, size.y + WORK_GROUP_SIZE - size.y % WORK_GROUP_SIZE };

			myHBAOPipeline->Execute(size.x / 16u, size.y / 16u, 16u);
			myHBAOPipeline->Clear();

			Renderer::EndSection("HBAO Main");
		}

		// Reinterleaving
		{
			Renderer::BeginFullscreenPass(myReinterleavingPass, aCamera);

			std::vector<ID3D11ShaderResourceView*> views{};
			views.reserve(16);
			for (uint32_t i = 0; i < 16; i++)
			{
				views.emplace_back(myHBAOPass.framebuffer->GetColorAttachment(i)->GetSRV().Get());
			}

			context->PSSetShaderResources(0, (uint32_t)views.size(), views.data());

			Renderer::DrawFullscreenTriangleWithShader(myReinterleavingPass.overrideShader);

			std::vector<ID3D11ShaderResourceView*> nullSRVs{ 16, nullptr };

			context->PSSetShaderResources(0, (uint32_t)nullSRVs.size(), nullSRVs.data());

			Renderer::EndFullscreenPass();
		}

		// Blur
		{
			// Pass 1
			{
				context->PSSetShaderResources(0, 1, myReinterleavingPass.framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());

				Renderer::BeginFullscreenPass(myHBAOBlurPass[0], aCamera);
				myHBAOData.invResDirection = { 1.f / myResizeSize.x, 0.f };
				myHBAOBuffer->SetData(&myHBAOData, sizeof(HBAOData));
				Renderer::DrawFullscreenTriangleWithShader(myHBAOBlurPass[0].overrideShader);
				Renderer::EndFullscreenPass();
			}

			// Pass 2
			{
				context->PSSetShaderResources(0, 1, myHBAOBlurPass[0].framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());

				Renderer::BeginFullscreenPass(myHBAOBlurPass[1], aCamera);
				myHBAOData.invResDirection = { 0.f, 1.f / myResizeSize.y };
				myHBAOBuffer->SetData(&myHBAOData, sizeof(HBAOData));
				Renderer::DrawFullscreenTriangleWithShader(myHBAOBlurPass[1].overrideShader);
				Renderer::EndFullscreenPass();
			}
		}

		RenderCommand::EndAnnotation();
	}

	void SceneRenderer::VoxelPass(Ref<Camera> aCamera)
	{
		const float f = 0.05f / myVoxelData.voxelSize;
		gem::vec3 center = { std::floor(aCamera->GetPosition().x * f) / f, std::floor(aCamera->GetPosition().y * f) / f, std::floor(aCamera->GetPosition().z * f) / f };

		myVoxelData.center = center;
	}

	void SceneRenderer::CollectStaticMeshCommands()
	{
		VT_PROFILE_FUNCTION();

		myHighlightedMeshes.clear();
		auto& registry = myScene->GetRegistry();
		std::vector<std::vector<Wire::EntityId>> perThreadViews;

		const auto meshViews = registry.GetSingleComponentView<MeshComponent>();

		if (meshViews.empty())
		{
			return;
		}

		for (const auto& id : meshViews)
		{
			const auto& meshComp = registry.GetComponent<MeshComponent>(id);
			auto& transformComp = registry.GetComponent<TransformComponent>(id);
			const auto& dataComp = registry.GetComponent<EntityDataComponent>(id);

			if (meshComp.handle != Asset::Null() && transformComp.visible)
			{
				if (!AssetManager::Get().IsLoaded(meshComp.handle))
				{
					AssetManager::QueueAsset<Mesh>(meshComp.handle);
					continue;
				}

				auto mesh = AssetManager::GetAsset<Mesh>(meshComp.handle);
				if (!mesh || !mesh->IsValid())
				{
					continue;
				}

				if (dataComp.isHighlighted)
				{
					myHighlightedMeshes.emplace_back(meshComp.handle, id);
				}

				Ref<Material> material;
				if (meshComp.overrideMaterial != Asset::Null())
				{
					auto overrideMat = AssetManager::GetAsset<Material>(meshComp.overrideMaterial);
					if (overrideMat && overrideMat->IsValid())
					{
						material = overrideMat;
					}
				}

				const gem::mat4 transform = myScene->GetWorldSpaceTransform(Entity(id, myScene.get()), myScene->IsPlaying());

				// #TODO: Add ability to have override materials on single submesh draws
				if (meshComp.subMeshIndex != -1)
				{
					Renderer::Submit(mesh, (uint32_t)meshComp.subMeshIndex, transform, id, dataComp.timeSinceCreation, meshComp.castShadows, meshComp.castAO);
				}
				else if (material)
				{
					if (meshComp.subMaterialIndex != -1)
					{
						Renderer::Submit(mesh, material->GetSubMaterials().at(meshComp.subMaterialIndex), transform, id, dataComp.timeSinceCreation, meshComp.castShadows, meshComp.castAO);
					}
					else
					{
						Renderer::Submit(mesh, material, transform, id, dataComp.timeSinceCreation, meshComp.castShadows, meshComp.castAO);
					}
				}
				else
				{
					if (meshComp.subMaterialIndex != -1)
					{
						Renderer::Submit(mesh, mesh->GetMaterial()->GetSubMaterials().at(meshComp.subMaterialIndex), transform, id, dataComp.timeSinceCreation, meshComp.castShadows, meshComp.castAO);
					}
					else
					{
						Renderer::Submit(mesh, transform, id, dataComp.timeSinceCreation, meshComp.castShadows, meshComp.castAO);
					}
				}
			}
		};
	}

	void SceneRenderer::CollectAnimatedMeshCommands()
	{
		VT_PROFILE_FUNCTION();

		myHighlightedAnimatedMeshes.clear();
		auto& registry = myScene->GetRegistry();

		const auto meshViews = registry.GetComponentView<AnimatedCharacterComponent, TransformComponent, EntityDataComponent>();
		std::vector<std::vector<Wire::EntityId>> perThreadViews(gem::min((uint32_t)meshViews.size(), myMaxCollectionThreads));

		if (meshViews.empty())
		{
			return;
		}

		for (const auto& id : meshViews)
		{
			const auto& animCharComp = registry.GetComponent<AnimatedCharacterComponent>(id);
			const auto& dataComp = registry.GetComponent<EntityDataComponent>(id);
			const auto& transformComp = registry.GetComponent<TransformComponent>(id);

			if (animCharComp.animatedCharacter != Asset::Null() && transformComp.visible)
			{
				if (!AssetManager::Get().IsLoaded(animCharComp.animatedCharacter))
				{
					AssetManager::QueueAsset<AnimatedCharacter>(animCharComp.animatedCharacter);
					continue;
				}

				auto character = AssetManager::GetAsset<AnimatedCharacter>(animCharComp.animatedCharacter);
				if (character && character->IsValid())
				{
					if (dataComp.isHighlighted)
					{
						myHighlightedAnimatedMeshes.emplace_back(animCharComp.animatedCharacter, id, animCharComp);
					}

					const gem::mat4 transform = myScene->GetWorldSpaceTransform(Entity(id, myScene.get()));

					std::vector<gem::mat4> animSamples;

					if (animCharComp.characterStateMachine && animCharComp.characterStateMachine->GetStateCount() > 0)
					{
						animSamples = animCharComp.characterStateMachine->Sample();
					}
					else
					{
						animSamples = character->SampleAnimation(animCharComp.currentAnimation, animCharComp.currentStartTime, animCharComp.isLooping);
					}

					Renderer::Submit(character->GetSkin(), transform, animSamples, id, dataComp.timeSinceCreation, animCharComp.castShadows);
				}
			}
		}
	}

	auto DinMamma()
	{
		struct Mother
		{
			void SetSize(uint64_t val) { mySize = val; }
			void Clear() { mySize = 0; }
			uint64_t mySize;
		} myMother{};

		struct Pizza
		{
			void SetContent(Mother aMother) { myMother = aMother; }
			Mother myMother;
		} myPizza{};

		myMother.SetSize(UINT64_MAX);
		myPizza.SetContent(myMother);
		myMother.Clear();

		return myPizza;
	}
	}
