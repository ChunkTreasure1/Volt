#include "GameLayer.h"

#include <Volt/Scene/Scene.h>
#include <Volt/Asset/AssetManager.h>

#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Framebuffer.h>

#include <Volt/AI/NavMesh/NavigationsSystem.h>

#include <Volt/Core/Application.h>

void GameLayer::OnAttach()
{
	myScene = Volt::AssetManager::GetAsset<Volt::Scene>("Assets/Levels/LogoLevel/LogoLevel.vtscene");
	mySceneRenderer = CreateRef<Volt::SceneRenderer>(myScene);
	myNavigationsSystem = CreateRef<Volt::NavigationsSystem>(myScene);
}

void GameLayer::OnDetach()
{
	myScene->OnRuntimeEnd();

	mySceneRenderer = nullptr;
	myScene = nullptr;
	myNavigationsSystem = nullptr;
}

void GameLayer::OnEvent(Volt::Event& e)
{
	Volt::EventDispatcher dispatcher{ e };
	dispatcher.Dispatch<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(GameLayer::OnUpdateEvent));
	dispatcher.Dispatch<Volt::AppRenderEvent>(VT_BIND_EVENT_FN(GameLayer::OnRenderEvent));
	dispatcher.Dispatch<Volt::WindowResizeEvent>(VT_BIND_EVENT_FN(GameLayer::OnWindowResizeEvent));
	dispatcher.Dispatch<Volt::OnSceneTransitionEvent>(VT_BIND_EVENT_FN(GameLayer::OnSceneTransition));
	dispatcher.Dispatch<Volt::OnSceneLoadedEvent>(VT_BIND_EVENT_FN(GameLayer::OnSceneLoaded));
	dispatcher.Dispatch<Volt::OnGameStateChangedEvent>(VT_BIND_EVENT_FN(GameLayer::OnGameStateChanged));

	myScene->OnEvent(e);
}

void GameLayer::LoadStartScene()
{
	myScene->OnRuntimeStart();
	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);
}

bool GameLayer::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	if (!isPaused && !myIsLoadingScene)
	{
		myScene->Update(e.GetTimestep());
		myNavigationsSystem->OnRuntimeUpdate(e.GetTimestep());
	}

	if (myShouldLoadNewScene)
	{
		TransitionToNewScene();
	}

	if (myIsLoadingScene && Volt::Scene::IsSceneFullyLoaded(myScene->path))
	{
		myIsLoadingScene = false;

		Volt::OnSceneLoadedEvent loadEvent{ myScene };
		Volt::Application::Get().OnEvent(loadEvent);

		myScene->OnRuntimeStart();

		Volt::OnScenePlayEvent playEvent{};
		Volt::Application::Get().OnEvent(playEvent);

		Volt::ViewportResizeEvent resizeEvent{ 0, 0, myLastWidth, myLastHeight };
		myScene->OnEvent(resizeEvent);
	}

	return false;
}

bool GameLayer::OnRenderEvent(Volt::AppRenderEvent& e)
{
	if (!myIsLoadingScene)
	{
		mySceneRenderer->OnRenderRuntime();
	}
	return false;
}

bool GameLayer::OnWindowResizeEvent(Volt::WindowResizeEvent& e)
{
	mySceneRenderer->Resize(e.GetWidth(), e.GetHeight());
	myScene->SetRenderSize(e.GetWidth(), e.GetHeight());

	Volt::ViewportResizeEvent resizeEvent{ e.GetX(), e.GetY(), e.GetWidth(), e.GetHeight() };

	return false;
}

bool GameLayer::OnSceneTransition(Volt::OnSceneTransitionEvent& e)
{
	myStoredScene = Volt::AssetManager::GetAsset<Volt::Scene>(e.GetHandle());
	myShouldLoadNewScene = true;

	return true;
}

bool GameLayer::OnSceneLoaded(Volt::OnSceneLoadedEvent& e)
{
	// AI
	return false;
}

bool GameLayer::OnGameStateChanged(Volt::OnGameStateChangedEvent& e)
{
	isPaused = e.GetState();
	return false;
}

void GameLayer::TransitionToNewScene()
{
	myScene->OnRuntimeEnd();

	Volt::AssetManager::Get().Unload(myScene->handle);

	myLastWidth = mySceneRenderer->GetFinalFramebuffer()->GetWidth();
	myLastHeight = mySceneRenderer->GetFinalFramebuffer()->GetHeight();

	myScene = myStoredScene;
	mySceneRenderer = CreateRef<Volt::SceneRenderer>(myScene);

	mySceneRenderer->Resize(myLastWidth, myLastHeight);
	myScene->SetRenderSize(myLastWidth, myLastHeight);

	myNavigationsSystem->OnSceneLoad();

	Volt::Scene::PreloadSceneAssets(myScene->path);
	myIsLoadingScene = true;
	myShouldLoadNewScene = false;
}