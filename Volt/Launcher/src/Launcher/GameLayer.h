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

	Ref<Volt::SceneRenderer>& GetSceneRenderer() { return m_sceneRenderer; }

private:
	bool OnUpdateEvent(Volt::AppUpdateEvent& e);
	bool OnRenderEvent(Volt::AppRenderEvent& e);
	bool OnWindowResizeEvent(Volt::WindowResizeEvent& e);
	bool OnSceneTransition(Volt::OnSceneTransitionEvent& e);
	bool OnSceneLoaded(Volt::OnSceneLoadedEvent& e);
	bool OnGameStateChanged(Volt::OnGameStateChangedEvent& e);

	Volt::SceneRendererSettings LoadGraphicSettings();

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
