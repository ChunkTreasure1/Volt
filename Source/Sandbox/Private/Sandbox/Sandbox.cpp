#include "sbpch.h"
#include "Sandbox.h"

#include "Sandbox/Camera/EditorCameraController.h"

#include "Sandbox/UISystems/ModalSystem.h"

#include "Sandbox/NodeGraph/IONodeGraphEditorHelpers.h"

#include "Sandbox/Window/PropertiesPanel.h"
#include "Sandbox/Window/ViewportPanel.h"
#include "Sandbox/Window/GameViewPanel.h"
#include "Sandbox/Window/SceneViewPanel.h"
#include "Sandbox/Window/AssetBrowser/AssetBrowserPanel.h"
#include "Sandbox/Window/LogPanel.h"
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
#include "Sandbox/Window/PrefabEditorPanel.h"
#include "Sandbox/Window/Sequencer.h"
#include "Sandbox/Window/BlendSpaceEditorPanel.h"
#include "Sandbox/Window/CurveGraphPanel.h"
#include "Sandbox/Window/Timeline.h"
#include "Sandbox/Window/ShaderEditorPanel.h"
#include "Sandbox/Window/NavigationPanel.h"
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

#include <AssetSystem/AssetManager.h>

#include <Volt/Components/CoreComponents.h>
#include <Volt/Components/LightComponents.h>

#include <Volt/Scene/Entity.h>
#include <Volt/Scene/Scene.h>
#include <Volt/Scene/SceneManager.h>

#include <InputModule/Input.h>
#include <InputModule/InputCodes.h>

#include <Volt/Rendering/Camera/Camera.h>

#include <Volt/Rendering/SceneRenderer.h>

#include <Volt/Utility/FileSystem.h>
#include <Volt/Utility/UIUtility.h>

#include <Volt/Project/ProjectManager.h>

#include <Volt/Events/ApplicationEvents.h>

//#include <DiscordPlugin/Plugin.h>
//#include <DiscordPlugin/DiscordManagerInterface.h>

#include <WindowModule/Events/WindowEvents.h>
#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

#include <NavigationEditor/Tools/NavMeshDebugDrawer.h>

#include <RHIModule/Images/Image.h>

#include <EventSystem/EventSystem.h>

#include <imgui.h>

#include "Circuit/Widgets/SliderWidget.h"

#include "Circuit/CircuitManager.h"

#include "Circuit/Input/CircuitInput.h"

Sandbox::Sandbox()
{
	VT_ASSERT_MSG(!s_instance, "Sandbox already exists!");
	s_instance = this;
}

Sandbox::~Sandbox()
{
	s_instance = nullptr;
}

void Sandbox::OnAttach()
{
	RegisterEventListeners();

	SelectionManager::Initialize();

	if (!Volt::ProjectManager::GetProject().isDeprecated)
	{
		EditorResources::Initialize();
	}

	Circuit::CircuitManager::Initialize(std::bind(&Sandbox::HandleCircuitTellEvents, this, std::placeholders::_1));
	Circuit::OpenWindowParams params;
	params.title = "Test Circuit Window";
	params.startWidth = 600;
	params.startHeight = 400;
	Circuit::CircuitWindow& window = Circuit::CircuitManager::Get().OpenWindow(params);

	window.SetWidget(CreateWidget(Circuit::SliderWidget)
		.X(100)
		.Y(100)
		.Max(100)
		.Min(0)
		.Value(50.f));

	EditorResources::Initialize();
	VersionControl::Initialize(VersionControlSystem::Perforce);

	NodeEditorHelpers::Initialize();
	IONodeGraphEditorHelpers::Initialize();

	Volt::WindowManager::Get().GetMainWindow().Resize(300, 500);

	m_editorCameraController = CreateRef<EditorCameraController>(60.f, 1.f, 100000.f);

	UserSettingsManager::LoadUserSettings();
	const auto& userSettings = UserSettingsManager::GetSettings();

	if (userSettings.sceneSettings.defaultOpenScene != Volt::Asset::Null())
	{
		OpenScene(userSettings.sceneSettings.defaultOpenScene);
		if (m_runtimeScene)
		{
			auto& worldEngine = m_runtimeScene->GetWorldEngineMutable();
			for (const auto& cell : worldEngine.GetCells())
			{
				worldEngine.BeginStreamingCell(cell.cellId);
			}
		}
	}

	if (!m_runtimeScene)
	{
		NewScene();
	}

	RegisterPanels();

	m_fileWatcher = CreateRef<FileWatcher>();
	CreateWatches();

	ImGuizmo::AllowAxisFlip(false);

	InitializeModals();

	//constexpr int64_t discordAppId = 1108502963447681106;

	//DiscordPlugin::GetInstance().GetManager().SetApplicationID(discordAppId);
	//DiscordPlugin::GetInstance().GetManager().SetLargeImage("icon_volt");
	//DiscordPlugin::GetInstance().GetManager().SetLargeText("Volt");
	//DiscordPlugin::GetInstance().GetManager().SetActivityType(ActivityType::Playing);
	//DiscordPlugin::GetInstance().GetManager().UpdateChanges();

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

void Sandbox::RegisterPanels()
{
	// Shelved Panels (So panel tab doesn't get cluttered up).
#ifdef VT_DEBUG
	EditorLibrary::RegisterWithType<PrefabEditorPanel>("", AssetTypes::Prefab);
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
	EditorLibrary::Register<VertexPainterPanel>("", m_runtimeScene, m_editorCameraController);

	EditorLibrary::Register<SceneSettingsPanel>("", m_runtimeScene);
	EditorLibrary::Register<WorldEnginePanel>("", m_runtimeScene);
	EditorLibrary::Register<GameUIEditorPanel>("UI");

	m_navigationPanel = EditorLibrary::Register<NavigationPanel>("Advanced", m_runtimeScene);
	m_viewportPanel = EditorLibrary::Register<ViewportPanel>("Level Editor", m_sceneRenderer, m_runtimeScene, m_editorCameraController.get(), m_sceneState);
	m_gameViewPanel = EditorLibrary::Register<GameViewPanel>("Level Editor", m_gameSceneRenderer, m_runtimeScene, m_sceneState);
	m_assetBrowserPanel = EditorLibrary::Register<AssetBrowserPanel>("", m_runtimeScene, "##Main");

	EditorLibrary::RegisterWithType<MosaicEditorPanel>("", AssetTypes::Material);
	EditorLibrary::RegisterWithType<CharacterEditorPanel>("Animation", AssetTypes::AnimatedCharacter);
	//EditorLibrary::RegisterWithType<MaterialEditorPanel>("", , myRuntimeScene);
	EditorLibrary::RegisterWithType<SkeletonEditorPanel>("Animation", AssetTypes::Skeleton);
	EditorLibrary::RegisterWithType<AnimationEditorPanel>("Animation", AssetTypes::Animation);
	EditorLibrary::RegisterWithType<ParticleEmitterEditor>("", AssetTypes::ParticlePreset);
	EditorLibrary::RegisterWithType<BlendSpaceEditorPanel>("Animation", AssetTypes::BlendSpace);
	EditorLibrary::RegisterWithType<MeshPreviewPanel>("", AssetTypes::Mesh);
	EditorLibrary::RegisterWithType<ShaderEditorPanel>("Shader", AssetTypes::ShaderDefinition);
	EditorLibrary::RegisterWithType<MotionWeaveDatabasePanel>("Animation", AssetTypes::MotionWeave);

	EditorLibrary::Sort();

	UserSettingsManager::SetupPanels();
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

		m_sceneRenderer = CreateRef<Volt::SceneRenderer>(spec);
		m_gameSceneRenderer = CreateRef<Volt::SceneRenderer>(gameSpec);
	}

	Volt::SceneManager::SetActiveScene(m_runtimeScene);
	Volt::OnSceneLoadedEvent loadEvent{ m_runtimeScene };
	Volt::EventSystem::DispatchEvent(loadEvent);
}

void Sandbox::InitializeModals()
{
	auto& meshModal = ModalSystem::AddModal<MeshImportModal>("Import Mesh##sandbox");
	m_meshImportModal = meshModal.GetID();
}

void Sandbox::OnDetach()
{
	m_isInitialized = false;

	if (m_sceneState == SceneState::Play)
	{
		OnSceneStop();
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

void Sandbox::OnScenePlay()
{
	m_sceneState = SceneState::Play;
	SelectionManager::DeselectAll();

	m_intermediateScene = m_runtimeScene;

	m_runtimeScene = CreateRef<Volt::Scene>();
	m_intermediateScene->CopyTo(m_runtimeScene);

	SetupNewSceneData();
	SetPlayHasMouseControl();
	Volt::Input::DisableInput(false);
	m_gameViewPanel->Focus();

	m_runtimeScene->OnRuntimeStart();

	Volt::OnScenePlayEvent playEvent{};
	Volt::EventSystem::DispatchEvent(playEvent);

	Volt::ViewportResizeEvent e2 = { m_viewportPosition.x,m_viewportPosition.y, m_viewportSize.x, m_viewportSize.y };
	Volt::EventSystem::DispatchEvent(e2);
}

void Sandbox::OnSceneStop()
{
	SelectionManager::DeselectAll();

	Volt::OnSceneStopEvent stopEvent{};
	Volt::EventSystem::DispatchEvent(stopEvent);

	Volt::ViewportResizeEvent e2 = { m_viewportPosition.x,m_viewportPosition.y, m_viewportSize.x, m_viewportSize.y };
	Volt::EventSystem::DispatchEvent(e2);

	m_runtimeScene->OnRuntimeEnd();

	SetEditorHasMouseControl();
	m_viewportPanel->Focus();

	m_runtimeScene = m_intermediateScene;

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
	Volt::EventSystem::DispatchEvent(playEvent);
}

void Sandbox::OnSimulationStop()
{
	SelectionManager::DeselectAll();

	Volt::OnSceneStopEvent stopEvent{};
	Volt::EventSystem::DispatchEvent(stopEvent);

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

	m_runtimeScene = Volt::Scene::CreateDefaultScene("New Scene", true);
	SetupNewSceneData();
}

void Sandbox::OpenScene()
{
	const std::filesystem::path loadPath = FileSystem::OpenFileDialogue({ { "Scene(*.vtscene)", "vtscene" } }, Volt::ProjectManager::GetAssetsDirectory());
	OpenScene(Volt::AssetManager::GetRelativePath(loadPath));
}

void Sandbox::OpenScene(const std::filesystem::path& path)
{
	if (!path.empty() && FileSystem::Exists(Volt::ProjectManager::GetRootDirectory() / path))
	{
		OpenScene(Volt::AssetManager::GetAssetHandleFromFilePath(path));
	}
}

void Sandbox::OpenScene(Volt::AssetHandle sceneHandle)
{
	if (sceneHandle == Volt::Asset::Null())
	{
		return;
	}

	SelectionManager::DeselectAll();

	Volt::AssetMetadata metadata;

	if (m_runtimeScene)
	{
		metadata = Volt::AssetManager::GetMetadataFromHandle(m_runtimeScene->handle);
	}

	const auto newScene = Volt::AssetManager::GetAsset<Volt::Scene>(sceneHandle);
	if (!newScene)
	{
		return;
	}

	VT_ENSURE(Volt::AssetManager::GetMetadataFromHandle(newScene->handle).type == AssetTypes::Scene);

	if (metadata.handle == sceneHandle)
	{
		Volt::AssetManager::Get().ReloadAsset(m_runtimeScene->handle);
	}
	else if (m_runtimeScene && !metadata.filePath.empty())
	{
		Volt::AssetManager::Get().Unload(m_runtimeScene->handle);
	}

	m_runtimeScene = newScene;

	SetupNewSceneData();
}

bool Sandbox::LoadScene(Volt::OnSceneTransitionEvent& e)
{
	m_storedScene = Volt::AssetManager::GetAsset<Volt::Scene>(e.GetHandle());
	m_shouldLoadNewScene = true;

	return true;
}

void Sandbox::HandleCircuitTellEvents(const Circuit::TellEvent& e)
{
	//switch (e.GetType())
	//{
	//	case Circuit::CircuitTellEventType::OpenWindow:
	//	{
	//		const Circuit::OpenWindowTellEvent& openWindowEvent = reinterpret_cast<const Circuit::OpenWindowTellEvent&>(e);

	//		Volt::WindowProperties windowProperties;
	//		windowProperties.Title = openWindowEvent.GetParams().title;
	//		windowProperties.VSync = false;

	//		windowProperties.Width = openWindowEvent.GetParams().startWidth;
	//		windowProperties.Height = openWindowEvent.GetParams().startHeight;

	//		windowProperties.UseTitlebar = true;
	//		windowProperties.UseCustomTitlebar = true;

	//		Volt::WindowHandle handle = Volt::WindowManager::Get().CreateNewWindow(windowProperties);
	//		Volt::Window& window = Volt::WindowManager::Get().GetWindow(handle);
	//		window.SetEventCallback(VT_BIND_EVENT_FN(Sandbox::HandleCircuitWindowEventCallback));


	//		Circuit::WindowOpenedListenEvent ev(handle);
	//		Circuit::CircuitManager::Get().BroadcastListenEvent(ev);

	//		m_circuitRenderers.push_back(CreateScope<CircuitRenderer>(handle));

	//		break;
	//	}
	//	default:
	//		VT_LOG(Error, "Unhandled Circuit Event!");
	//		break;
	//}
}

void Sandbox::HandleCircuitWindowEventCallback(Volt::Event& e)
{
	/*Volt::EventDispatcher dispatcher(e);

	dispatcher.Dispatch<Volt::KeyPressedEvent>([](Volt::KeyPressedEvent& e)
	{
		Circuit::CircuitInput::Get().OnKeyPressed(VoltKeyCodeToCircuitKeyCode(e.GetKeyCode()));
		return false;
	});

	dispatcher.Dispatch<Volt::KeyReleasedEvent>([](Volt::KeyReleasedEvent& e)
	{
		Circuit::CircuitInput::Get().OnKeyReleased(VoltKeyCodeToCircuitKeyCode(e.GetKeyCode()));
		return false;
	});

	dispatcher.Dispatch<Volt::MouseButtonPressedEvent>([](Volt::MouseButtonPressedEvent& e)
	{
		Circuit::CircuitInput::Get().OnKeyPressed(VoltMouseCodeToCircuitKeyCode(e.GetMouseButton()));
		return false;
	});

	dispatcher.Dispatch<Volt::MouseButtonReleasedEvent>([](Volt::MouseButtonReleasedEvent& e)
	{
		Circuit::CircuitInput::Get().OnKeyReleased(VoltMouseCodeToCircuitKeyCode(e.GetMouseButton()));
		return false;
	});

	dispatcher.Dispatch<Volt::MouseMovedEvent>([](Volt::MouseMovedEvent& e)
	{
		Circuit::CircuitInput::Get().OnMouseMoved(e.GetX(), e.GetY());
		return false;
	});

	VT_LOG(Info,"Window Event: {0}", e.ToString());*/
}

Circuit::KeyCode Sandbox::VoltKeyCodeToCircuitKeyCode(uint32_t keyCode)
{
	switch (keyCode)
	{
		case VT_KEY_SPACE: return Circuit::KeyCode::Spacebar;
		case VT_KEY_APOSTROPHE: return Circuit::KeyCode::Apostrophe;
		case VT_KEY_COMMA: return Circuit::KeyCode::Comma;
		case VT_KEY_MINUS: return Circuit::KeyCode::Minus;
		case VT_KEY_PERIOD: return Circuit::KeyCode::Period;
		case VT_KEY_SLASH: return Circuit::KeyCode::Slash;
		case VT_KEY_0: return Circuit::KeyCode::Key_0;
		case VT_KEY_1: return Circuit::KeyCode::Key_1;
		case VT_KEY_2: return Circuit::KeyCode::Key_2;
		case VT_KEY_3: return Circuit::KeyCode::Key_3;
		case VT_KEY_4: return Circuit::KeyCode::Key_4;
		case VT_KEY_5: return Circuit::KeyCode::Key_5;
		case VT_KEY_6: return Circuit::KeyCode::Key_6;
		case VT_KEY_7: return Circuit::KeyCode::Key_7;
		case VT_KEY_8: return Circuit::KeyCode::Key_8;
		case VT_KEY_9: return Circuit::KeyCode::Key_9;
		case VT_KEY_SEMICOLON: return Circuit::KeyCode::Semicolon;
		case VT_KEY_EQUAL: return Circuit::KeyCode::Equal;
		case VT_KEY_A: return Circuit::KeyCode::A;
		case VT_KEY_B: return Circuit::KeyCode::B;
		case VT_KEY_C: return Circuit::KeyCode::C;
		case VT_KEY_D: return Circuit::KeyCode::D;
		case VT_KEY_E: return Circuit::KeyCode::E;
		case VT_KEY_F: return Circuit::KeyCode::F;
		case VT_KEY_G: return Circuit::KeyCode::G;
		case VT_KEY_H: return Circuit::KeyCode::H;
		case VT_KEY_I: return Circuit::KeyCode::I;
		case VT_KEY_J: return Circuit::KeyCode::J;
		case VT_KEY_K: return Circuit::KeyCode::K;
		case VT_KEY_L: return Circuit::KeyCode::L;
		case VT_KEY_M: return Circuit::KeyCode::M;
		case VT_KEY_N: return Circuit::KeyCode::N;
		case VT_KEY_O: return Circuit::KeyCode::O;
		case VT_KEY_P: return Circuit::KeyCode::P;
		case VT_KEY_Q: return Circuit::KeyCode::Q;
		case VT_KEY_R: return Circuit::KeyCode::R;
		case VT_KEY_S: return Circuit::KeyCode::S;
		case VT_KEY_T: return Circuit::KeyCode::T;
		case VT_KEY_U: return Circuit::KeyCode::U;
		case VT_KEY_V: return Circuit::KeyCode::V;
		case VT_KEY_W: return Circuit::KeyCode::W;
		case VT_KEY_X: return Circuit::KeyCode::X;
		case VT_KEY_Y: return Circuit::KeyCode::Y;
		case VT_KEY_Z: return Circuit::KeyCode::Z;
		case VT_KEY_LEFT_BRACKET: return Circuit::KeyCode::LeftBracket;
		case VT_KEY_BACKSLASH: return Circuit::KeyCode::Backslash;
		case VT_KEY_RIGHT_BRACKET: return Circuit::KeyCode::RightBracket;
		case VT_KEY_GRAVE_ACCENT: return Circuit::KeyCode::GraveAccent;
		case VT_KEY_WORLD_1: return Circuit::KeyCode::World_1;
		case VT_KEY_WORLD_2: return Circuit::KeyCode::World_2;
		case VT_KEY_ESCAPE: return Circuit::KeyCode::Esc;
		case VT_KEY_ENTER: return Circuit::KeyCode::Return;
		case VT_KEY_TAB: return Circuit::KeyCode::Tab;
		case VT_KEY_BACKSPACE: return Circuit::KeyCode::Backspace;
		case VT_KEY_INSERT: return Circuit::KeyCode::Insert;
		case VT_KEY_DELETE: return Circuit::KeyCode::Delete;
		case VT_KEY_RIGHT: return Circuit::KeyCode::RightArrow;
		case VT_KEY_LEFT: return Circuit::KeyCode::LeftArrow;
		case VT_KEY_DOWN: return Circuit::KeyCode::DownArrow;
		case VT_KEY_UP: return Circuit::KeyCode::UpArrow;
		case VT_KEY_PAGE_UP: return Circuit::KeyCode::PageUp;
		case VT_KEY_PAGE_DOWN: return Circuit::KeyCode::PageDown;
		case VT_KEY_HOME: return Circuit::KeyCode::Home;
		case VT_KEY_END: return Circuit::KeyCode::End;
		case VT_KEY_CAPS_LOCK: return Circuit::KeyCode::CapsLock;
		case VT_KEY_SCROLL_LOCK: return Circuit::KeyCode::ScrollLock;
		case VT_KEY_NUM_LOCK: return Circuit::KeyCode::NumLock;
		case VT_KEY_PRINT_SCREEN: return Circuit::KeyCode::PrintScreen;
		case VT_KEY_PAUSE: return Circuit::KeyCode::Pause;
		case VT_KEY_F1: return Circuit::KeyCode::F1;
		case VT_KEY_F2: return Circuit::KeyCode::F2;
		case VT_KEY_F3: return Circuit::KeyCode::F3;
		case VT_KEY_F4: return Circuit::KeyCode::F4;
		case VT_KEY_F5: return Circuit::KeyCode::F5;
		case VT_KEY_F6: return Circuit::KeyCode::F6;
		case VT_KEY_F7: return Circuit::KeyCode::F7;
		case VT_KEY_F8: return Circuit::KeyCode::F8;
		case VT_KEY_F9: return Circuit::KeyCode::F9;
		case VT_KEY_F10: return Circuit::KeyCode::F10;
		case VT_KEY_F11: return Circuit::KeyCode::F11;
		case VT_KEY_F12: return Circuit::KeyCode::F12;
		case VT_KEY_F13: return Circuit::KeyCode::F13;
		case VT_KEY_F14: return Circuit::KeyCode::F14;
		case VT_KEY_F15: return Circuit::KeyCode::F15;
		case VT_KEY_F16: return Circuit::KeyCode::F16;
		case VT_KEY_F17: return Circuit::KeyCode::F17;
		case VT_KEY_F18: return Circuit::KeyCode::F18;
		case VT_KEY_F19: return Circuit::KeyCode::F19;
		case VT_KEY_F20: return Circuit::KeyCode::F20;
		case VT_KEY_F21: return Circuit::KeyCode::F21;
		case VT_KEY_F22: return Circuit::KeyCode::F22;
		case VT_KEY_F23: return Circuit::KeyCode::F23;
		case VT_KEY_F24: return Circuit::KeyCode::F24;
		case VT_KEY_KP_0: return Circuit::KeyCode::Numpad_0;
		case VT_KEY_KP_1: return Circuit::KeyCode::Numpad_1;
		case VT_KEY_KP_2: return Circuit::KeyCode::Numpad_2;
		case VT_KEY_KP_3: return Circuit::KeyCode::Numpad_3;
		case VT_KEY_KP_4: return Circuit::KeyCode::Numpad_4;
		case VT_KEY_KP_5: return Circuit::KeyCode::Numpad_5;
		case VT_KEY_KP_6: return Circuit::KeyCode::Numpad_6;
		case VT_KEY_KP_7: return Circuit::KeyCode::Numpad_7;
		case VT_KEY_KP_8: return Circuit::KeyCode::Numpad_8;
		case VT_KEY_KP_9: return Circuit::KeyCode::Numpad_9;
		case VT_KEY_KP_DECIMAL: return Circuit::KeyCode::Decimal;
		case VT_KEY_KP_DIVIDE: return Circuit::KeyCode::Divide;
		case VT_KEY_KP_MULTIPLY: return Circuit::KeyCode::Multiply;
		case VT_KEY_KP_SUBTRACT: return Circuit::KeyCode::Subtract;
		case VT_KEY_KP_ADD: return Circuit::KeyCode::Add;
		case VT_KEY_KP_ENTER: return Circuit::KeyCode::Enter;
		case VT_KEY_KP_EQUAL: return Circuit::KeyCode::Equal;
		case VT_KEY_LEFT_SHIFT: return Circuit::KeyCode::LeftShift;
		case VT_KEY_LEFT_CONTROL: return Circuit::KeyCode::LeftControl;
		case VT_KEY_LEFT_ALT: return Circuit::KeyCode::LeftAlt;
		case VT_KEY_LEFT_SUPER: return Circuit::KeyCode::LeftSuper;
		case VT_KEY_RIGHT_SHIFT: return Circuit::KeyCode::RightShift;
		case VT_KEY_RIGHT_CONTROL: return Circuit::KeyCode::RightControl;
		case VT_KEY_RIGHT_ALT: return Circuit::KeyCode::RightAlt;
		case VT_KEY_RIGHT_SUPER: return Circuit::KeyCode::RightSuper;
		case VT_KEY_MENU: return Circuit::KeyCode::Menu;
		default: return Circuit::KeyCode::Unknown;
	}
}

Circuit::KeyCode Sandbox::VoltMouseCodeToCircuitKeyCode(uint32_t keyCode)
{
	switch (keyCode)
	{
		case VT_MOUSE_BUTTON_1: return Circuit::KeyCode::Mouse_LB;
		case VT_MOUSE_BUTTON_2: return Circuit::KeyCode::Mouse_RB;
		case VT_MOUSE_BUTTON_3: return Circuit::KeyCode::Mouse_MB;
		case VT_MOUSE_BUTTON_4: return Circuit::KeyCode::Mouse_X1;
		case VT_MOUSE_BUTTON_5: return Circuit::KeyCode::Mouse_X2;
		case VT_MOUSE_BUTTON_6: return Circuit::KeyCode::Mouse_X3;
		case VT_MOUSE_BUTTON_7: return Circuit::KeyCode::Mouse_X4;
		case VT_MOUSE_BUTTON_8: return Circuit::KeyCode::Mouse_X5;
	}

	return Circuit::KeyCode::A;
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
	Volt::EventSystem::DispatchEvent(windowResizeEvent);

	Volt::OnScenePlayEvent playEvent{};
	Volt::EventSystem::DispatchEvent(playEvent);

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

		FileSystem::CopyFileToDirectory("../Tools/MayaExporter/voltTranslator.py", pluginsPath);
		FileSystem::CopyFileToDirectory("../Tools/MayaExporter/voltExport.py", scriptsPath);
		FileSystem::CopyFileToDirectory("../Tools/MayaExporter/voltTranslatorOpts.mel", scriptsPath);

		FileSystem::Copy("../Tools/MayaExporter/yaml", scriptsPath / "yaml");
	}

	UI::Notify(NotificationType::Success, "Successfully installed Maya tools!", "The Maya tools were successfully installed!");
}

void Sandbox::RegisterEventListeners()
{
	auto isInitializedPred = [this]() { return m_isInitialized; };

	RegisterListener<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(Sandbox::OnUpdateEvent), isInitializedPred);
	RegisterListener<Volt::AppImGuiUpdateEvent>(VT_BIND_EVENT_FN(Sandbox::OnImGuiUpdateEvent), isInitializedPred);
	RegisterListener<Volt::WindowRenderEvent>(VT_BIND_EVENT_FN(Sandbox::OnRenderEvent), isInitializedPred);
	RegisterListener<Volt::KeyPressedEvent>(VT_BIND_EVENT_FN(Sandbox::OnKeyPressedEvent), isInitializedPred);
	RegisterListener<Volt::ViewportResizeEvent>(VT_BIND_EVENT_FN(Sandbox::OnViewportResizeEvent), isInitializedPred);
	RegisterListener<Volt::OnSceneLoadedEvent>(VT_BIND_EVENT_FN(Sandbox::OnSceneLoadedEvent), isInitializedPred);
	RegisterListener<Volt::OnSceneTransitionEvent>(VT_BIND_EVENT_FN(Sandbox::LoadScene), isInitializedPred);
	
	RegisterListener<Volt::WindowTitlebarHittestEvent>([&](Volt::WindowTitlebarHittestEvent& e)
	{
		e.SetHit(m_titlebarHovered);
		return false;
	}, isInitializedPred);
}

bool Sandbox::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	VT_PROFILE_FUNCTION();

	EditorCommandStack::GetInstance().Update(100);

	auto mousePos = Volt::Input::GetMousePosition();
	Volt::Input::SetViewportMousePosition(m_gameViewPanel->GetViewportLocalPosition(mousePos));

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

	Circuit::CircuitManager::Get().Update();

	return false;
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

bool Sandbox::OnRenderEvent(Volt::WindowRenderEvent& e)
{
	VT_PROFILE_FUNCTION();

	for (Scope<CircuitRenderer>& renderer : m_circuitRenderers)
	{
		renderer->OnRender();
	}

	/*for (auto& circuitWindowPair : Circuit::CircuitManager::Get().GetWindows())
	{
		Volt::WindowHandle handle = circuitWindowPair.first;
		Volt::Window& window = Volt::Application::GetWindowManager().GetWindow(handle);
		Circuit::CircuitWindow& circuitWindow = *circuitWindowPair.second;


		circuitWindow.GetDrawCommands();
	}*/

	//mySceneRenderer->ClearOutlineCommands();

	RenderSelection(m_editorCameraController->GetCamera());
	//RenderGizmos(myRuntimeScene, myEditorCameraController->GetCamera());

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
	const bool ctrlPressed = Volt::Input::IsButtonDown(Volt::InputCode::LeftControl);
	const bool shiftPressed = Volt::Input::IsButtonDown(Volt::InputCode::LeftShift);

	switch (e.GetKeyCode())
	{
		case Volt::InputCode::Z:
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

		case Volt::InputCode::Y:
		{
			if (ctrlPressed)
			{
				EditorCommandStack::GetInstance().Redo();
			}
			break;
		}

		case Volt::InputCode::S:
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

		case Volt::InputCode::O:
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

		case Volt::InputCode::N:
		{
			if (ctrlPressed)
			{
				NewScene();
			}

			break;
		}

		case Volt::InputCode::F:
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

		case Volt::InputCode::Spacebar :
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

		case Volt::InputCode::Esc:
		{
			if (m_sceneState == SceneState::Play)
			{
				SetEditorHasMouseControl();
			}

			break;
		}

		case Volt::InputCode::G:
		{
			if (ctrlPressed && m_sceneState != SceneState::Play)
			{
				// #TODO_Ivar: Reimplement moving player to editor camera.
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

bool Sandbox::OnSceneLoadedEvent(Volt::OnSceneLoadedEvent& e)
{
	m_sceneRenderer->Resize(m_viewportSize.x, m_viewportSize.y);

	if (m_gameSceneRenderer)
	{
		m_gameSceneRenderer->Resize(m_viewportSize.x, m_viewportSize.y);
	}

	e.GetScene()->SetRenderSize(m_viewportSize.x, m_viewportSize.y);

	Volt::ViewportResizeEvent e2 = { m_viewportPosition.x,m_viewportPosition.y, m_viewportSize.x, m_viewportSize.y };
	Volt::EventSystem::DispatchEvent(e2);

	auto scene = e.GetScene();
	
	/*DiscordPlugin::GetInstance().GetManager().SetState(scene->GetName());
	DiscordPlugin::GetInstance().GetManager().SetPartySize(m_runtimeScene->GetActiveLayer() + 1);
	DiscordPlugin::GetInstance().GetManager().SetMaxPartySize(static_cast<int32_t>(m_runtimeScene->GetLayers().size()));
	DiscordPlugin::GetInstance().GetManager().SetStartTime(std::time(nullptr));

	DiscordPlugin::GetInstance().GetManager().UpdateChanges();*/

	return false;
}
