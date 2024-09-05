#pragma once

#include <Volt/Core/Layer/Layer.h>
#include <Volt/Events/ApplicationEvents.h>

#include <EventSystem/EventListener.h>

namespace Volt
{
	class Scene;
	class SceneRenderer;
	struct SceneRendererSettings;

	class WindowRenderEvent;
	class WindowResizeEvent;

	class OnSceneTransitionEvent;
	class OnSceneLoadedEvent;
}

class GameLayer : public Volt::Layer, public Volt::EventListener
{
public:
	GameLayer() = default;
	~GameLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void LoadStartScene(); // remove

	Ref<Volt::SceneRenderer>& GetSceneRenderer() { return m_sceneRenderer; }

private:
	bool OnUpdateEvent(Volt::AppUpdateEvent& e);
	bool OnRenderEvent(Volt::WindowRenderEvent& e);
	bool OnWindowResizeEvent(Volt::WindowResizeEvent& e);
	bool OnSceneTransition(Volt::OnSceneTransitionEvent& e);
	bool OnSceneLoaded(Volt::OnSceneLoadedEvent& e);
	//bool OnGameStateChanged(Volt::OnGameStateChangedEvent& e);

	void TrySceneTransition();

	Ref<Volt::SceneRenderer> m_sceneRenderer;
	Ref<Volt::Scene> m_scene;

	Ref<Volt::Scene> m_storedScene;
	bool m_isPaused = false;

	// Loading
	bool m_shouldLoadNewScene = false;
	uint32_t m_lastWidth = 0;
	uint32_t m_lastHeight = 0;
};
