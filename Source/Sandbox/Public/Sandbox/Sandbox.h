#pragma once

#include "Sandbox/Utility/Helpers.h"
#include "Sandbox/VersionControl/VersionControl.h"
#include "Sandbox/FileWatcher/FileWatcher.h"
#include "Sandbox/GameBuilder.h"

#include <Volt/Scene/Entity.h>
#include <Volt/Core/Layer/Layer.h>

#include <EventSystem/EventListener.h>

#include <imgui.h>
#include <ImGuizmo.h>

namespace Volt
{
	class SceneRenderer;
	class Scene;
	class Mesh;
	class Material;
	class Camera;
	class Texture2D;

	class Event;
	class AppUpdateEvent;
	class AppImGuiUpdateEvent;
	class WindowRenderEvent;
	class KeyPressedEvent;
	class ViewportResizeEvent;
	class OnSceneLoadedEvent;
	class OnSceneTransitionEvent;
}

enum class SceneState
{
	Edit,
	Play,
	Pause,
	Simulating
};

struct ImGuiWindow;
class ViewportPanel;
class GameViewPanel;
class NavigationPanel;
class AssetBrowserPanel;

class EditorWindow;
class EditorCameraController;

class Sandbox : public Volt::Layer, public Volt::EventListener
{
public:
	Sandbox();
	~Sandbox() override;

	void OnAttach() override;
	void OnDetach() override;

	void OnScenePlay();
	void OnSceneStop();

	void OnSimulationStart();
	void OnSimulationStop();

	void SetEditorHasMouseControl();
	void SetPlayHasMouseControl();

	VT_NODISCARD VT_INLINE static Sandbox& Get() { return *s_instance; }

	Ref<Volt::SceneRenderer>& GetSceneRenderer() { return m_sceneRenderer; }
	VT_NODISCARD VT_INLINE const SceneState GetSceneState() const { return m_sceneState; }
	
	VT_NODISCARD VT_INLINE UUID64 GetMeshImportModalID() const { return m_meshImportModal; }
	VT_NODISCARD VT_INLINE UUID64 GetTextureImportModalID() const { return m_textureImportModal; }

	void NewScene();
	void OpenScene();
	void OpenScene(const std::filesystem::path& path);
	void OpenScene(Volt::AssetHandle sceneHandle);
	void SaveScene();
	void TransitionToNewScene();

	bool CheckForUpdateNavMesh(Volt::Entity entity);
	void BakeNavMesh();

private:
	struct SaveSceneAsData
	{
		std::string name = "New Scene";
		std::filesystem::path destinationPath = "Assets/Scenes/";
	} m_saveSceneData;

	void SaveSceneAs();
	void InstallMayaTools();
	void RegisterEventListeners();

	bool OnUpdateEvent(Volt::AppUpdateEvent& e);
	bool OnImGuiUpdateEvent(Volt::AppImGuiUpdateEvent& e);
	bool OnRenderEvent(Volt::WindowRenderEvent& e);
	bool OnKeyPressedEvent(Volt::KeyPressedEvent& e);
	bool OnViewportResizeEvent(Volt::ViewportResizeEvent& e);
	bool OnSceneLoadedEvent(Volt::OnSceneLoadedEvent& e);
	bool LoadScene(Volt::OnSceneTransitionEvent& e);

	void CreateWatches();
	void RegisterPanels();

	void SetupNewSceneData();
	void InitializeModals();

	/////ImGui/////
	void UpdateDockSpace();
	void SaveSceneAsModal();

	void BuildGameModal();
	void RenderProgressBar(float progress);

	void RenderWindowOuterBorders(ImGuiWindow* window);
	void HandleManualWindowResize();
	bool UpdateWindowManualResize(ImGuiWindow* window, ImVec2& newSize, ImVec2& newPosition);

	float DrawTitlebar();
	void DrawMenuBar();
	
	void RenderGameView();
	///////////////

	///// Debug Rendering /////
	void RenderSelection(Ref<Volt::Camera> camera);
	void RenderGizmos(Ref<Volt::Scene> scene, Ref<Volt::Camera> camera);
	///////////////////////////

	///// File Watchers /////
	void CreateModifiedWatch();
	void CreateDeleteWatch();
	void CreateAddWatch();
	void CreateMovedWatch();
	/////////////////////////

	BuildInfo m_buildInfo;

	Ref<EditorCameraController> m_editorCameraController;

	Ref<Volt::SceneRenderer> m_sceneRenderer;
	Ref<Volt::SceneRenderer> m_gameSceneRenderer;

	///// File watcher /////
	Ref<FileWatcher> m_fileWatcher;
	std::mutex m_fileWatcherMutex;
	Vector<std::function<void()>> m_fileChangeQueue;
	////////////////////////

	///// Modals /////
	UUID64 m_meshImportModal;
	UUID64 m_textureImportModal;
	//////////////////

	Ref<Volt::Scene> m_runtimeScene;
	Ref<Volt::Scene> m_intermediateScene;

	SceneState m_sceneState = SceneState::Edit;

	Ref<ViewportPanel> m_viewportPanel;
	Ref<GameViewPanel> m_gameViewPanel;

	Ref<NavigationPanel> m_navigationPanel;

	Ref<AssetBrowserPanel> m_assetBrowserPanel;

	glm::uvec2 m_viewportSize = { 1280, 720 };
	glm::uvec2 m_viewportPosition = { 0, 0 };

	bool m_shouldOpenSaveSceneAs = false;
	bool m_openShouldSaveScenePopup = false;
	bool m_shouldResetLayout = false;
	bool m_titlebarHovered = false;
	bool m_buildStarted = false;
	bool m_playHasMouseControl = false;
	bool m_isInitialized = false;

	Ref<Volt::Scene> m_storedScene;
	bool m_shouldLoadNewScene = false;
	uint32_t m_assetBrowserCount = 0;

	inline static Sandbox* s_instance = nullptr;
};
