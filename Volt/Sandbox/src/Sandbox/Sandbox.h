#pragma once

#include "Sandbox/Utility/Helpers.h"
#include "Sandbox/VersionControl/VersionControl.h"
#include "Sandbox/FileWatcher/FileWatcher.h"
#include "Sandbox/GameBuilder.h"

#include <Volt/Scene/Entity.h>
#include <Volt/Core/Layer/Layer.h>
#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Events/KeyEvent.h>
#include <Volt/Events/MouseEvent.h>

#include <imgui.h>
#include <ImGuizmo.h>

namespace Volt
{
	class SceneRenderer;
	class Scene;
	class Mesh;
	class Framebuffer;
	class Material;
	class Camera;
	class Texture2D;
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

class Sandbox : public Volt::Layer
{
public:
	Sandbox();
	~Sandbox() override;

	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Volt::Event& e) override;

	void OnScenePlay();
	void OnSceneStop();

	void OnSimulationStart();
	void OnSimulationStop();

	void SetEditorHasMouseControl();
	void SetPlayHasMouseControl();

	inline static Sandbox& Get() { return *myInstance; }

	inline Ref<Volt::SceneRenderer>& GetSceneRenderer() { return mySceneRenderer; }
	inline const SceneState GetSceneState() const { return mySceneState; }

	void NewScene();
	void OpenScene();
	void OpenScene(const std::filesystem::path& path);
	void SaveScene();
	void TransitionToNewScene();

	bool CheckForUpdateNavMesh(Volt::Entity entity);
	void BakeNavMesh();

private:
	struct SaveSceneAsData
	{
		std::string name = "New Scene";
		std::filesystem::path destinationPath = "Assets/Scenes/";
	} mySaveSceneData;

	void SaveSceneAs();

	void InstallMayaTools();

	bool OnUpdateEvent(Volt::AppUpdateEvent& e);
	bool OnImGuiUpdateEvent(Volt::AppImGuiUpdateEvent& e);
	bool OnRenderEvent(Volt::AppRenderEvent& e);
	bool OnKeyPressedEvent(Volt::KeyPressedEvent& e);
	bool OnViewportResizeEvent(Volt::ViewportResizeEvent& e);
	bool OnSceneLoadedEvent(Volt::OnSceneLoadedEvent& e);
	bool LoadScene(Volt::OnSceneTransitionEvent& e);

	void CreateWatches();

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

	///// Modals /////
	Volt::UUID m_projectUpgradeModal = 0;
	//////////////////

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

	BuildInfo myBuildInfo;

	Ref<EditorCameraController> myEditorCameraController;

	//Ref<Volt::SceneRenderer> mySceneRenderer;
	//Ref<Volt::SceneRenderer> myGameSceneRenderer;
	//Ref<Volt::Material> myGridMaterial;

	Ref<Volt::SceneRenderer> mySceneRenderer;
	Ref<Volt::SceneRenderer> myGameSceneRenderer;

	///// File watcher /////
	Ref<FileWatcher> myFileWatcher;
	std::mutex myFileWatcherMutex;
	std::vector<std::function<void()>> myFileChangeQueue;
	////////////////////////

	Ref<Volt::Scene> myRuntimeScene;
	Ref<Volt::Scene> myIntermediateScene;

	SceneState mySceneState = SceneState::Edit;

	Ref<ViewportPanel> myViewportPanel;
	Ref<GameViewPanel> myGameViewPanel;

	Ref<NavigationPanel> myNavigationPanel;

	Ref<AssetBrowserPanel> myAssetBrowserPanel;

	glm::uvec2 myViewportSize = { 1280, 720 };
	glm::uvec2 myViewportPosition = { 0, 0 };

	bool myShouldOpenSaveSceneAs = false;
	bool myOpenShouldSaveScenePopup = false;
	bool myShouldResetLayout = false;
	bool myTitlebarHovered = false;
	bool myBuildStarted = false;
	bool myPlayHasMouseControl = false;
	bool m_isInitialized = false;

	bool myShouldMovePlayer = false;
	glm::vec3 myMovePlayerToPosition = 0.f;

	Ref<Volt::Scene> myStoredScene;
	bool myShouldLoadNewScene = false;
	uint32_t myAssetBrowserCount = 0;

	inline static Sandbox* myInstance = nullptr;
};
