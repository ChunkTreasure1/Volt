#include "sbpch.h"
#include "Sandbox.h"

#include "Sandbox/Camera/EditorCameraController.h"

#include "Sandbox/Window/PropertiesPanel.h"
#include "Sandbox/Window/ViewportPanel.h"
#include "Sandbox/Window/SceneViewPanel.h"
#include "Sandbox/Window/AssetBrowser/AssetBrowserPanel.h"
#include "Sandbox/Window/CreatePanel.h"
#include "Sandbox/Window/LogPanel.h"
#include "Sandbox/Window/AnimationTreeEditor.h"
#include "Sandbox/Window/MaterialEditorPanel.h"
#include "Sandbox/Window/SplinePanel.h"
#include "Sandbox/Window/EngineStatisticsPanel.h"
#include "Sandbox/Window/NavigationPanel.h"
#include "Sandbox/Window/ParticleEmitterEditor.h"
#include "Sandbox/Window/CharacterEditorPanel.h"
#include "Sandbox/Window/AssetRegistryPanel.h"
#include "Sandbox/Window/ThemesPanel.h"
#include "Sandbox/Window/EditorSettingsPanel.h"
#include "Sandbox/Window/PhysicsPanel.h"
#include "Sandbox/Window/RendererSettingsPanel.h"
#include "Sandbox/Window/MeshPreviewPanel.h"
#include "Sandbox/Window/TestNodeEditor/TestNodeEditor.h"
#include "Sandbox/Utility/EditorIconLibrary.h"
#include "Sandbox/Utility/AssetIconLibrary.h"
#include "Sandbox/Utility/EditorLibrary.h"

#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/GlobalEditorStates.h"
#include "Sandbox/UserSettingsManager.h"

#include <Volt/Core/Application.h>
#include <Volt/Core/Window.h>

#include <Volt/Asset/AssetManager.h>
#include <Volt/Rendering/Renderer.h>

#include <Volt/Components/Components.h>
#include <Volt/Components/LightComponents.h>
#include <Volt/Components/PhysicsComponents.h>
#include <Volt/Components/PostProcessComponents.h>

#include <Volt/Asset/Mesh/Mesh.h>
#include <Volt/Asset/Mesh/Material.h>
#include <Volt/Asset/Mesh/SubMaterial.h>

#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>

#include <Volt/Input/KeyCodes.h>
#include <Volt/Input/MouseButtonCodes.h>
#include <Volt/Input/Input.h>

#include <Volt/Rendering/Framebuffer.h>
#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Camera/Camera.h>
#include <Volt/Rendering/Shader/ShaderRegistry.h>
#include <Volt/Rendering/Buffer/ConstantBuffer.h>
#include <Volt/Rendering/Texture/Texture2D.h>

#include <Volt/Utility/FileSystem.h>
#include <Volt/Utility/UIUtility.h>
#include <Volt/Utility/Math.h>

#include <Volt/AI/NavMesh/NavigationsSystem.h>
#include <Volt/AI/NavMesh2/NavMesh2.h>
#include "Volt/Audio/AudioManager.h"

#include <Game/Game.h>

#include <imgui.h>
#include <csignal>

#include <dpp/dpp.h>
#include <ShlObj.h>

#include <gem/noise.h>

std::string GetSIGEventFromInt(int signal)
{
	switch (signal)
	{
		case 8: return "Floating point exception"; break;
		case 11: return "Memory access violation"; break;
		case 22: return "Abort"; break;
	
		default:
			return "Unknown";
	}
}

void SignalHandler(int signal)
{
	dpp::cluster bot("");
	dpp::webhook wh("https://discord.com/api/webhooks/1044616206520438825/lA7ONWakE8XwFQbSFY-ip9aleuAiMaGF8WiinDq1-eBLOLSqqD0MdhSNe-7KNMovnApL");

	const std::string user = FileSystem::GetCurrentUserName();

	auto msg = dpp::message(std::format("{0} just crashed! <:ivar_point:1044955145139662878> It was a {1} error!", user, GetSIGEventFromInt(signal)));
	bot.execute_webhook_sync(wh, msg);
}

Sandbox::Sandbox()
{
	VT_ASSERT(!myInstance, "Sandbox already exists!");
	myInstance = this;

	if (std::signal(SIGSEGV, SignalHandler) == SIG_ERR)
	{
		VT_CORE_ERROR("Unable to create signal handler!");
	}

	if (std::signal(SIGFPE, SignalHandler) == SIG_ERR)
	{
		VT_CORE_ERROR("Unable to create signal handler!");
	}

	if (std::signal(SIGABRT, SignalHandler) == SIG_ERR)
	{
		VT_CORE_ERROR("Unable to create signal handler!");
	}
}

Sandbox::~Sandbox()
{
	myInstance = nullptr;
}

void Sandbox::OnAttach()
{
	// Set working directory
	if (FileSystem::HasEnvironmentVariable("VOLT_PATH"))
	{
		const std::string pathEnv = FileSystem::GetEnvVariable("VOLT_PATH");
		if (!pathEnv.empty())
		{
			std::filesystem::current_path(pathEnv);
		}
	}

	EditorIconLibrary::Initialize();
	AssetIconLibrary::Initialize();
	VersionControl::Initialize(VersionControlSystem::Perforce);

	Volt::Application::Get().GetWindow().Maximize();

	myEditorCameraController = CreateRef<EditorCameraController>(60.f, 1.f, 100000.f);

	myGizmoShader = Volt::ShaderRegistry::Get("EntityGizmo");
	myGridShader = Volt::ShaderRegistry::Get("Grid");

	NewScene();

	myNavigationsSystem = CreateRef<Volt::NavigationsSystem>(myRuntimeScene);

	myEditorWindows.emplace_back(CreateRef<PropertiesPanel>(myRuntimeScene));

	myEditorWindows.emplace_back(CreateRef<ViewportPanel>(mySceneRenderer, myRuntimeScene, myEditorCameraController.get(), mySceneState));
	myViewportPanel = std::reinterpret_pointer_cast<ViewportPanel>(myEditorWindows.back()); // #TODO: This is bad

	myEditorWindows.emplace_back(CreateRef<SceneViewPanel>(myRuntimeScene));
	myEditorWindows.emplace_back(CreateRef<AssetBrowserPanel>(myRuntimeScene));

	myEditorWindows.emplace_back(CreateRef<CharacterEditorPanel>());
	EditorLibrary::Register(Volt::AssetType::AnimatedCharacter, myEditorWindows.back());

	myEditorWindows.emplace_back(CreateRef<MaterialEditorPanel>(myRuntimeScene));
	EditorLibrary::Register(Volt::AssetType::Material, myEditorWindows.back());

	myEditorWindows.emplace_back(CreateRef<ParticleEmitterEditor>());
	EditorLibrary::Register(Volt::AssetType::ParticlePreset, myEditorWindows.back());

	myEditorWindows.emplace_back(CreateRef<MeshPreviewPanel>());
	EditorLibrary::Register(Volt::AssetType::Mesh, myEditorWindows.back());

	myEditorWindows.emplace_back(CreateRef<AssetRegistryPanel>());

	myEditorWindows.emplace_back(CreateRef<LogPanel>());
	myEditorWindows.emplace_back(CreateRef<SplinePanel>(myRuntimeScene));
	myEditorWindows.emplace_back(CreateRef<EngineStatisticsPanel>(myRuntimeScene));
	myEditorWindows.emplace_back(CreateRef<NavigationPanel>(myRuntimeScene));
	myEditorWindows.emplace_back(CreateRef<AnimationTreeEditor>());
	myEditorWindows.emplace_back(CreateRef<EditorSettingsPanel>(UserSettingsManager::GetSettings()));
	myEditorWindows.emplace_back(CreateRef<PhysicsPanel>());
	myEditorWindows.emplace_back(CreateRef<RendererSettingsPanel>(mySceneRenderer));
	myEditorWindows.emplace_back(CreateRef<TestNodeEditor>());

	myFileWatcher = CreateRef<FileWatcher>(std::chrono::milliseconds(2000));
	myFileWatcher->WatchFolder("Engine/Shaders/HLSL/");
	myFileWatcher->WatchFolder("Assets/");

	ImGuizmo::AllowAxisFlip(false);

	//SetupProjectInfo();
	UserSettingsManager::LoadUserSettings(myEditorWindows);

	if (!UserSettingsManager::GetSettings().sceneSettings.lastOpenScene.empty())
	{
		OpenScene(UserSettingsManager::GetSettings().sceneSettings.lastOpenScene);
	}
}

void Sandbox::OnDetach()
{
	if (myRuntimeScene && !myRuntimeScene->path.empty())
	{
		UserSettingsManager::GetSettings().sceneSettings.lastOpenScene = myRuntimeScene->path;
	}

	UserSettingsManager::SaveUserSettings(myEditorWindows);

 	myEditorWindows.clear();
	EditorLibrary::Clear();

	myFileWatcher = nullptr;
	myEditorCameraController = nullptr;
	mySceneRenderer = nullptr;
	myGizmoShader = nullptr;
	myGridShader = nullptr;
	myNavigationsSystem = nullptr;

	myRuntimeScene = nullptr;
	myIntermediateScene = nullptr;
	myGame = nullptr;

	VersionControl::Shutdown();
	AssetIconLibrary::Shutdowm();
	EditorIconLibrary::Shutdown();
}

void Sandbox::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(Sandbox::OnUpdateEvent));
	dispatcher.Dispatch<Volt::AppImGuiUpdateEvent>(VT_BIND_EVENT_FN(Sandbox::OnImGuiUpdateEvent));
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(Sandbox::OnRenderEvent));
	dispatcher.Dispatch<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(Sandbox::OnKeyPressedEvent));
	dispatcher.Dispatch<Volt::ViewportResizeEvent>(VT_BIND_EVENT_FN(Sandbox::OnViewportResizeEvent));
	dispatcher.Dispatch<Volt::OnSceneLoadedEvent>(VT_BIND_EVENT_FN(Sandbox::OnSceneLoadedEvent));
	dispatcher.Dispatch<Volt::OnSceneTransitionEvent>(VT_BIND_EVENT_FN(Sandbox::LoadScene));
	dispatcher.Dispatch<Volt::WindowTitlebarHittestEvent>([&](Volt::WindowTitlebarHittestEvent& e)
		{
			e.SetHit(myTitlebarHovered);
			return true;
		});

	myEditorCameraController->OnEvent(e);

	for (auto& window : myEditorWindows)
	{
		if (window->IsOpen())
		{
			window->OnEvent(e);
		}
	}

	if (e.GetEventType() == Volt::MouseButtonPressed && myRuntimeScene->IsPlaying())
	{
		if (myViewportPanel->IsHovered())
		{
			((Volt::MouseButtonPressedEvent&)e).overViewport = true;
		}
	}

	switch (mySceneState)
	{
		case SceneState::Edit:
			break;
		case SceneState::Play:
			myRuntimeScene->OnEvent(e);
			break;
		case SceneState::Pause:
			break;
		case SceneState::Simulating:
			break;
	}
}

void Sandbox::OnScenePlay()
{
	mySceneState = SceneState::Play;
	SelectionManager::DeselectAll();

	myIntermediateScene = myRuntimeScene;

	myRuntimeScene = CreateRef<Volt::Scene>();
	myIntermediateScene->CopyTo(myRuntimeScene);
	mySceneRenderer = CreateRef<Volt::SceneRenderer>(myRuntimeScene, "Main");

	Volt::OnSceneLoadedEvent loadEvent{ myRuntimeScene };
	Volt::Application::Get().OnEvent(loadEvent);

	myGame = CreateRef<Game>();
	myGame->OnStart();
	myRuntimeScene->OnRuntimeStart();

	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);
}

void Sandbox::OnSceneStop()
{
	mySceneState = SceneState::Edit;
	SelectionManager::DeselectAll();

	Volt::OnSceneStopEvent stopEvent{};
	Volt::Application::Get().OnEvent(stopEvent);

	myRuntimeScene->OnRuntimeEnd();
	myGame->OnStop();

	myRuntimeScene = myIntermediateScene;
	mySceneRenderer = CreateRef<Volt::SceneRenderer>(myRuntimeScene, "Main");
	SetupRenderCallbacks();

	Volt::OnSceneLoadedEvent loadEvent{ myRuntimeScene };
	Volt::Application::Get().OnEvent(loadEvent);

	myIntermediateScene = nullptr;
	myGame = nullptr;
}

void Sandbox::OnSimulationStart()
{
	mySceneState = SceneState::Simulating;
	SelectionManager::DeselectAll();

	myIntermediateScene = myRuntimeScene;

	myRuntimeScene = CreateRef<Volt::Scene>();
	myIntermediateScene->CopyTo(myRuntimeScene);
	mySceneRenderer = CreateRef<Volt::SceneRenderer>(myRuntimeScene, "Main");

	Volt::OnSceneLoadedEvent loadEvent{ myRuntimeScene };
	Volt::Application::Get().OnEvent(loadEvent);

	myRuntimeScene->OnSimulationStart();

	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);
}

void Sandbox::OnSimulationStop()
{
	mySceneState = SceneState::Edit;
	SelectionManager::DeselectAll();

	Volt::OnSceneStopEvent stopEvent{};
	Volt::Application::Get().OnEvent(stopEvent);

	myRuntimeScene->OnSimulationEnd();

	myRuntimeScene = myIntermediateScene;
	mySceneRenderer = CreateRef<Volt::SceneRenderer>(myRuntimeScene, "Main");
	SetupRenderCallbacks();

	Volt::OnSceneLoadedEvent loadEvent{ myRuntimeScene };
	Volt::Application::Get().OnEvent(loadEvent);
}

void Sandbox::ExecuteUndo()
{
	EditorCommandStack* cmdStack = nullptr;

	for (const auto& window : myEditorWindows)
	{
		if (window->IsFocused())
		{
			cmdStack = &window->GetCommandStack();
			break;
		}
	}

	if (cmdStack)
	{
		cmdStack->Undo();
	}
}

void Sandbox::NewScene()
{
	SelectionManager::DeselectAll();
	myRuntimeScene = CreateRef<Volt::Scene>("New Scene");

	// Setup new scene
	{
		// Cube
		{
			auto ent = myRuntimeScene->CreateEntity();
			auto& meshComp = ent.AddComponent<Volt::MeshComponent>();
			auto& tagComp = ent.GetComponent<Volt::TagComponent>().tag = "Cube";
			meshComp.handle = Volt::AssetManager::GetAsset<Volt::Mesh>("Assets/Meshes/Primitives/Cube.vtmesh")->handle;
		}

		// Light
		{
			auto ent = myRuntimeScene->CreateEntity();
			ent.AddComponent<Volt::DirectionalLightComponent>();

			auto& trans = ent.GetComponent<Volt::TransformComponent>();
			auto& tagComp = ent.GetComponent<Volt::TagComponent>().tag = "Directional Light";

			trans.rotation = { gem::pi() / 4.f, gem::pi() / 4.f, gem::pi() / 4.f };
		}

		// Skylight
		{
			auto ent = myRuntimeScene->CreateEntity();
			auto& skyLight = ent.AddComponent<Volt::SkylightComponent>();
			auto& tagComp = ent.GetComponent<Volt::TagComponent>().tag = "Skylight";

			skyLight.environmentHandle = Volt::AssetManager::GetAsset<Volt::Texture2D>("Assets/Textures/HDRIs/studio_small_08_4k.hdr")->handle;
		}

		{
			auto ent = myRuntimeScene->CreateEntity();
			ent.GetComponent<Volt::TagComponent>().tag = "Post Processing";
			ent.AddComponent<Volt::BloomComponent>();
			ent.AddComponent<Volt::FXAAComponent>();
			ent.AddComponent<Volt::HBAOComponent>();
		}
	}
	mySceneRenderer = CreateRef<Volt::SceneRenderer>(myRuntimeScene, "Main");

	Volt::OnSceneLoadedEvent loadEvent{ myRuntimeScene };
	Volt::Application::Get().OnEvent(loadEvent);
}

void Sandbox::OpenScene()
{
	const std::filesystem::path loadPath = FileSystem::OpenFile("Scene (*.vtscene)\0*.vtscene\0");
	OpenScene(loadPath);
}

void Sandbox::OpenScene(const std::filesystem::path& path)
{
	if (!path.empty() && FileSystem::Exists(path))
	{
		SelectionManager::DeselectAll();

		if (myRuntimeScene->path == path)
		{
			Volt::AssetManager::Get().ReloadAsset(myRuntimeScene->handle);
		}
		else if (myRuntimeScene)
		{
			Volt::AssetManager::Get().Unload(myRuntimeScene->handle);
		}
		
		myRuntimeScene = Volt::AssetManager::GetAsset<Volt::Scene>(path);
		mySceneRenderer = CreateRef<Volt::SceneRenderer>(myRuntimeScene, "Main");

		Volt::OnSceneLoadedEvent loadEvent{ myRuntimeScene };
		Volt::Application::Get().OnEvent(loadEvent);
	}
}

bool Sandbox::LoadScene(Volt::OnSceneTransitionEvent& e)
{
	myStoredScene = Volt::AssetManager::GetAsset<Volt::Scene>(e.GetHandle());
	myShouldLoadNewScene = true;

	return true;
}

void Sandbox::SaveScene()
{
	if (myRuntimeScene)
	{
		if (!myRuntimeScene->path.empty())
		{
			if (FileSystem::IsWriteable(myRuntimeScene->path))
			{
				Volt::AssetManager::Get().SaveAsset(myRuntimeScene);
				UI::Notify(NotificationType::Success, "Scene saved!", std::format("Scene {0} was saved successfully!", myRuntimeScene->path.string()));
			}
			else
			{
				UI::Notify(NotificationType::Error, "Unable to save scene!", std::format("Scene {0} was is not writeable!", myRuntimeScene->path.string()));
			}
		}
		else
		{
			SaveSceneAs();
		}
	}
}

void Sandbox::TransitionToNewScene()
{
	myRuntimeScene->OnRuntimeEnd();

	SelectionManager::DeselectAll();

	Volt::AssetManager::Get().Unload(myRuntimeScene->handle);

	myRuntimeScene = myStoredScene;
	mySceneRenderer = CreateRef<Volt::SceneRenderer>(myRuntimeScene, "Main");

	AUDIOMANAGER.ResetListener();

	Volt::OnSceneLoadedEvent loadEvent{ myRuntimeScene };
	Volt::Application::Get().OnEvent(loadEvent);

	myRuntimeScene->OnRuntimeStart();

	Volt::ViewportResizeEvent windowResizeEvent{ myViewportPosition.x,myViewportPosition.y ,myViewportSize.x, myViewportSize.y };
	Volt::Application::Get().OnEvent(windowResizeEvent);

	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);

	myShouldLoadNewScene = false;
}


void Sandbox::SaveSceneAs()
{
	myShouldOpenSaveSceneAs = true;
}

void Sandbox::InstallMayaTools()
{
	const std::filesystem::path documentsPath = FileSystem::GetDocumentsPath();
	const std::filesystem::path mayaPath = documentsPath / "maya";
	if (!std::filesystem::exists(mayaPath))
	{
		UI::Notify(NotificationType::Error, "Failed to install Maya tools", "Unable to install Maya tools because no installation was found!");
		return;
	}

	for (const auto& it : std::filesystem::directory_iterator(mayaPath))
	{
		if (!it.is_directory())
		{
			continue;
		}

		const std::string folderName = it.path().stem().string();
		if (!std::all_of(folderName.begin(), folderName.end(), ::isdigit))
		{
			continue;
		}

		const std::filesystem::path pluginsPath = it.path() / "plug-ins";
		const std::filesystem::path scriptsPath = it.path() / "scripts";
		const std::filesystem::path yamlPath = scriptsPath / "yaml";

		if (!std::filesystem::exists(pluginsPath))
		{
			std::filesystem::create_directories(pluginsPath);
		}

		if (!std::filesystem::exists(scriptsPath))
		{
			std::filesystem::create_directories(scriptsPath);
		}

		if (!std::filesystem::exists(yamlPath))
		{
			std::filesystem::create_directory(yamlPath);
		}

		FileSystem::CopyFileTo("../Tools/MayaExporter/voltTranslator.py", pluginsPath);
		FileSystem::CopyFileTo("../Tools/MayaExporter/voltExport.py", scriptsPath);
		FileSystem::CopyFileTo("../Tools/MayaExporter/voltTranslatorOpts.mel", scriptsPath);

		FileSystem::Copy("../Tools/MayaExporter/yaml", scriptsPath / "yaml");
	}

	FileSystem::SetEnvVariable("VOLT_PATH", std::filesystem::current_path().string());
	UI::Notify(NotificationType::Success, "Successfully installed Maya tools!", "The Maya tools were successfully installed!");
}

void Sandbox::SetupRenderCallbacks()
{
	mySceneRenderer->AddExternalPassCallback([this](Ref<Volt::Scene> scene, Ref<Volt::Camera> camera)
		{
			if (mySceneState == SceneState::Play)
			{
				return;
			}

			// Selected geometry pass
			{
				Volt::Renderer::SetDepthState(Volt::DepthState::ReadWrite);
				Volt::Renderer::BeginPass(mySelectedGeometryPass, camera);

				auto& registry = scene->GetRegistry();

				for (const auto& ent : SelectionManager::GetSelectedEntities())
				{
					auto& transComp = registry.GetComponent<Volt::TransformComponent>(ent);
					if (transComp.visible && registry.HasComponent<Volt::MeshComponent>(ent))
					{
						auto& meshComp = registry.GetComponent<Volt::MeshComponent>(ent);
						auto mesh = Volt::AssetManager::GetAsset<Volt::Mesh>(meshComp.handle);
						if (mesh && mesh->IsValid())
						{
							if (meshComp.subMeshIndex != -1)
							{
								Volt::Renderer::DrawMesh(mesh, mesh->GetMaterial(), (uint32_t)meshComp.subMeshIndex, scene->GetWorldSpaceTransform(Volt::Entity{ ent, scene.get() }));
							}
							else
							{
								Volt::Renderer::DrawMesh(mesh, scene->GetWorldSpaceTransform(Volt::Entity{ ent, scene.get() }));
							}
						}
					}
				}

				Volt::Renderer::EndPass();
			}

			auto context = Volt::GraphicsContext::GetContext(); // #TODO: Find better way to bind textures here

			// Jump Flood Init
			{
				Volt::Renderer::BeginPass(myJumpFloodInitPass, camera);

				context->PSSetShaderResources(0, 1, mySelectedGeometryPass.framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());

				Volt::Renderer::DrawFullscreenTriangleWithShader(myJumpFloodInitPass.overrideShader);
				Volt::Renderer::EndPass();
			}

			// Jump Flood Pass
			{
				int32_t steps = 2;
				int32_t step = (int32_t)std::round(std::pow(steps - 1, 2));
				int32_t index = 0;

				struct FloodPassData
				{
					gem::vec2 texelSize;
					int32_t step;
					int32_t padding;
				} floodPassData;

				auto framebuffer = myJumpFloodPass[0].framebuffer;
				floodPassData.texelSize = { 1.f / (float)framebuffer->GetWidth(), 1.f / (float)framebuffer->GetHeight() };
				floodPassData.step = step;

				while (step != 0)
				{
					Volt::Renderer::BeginPass(myJumpFloodPass[index], camera);
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

					Volt::Renderer::DrawFullscreenQuadWithShader(myJumpFloodPass[index].overrideShader);
					Volt::Renderer::EndPass();

					ID3D11ShaderResourceView* nullSRV = nullptr;
					context->PSSetShaderResources(0, 1, &nullSRV);

					index = (index + 1) % 2;
					step /= 2;

					floodPassData.step = step;
				}
			}

			// Jump Flood Composite
			{
				Volt::Renderer::SetDepthState(Volt::DepthState::None);
				Volt::Renderer::BeginPass(myJumpFloodCompositePass, camera);

				const gem::vec4 color = { 1.f, 0.5f, 0.f, 1.f };

				myJumpFloodBuffer->SetData(&color, sizeof(gem::vec4));
				myJumpFloodBuffer->Bind(13);

				context->PSSetShaderResources(0, 1, myJumpFloodPass[0].framebuffer->GetColorAttachment(0)->GetSRV().GetAddressOf());
				Volt::Renderer::DrawFullscreenQuadWithShader(myJumpFloodCompositePass.overrideShader);
				Volt::Renderer::EndPass();
			}

			// Gizmo pass
			{
				Volt::Renderer::BeginPass(myGizmoPass, camera, false);

				auto& registry = scene->GetRegistry();
				Ref<Volt::Texture2D> gizmoTexture = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/icon_entityGizmo.dds");
				Ref<Volt::Texture2D> lightGizmoTexture = Volt::AssetManager::GetAsset<Volt::Texture2D>("Editor/Textures/Icons/icon_lightGizmo.dds");

				registry.ForEach<Volt::TransformComponent>([&](Wire::EntityId id, const Volt::TransformComponent& transformComp)
					{
						if (transformComp.visible && myShouldRenderGizmos)
						{
							gem::vec3 p, s, r;
							Volt::Math::DecomposeTransform(myRuntimeScene->GetWorldSpaceTransform(Volt::Entity{ id, myRuntimeScene.get() }), p, r, s);

							const float maxDist = 5000.f;
							const float lerpStartDist = 4000.f;
							const float maxScale = 1.f;
							const float distance = gem::distance(camera->GetPosition(), p);

							float alpha = 1.f;

							if (distance >= lerpStartDist)
							{
								alpha = gem::lerp(1.f, 0.f, (distance - lerpStartDist) / (maxDist - lerpStartDist));
							}

							if (distance < maxDist)
							{
								float scale = gem::min(distance / maxDist, maxScale);

								Ref<Volt::Texture2D> gizmo = registry.HasComponent<Volt::PointLightComponent>(id) ? lightGizmoTexture : gizmoTexture;
								Volt::Renderer::SubmitBillboard(gizmo, p, gem::vec3{ scale }, id, gem::vec4{ 1.f, 1.f, 1.f, alpha });
							}
						}
					});

				Volt::Renderer::DispatchBillboardsWithShader(myGizmoShader);
				Volt::Renderer::EndPass();
			}

			auto& registry = scene->GetRegistry();

			///// Collider Visualization /////
			{
				Volt::Renderer::BeginPass(myColliderVisualizationPass, camera, false);
				Volt::Renderer::SetDepthState(Volt::DepthState::ReadWrite);

				auto collisionMaterial = Volt::AssetManager::GetAsset<Volt::Material>("Assets/Materials/M_ColliderDebug.vtmat");
				registry.ForEach<Volt::BoxColliderComponent>([&](Wire::EntityId id, const Volt::BoxColliderComponent& collider)
					{
						if (!SelectionManager::IsSelected(id))
						{
							return;
						}

						auto trs = myRuntimeScene->GetWorldSpaceTRS(Volt::Entity{ id, myRuntimeScene.get() });

						const gem::vec3 cubeHalfSize = 50.f;
						const gem::vec3 colliderScale = collider.halfSize / cubeHalfSize;
						const gem::vec3 resultScale = colliderScale * trs.scale;
						const gem::mat4 transform = gem::translate(gem::mat4(1.f), trs.position + collider.offset) * gem::mat4_cast(gem::quat(trs.rotation)) * gem::scale(gem::mat4(1.f), resultScale);

						auto cubeMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Assets/Meshes/Primitives/Cube.vtmesh");

						Volt::Renderer::DrawMesh(cubeMesh, collisionMaterial, transform);
					});

				registry.ForEach<Volt::SphereColliderComponent>([&](Wire::EntityId id, const Volt::SphereColliderComponent& collider)
					{
						if (!SelectionManager::IsSelected(id))
						{
							return;
						}

						auto trs = myRuntimeScene->GetWorldSpaceTRS(Volt::Entity{ id, myRuntimeScene.get() });

						const gem::vec3 sphereRadius = 50.f;
						const float maxScale = gem::max(trs.scale.x, gem::max(trs.scale.y, trs.scale.z));
						const gem::vec3 resultScale = maxScale * collider.radius / sphereRadius;
						const gem::mat4 transform = gem::translate(gem::mat4(1.f), trs.position + collider.offset) * gem::mat4_cast(gem::quat(trs.rotation)) * gem::scale(gem::mat4(1.f), resultScale);

						auto cubeMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Assets/Meshes/Primitives/Sphere.vtmesh");

						Volt::Renderer::DrawMesh(cubeMesh, collisionMaterial, transform);
					});

				registry.ForEach<Volt::CapsuleColliderComponent>([&](Wire::EntityId id, const Volt::CapsuleColliderComponent& collider)
					{
						if (!SelectionManager::IsSelected(id))
						{
							return;
						}

						auto trs = myRuntimeScene->GetWorldSpaceTRS(Volt::Entity{ id, myRuntimeScene.get() });

						const float capsuleRadius = 50.f;
						const float capsuleHeight = 50.f;

						const float radiusScale = gem::max(trs.scale.x, trs.scale.y);
						const float heightScale = trs.scale.y;

						const gem::vec3 resultScale = { radiusScale * collider.radius / capsuleRadius, heightScale * collider.height / capsuleHeight, radiusScale * collider.radius / capsuleRadius };
						const gem::mat4 transform = gem::translate(gem::mat4(1.f), trs.position + collider.offset) * gem::mat4_cast(gem::quat(trs.rotation)) * gem::scale(gem::mat4(1.f), resultScale);

						auto cubeMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Assets/Meshes/Primitives/Capsule.vtmesh");

						Volt::Renderer::DrawMesh(cubeMesh, collisionMaterial, transform);
					});

				Volt::Renderer::EndPass();
			}
			//////////////////////////////////

			{
				auto material = Volt::AssetManager::GetAsset<Volt::Material>("Assets/Materials/M_ColliderDebug.vtmat");
				auto arrowMesh = Volt::AssetManager::GetAsset<Volt::Mesh>("Assets/Meshes/Editor/3dpil.vtmesh");

				Volt::Renderer::BeginPass(myForwardExtraPass, camera);
				registry.ForEach<Volt::DecalComponent>([&](Wire::EntityId id, const Volt::DecalComponent& decalComp)
					{
						if (!SelectionManager::IsSelected(id))
						{
							return;
						}

						auto ent = Volt::Entity{ id, myRuntimeScene.get() };
						auto trs = myRuntimeScene->GetWorldSpaceTRS(ent);

						gem::vec3 newRot = trs.rotation;
						newRot.x += gem::radians(-90.f);

						constexpr float uniformScale = 0.25f * 0.25f;
						gem::mat4 transform = gem::translate(gem::mat4(1.f), trs.position) * gem::mat4_cast(gem::quat(newRot)) * gem::scale(gem::mat4(1.f), { uniformScale, uniformScale, uniformScale });

						Volt::Renderer::DrawMesh(arrowMesh, material, transform);
					});

				Volt::Renderer::DispatchLines();

				Volt::Renderer::SubmitSprite(gem::mat4{ 1.f }, { 1.f, 1.f, 1.f, 1.f });
				Volt::Renderer::DispatchSpritesWithShader(myGridShader);

				Volt::Renderer::EndPass();
			}
		});
}

void Sandbox::SetupEditorRenderPasses()
{
	// Selected Geometry
	{
		Volt::FramebufferSpecification spec{};
		spec.attachments =
		{
			{ Volt::ImageFormat::RGBA32F, gem::vec4{ 0.f, 0.f, 0.f, 0.f } },
			Volt::ImageFormat::DEPTH32F
		};

		spec.width = myViewportSize.x;
		spec.height = myViewportSize.y;

		mySelectedGeometryPass.framebuffer = Volt::Framebuffer::Create(spec);
		mySelectedGeometryPass.overrideShader = Volt::ShaderRegistry::Get("SelectedGeometry");
		mySelectedGeometryPass.debugName = "Selected Geometry";
	}

	// Jump Flood
	{
		// Init
		{
			Volt::FramebufferSpecification spec{};
			spec.attachments =
			{
				{Volt::ImageFormat::RGBA32F, gem::vec4{ 1.f, 1.f, 1.f, 0.f }}
			};

			spec.width = myViewportSize.x;
			spec.height = myViewportSize.y;

			myJumpFloodInitPass.framebuffer = Volt::Framebuffer::Create(spec);
			myJumpFloodInitPass.overrideShader = Volt::ShaderRegistry::Get("JumpFloodInit");
			myJumpFloodInitPass.debugName = "Jump Flood Init";
		}

		// Pass
		{
			myJumpFloodBuffer = Volt::ConstantBuffer::Create(nullptr, sizeof(gem::vec2) + sizeof(int32_t) * 2, Volt::ShaderStage::Vertex | Volt::ShaderStage::Pixel);

			for (uint32_t i = 0; i < 2; i++)
			{
				Volt::FramebufferSpecification spec{};
				spec.attachments =
				{
					Volt::ImageFormat::RGBA32F
				};

				spec.width = myViewportSize.x;
				spec.height = myViewportSize.y;

				myJumpFloodPass[i].framebuffer = Volt::Framebuffer::Create(spec);
				myJumpFloodPass[i].overrideShader = Volt::ShaderRegistry::Get("JumpFloodPass");
				myJumpFloodPass[i].debugName = "Jump Flood Pass" + std::to_string(i);
			}
		}

		// Composite
		{
			Volt::FramebufferSpecification spec{};
			spec.attachments =
			{
				{ Volt::ImageFormat::RGBA },
			};

			spec.existingImages =
			{
				{ 0, mySceneRenderer->GetFinalFramebuffer()->GetColorAttachment(0) },
			};

			spec.existingDepth = mySceneRenderer->GetFinalObjectFramebuffer()->GetDepthAttachment();

			spec.width = myViewportSize.x;
			spec.height = myViewportSize.y;

			myJumpFloodCompositePass.framebuffer = Volt::Framebuffer::Create(spec);
			myJumpFloodCompositePass.overrideShader = Volt::ShaderRegistry::Get("JumpFloodComposite");
			myJumpFloodCompositePass.debugName = "Jump Flood Composite";
		}
	}

	// Gizmo
	{
		Volt::FramebufferSpecification spec{};
		spec.attachments =
		{
			{ Volt::ImageFormat::RGBA },
			{ Volt::ImageFormat::R32UI }
		};

		auto finalFramebuffer = mySceneRenderer->GetFinalFramebuffer();
		auto selectionFramebuffer = mySceneRenderer->GetSelectionFramebuffer();

		spec.width = 1280;
		spec.width = 720;

		spec.existingImages =
		{
			{ 0, finalFramebuffer->GetColorAttachment(0) },
			{ 1, selectionFramebuffer->GetColorAttachment(3) }
		};

		myGizmoPass.framebuffer = Volt::Framebuffer::Create(spec);
		myGizmoPass.debugName = "Gizmos";
	}

	// Collider visualization
	{
		Volt::FramebufferSpecification spec{};

		spec.attachments =
		{
			{ Volt::ImageFormat::RGBA32F }, // Color
			{ Volt::ImageFormat::DEPTH32F }
		};

		spec.width = 1280;
		spec.height = 720;

		spec.existingImages =
		{
			{ 0, mySceneRenderer->GetFinalFramebuffer()->GetColorAttachment(0) },
		};

		spec.existingDepth = mySceneRenderer->GetFinalObjectFramebuffer()->GetDepthAttachment();

		myColliderVisualizationPass.framebuffer = Volt::Framebuffer::Create(spec);
		myColliderVisualizationPass.debugName = "Collider Visualization";
		myColliderVisualizationPass.exclusiveShaderHash = Volt::ShaderRegistry::Get("ColliderDebug")->GetHash();
	}

	// Forward Extra
	{
		Volt::FramebufferSpecification spec{};

		spec.attachments =
		{
			{ Volt::ImageFormat::RGBA32F }, // Color
			{ Volt::ImageFormat::R32UI }, // ID
			{ Volt::ImageFormat::DEPTH32F }
		};

		spec.width = 1280;
		spec.height = 720;

		spec.existingImages =
		{
			{ 0, mySceneRenderer->GetFinalFramebuffer()->GetColorAttachment(0) },
			{ 1, mySceneRenderer->GetSelectionFramebuffer()->GetColorAttachment(2) },
		};

		spec.existingDepth = mySceneRenderer->GetFinalObjectFramebuffer()->GetDepthAttachment();

		myForwardExtraPass.framebuffer = Volt::Framebuffer::Create(spec);
		myForwardExtraPass.debugName = "Forward Extra";
	}
}

void Sandbox::HandleChangedFiles()
{
	if (myFileWatcher->AnyFileChanged())
	{
		const auto changedFile = myFileWatcher->QueryChangedFile();
		Volt::AssetType assetType = Volt::AssetManager::Get().GetAssetTypeFromPath(changedFile.filePath);
		switch (changedFile.status)
		{
			case FileStatus::Modified:
			{
				switch (assetType)
				{
					case Volt::AssetType::ShaderSource:
						Volt::ShaderRegistry::ReloadShadersWithShader(changedFile.filePath);
						break;
				}

				break;
			}

			case FileStatus::Removed:
			{
				if (assetType != Volt::AssetType::None)
				{
					Volt::AssetManager::Get().RemoveFromRegistry(changedFile.filePath);
				}
				break;
			}

			case FileStatus::Added:
			{
				break;
			}
		}
	}
}

bool Sandbox::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	EditorCommandStack::GetInstance().Update(100);

	Volt::Entity a(1, myRuntimeScene.get());
	switch (mySceneState)
	{
		case SceneState::Edit:
			myRuntimeScene->UpdateEditor(e.GetTimestep());
			AUDIOMANAGER.StopAll();
			initiated = false;
			break;

		case SceneState::Play:
			myRuntimeScene->Update(e.GetTimestep());
			AUDIOMANAGER.Update(e.GetTimestep());

			// AI
			myNavigationsSystem->OnRuntimeUpdate(e.GetTimestep());
			break;

		case SceneState::Pause:
			break;

		case SceneState::Simulating:
			myRuntimeScene->UpdateSimulation(e.GetTimestep());
			break;
	}

	for (const auto& ent : SelectionManager::GetSelectedEntities()) // #TODO(Ivar): maybe move into selection manager update?
	{
		if (!myRuntimeScene->GetRegistry().Exists(ent))
		{
			SelectionManager::Deselect(ent);
		}
	}

	if (myShouldResetLayout)
	{
		ImGui::LoadIniSettingsFromDisk("Editor/imgui.ini");
		myShouldResetLayout = false;
	}

	if (myBuildStarted)
	{
		if (!GameBuilder::IsBuilding())
		{
			UI::Notify(NotificationType::Success, "Build Finished!", std::format("Build finished successfully in {0} seconds!", GameBuilder::GetCurrentBuildTime()));
			myBuildStarted = false;
		}
	}

	HandleChangedFiles();

	return true;
}

bool Sandbox::OnImGuiUpdateEvent(Volt::AppImGuiUpdateEvent& e)
{
	ImGuizmo::BeginFrame();

	if (SaveReturnState returnState = EditorUtils::SaveFilePopup("Do you want to save scene?##OpenScene"); returnState != SaveReturnState::None)
	{
		if (returnState == SaveReturnState::Save)
		{
			SaveScene();
		}

		OpenScene();
	}

	if (SaveReturnState returnState = EditorUtils::SaveFilePopup("Do you want to save scene?##NewScene"); returnState != SaveReturnState::None)
	{
		if (returnState == SaveReturnState::Save)
		{
			SaveScene();
		}

		NewScene();
	}

	UpdateDockSpace();

	if (myShouldOpenSaveSceneAs)
	{
		UI::OpenModal("Save As");
		myShouldOpenSaveSceneAs = false;
	}

	SaveSceneAsModal();
	BuildGameModal();

	for (auto& window : myEditorWindows)
	{
		if (window->Begin())
		{
			window->UpdateMainContent();
			window->End();

			window->UpdateContent();
		}
	}

	if (myBuildStarted)
	{
		const float buildProgess = GameBuilder::GetBuildProgress();
		RenderProgressBar(buildProgess);
	}

	return false;
}

bool Sandbox::OnRenderEvent(Volt::AppRenderEvent& e)
{
	switch (mySceneState)
	{
		case SceneState::Edit:
			Volt::NavigationsSystem::Get().Draw();
			mySceneRenderer->OnRenderEditor(myEditorCameraController->GetCamera());
			break;

		case SceneState::Play:
			Volt::NavigationsSystem::Get().Draw();
			mySceneRenderer->OnRenderRuntime();
			break;

		case SceneState::Pause:
			mySceneRenderer->OnRenderRuntime();
			break;

		case SceneState::Simulating:
			mySceneRenderer->OnRenderEditor(myEditorCameraController->GetCamera());
			break;
	}

	if (myShouldLoadNewScene)
	{
		TransitionToNewScene();
	}

	return false;
}

bool Sandbox::OnKeyPressedEvent(Volt::KeyPressedEvent& e)
{
	const bool ctrlPressed = Volt::Input::IsKeyDown(VT_KEY_LEFT_CONTROL);
	const bool shiftPressed = Volt::Input::IsKeyDown(VT_KEY_LEFT_SHIFT);

	switch (e.GetKeyCode())
	{
		case VT_KEY_Z:
		{
			if (ctrlPressed && shiftPressed)
			{
				EditorCommandStack::GetInstance().Redo();
			}
			else if (ctrlPressed)
			{
				EditorCommandStack::GetInstance().Undo();
			}
			break;
		}

		case VT_KEY_Y:
		{
			if (ctrlPressed)
			{
				EditorCommandStack::GetInstance().Redo();
			}
			break;
		}

		case VT_KEY_S:
		{
			if (ctrlPressed && !shiftPressed)
			{
				SaveScene();
			}
			else if (ctrlPressed && shiftPressed)
			{
				SaveSceneAs();
			}

			break;
		}

		case VT_KEY_O:
		{
			if (ctrlPressed)
			{
				if (myRuntimeScene)
				{
					myOpenShouldSaveScenePopup = true;
				}
				else
				{
					OpenScene();
				}
			}

			break;
		}

		case VT_KEY_N:
		{
			if (ctrlPressed)
			{
				NewScene();
			}

			break;
		}

		case VT_KEY_DELETE:
		{
			std::vector<Volt::Entity> entitiesToRemove;

			auto selection = SelectionManager::GetSelectedEntities();
			for (const auto& selectedEntity : selection)
			{
				Volt::Entity tempEnt = Volt::Entity(selectedEntity, myRuntimeScene.get());
				entitiesToRemove.push_back(tempEnt);

				SelectionManager::Deselect(tempEnt.GetId());
			}

			Ref<ObjectStateCommand> command = CreateRef<ObjectStateCommand>(entitiesToRemove, ObjectStateAction::Delete);
			EditorCommandStack::GetInstance().PushUndo(command);

			for (int i = 0; i < entitiesToRemove.size(); i++)
			{
				myRuntimeScene->RemoveEntity(entitiesToRemove[i]);
			}

			break;
		}

		case VT_KEY_F:
		{
			if (SelectionManager::GetSelectedCount() > 0)
			{
				Volt::Entity ent = { SelectionManager::GetSelectedEntities().at(0), myRuntimeScene.get() };
				myEditorCameraController->Focus(ent.GetWorldPosition());
			}

			break;
		}

		default:
			break;
	}

	return false;
}

bool Sandbox::OnViewportResizeEvent(Volt::ViewportResizeEvent& e)
{
	myViewportSize = { e.GetWidth(), e.GetHeight() };
	myViewportPosition = { e.GetX(), e.GetY() };

	myRuntimeScene->SetRenderSize(e.GetWidth(), e.GetHeight());
	myGizmoPass.framebuffer->Resize(e.GetWidth(), e.GetHeight());

	mySelectedGeometryPass.framebuffer->Resize(e.GetWidth(), e.GetHeight());
	myJumpFloodInitPass.framebuffer->Resize(e.GetWidth(), e.GetHeight());

	for (const auto& pass : myJumpFloodPass)
	{
		pass.framebuffer->Resize(e.GetWidth(), e.GetHeight());
	}

	myJumpFloodCompositePass.framebuffer->Resize(e.GetWidth(), e.GetHeight());

	myForwardExtraPass.framebuffer->Resize(e.GetWidth(), e.GetHeight());
	myColliderVisualizationPass.framebuffer->Resize(e.GetWidth(), e.GetHeight());

	return false;
}

bool Sandbox::OnSceneLoadedEvent(Volt::OnSceneLoadedEvent& e)
{
	SetupEditorRenderPasses();
	SetupRenderCallbacks();

	mySceneRenderer->Resize(myViewportSize.x, myViewportSize.y);
	e.GetScene()->SetRenderSize(myViewportSize.x, myViewportSize.y);

	Volt::ViewportResizeEvent e2 = { 0, 0, myViewportSize.x, myViewportSize.y };
	OnViewportResizeEvent(e2);

	// AI
	if (myNavigationsSystem)
	{
		myNavigationsSystem->OnSceneLoad();
	}

	return false;
}
