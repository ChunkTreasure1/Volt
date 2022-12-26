#pragma once

#include "Sandbox/Utility/Helpers.h"
#include "Sandbox/VersionControl/VersionControl.h"
#include "Sandbox/FileWatcher/FileWatcher.h"
#include "Sandbox/GameBuilder.h"

#include <Volt/Scene/Entity.h>
#include <Volt/Core/Layer/Layer.h>
#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Events/KeyEvent.h>
#include <Volt/Utility/DLLHandler.h>

#include <Volt/Rendering/RenderPass.h>

#include <Game/Game.h>

#include <imgui.h>
#include <ImGuizmo.h>

namespace Volt
{
	class SceneRenderer;
	class NavigationsSystem;
	class Scene;
	class Mesh;
	class Framebuffer;
	class Material;
	class Shader;
	class ConstantBuffer;
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

	inline void SetShouldRenderGizmos(bool shouldRender) { myShouldRenderGizmos = shouldRender; }

	inline static Sandbox& Get() { return *myInstance; }

	Ref<Volt::SceneRenderer>& GetSceneRenderer() { return mySceneRenderer; }
	
	void NewScene();
	void OpenScene();
	void OpenScene(const std::filesystem::path& path);
	void SaveScene();
	void TransitionToNewScene();

	const std::vector<Ref<EditorWindow>>& GetEditorWindows() { return myEditorWindows; };

private:
	struct SaveSceneAsData
	{
		std::string name = "New Level";
		std::filesystem::path destinationPath = "Assets/Levels/";
	} mySaveSceneData;

	void ExecuteUndo();
	void SaveSceneAs();

	void InstallMayaTools();
	void HandleChangedFiles();
	void SetupRenderCallbacks();
	void SetupEditorRenderPasses();

	bool OnUpdateEvent(Volt::AppUpdateEvent& e);
	bool OnImGuiUpdateEvent(Volt::AppImGuiUpdateEvent& e);
	bool OnRenderEvent(Volt::AppRenderEvent& e);
	bool OnKeyPressedEvent(Volt::KeyPressedEvent& e);
	bool OnViewportResizeEvent(Volt::ViewportResizeEvent& e);
	bool OnSceneLoadedEvent(Volt::OnSceneLoadedEvent& e);
	bool LoadScene(Volt::OnSceneTransitionEvent& e);

	void SaveUserSettings();
	void LoadUserSettings();

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
	///////////////

	BuildInfo myBuildInfo;

	std::vector<Ref<EditorWindow>> myEditorWindows;
	Ref<EditorCameraController> myEditorCameraController;

	Ref<Volt::SceneRenderer> mySceneRenderer;
	Ref<Volt::Material> myGridMaterial;
	Ref<FileWatcher> myFileWatcher;

	Ref<Volt::Scene> myRuntimeScene;
	Ref<Volt::Scene> myIntermediateScene;

	/////Outline/////
	Volt::RenderPass mySelectedGeometryPass;
	Volt::RenderPass myJumpFloodInitPass;
	Volt::RenderPass myJumpFloodCompositePass;

	Volt::RenderPass myJumpFloodPass[2];
	Ref<Volt::ConstantBuffer> myJumpFloodBuffer;
	/////////////////

	/////Gizmos/////
	Ref<Volt::Shader> myGizmoShader;
	Volt::RenderPass myGizmoPass;
	//////////////////

	///// Forward Extra /////
	Volt::RenderPass myColliderVisualizationPass;
	Volt::RenderPass myForwardExtraPass;
	/////////////////////////

	Ref<Game> myGame;

	SceneState mySceneState = SceneState::Edit;

	Ref<Volt::NavigationsSystem> myNavigationsSystem;
	Ref<ViewportPanel> myViewportPanel;

	gem::vec2ui myViewportSize = { 1280, 720 };
	gem::vec2ui myViewportPosition = { 0, 0 };

	bool myShouldOpenSaveSceneAs = false;
	bool myOpenShouldSaveScenePopup = false;
	bool myShouldResetLayout = false;
	bool myShouldRenderGizmos = true;
	bool myTitlebarHovered = false;
	bool myBuildStarted = false;

	Ref<Volt::Scene> myStoredScene;
	bool myShouldLoadNewScene = false;

	inline static Sandbox* myInstance = nullptr;

	bool initiated = false;
};