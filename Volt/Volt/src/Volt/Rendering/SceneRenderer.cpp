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

#include "Volt/Math/MatrixUtilities.h"
#include "Volt/Utility/Noise.h"

#include <gem/gem.h>
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

		Renderer::PushCollection();

		// Handle resize
		if (myShouldResize)
		{
			myShouldResize = false;
			Renderer::SubmitCustom([&]()
				{
					VT_PROFILE_SCOPE("Resize");

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
					myDecalPass.framebuffer->Resize(width, height);

					myReinterleavingPass.framebuffer->Resize(width, height);
					myHBAOBlurPass[0].framebuffer->Resize(width, height);
					myHBAOBlurPass[1].framebuffer->Resize(width, height);
					myAOCompositePass.framebuffer->Resize(width, height);

					myDeinterleavingPass[0].framebuffer->Resize(quarterWidth, quarterHeight);
					myDeinterleavingPass[1].framebuffer->Resize(quarterWidth, quarterHeight);
					myHBAOPass.framebuffer->Resize(quarterWidth, quarterHeight);

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
				});
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
					Renderer::SubmitText(textComp.text, fontAsset, myScene->GetWorldSpaceTransform(Entity{ id, myScene.get() }), textComp.maxWidth);
				}
			});

		registry.ForEach<VideoPlayerComponent>([&](Wire::EntityId id, const VideoPlayerComponent& videoPlayer)
			{
				if (videoPlayer.videoHandle != Asset::Null())
				{
					Ref<Video> video = AssetManager::GetAsset<Video>(videoPlayer.videoHandle);
					if (video && video->IsValid())
					{
						Renderer::SubmitSprite(video->GetTexture(), myScene->GetWorldSpaceTransform(Entity{ id, myScene.get() }), nullptr, { 1.f, 1.f, 1.f, 1.f }, id);
					}
				}
			});

		registry.ForEach<SpriteComponent>([&](Wire::EntityId id, const SpriteComponent& comp)
			{
				Ref<Texture2D> texture = nullptr;
				Ref<Material> material = nullptr;
				if (comp.materialHandle != Asset::Null())
				{
					material = AssetManager::GetAsset<Material>(comp.materialHandle);
					if (material && material->IsValid())
					{
						if (material->GetSubMaterialAt(0)->GetTextures().contains(0))
						{
							texture = material->GetSubMaterialAt(0)->GetTextures().at(0);
						}
					}
				}
				Renderer::SubmitSprite(texture, myScene->GetWorldSpaceTransform(Entity{ id, myScene.get() }), material, id);
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

		Renderer::Begin(Context::Deferred, myContext);
		Renderer::SetFullResolution({ (float)myResizeSize.x, (float)myResizeSize.y });

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

		// Pre depth
		{
			Renderer::BeginPass(myPreDepthPass, aCamera, true, false, true);
			Renderer::DispatchRenderCommandsInstanced();
			Renderer::EndPass();
		}

		// Deferred
		{
			Renderer::BeginPass(myDeferredPass, aCamera);
			Renderer::DispatchRenderCommandsInstanced();
			Renderer::EndPass();
		}

		// Skybox
		{
			Renderer::BeginPass(mySkyboxPass, aCamera);
			Renderer::BindTexturesToStage(ShaderStage::Pixel, { sceneEnvironment.radianceMap }, 11);

			Renderer::SubmitCustom([env = sceneEnvironment, skyboxData = mySkyboxData, buffer = mySkyboxBuffer]()
				{
					SkyboxData data = skyboxData;

					data.intensity = env.intensity;
					data.textureLod = env.lod;

					buffer->SetData(&data, sizeof(SkyboxData));
					buffer->Bind(13);
				});

			Renderer::DrawMesh(mySkyboxMesh, { 1.f });
			Renderer::EndPass();
		}

		// Decals
		{
			auto context = GraphicsContext::GetImmediateContext();

			Renderer::BeginSection("Decals");
			Renderer::BindTexturesToStage(ShaderStage::Pixel, { myDeferredPass.framebuffer->GetColorAttachment(5), myDeferredPass.framebuffer->GetColorAttachment(3) }, 11);

			Renderer::SubmitCustom([&]()
				{
					myDecalPass.framebuffer->Clear();
					myDecalPass.framebuffer->Bind();
				});

			Renderer::DispatchDecalsWithShader(myDecalPass.overrideShader);

			Renderer::SubmitCustom([&]
				{
					myDecalPass.framebuffer->Unbind();
				});

			Renderer::ClearTexturesAtStage(ShaderStage::Pixel, 11, 2);
			Renderer::EndSection("Decals");
		}

		ShadingPass(sceneEnvironment);

		// Forward Pass
		{
			Renderer::BeginPass(myForwardPass, aCamera);

			auto context = GraphicsContext::GetImmediateContext();

			Renderer::BindTexturesToStage(ShaderStage::Pixel,
				{
					Renderer::GetDefaultData().brdfLut,
					sceneEnvironment.irradianceMap,
					sceneEnvironment.radianceMap,
					myDirectionalShadowPass.framebuffer->GetDepthAttachment(),
					myPointLightPass.framebuffer->GetDepthAttachment(),
				}, 11);

			Renderer::BindTexturesToStage(ShaderStage::Pixel, { myPreDepthPass.framebuffer->GetDepthAttachment() }, 20);
			Renderer::DispatchRenderCommandsInstanced();
			Renderer::ClearTexturesAtStage(ShaderStage::Pixel, 0, 20);

			Renderer::SubmitCustom([]()
				{
					RenderCommand::SetDepthState(DepthState::Read);
				});

			Renderer::DispatchBillboardsWithShader();

			//myScene->myParticleSystem->RenderParticles();

			Renderer::SubmitCustom([]()
				{
					RenderCommand::SetDepthState(DepthState::ReadWrite);
				});

			Renderer::DispatchText();
			Renderer::DispatchSpritesWithMaterial();
			Renderer::EndPass();
		}

		PostProcessPasses(aCamera);

		for (const auto& callback : myExternalPassRenderCallbacks)
		{
			callback(myScene, aCamera);
		}

		Renderer::End();
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

	void SceneRenderer::AddExternalPassCallback(std::function<void(Ref<Scene>, Ref<Camera>)>&& callback)
	{
		myExternalPassRenderCallbacks.emplace_back(callback);
	}

	void SceneRenderer::UpdateVignetteSettings()
	{
		//myVignetteMaterial->GetSubMaterials().at(0)->SetParameter("width", myVignetteSettings.width);
		//myVignetteMaterial->GetSubMaterials().at(0)->SetParameter("sharpness", myVignetteSettings.sharpness);
		//myVignetteMaterial->GetSubMaterials().at(0)->SetParameter("colorTint", myVignetteSettings.color);
	}

	Ref<Framebuffer> SceneRenderer::GetFinalFramebuffer()
	{
		return myGammaCorrectionPass.framebuffer;
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
				{ ImageFormat::RGBA32F }, // Luminance
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

		// Voxel
		{
			myVoxelPass.overrideShader = ShaderRegistry::Get("Voxelization");
			myVoxelPass.debugName = "Voxel Pass";
			myVoxelBuffer = ConstantBuffer::Create(nullptr, sizeof(VoxelSceneData), ShaderStage::Geometry | ShaderStage::Pixel);
			myVoxelResultBuffer = StructuredBuffer::Create(sizeof(VoxelType), 100000, ShaderStage::Pixel, true);
		}

		// Debanding
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA32F, { 0.05f, 0.05f, 0.05f, 1.f }, TextureBlend::Alpha, "Debanding Output" },
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			myDebandingPass.framebuffer = Framebuffer::Create(spec);
			myDebandingPass.debugName = "Debanding";
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

		// Gamma Correction
		{
			FramebufferSpecification spec{};

			spec.attachments =
			{
				{ ImageFormat::RGBA32F, { 0.05f, 0.05f, 0.05f, 1.f }, TextureBlend::None, "Final Color" },
			};

			spec.width = isLowRes ? 640 : 1920;
			spec.height = isLowRes ? 360 : 1080;

			myGammaCorrectionPass.framebuffer = Framebuffer::Create(spec);
			myGammaCorrectionPass.debugName = "Gamma Correction";
			myGammaCorrectionPass.overrideShader = ShaderRegistry::Get("GammaCorrection");
		}

		myBillboardShader = ShaderRegistry::Get("Billboard");
	}

	void SceneRenderer::ResetPostProcess()
	{
		myHBAOSettings.enabled = false;
		myBloomSettings.enabled = false;
		myFXAASettings.enabled = false;
		myVignetteSettings.enabled = false;
	}

	void SceneRenderer::PostProcessPasses(Ref<Camera> aCamera)
	{
		Renderer::BeginSection("Post Process");

		if (myHBAOSettings.enabled)
		{
			HBAOPass(aCamera);

			// AO Composite
			{
				Renderer::BeginFullscreenPass(myAOCompositePass, aCamera);
				Renderer::BindTexturesToStage(ShaderStage::Pixel, { myHBAOBlurPass[1].framebuffer->GetColorAttachment(0) }, 0);
				Renderer::DrawFullscreenTriangleWithShader(myAOCompositePass.overrideShader);
				Renderer::ClearTexturesAtStage(ShaderStage::Pixel, 0, 1);
				Renderer::EndFullscreenPass();
			}
		}

		BloomPass(myForwardPass.framebuffer->GetColorAttachment(1));

		//// Debanding
		//{
		//	Renderer::BeginFullscreenPass(myDebandingPass, aCamera);
		//	Renderer::BindTexturesToStage(ShaderStage::Pixel, { myBloomCompositePass.framebuffer->GetColorAttachment(0) }, 0);
		//	Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("Debanding"));
		//	Renderer::ClearTexturesAtStage(ShaderStage::Pixel, 0, 1);
		//	Renderer::EndFullscreenPass();
		//}

		// FXAA
		if (myFXAASettings.enabled)
		{
			Renderer::BeginFullscreenPass(myFXAAPass, aCamera);
			Renderer::BindTexturesToStage(ShaderStage::Pixel, { myBloomCompositePass.framebuffer->GetColorAttachment(0) }, 0);
			Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("FXAA"));
			Renderer::ClearTexturesAtStage(ShaderStage::Pixel, 0, 1);
			Renderer::EndFullscreenPass();
		}

		// Gamma Correction
		{
			Renderer::BeginFullscreenPass(myGammaCorrectionPass, aCamera);
			Renderer::BindTexturesToStage(ShaderStage::Pixel, { myFXAAPass.framebuffer->GetColorAttachment(0) }, 0);
			Renderer::DrawFullscreenTriangleWithShader(myGammaCorrectionPass.overrideShader);
			Renderer::ClearTexturesAtStage(ShaderStage::Pixel, 0, 1);
			Renderer::EndFullscreenPass();
		}

		Renderer::EndSection("Post Process");
	}

	void SceneRenderer::ShadingPass(const SceneEnvironment& sceneEnv)
	{
		VT_PROFILE_FUNCTION();

		Renderer::BeginFullscreenPass(myShadingPass, nullptr);
		//Renderer::ExecuteLightCulling(myPreDepthPass.framebuffer->GetDepthAttachment());

		std::vector<Ref<Image2D>> mainImages =
		{
			myDeferredPass.framebuffer->GetColorAttachment(0),
			myDeferredPass.framebuffer->GetColorAttachment(1),
			myDeferredPass.framebuffer->GetColorAttachment(2),
			myDeferredPass.framebuffer->GetColorAttachment(3),
			myDeferredPass.framebuffer->GetColorAttachment(5),
			nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			Renderer::GetDefaultData().brdfLut,
			sceneEnv.irradianceMap,
			sceneEnv.radianceMap,
			myDirectionalShadowPass.framebuffer->GetDepthAttachment(),
			myPointLightPass.framebuffer->GetDepthAttachment()
		};

		Renderer::BindTexturesToStage(ShaderStage::Pixel, mainImages, 0);
		Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("Shading"));
		Renderer::ClearTexturesAtStage(ShaderStage::Pixel, 0, 16);
		Renderer::EndFullscreenPass();
	}

	void SceneRenderer::BloomPass(Ref<Image2D> sourceImage)
	{
		VT_PROFILE_FUNCTION();

		Renderer::BeginSection("Bloom");

		Ref<Image2D> lastSource = sourceImage;

		// Downsample
		{
			for (auto& pass : myBloomDownsamplePasses)
			{
				Renderer::BindTexturesToStage(ShaderStage::Pixel, { lastSource }, 0);

				Renderer::SubmitCustom([p = pass]()
					{
						p.framebuffer->Clear();
						p.framebuffer->Bind();
					});

				Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("BloomDownsamplePS"));

				Renderer::SubmitCustom([p = pass]()
					{
						p.framebuffer->Unbind();
					});

				lastSource = pass.framebuffer->GetColorAttachment(0);
			}
		}

		// Upsample
		{
			Renderer::SubmitCustom([&]()
				{
					struct BloomUpsampleData
					{
						float filterRadius;
						gem::vec3 padding;
					} data{};

					data.filterRadius = 0.005f;
					myBloomUpsampleBuffer->SetData(&data, sizeof(BloomUpsampleData));
					myBloomUpsampleBuffer->Bind(13);
				});


			for (uint32_t i = 0; i < (uint32_t)myBloomUpsamplePasses.size() - 1; i++)
			{
				auto currMip = myBloomUpsamplePasses[i].framebuffer;
				auto nextMip = myBloomUpsamplePasses[i + 1].framebuffer;

				Renderer::BindTexturesToStage(ShaderStage::Pixel, { currMip->GetColorAttachment(0) }, 0);
				Renderer::SubmitCustom([f = nextMip]()
					{
						f->Bind();
					});
				Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("BloomUpsamplePS"));
				Renderer::SubmitCustom([f = nextMip]()
					{
						f->Unbind();
					});
			}
		}

		// Composite
		{
			Renderer::BindTexturesToStage(ShaderStage::Pixel, { myAOCompositePass.framebuffer->GetColorAttachment(0), myBloomUpsamplePasses.back().framebuffer->GetColorAttachment(0) }, 0);

			Renderer::SubmitCustom([&]()
				{
					myBloomCompositePass.framebuffer->Clear();
					myBloomCompositePass.framebuffer->Bind();
				});

			Renderer::DrawFullscreenTriangleWithShader(ShaderRegistry::Get("BloomComposite"));

			Renderer::SubmitCustom([&]()
				{
					myBloomCompositePass.framebuffer->Unbind();
				});

			Renderer::ClearTexturesAtStage(ShaderStage::Pixel, 0, 2);
		}

		Renderer::EndSection("Bloom");
	}

	void SceneRenderer::HBAOPass(Ref<Camera> aCamera)
	{
		Renderer::BeginSection("HBAO");

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

			Renderer::SubmitCustom([hbaoData = myHBAOData, hbaoBuffer = myHBAOBuffer]()
				{
					hbaoBuffer->SetData(&hbaoData, sizeof(HBAOData));
					hbaoBuffer->Bind(13);
				});

			Renderer::BindTexturesToStage(ShaderStage::Pixel, { myPreDepthPass.framebuffer->GetDepthAttachment() }, 0);
			Renderer::DrawFullscreenTriangleWithShader(myDeinterleavingPass[i].overrideShader);
			Renderer::ClearTexturesAtStage(ShaderStage::Pixel, 0, 1);
			Renderer::EndFullscreenPass();
		}

		// Main Pass
		{
			Renderer::BeginSection("HBAO Main");

			Renderer::SubmitCustom([&]()
				{
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
				});

			Renderer::EndSection("HBAO Main");
		}

		// Reinterleaving
		{
			Renderer::BeginFullscreenPass(myReinterleavingPass, aCamera);

			std::vector<Ref<Image2D>> views{};
			views.reserve(16);
			for (uint32_t i = 0; i < 16; i++)
			{
				views.emplace_back(myHBAOPass.framebuffer->GetColorAttachment(i));
			}

			Renderer::BindTexturesToStage(ShaderStage::Pixel, views, 0);
			Renderer::DrawFullscreenTriangleWithShader(myReinterleavingPass.overrideShader);
			Renderer::ClearTexturesAtStage(ShaderStage::Pixel, 0, 16);
			Renderer::EndFullscreenPass();
		}

		// Blur
		{
			// Pass 1
			{
				Renderer::BindTexturesToStage(ShaderStage::Pixel, { myReinterleavingPass.framebuffer->GetColorAttachment(0) }, 0);
				Renderer::BeginFullscreenPass(myHBAOBlurPass[0], aCamera);

				myHBAOData.invResDirection = { 1.f / myResizeSize.x, 0.f };
				Renderer::SubmitCustom([hbaoData = myHBAOData, hbaoBuffer = myHBAOBuffer]()
					{
						hbaoBuffer->SetData(&hbaoData, sizeof(HBAOData));
					});

				Renderer::DrawFullscreenTriangleWithShader(myHBAOBlurPass[0].overrideShader);
				Renderer::EndFullscreenPass();
			}

			// Pass 2
			{
				Renderer::BindTexturesToStage(ShaderStage::Pixel, { myHBAOBlurPass[0].framebuffer->GetColorAttachment(0) }, 0);
				Renderer::BeginFullscreenPass(myHBAOBlurPass[1], aCamera);
				myHBAOData.invResDirection = { 0.f, 1.f / myResizeSize.y };

				Renderer::SubmitCustom([hbaoData = myHBAOData, hbaoBuffer = myHBAOBuffer]()
					{
						hbaoBuffer->SetData(&hbaoData, sizeof(HBAOData));
					});

				Renderer::DrawFullscreenTriangleWithShader(myHBAOBlurPass[1].overrideShader);
				Renderer::EndFullscreenPass();
			}
		}

		Renderer::EndSection("HBAO");
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

					animSamples = character->SampleAnimation(animCharComp.currentAnimation, animCharComp.currentStartTime, animCharComp.isLooping);
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
