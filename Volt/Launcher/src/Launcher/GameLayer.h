#pragma once

#include <Volt/Core/Layer/Layer.h>
#include <Volt/Events/ApplicationEvent.h>
#include <Volt/Events/GameEvent.h>

#include <Game/Game.h>

namespace Volt
{
	class SceneRenderer;
	class NavigationsSystem;
	class Scene;
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

	Ref<Volt::SceneRenderer> mySceneRenderer;
	Ref<Volt::NavigationsSystem> myNavigationsSystem;
	Ref<Volt::Scene> myScene;
	Ref<Game> myGame;

	Ref<Volt::Scene> myStoredScene;
	bool isPaused = false;

	// Loading
	bool myShouldLoadNewScene = false;
	bool myIsLoadingScene = false;
	uint32_t myLastWidth = 0;
	uint32_t myLastHeight = 0;
};