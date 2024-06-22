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
#include "Sandbox/Window/NavigationPanel.h"
#include "Sandbox/Window/Net/NetPanel.h"
#include "Sandbox/Window/Net/NetContractPanel.h"
#include "Sandbox/Window/BehaviourGraph/BehaviorPanel.h"
#include "Sandbox/Window/SceneSettingsPanel.h"
#include "Sandbox/Window/WorldEnginePanel.h"
#include "Sandbox/Window/MosaicEditor/MosaicEditorPanel.h"
#include "Sandbox/Window/SkeletonEditorPanel.h"
#include "Sandbox/Window/AnimationEditorPanel.h"
#include "Sandbox/Window/GameUIEditorPanel.h"
#include "Sandbox/Window/MotionWeaveDatabasePanel.h"
#include "Sandbox/VertexPainting/VertexPainterPanel.h"

#include "Sandbox/Modals/MeshImportModal.h"

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

#include <Volt/Rendering/Camera/Camera.h>

#include <Volt/Rendering/SceneRenderer.h>

#include <Volt/Utility/FileSystem.h>
#include <Volt/Utility/UIUtility.h>

#include <Volt/Project/ProjectManager.h>
#include <Volt/Scripting/Mono/MonoScriptEngine.h>

#include <Volt/Events/SettingsEvent.h>

#include <Volt/Discord/DiscordSDK.h>

#include <NavigationEditor/Tools/NavMeshDebugDrawer.h>

#include <VoltRHI/Images/Image2D.h>

#include <imgui.h>

Sandbox::Sandbox()
{
	VT_ASSERT(!s_instance, "Sandbox already exists!");
	s_instance = this;
}

Sandbox::~Sandbox()
{
	s_instance = nullptr;
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

	m_editorCameraController = CreateRef<EditorCameraController>(60.f, 1.f, 100000.f);

	UserSettingsManager::LoadUserSettings();
	const auto& userSettings = UserSettingsManager::GetSettings();

	NewScene();

	// Shelved Panels (So panel tab doesn't get cluttered up).
#ifdef VT_DEBUG
	EditorLibrary::RegisterWithType<PrefabEditorPanel>("", Volt::AssetType::Prefab);
	EditorLibrary::Register<SplinePanel>("", m_runtimeScene);
	EditorLibrary::Register<Sequencer>("", m_runtimeScene);
	EditorLibrary::Register<TaigaPanel>("Advanced");
	EditorLibrary::Register<ThemesPanel>("Advanced");
	EditorLibrary::Register<CurveGraphPanel>("Advanced");
#endif

	EditorLibrary::Register<PropertiesPanel>("Level Editor", m_runtimeScene, m_sceneRenderer, m_sceneState, "");
	EditorLibrary::Register<LogPanel>("Advanced");
	EditorLibrary::Register<SceneViewPanel>("Level Editor", m_runtimeScene, "");
	EditorLibrary::Register<AssetRegistryPanel>("Advanced");
	EditorLibrary::Register<VisionPanel>("", m_runtimeScene, m_editorCameraController.get());
	EditorLibrary::Register<EngineStatisticsPanel>("Advanced", m_runtimeScene, m_sceneRenderer, m_gameSceneRenderer);
	EditorLibrary::Register<EditorSettingsPanel>("Advanced", UserSettingsManager::GetSettings());
	EditorLibrary::Register<PhysicsPanel>("Physics");
	EditorLibrary::Register<RendererSettingsPanel>("Advanced", m_sceneRenderer);
	EditorLibrary::Register<GraphKeyPanel>("", m_runtimeScene);
	EditorLibrary::Register<VertexPainterPanel>("", m_runtimeScene, m_editorCameraController);

	EditorLibrary::Register<NetPanel>("Advanced");
	EditorLibrary::Register<NetContractPanel>("Advanced");
	EditorLibrary::Register<SceneSettingsPanel>("", m_runtimeScene);
	EditorLibrary::Register<WorldEnginePanel>("", m_runtimeScene);
	EditorLibrary::Register<GameUIEditorPanel>("UI");

	m_navigationPanel = EditorLibrary::Register<NavigationPanel>("Advanced", m_runtimeScene);
	m_viewportPanel = EditorLibrary::Register<ViewportPanel>("Level Editor", m_sceneRenderer, m_runtimeScene, m_editorCameraController.get(), m_sceneState);
	m_gameViewPanel = EditorLibrary::Register<GameViewPanel>("Level Editor", m_gameSceneRenderer, m_runtimeScene, m_sceneState);
	m_assetBrowserPanel = EditorLibrary::Register<AssetBrowserPanel>("", m_runtimeScene, "##Main");

	EditorLibrary::RegisterWithType<MosaicEditorPanel>("", Volt::AssetType::Material);
	EditorLibrary::RegisterWithType<CharacterEditorPanel>("Animation", Volt::AssetType::AnimatedCharacter);
	//EditorLibrary::RegisterWithType<MaterialEditorPanel>("", , myRuntimeScene);
	EditorLibrary::RegisterWithType<SkeletonEditorPanel>("Animation", Volt::AssetType::Skeleton);
	EditorLibrary::RegisterWithType<AnimationEditorPanel>("Animation", Volt::AssetType::Animation);
	EditorLibrary::RegisterWithType<ParticleEmitterEditor>("", Volt::AssetType::ParticlePreset);
	EditorLibrary::RegisterWithType<AnimationGraphPanel>("Animation", Volt::AssetType::AnimationGraph, m_runtimeScene);
	EditorLibrary::RegisterWithType<BehaviorPanel>("", Volt::AssetType::BehaviorGraph);
	EditorLibrary::RegisterWithType<BlendSpaceEditorPanel>("Animation", Volt::AssetType::BlendSpace);
	EditorLibrary::RegisterWithType<MeshPreviewPanel>("", Volt::AssetType::Mesh);
	EditorLibrary::RegisterWithType<ShaderEditorPanel>("Shader", Volt::AssetType::ShaderDefinition);
	EditorLibrary::RegisterWithType<MotionWeaveDatabasePanel>("Animation", Volt::AssetType::MotionWeave);

	EditorLibrary::Sort();

	UserSettingsManager::SetupPanels();

	m_fileWatcher = CreateRef<FileWatcher>();
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

	if (!m_runtimeScene)
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

	m_isInitialized = true;
}

void Sandbox::CreateWatches()
{
	m_fileWatcher->AddWatch(Volt::ProjectManager::GetEngineDirectory());
	m_fileWatcher->AddWatch(Volt::ProjectManager::GetAssetsDirectory());
	m_fileWatcher->AddWatch(Volt::ProjectManager::GetMonoBinariesDirectory());

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

	m_playHasMouseControl = false;
}

void Sandbox::SetPlayHasMouseControl()
{
	Volt::Input::ShowCursor(false);
	UI::SetInputEnabled(false);
	Volt::Input::DisableInput(false);

	m_playHasMouseControl = true;
}

void Sandbox::SetupNewSceneData()
{
	EditorCommandStack::Clear();

	// Scene Renderers
	{
		Volt::SceneRendererSpecification spec{};
		Volt::SceneRendererSpecification gameSpec{};

		spec.debugName = "Editor Viewport";
		spec.scene = m_runtimeScene;

		gameSpec.debugName = "Game Viewport";
		gameSpec.scene = m_runtimeScene;

		if (m_sceneRenderer)
		{
			spec.initialResolution = { m_sceneRenderer->GetFinalImage()->GetWidth(), m_sceneRenderer->GetFinalImage()->GetHeight() };
		}

		if (m_gameSceneRenderer)
		{
			gameSpec.initialResolution = { m_gameSceneRenderer->GetFinalImage()->GetWidth(), m_gameSceneRenderer->GetFinalImage()->GetHeight() };
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

		m_sceneRenderer = CreateRef<Volt::SceneRenderer>(spec);
		m_gameSceneRenderer = CreateRef<Volt::SceneRenderer>(gameSpec);
	}

	Volt::SceneManager::SetActiveScene(m_runtimeScene);
	Volt::OnSceneLoadedEvent loadEvent{ m_runtimeScene };
	Volt::Application::Get().OnEvent(loadEvent);
}

void Sandbox::InitializeModals()
{
	auto& meshModal = ModalSystem::AddModal<MeshImportModal>("Import Mesh##sandbox");
	m_meshImportModal = meshModal.GetID();
}

void Sandbox::OnDetach()
{
	m_isInitialized = false;

	Volt::Log::ClearCallbacks();

	if (m_sceneState == SceneState::Play)
	{
		OnSceneStop();
	}

	m_runtimeScene->ShutdownEngineScripts();

	{
		// #TODO_Ivar: Change so that scene copy creates a memory asset instead
		const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(m_runtimeScene->handle);
		if (m_runtimeScene && !metadata.filePath.empty())
		{
			UserSettingsManager::GetSettings().sceneSettings.lastOpenScene = metadata.filePath;
		}
	}
	UserSettingsManager::SaveUserSettings();
	EditorLibrary::Clear();
	EditorResources::Shutdown();

	NavMeshDebugDrawer::Shutdown();

	m_fileWatcher = nullptr;
	m_editorCameraController = nullptr;
	m_sceneRenderer = nullptr;
	m_gameSceneRenderer = nullptr;

	m_gameViewPanel = nullptr;

	m_runtimeScene = nullptr;
	m_intermediateScene = nullptr;

	s_instance = nullptr;

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
		e.SetHit(m_titlebarHovered);
		return true;
	});

	dispatcher.Dispatch<Volt::OnRenderScaleChangedEvent>([&](Volt::OnRenderScaleChangedEvent& e)
	{
		if (m_sceneState == SceneState::Play)
		{
			auto sceneRenderer = m_sceneRenderer;

			//if (!lowMemory)
			//{
			//	sceneRenderer = m_gameSceneRenderer;
			//}
			//sceneRenderer->GetSettings().renderScale = e.GetRenderScale();
			//sceneRenderer->ApplySettings();
		}

		return true;
	});

	dispatcher.Dispatch<Volt::OnRendererSettingsChangedEvent>([&](Volt::OnRendererSettingsChangedEvent& e)
	{
		if (m_sceneState == SceneState::Play)
		{
			auto sceneRenderer = m_sceneRenderer;

			//if (!lowMemory)
			//{
			//	sceneRenderer = m_gameSceneRenderer;
			//}

			//sceneRenderer->UpdateSettings(e.GetSettings());
			//sceneRenderer->ApplySettings();
		}
		return true;
	});

	if (!m_playHasMouseControl)
	{
		m_editorCameraController->OnEvent(e);
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

	if (!m_playHasMouseControl)
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

	m_runtimeScene->OnEvent(e);
}

void Sandbox::OnScenePlay()
{
	if (!Volt::Application::Get().GetNetHandler().IsRunning())
		Volt::Application::Get().GetNetHandler().StartSinglePlayer();

	m_sceneState = SceneState::Play;
	SelectionManager::DeselectAll();

	m_intermediateScene = m_runtimeScene;
	m_intermediateScene->ShutdownEngineScripts();

	m_runtimeScene = CreateRef<Volt::Scene>();
	m_intermediateScene->CopyTo(m_runtimeScene);

	SetupNewSceneData();
	SetPlayHasMouseControl();
	Volt::Input::DisableInput(false);
	m_gameViewPanel->Focus();

	m_runtimeScene->OnRuntimeStart();

	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);

	Volt::ViewportResizeEvent e2 = { m_viewportPosition.x,m_viewportPosition.y, m_viewportSize.x, m_viewportSize.y };
	Volt::Application::Get().OnEvent(e2);

	if (m_shouldMovePlayer)
	{
		m_shouldMovePlayer = false;

		m_runtimeScene->ForEachWithComponents<const Volt::MonoScriptComponent>([&](const entt::entity id, const Volt::MonoScriptComponent& scriptComp)
		{
			for (const auto& name : scriptComp.scriptNames)
			{
				if (name == "Project.GameManager")
				{
					Volt::Entity entity{ id, m_runtimeScene.get() };
					entity.SetPosition(m_movePlayerToPosition);

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

	Volt::ViewportResizeEvent e2 = { m_viewportPosition.x,m_viewportPosition.y, m_viewportSize.x, m_viewportSize.y };
	Volt::Application::Get().OnEvent(e2);

	m_runtimeScene->OnRuntimeEnd();

	SetEditorHasMouseControl();
	m_viewportPanel->Focus();

	m_runtimeScene = m_intermediateScene;
	m_runtimeScene->InitializeEngineScripts();

	m_sceneState = SceneState::Edit;

	SetupNewSceneData();

	m_intermediateScene = nullptr;

	//Amp::MusicManager::RemoveEvent();
}

void Sandbox::OnSimulationStart()
{
	m_sceneState = SceneState::Simulating;
	SelectionManager::DeselectAll();

	m_intermediateScene = m_runtimeScene;

	m_runtimeScene = CreateRef<Volt::Scene>();
	m_intermediateScene->CopyTo(m_runtimeScene);

	SetupNewSceneData();

	m_runtimeScene->OnSimulationStart();

	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);
}

void Sandbox::OnSimulationStop()
{
	SelectionManager::DeselectAll();

	Volt::OnSceneStopEvent stopEvent{};
	Volt::Application::Get().OnEvent(stopEvent);

	m_runtimeScene->OnSimulationEnd();
	m_runtimeScene = m_intermediateScene;
	m_sceneState = SceneState::Edit;

	SetupNewSceneData();
}

void Sandbox::NewScene()
{
	SelectionManager::DeselectAll();
	if (m_runtimeScene)
	{
		Volt::AssetManager::Get().Unload(m_runtimeScene->handle);
	}

	if (m_runtimeScene)
	{
		m_runtimeScene->ShutdownEngineScripts();
	}

	m_runtimeScene = Volt::Scene::CreateDefaultScene("New Scene", true);
	m_runtimeScene->InitializeEngineScripts();
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

		const auto& metadata = Volt::AssetManager::GetMetadataFromHandle(m_runtimeScene->handle);

		if (m_runtimeScene)
		{
			m_runtimeScene->ShutdownEngineScripts();
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
			Volt::AssetManager::Get().ReloadAsset(m_runtimeScene->handle);
		}
		else if (m_runtimeScene && !metadata.filePath.empty())
		{
			Volt::AssetManager::Get().Unload(m_runtimeScene->handle);
		}

		m_runtimeScene = newScene;
		m_runtimeScene->InitializeEngineScripts();

		SetupNewSceneData();
	}
}

bool Sandbox::LoadScene(Volt::OnSceneTransitionEvent& e)
{
	m_storedScene = Volt::AssetManager::GetAsset<Volt::Scene>(e.GetHandle());
	m_shouldLoadNewScene = true;

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
	m_navigationPanel->Bake();
}

void Sandbox::SaveScene()
{
	if (m_runtimeScene)
	{
		if (Volt::AssetManager::ExistsInRegistry(m_runtimeScene->handle))
		{
			if (FileSystem::IsWriteable(Volt::AssetManager::GetFilesystemPath(m_runtimeScene->handle)))
			{
				Volt::AssetManager::Get().SaveAsset(m_runtimeScene);
				UI::Notify(NotificationType::Success, "Scene saved!", std::format("Scene {0} was saved successfully!", m_runtimeScene->assetName));
			}
			else
			{
				UI::Notify(NotificationType::Error, "Unable to save scene!", std::format("Scene {0} was is not writeable!", m_runtimeScene->assetName));
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
	m_runtimeScene->OnRuntimeEnd();

	SelectionManager::DeselectAll();
	Volt::AssetManager::Get().Unload(m_runtimeScene->handle);

	m_runtimeScene = CreateRef<Volt::Scene>();
	m_storedScene->CopyTo(m_runtimeScene);

	SetupNewSceneData();

	m_runtimeScene->OnRuntimeStart();

	Volt::ViewportResizeEvent windowResizeEvent{ m_viewportPosition.x, m_viewportPosition.y, m_viewportSize.x, m_viewportSize.y };
	Volt::Application::Get().OnEvent(windowResizeEvent);

	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);

	m_shouldLoadNewScene = false;
	m_storedScene = nullptr;
}

void Sandbox::SaveSceneAs()
{
	m_shouldOpenSaveSceneAs = true;
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
	Volt::Input::SetViewportMousePosition(m_gameViewPanel->GetViewportLocalPosition({ mousePos.first, mousePos.second }));

	switch (m_sceneState)
	{
		case SceneState::Edit:
			m_runtimeScene->UpdateEditor(e.GetTimestep());
			break;

		case SceneState::Play:
			m_runtimeScene->Update(e.GetTimestep());
			break;

		case SceneState::Pause:
			break;

		case SceneState::Simulating:
			m_runtimeScene->UpdateSimulation(e.GetTimestep());
			break;
	}

	SelectionManager::Update(m_runtimeScene);

	if (m_shouldResetLayout)
	{
		ImGui::LoadIniSettingsFromDisk("Editor/imgui.ini");
		m_shouldResetLayout = false;
	}

	if (m_buildStarted)
	{
		if (!GameBuilder::IsBuilding())
		{
			UI::Notify(NotificationType::Success, "Build Finished!", std::format("Build finished successfully in {0} seconds!", GameBuilder::GetCurrentBuildTime()));
			m_buildStarted = false;
		}
	}

	VT_PROFILE_SCOPE("File watcher");

	std::scoped_lock lock{ m_fileWatcherMutex };
	for (const auto& f : m_fileChangeQueue)
	{
		f();
	}

	if (!m_fileChangeQueue.empty())
	{
		EditorLibrary::Get<AssetBrowserPanel>()->Reload();
	}

	m_fileChangeQueue.clear();

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

	if (m_shouldOpenSaveSceneAs)
	{
		UI::OpenModal("Save As");
		m_shouldOpenSaveSceneAs = false;
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

	if (m_buildStarted)
	{
		const float buildProgess = GameBuilder::GetBuildProgress();
		RenderProgressBar(buildProgess);
	}

	return false;
}

void Sandbox::RenderGameView()
{
	if (!m_gameViewPanel->IsOpen() || !m_gameSceneRenderer)
	{
		return;
	}

	switch (m_sceneState)
	{
		case SceneState::Edit:
		case SceneState::Play:
		case SceneState::Pause:
		case SceneState::Simulating:
		{
			Volt::Entity cameraEntity{};
			int32_t highestPrio = -1;

			m_runtimeScene->ForEachWithComponents<const Volt::CameraComponent>([&](const entt::entity id, const Volt::CameraComponent& camComp)
			{
				if ((int32_t)camComp.priority > highestPrio)
				{
					highestPrio = (int32_t)camComp.priority;
					cameraEntity = { id, m_runtimeScene.get() };
				}
			});

			if (!cameraEntity)
			{
				break;
			}

			const auto& camComp = cameraEntity.GetComponent<Volt::CameraComponent>();
			const auto finalImage = m_gameSceneRenderer->GetFinalImage();

			Ref<Volt::Camera> camera = CreateRef<Volt::Camera>(camComp.fieldOfView, (float)finalImage->GetWidth() / (float)finalImage->GetHeight(), camComp.nearPlane, camComp.farPlane);
			camera->SetPosition(cameraEntity.GetPosition());
			camera->SetRotation(glm::eulerAngles(cameraEntity.GetRotation()));

			m_gameSceneRenderer->OnRenderEditor(camera);
			break;
		}
	}
}

bool Sandbox::OnRenderEvent(Volt::AppRenderEvent& e)
{
	VT_PROFILE_FUNCTION();

	//mySceneRenderer->ClearOutlineCommands();

	RenderSelection(m_editorCameraController->GetCamera());
	RenderGizmos(m_runtimeScene, m_editorCameraController->GetCamera());

	switch (m_sceneState)
	{
		case SceneState::Edit:
		case SceneState::Play:
		case SceneState::Pause:
		case SceneState::Simulating:
			m_sceneRenderer->OnRenderEditor(m_editorCameraController->GetCamera());
			break;
	}

	if (m_shouldLoadNewScene)
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
				if (m_runtimeScene)
				{
					m_openShouldSaveScenePopup = true;
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
					Volt::Entity ent = m_runtimeScene->GetEntityFromUUID(id);
					avgPos += ent.GetPosition();
				}

				avgPos /= (float)SelectionManager::GetSelectedCount();

				m_editorCameraController->Focus(avgPos);
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
			if (m_sceneState == SceneState::Play)
			{
				SetEditorHasMouseControl();
			}

			break;
		}

		case VT_KEY_G:
		{
			if (ctrlPressed && m_sceneState != SceneState::Play)
			{
				m_shouldMovePlayer = true;
				m_movePlayerToPosition = m_editorCameraController->GetCamera()->GetPosition();
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
	m_runtimeScene->SetRenderSize(e.GetWidth(), e.GetHeight());
	m_viewportSize = { e.GetWidth(), e.GetHeight() };
	m_viewportPosition = { e.GetX(), e.GetY() };

	return false;
}

VT_OPTIMIZE_OFF

bool Sandbox::OnSceneLoadedEvent(Volt::OnSceneLoadedEvent& e)
{
	m_sceneRenderer->Resize(m_viewportSize.x, m_viewportSize.y);

	if (m_gameSceneRenderer)
	{
		m_gameSceneRenderer->Resize(m_viewportSize.x, m_viewportSize.y);
	}

	e.GetScene()->SetRenderSize(m_viewportSize.x, m_viewportSize.y);

	Volt::ViewportResizeEvent e2 = { m_viewportPosition.x,m_viewportPosition.y, m_viewportSize.x, m_viewportSize.y };
	Volt::Application::Get().OnEvent(e2);

	// Mono Scripts
	static bool notRuntimeScene = true;
	if (m_sceneState == SceneState::Edit && notRuntimeScene)
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
	act.GetParty().GetSize().SetCurrentSize(m_runtimeScene->GetActiveLayer() + 1);
	act.GetParty().GetSize().SetMaxSize(static_cast<int32_t>(m_runtimeScene->GetLayers().size()));
	act.GetTimestamps().SetStart(std::time(nullptr));

	Volt::DiscordSDK::UpdateRichPresence();

	return false;
}
