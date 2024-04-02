#include "sbpch.h"
#include "Sandbox.h"

#include "Sandbox/Camera/EditorCameraController.h"

#include "Sandbox/UISystems/ModalSystem.h"

#include "Sandbox/Window/PropertiesPanel.h"
#include "Sandbox/Window/ViewportPanel.h"
#include "Sandbox/Window/GameViewPanel.h"
#include "Sandbox/Window/SceneViewPanel.h"
#include "Sandbox/Window/AssetBrowser/AssetBrowserPanel.h"
#include "Sandbox/Window/LogPanel.h"
#include "Sandbox/Window/MaterialEditorPanel.h"
#include "Sandbox/Window/SplinePanel.h"
#include "Sandbox/Window/VisonPanel.h"
#include "Sandbox/Window/EngineStatisticsPanel.h"
#include "Sandbox/Window/ParticleEmitterEditor.h"
#include "Sandbox/Window/CharacterEditorPanel.h"
#include "Sandbox/Window/AssetRegistryPanel.h"
#include "Sandbox/Window/ThemesPanel.h"
#include "Sandbox/Window/Taiga/TaigaPanel.h"
#include "Sandbox/Window/EditorSettingsPanel.h"
#include "Sandbox/Window/PhysicsPanel.h"
#include "Sandbox/Window/RendererSettingsPanel.h"
#include "Sandbox/Window/MeshPreviewPanel.h"
#include "Sandbox/Window/GraphKey/GraphKeyPanel.h"
#include "Sandbox/Window/PrefabEditorPanel.h"
#include "Sandbox/Window/AnimationGraph/AnimationGraphPanel.h"
#include "Sandbox/Window/Sequencer.h"
#include "Sandbox/Window/BlendSpaceEditorPanel.h"
#include "Sandbox/Window/CurveGraphPanel.h"
#include "Sandbox/Window/Timeline.h"
#include "Sandbox/Window/ShaderEditorPanel.h"
#include "Sandbox/Window/PostProcessingStackPanel.h"
#include "Sandbox/Window/PostProcessingMaterialPanel.h"
#include "Sandbox/Window/NavigationPanel.h"
#include "Sandbox/Window/Net/NetPanel.h"
#include "Sandbox/Window/Net/NetContractPanel.h"
#include "Sandbox/Window/BehaviourGraph/BehaviorPanel.h"
#include "Sandbox/Window/SceneSettingsPanel.h"
#include "Sandbox/Window/WorldEnginePanel.h"
#include "Sandbox/Window/MosaicEditor/MosaicEditorPanel.h"
#include "Sandbox/Window/SkeletonEditorPanel.h"
#include "Sandbox/Window/AnimationEditorPanel.h"
#include "Sandbox/VertexPainting/VertexPainterPanel.h"

#include "Sandbox/Utility/EditorResources.h"
#include "Sandbox/Utility/EditorLibrary.h"
#include "Sandbox/Utility/SelectionManager.h"
#include "Sandbox/Utility/NodeEditorHelpers.h"

#include "Sandbox/UserSettingsManager.h"

#include <Volt/Core/Application.h>
#include <Volt/Core/Window.h>

#include <Volt/Asset/AssetManager.h>

#include <Volt/Components/CoreComponents.h>
#include <Volt/Components/LightComponents.h>

#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>
#include <Volt/Scene/SceneManager.h>

#include <Volt/Input/KeyCodes.h>
#include <Volt/Input/Input.h>

#include <Volt/Rendering/Texture/Image2D.h>
#include <Volt/Rendering/Camera/Camera.h>
#include <Volt/Rendering/Renderer.h>

#include <Volt/RenderingNew/SceneRendererNew.h>

#include <Volt/Utility/FileSystem.h>
#include <Volt/Utility/UIUtility.h>

#include <Volt/Project/ProjectManager.h>
#include <Volt/Scripting/Mono/MonoScriptEngine.h>

#include <Volt/Events/SettingsEvent.h>

#include <Volt/Discord/DiscordSDK.h>

#include <NavigationEditor/Tools/NavMeshDebugDrawer.h>

#include <VoltRHI/Images/Image2D.h>

#include <imgui.h>

#include "Volt/Asset/Serializers/SourceTextureSerializer.h"
#include "Volt/Asset/Serializers/TextureSerializer.h"

Sandbox::Sandbox()
{
	VT_ASSERT(!myInstance, "Sandbox already exists!");
	myInstance = this;
}

Sandbox::~Sandbox()
{
	myInstance = nullptr;
}

void Sandbox::OnAttach()
{
	SelectionManager::Init();

	if (!Volt::ProjectManager::GetProject().isDeprecated)
	{
		EditorResources::Initialize();
	}

	VersionControl::Initialize(VersionControlSystem::Perforce);
	NodeEditorHelpers::Initialize();
	IONodeGraphEditorHelpers::Initialize();

	Volt::Application::Get().GetWindow().Maximize();

	myEditorCameraController = CreateRef<EditorCameraController>(60.f, 1.f, 100000.f);

	UserSettingsManager::LoadUserSettings();
	const auto& userSettings = UserSettingsManager::GetSettings();

	NewScene();

	// Shelved Panels (So panel tab doesn't get cluttered up).
#ifdef VT_DEBUG
	EditorLibrary::RegisterWithType<PrefabEditorPanel>("", Volt::AssetType::Prefab);
	EditorLibrary::Register<SplinePanel>("", myRuntimeScene);
	EditorLibrary::Register<Sequencer>("", myRuntimeScene);
	EditorLibrary::Register<TaigaPanel>("Advanced");
	EditorLibrary::Register<ThemesPanel>("Advanced");
	EditorLibrary::Register<CurveGraphPanel>("Advanced");
#endif

	myNavigationPanel = EditorLibrary::Register<NavigationPanel>("Advanced", myRuntimeScene);
	EditorLibrary::Register<PropertiesPanel>("Level Editor", myRuntimeScene, mySceneRenderer, mySceneState, "");
	myViewportPanel = EditorLibrary::Register<ViewportPanel>("Level Editor", mySceneRenderer, myRuntimeScene, myEditorCameraController.get(), mySceneState);
	EditorLibrary::Register<LogPanel>("Advanced");
	EditorLibrary::Register<SceneViewPanel>("Level Editor", myRuntimeScene, "");
	EditorLibrary::Register<AssetRegistryPanel>("Advanced");
	EditorLibrary::Register<VisionPanel>("", myRuntimeScene, myEditorCameraController.get());
	EditorLibrary::Register<EngineStatisticsPanel>("Advanced", myRuntimeScene, mySceneRenderer, myGameSceneRenderer);
	EditorLibrary::Register<EditorSettingsPanel>("Advanced", UserSettingsManager::GetSettings());
	EditorLibrary::Register<PhysicsPanel>("Physics");
	EditorLibrary::Register<RendererSettingsPanel>("Advanced", mySceneRenderer);
	EditorLibrary::Register<GraphKeyPanel>("", myRuntimeScene);
	EditorLibrary::Register<VertexPainterPanel>("", myRuntimeScene, myEditorCameraController);

	EditorLibrary::Register<NetPanel>("Advanced");
	EditorLibrary::Register<NetContractPanel>("Advanced");
	EditorLibrary::Register<SceneSettingsPanel>("", myRuntimeScene);
	EditorLibrary::Register<WorldEnginePanel>("", myRuntimeScene);

	EditorLibrary::RegisterWithType<MosaicEditorPanel>("", Volt::AssetType::Material);

	if (userSettings.sceneSettings.lowMemoryUsage)
	{
		myGameViewPanel = EditorLibrary::Register<GameViewPanel>("Level Editor", mySceneRenderer, myRuntimeScene, mySceneState);
	}
	else
	{
		myGameViewPanel = EditorLibrary::Register<GameViewPanel>("Level Editor", myGameSceneRenderer, myRuntimeScene, mySceneState);
	}

	myAssetBrowserPanel = EditorLibrary::Register<AssetBrowserPanel>("", myRuntimeScene, "##Main");

	EditorLibrary::RegisterWithType<CharacterEditorPanel>("Animation", Volt::AssetType::AnimatedCharacter);
	//EditorLibrary::RegisterWithType<MaterialEditorPanel>("", , myRuntimeScene);
	EditorLibrary::RegisterWithType<SkeletonEditorPanel>("Animation", Volt::AssetType::Skeleton);
	EditorLibrary::RegisterWithType<AnimationEditorPanel>("Animation", Volt::AssetType::Animation);
	EditorLibrary::RegisterWithType<ParticleEmitterEditor>("", Volt::AssetType::ParticlePreset);
	EditorLibrary::RegisterWithType<AnimationGraphPanel>("Animation", Volt::AssetType::AnimationGraph, myRuntimeScene);
	EditorLibrary::RegisterWithType<BehaviorPanel>("", Volt::AssetType::BehaviorGraph);
	EditorLibrary::RegisterWithType<BlendSpaceEditorPanel>("Animation", Volt::AssetType::BlendSpace);
	EditorLibrary::RegisterWithType<MeshPreviewPanel>("", Volt::AssetType::Mesh);
	EditorLibrary::RegisterWithType<ShaderEditorPanel>("Shader", Volt::AssetType::ShaderDefinition);
	EditorLibrary::RegisterWithType<PostProcessingStackPanel>("Shader", Volt::AssetType::PostProcessingStack);
	EditorLibrary::RegisterWithType<PostProcessingMaterialPanel>("Shader", Volt::AssetType::PostProcessingMaterial);

	EditorLibrary::Sort();

	myFileWatcher = CreateRef<FileWatcher>();
	CreateWatches();

	ImGuizmo::AllowAxisFlip(false);

	if (!userSettings.versionControlSettings.password.empty() && !userSettings.versionControlSettings.user.empty() && !userSettings.versionControlSettings.server.empty())
	{
		VersionControl::Connect(userSettings.versionControlSettings.server, userSettings.versionControlSettings.user, userSettings.versionControlSettings.password);
		if (VersionControl::IsConnected())
		{
			VersionControl::RefreshStreams();
			VersionControl::RefreshWorkspaces();

			if (!userSettings.versionControlSettings.workspace.empty())
			{
				VersionControl::SwitchWorkspace(userSettings.versionControlSettings.workspace);
			}

			if (!userSettings.versionControlSettings.stream.empty())
			{
				VersionControl::SwitchStream(userSettings.versionControlSettings.stream);
			}
		}
	}

	InitializeModals();

	if (!userSettings.sceneSettings.lastOpenScene.empty())
	{
		OpenScene(userSettings.sceneSettings.lastOpenScene);
	}

	if(!myRuntimeScene)
	{
		NewScene();
	}

	constexpr int64_t discordAppId = 1108502963447681106;

	Volt::DiscordSDK::Init(discordAppId, false);

	auto& act = Volt::DiscordSDK::GetRichPresence();

	act.SetApplicationId(discordAppId);
	act.GetAssets().SetLargeImage("icon_volt");
	act.GetAssets().SetLargeText("Volt");
	act.SetType(discord::ActivityType::Playing);

	Volt::DiscordSDK::UpdateRichPresence();

	myRuntimeScene->InitializeEngineScripts();
	m_isInitialized = true;
}

void Sandbox::CreateWatches()
{
	myFileWatcher->AddWatch(Volt::ProjectManager::GetEngineDirectory());
	myFileWatcher->AddWatch(Volt::ProjectManager::GetAssetsDirectory());
	myFileWatcher->AddWatch(Volt::ProjectManager::GetMonoBinariesDirectory());

	CreateModifiedWatch();
	CreateDeleteWatch();
	CreateAddWatch();
	CreateMovedWatch();
}

void Sandbox::SetEditorHasMouseControl()
{
	Volt::Input::ShowCursor(true);
	UI::SetInputEnabled(true);
	Volt::Input::DisableInput(true);

	myPlayHasMouseControl = false;
}

void Sandbox::SetPlayHasMouseControl()
{
	Volt::Input::ShowCursor(false);
	UI::SetInputEnabled(false);
	Volt::Input::DisableInput(false);

	myPlayHasMouseControl = true;
}

void Sandbox::SetupNewSceneData()
{
	EditorCommandStack::Clear();

	// Scene Renderers
	{
		const auto& lowMemory = UserSettingsManager::GetSettings().sceneSettings.lowMemoryUsage;

		Volt::SceneRendererSpecification spec{};
		Volt::SceneRendererSpecification gameSpec{};

		spec.debugName = "Editor Viewport";
		spec.scene = myRuntimeScene;

		gameSpec.debugName = "Game Viewport";
		gameSpec.scene = myRuntimeScene;

		if (mySceneRenderer)
		{
			spec.initialResolution = { mySceneRenderer->GetFinalImage()->GetWidth(), mySceneRenderer->GetFinalImage()->GetHeight() };
		}

		if (myGameSceneRenderer)
		{
			gameSpec.initialResolution = { myGameSceneRenderer->GetFinalImage()->GetWidth(), myGameSceneRenderer->GetFinalImage()->GetHeight() };
		}

		//{
		//	settings.enableIDRendering = true;
		//	settings.enableOutline = true;
		//	settings.enableDebugRenderer = true;
		//	settings.enableGrid = true;
		//	settings.enableUI = lowMemory;
		//	settings.enableVolumetricFog = true;

		//	//if (Volt::GraphicsContextVolt::GetPhysicalDevice()->GetCapabilities().supportsRayTracing)
		//	//{
		//	//	settings.enableRayTracing = true;
		//	//}

		//	gameSettings.enableIDRendering = false;
		//	gameSettings.enableOutline = false;
		//	gameSettings.enableDebugRenderer = false;
		//	gameSettings.enableGrid = false;
		//	gameSettings.enableUI = true;
		//	gameSettings.enablePostProcessing = true;
		//	gameSettings.enableVolumetricFog = true;
		//}

		mySceneRenderer = CreateRef<Volt::SceneRendererNew>(spec);

		if (!lowMemory)
		{
			myGameSceneRenderer = CreateRef<Volt::SceneRendererNew>(gameSpec);
		}
	}

	Volt::SceneManager::SetActiveScene(myRuntimeScene);
	Volt::OnSceneLoadedEvent loadEvent{ myRuntimeScene };
	Volt::Application::Get().OnEvent(loadEvent);
}

void Sandbox::InitializeModals()
{
}

void Sandbox::OnDetach()
{
	m_isInitialized = false;

	Volt::Log::ClearCallbacks();

	if (mySceneState == SceneState::Play)
	{
		OnSceneStop();
	}

	myRuntimeScene->ShutdownEngineScripts();

	{
		// #TODO_Ivar: Change so that scene copy creates a memory asset instead
		const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(myRuntimeScene->handle);
		if (myRuntimeScene && !metadata.filePath.empty())
		{
			UserSettingsManager::GetSettings().sceneSettings.lastOpenScene = metadata.filePath;
		}
	}
	UserSettingsManager::SaveUserSettings();
	EditorLibrary::Clear();
	EditorResources::Shutdown();

	NavMeshDebugDrawer::Shutdown();

	myFileWatcher = nullptr;
	myEditorCameraController = nullptr;
	mySceneRenderer = nullptr;
	myGameSceneRenderer = nullptr;

	myGameViewPanel = nullptr;

	myRuntimeScene = nullptr;
	myIntermediateScene = nullptr;

	myInstance = nullptr;

	NodeEditorHelpers::Shutdown();
	VersionControl::Shutdown();
}

void Sandbox::OnEvent(Volt::Event& e)
{
	if (!m_isInitialized)
	{
		return;
	}

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

	dispatcher.Dispatch<Volt::OnRenderScaleChangedEvent>([&](Volt::OnRenderScaleChangedEvent& e)
	{
		if (mySceneState == SceneState::Play)
		{
			const auto& lowMemory = UserSettingsManager::GetSettings().sceneSettings.lowMemoryUsage;

			auto sceneRenderer = mySceneRenderer;

			if (!lowMemory)
			{
				sceneRenderer = myGameSceneRenderer;
			}
			//sceneRenderer->GetSettings().renderScale = e.GetRenderScale();
			//sceneRenderer->ApplySettings();
		}

		return true;
	});

	dispatcher.Dispatch<Volt::OnRendererSettingsChangedEvent>([&](Volt::OnRendererSettingsChangedEvent& e)
	{
		if (mySceneState == SceneState::Play)
		{
			const auto& lowMemory = UserSettingsManager::GetSettings().sceneSettings.lowMemoryUsage;

			auto sceneRenderer = mySceneRenderer;

			if (!lowMemory)
			{
				sceneRenderer = myGameSceneRenderer;
			}

			//sceneRenderer->UpdateSettings(e.GetSettings());
			//sceneRenderer->ApplySettings();
		}
		return true;
	});

	if (!myPlayHasMouseControl)
	{
		myEditorCameraController->OnEvent(e);
	}

	{
		VT_PROFILE_SCOPE("Update Windows");
		for (auto& window : EditorLibrary::GetPanels())
		{
			VT_PROFILE_SCOPE(std::format("Update Window: {0}", window.editorWindow->GetTitle()).c_str());

			if (window.editorWindow->IsOpen())
			{
				window.editorWindow->OnEvent(e);
			}
		}
	}

	if (!myPlayHasMouseControl)
	{
		if ((e.GetCategoryFlags() & Volt::EventCategoryAnyInput) != 0)
		{
			return;
		}
	}

	if (e.handled)
	{
		return;
	}

	myRuntimeScene->OnEvent(e);
}

void Sandbox::OnScenePlay()
{
	if (!Volt::Application::Get().GetNetHandler().IsRunning())
		Volt::Application::Get().GetNetHandler().StartSinglePlayer();

	mySceneState = SceneState::Play;
	SelectionManager::DeselectAll();

	myIntermediateScene = myRuntimeScene;
	myIntermediateScene->ShutdownEngineScripts();

	myRuntimeScene = CreateRef<Volt::Scene>();
	myIntermediateScene->CopyTo(myRuntimeScene);

	SetupNewSceneData();
	SetPlayHasMouseControl();
	Volt::Input::DisableInput(false);
	myGameViewPanel->Focus();

	myRuntimeScene->OnRuntimeStart();

	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);

	Volt::ViewportResizeEvent e2 = { myViewportPosition.x,myViewportPosition.y, myViewportSize.x, myViewportSize.y };
	Volt::Application::Get().OnEvent(e2);

	if (myShouldMovePlayer)
	{
		myShouldMovePlayer = false;

		myRuntimeScene->ForEachWithComponents<const Volt::MonoScriptComponent>([&](const entt::entity id, const Volt::MonoScriptComponent& scriptComp) 
		{
			for (const auto& name : scriptComp.scriptNames)
			{
				if (name == "Project.GameManager")
				{
					Volt::Entity entity{ id, myRuntimeScene.get() };
					entity.SetPosition(myMovePlayerToPosition);

					break;
				}
			}
		});
	}

	//if (!Volt::Application::Get().GetNetHandler().IsRunning())
		//Volt::Application::Get().GetNetHandler().StartSinglePlayer();
}

void Sandbox::OnSceneStop()
{
	if (Volt::Application::Get().GetNetHandler().IsRunning())
		Volt::Application::Get().GetNetHandler().Stop();
	SelectionManager::DeselectAll();

	Volt::OnSceneStopEvent stopEvent{};
	Volt::Application::Get().OnEvent(stopEvent);

	Volt::ViewportResizeEvent e2 = { myViewportPosition.x,myViewportPosition.y, myViewportSize.x, myViewportSize.y };
	Volt::Application::Get().OnEvent(e2);

	myRuntimeScene->OnRuntimeEnd();

	SetEditorHasMouseControl();
	myViewportPanel->Focus();

	myRuntimeScene = myIntermediateScene;
	myRuntimeScene->InitializeEngineScripts();

	mySceneState = SceneState::Edit;

	SetupNewSceneData();

	myIntermediateScene = nullptr;

	//Amp::MusicManager::RemoveEvent();
}

void Sandbox::OnSimulationStart()
{
	mySceneState = SceneState::Simulating;
	SelectionManager::DeselectAll();

	myIntermediateScene = myRuntimeScene;

	myRuntimeScene = CreateRef<Volt::Scene>();
	myIntermediateScene->CopyTo(myRuntimeScene);

	SetupNewSceneData();

	myRuntimeScene->OnSimulationStart();

	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);
}

void Sandbox::OnSimulationStop()
{
	SelectionManager::DeselectAll();

	Volt::OnSceneStopEvent stopEvent{};
	Volt::Application::Get().OnEvent(stopEvent);

	myRuntimeScene->OnSimulationEnd();
	myRuntimeScene = myIntermediateScene;
	mySceneState = SceneState::Edit;

	SetupNewSceneData();
}

void Sandbox::NewScene()
{
	SelectionManager::DeselectAll();
	if (myRuntimeScene)
	{
		Volt::AssetManager::Get().Unload(myRuntimeScene->handle);
	}

	if (myRuntimeScene)
	{
		myRuntimeScene->ShutdownEngineScripts();
	}

	myRuntimeScene = Volt::Scene::CreateDefaultScene("New Scene", true);
	myRuntimeScene->InitializeEngineScripts();
	SetupNewSceneData();
}

void Sandbox::OpenScene()
{
	const std::filesystem::path loadPath = FileSystem::OpenFileDialogue({ { "Scene(*.vtscene)", "vtscene" } });
	OpenScene(Volt::AssetManager::GetRelativePath(loadPath));
}

void Sandbox::OpenScene(const std::filesystem::path& path)
{
	if (!path.empty() && FileSystem::Exists(Volt::ProjectManager::GetDirectory() / path))
	{
		SelectionManager::DeselectAll();

		const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(myRuntimeScene->handle);

		if (myRuntimeScene)
		{
			myRuntimeScene->ShutdownEngineScripts();
		}
			
		const auto newScene = Volt::AssetManager::GetAsset<Volt::Scene>(path);
		if (!newScene)
		{
			return;
		}

		const auto& newSceneMeta = Volt::AssetManager::GetMetadataFromHandle(newScene->handle);
		if (newSceneMeta.type != Volt::AssetType::Scene)
		{
			return;
		}

		if (metadata.filePath == path)
		{
			Volt::AssetManager::Get().ReloadAsset(myRuntimeScene->handle);
		}
		else if (myRuntimeScene && !metadata.filePath.empty())
		{
			Volt::AssetManager::Get().Unload(myRuntimeScene->handle);
		}

		myRuntimeScene = newScene;
		myRuntimeScene->InitializeEngineScripts();

		SetupNewSceneData();
	}
}

bool Sandbox::LoadScene(Volt::OnSceneTransitionEvent& e)
{
	myStoredScene = Volt::AssetManager::GetAsset<Volt::Scene>(e.GetHandle());
	myShouldLoadNewScene = true;

	return true;
}

bool Sandbox::CheckForUpdateNavMesh(Volt::Entity entity)
{
	for (auto child : entity.GetChildren())
	{
		if (CheckForUpdateNavMesh(child))
		{
			return true;
		}
	}

	return (entity.HasComponent<Volt::NavMeshComponent>() || entity.HasComponent<Volt::NavLinkComponent>()) && UserSettingsManager::GetSettings().navmeshBuildSettings.useAutoBaking;
}

void Sandbox::BakeNavMesh()
{
	myNavigationPanel->Bake();
}

void Sandbox::SaveScene()
{
	if (myRuntimeScene)
	{
		if (Volt::AssetManager::ExistsInRegistry(myRuntimeScene->handle))
		{
			if (FileSystem::IsWriteable(Volt::AssetManager::GetFilesystemPath(myRuntimeScene->handle)))
			{
				Volt::AssetManager::Get().SaveAsset(myRuntimeScene);
				UI::Notify(NotificationType::Success, "Scene saved!", std::format("Scene {0} was saved successfully!", myRuntimeScene->assetName));
			}
			else
			{
				UI::Notify(NotificationType::Error, "Unable to save scene!", std::format("Scene {0} was is not writeable!", myRuntimeScene->assetName));
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

	myRuntimeScene = CreateRef<Volt::Scene>();
	myStoredScene->CopyTo(myRuntimeScene);

	SetupNewSceneData();

	myRuntimeScene->OnRuntimeStart();

	Volt::ViewportResizeEvent windowResizeEvent{ myViewportPosition.x, myViewportPosition.y, myViewportSize.x, myViewportSize.y };
	Volt::Application::Get().OnEvent(windowResizeEvent);

	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);

	myShouldLoadNewScene = false;
	myStoredScene = nullptr;
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

	UI::Notify(NotificationType::Success, "Successfully installed Maya tools!", "The Maya tools were successfully installed!");
}

bool Sandbox::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	VT_PROFILE_FUNCTION();

	EditorCommandStack::GetInstance().Update(100);

	auto mousePos = Volt::Input::GetMousePosition();
	Volt::Input::SetViewportMousePosition(myGameViewPanel->GetViewportLocalPosition({ mousePos.first, mousePos.second }));

	switch (mySceneState)
	{
		case SceneState::Edit:
			myRuntimeScene->UpdateEditor(e.GetTimestep());
			break;

		case SceneState::Play:
			myRuntimeScene->Update(e.GetTimestep());
			break;

		case SceneState::Pause:
			break;

		case SceneState::Simulating:
			myRuntimeScene->UpdateSimulation(e.GetTimestep());
			break;
	}

	SelectionManager::Update(myRuntimeScene);

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

	VT_PROFILE_SCOPE("File watcher");

	std::scoped_lock lock{ myFileWatcherMutex };
	for (const auto& f : myFileChangeQueue)
	{
		f();
	}

	if (!myFileChangeQueue.empty())
	{
		EditorLibrary::Get<AssetBrowserPanel>()->Reload();
	}

	myFileChangeQueue.clear();

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

	ModalSystem::Update();

	if (myShouldOpenSaveSceneAs)
	{
		UI::OpenModal("Save As");
		myShouldOpenSaveSceneAs = false;
	}

	SaveSceneAsModal();
	BuildGameModal();

	for (auto& window : EditorLibrary::GetPanels())
	{
		if (window.editorWindow->Begin())
		{
			window.editorWindow->UpdateMainContent();
			window.editorWindow->End();

			window.editorWindow->UpdateContent();
		}
	}

	if (myBuildStarted)
	{
		const float buildProgess = GameBuilder::GetBuildProgress();
		RenderProgressBar(buildProgess);
	}

	if (ImGui::Begin("UI Test"))
	{
		ImGui::Image(UI::GetTextureID(mySceneRenderer->GetUIImage()), { 512, 512 });

		ImGui::End();
	}

	return false;
}

void Sandbox::RenderGameView()
{
	/*if (!myGameViewPanel->IsOpen() || !myGameSceneRenderer)
	{
		return;
	}

	switch (mySceneState)
	{
		case SceneState::Edit:
		case SceneState::Play:
		case SceneState::Pause:
		case SceneState::Simulating:
		{
			Volt::Entity cameraEntity{};
			int32_t highestPrio = -1;

			myRuntimeScene->ForEachWithComponents<const Volt::CameraComponent>([&](const entt::entity id, const Volt::CameraComponent& camComp)
			{
				if ((int32_t)camComp.priority > highestPrio)
				{
					highestPrio = (int32_t)camComp.priority;
					cameraEntity = { id, myRuntimeScene.get() };
				}
			});

			if (!cameraEntity)
			{
				break;
			}

			const auto& camComp = cameraEntity.GetComponent<Volt::CameraComponent>();
			const auto finalImage = myGameSceneRenderer->GetFinalImage();

			Ref<Volt::Camera> camera = CreateRef<Volt::Camera>(camComp.fieldOfView, (float)finalImage->GetWidth() / (float)finalImage->GetHeight(), camComp.nearPlane, camComp.farPlane);
			camera->SetPosition(cameraEntity.GetPosition());
			camera->SetRotation(glm::eulerAngles(cameraEntity.GetRotation()));

			myGameSceneRenderer->OnRenderEditor(camera);
			break;
		}
	}*/
}

bool Sandbox::OnRenderEvent(Volt::AppRenderEvent& e)
{
	VT_PROFILE_FUNCTION();

	//mySceneRenderer->ClearOutlineCommands();

	RenderSelection(myEditorCameraController->GetCamera());
	RenderGizmos(myRuntimeScene, myEditorCameraController->GetCamera());

	if (UserSettingsManager::GetSettings().sceneSettings.lowMemoryUsage)
	{
		switch (mySceneState)
		{
			case SceneState::Play:
				//mySceneRenderer->OnRenderRuntime();
				break;
			case SceneState::Edit:
			case SceneState::Pause:
			case SceneState::Simulating:
				mySceneRenderer->OnRenderEditor(myEditorCameraController->GetCamera());
				break;
		}
	}
	else
	{
		switch (mySceneState)
		{
			case SceneState::Edit:
			case SceneState::Play:
			case SceneState::Pause:
			case SceneState::Simulating:
				mySceneRenderer->OnRenderEditor(myEditorCameraController->GetCamera());
				break;
		}
	}

	if (myShouldLoadNewScene)
	{
		TransitionToNewScene();
	}

	RenderGameView();

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

		case VT_KEY_F:
		{
			if (SelectionManager::IsAnySelected())
			{
				glm::vec3 avgPos = 0.f;

				for (const auto& id : SelectionManager::GetSelectedEntities())
				{
					Volt::Entity ent = myRuntimeScene->GetEntityFromUUID(id);
					avgPos += ent.GetPosition();
				}

				avgPos /= (float)SelectionManager::GetSelectedCount();

				myEditorCameraController->Focus(avgPos);
			}

			break;
		}

		case VT_KEY_SPACE:
		{
			if (ctrlPressed)
			{
				for (const auto& window : EditorLibrary::GetPanels())
				{
					if (window.editorWindow->GetTitle() == "Asset Browser##Main")
					{
						if (!window.editorWindow->IsOpen())
						{
							window.editorWindow->Open();
						}
						else
						{
							window.editorWindow->Close();
						}
					}
				}
			}

			break;
		}

		case VT_KEY_ESCAPE:
		{
			if (mySceneState == SceneState::Play)
			{
				SetEditorHasMouseControl();
			}

			break;
		}

		case VT_KEY_G:
		{
			if (ctrlPressed && mySceneState != SceneState::Play)
			{
				myShouldMovePlayer = true;
				myMovePlayerToPosition = myEditorCameraController->GetCamera()->GetPosition();
				OnScenePlay();
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
	myRuntimeScene->SetRenderSize(e.GetWidth(), e.GetHeight());
	myViewportSize = { e.GetWidth(), e.GetHeight() };
	myViewportPosition = { e.GetX(), e.GetY() };

	return false;
}

VT_OPTIMIZE_OFF

bool Sandbox::OnSceneLoadedEvent(Volt::OnSceneLoadedEvent& e)
{
	mySceneRenderer->Resize(myViewportSize.x, myViewportSize.y);

	if (myGameSceneRenderer)
	{
		myGameSceneRenderer->Resize(myViewportSize.x, myViewportSize.y);
	}

	e.GetScene()->SetRenderSize(myViewportSize.x, myViewportSize.y);

	Volt::ViewportResizeEvent e2 = { myViewportPosition.x,myViewportPosition.y, myViewportSize.x, myViewportSize.y };
	Volt::Application::Get().OnEvent(e2);

	// Mono Scripts
	static bool notRuntimeScene = true;
	if (mySceneState == SceneState::Edit && notRuntimeScene)
	{
		Volt::MonoScriptEngine::OnSceneLoaded();
	}
	else if (!notRuntimeScene)
	{
		notRuntimeScene = true;
	}
	else
	{
		notRuntimeScene = false;
	}

	auto& act = Volt::DiscordSDK::GetRichPresence();

	auto scene = e.GetScene();

	act.SetDetails("Working on:");
	act.SetState(scene->GetName().c_str());
	act.GetParty().GetSize().SetCurrentSize(myRuntimeScene->GetActiveLayer() + 1);
	act.GetParty().GetSize().SetMaxSize(static_cast<int32_t>(myRuntimeScene->GetLayers().size()));
	act.GetTimestamps().SetStart(std::time(nullptr));

	Volt::DiscordSDK::UpdateRichPresence();

	return false;
}
