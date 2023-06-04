#pragma once

#include <Volt/Core/Layer/Layer.h>
#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Events/GameEvent.h>

namespace Volt
{
	class Scene;
	class SceneRenderer;
	struct SceneRendererSettings;
}

class GameLayer : public Volt::Layer
{
public:
	GameLayer() = default;
	~GameLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnEvent(Volt::Event& e) override;

	void LoadStartScene(); // remove

	Ref<Volt::SceneRenderer>& GetSceneRenderer() { return mySceneRenderer; }

private:
	bool OnUpdateEvent(Volt::AppUpdateEvent& e);
	bool OnRenderEvent(Volt::AppRenderEvent& e);
	bool OnWindowResizeEvent(Volt::WindowResizeEvent& e);
	bool OnSceneTransition(Volt::OnSceneTransitionEvent& e);
	bool OnSceneLoaded(Volt::OnSceneLoadedEvent& e);
	bool OnGameStateChanged(Volt::OnGameStateChangedEvent& e);

	void TransitionToNewScene();
	Volt::SceneRendererSettings LoadGraphicSettings();

	Ref<Volt::SceneRenderer> mySceneRenderer;
	Ref<Volt::Scene> myScene;

	Ref<Volt::Scene> myStoredScene;
	bool isPaused = false;

	// Loading
	bool myShouldLoadNewScene = false;
	bool myIsLoadingScene = false;
	uint32_t myLastWidth = 0;
	uint32_t myLastHeight = 0;
};
