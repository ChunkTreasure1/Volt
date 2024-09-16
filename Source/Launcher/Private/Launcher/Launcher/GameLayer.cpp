#include "Launcher/GameLayer.h"

#include <Volt/Scene/Scene.h>
#include <Volt/Scene/SceneManager.h>

#include <AssetSystem/AssetManager.h>

#include <Volt/Rendering/SceneRenderer.h>

#include <Navigation/Core/NavigationSystem.h>

#include <Volt/Core/Application.h>
#include <Volt/Project/ProjectManager.h>

#include <InputModule/InputCodes.h>

#include <LogModule/Log.h>

#include <WindowModule/Events/WindowEvents.h>
#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

#include <InputModule/Events/KeyboardEvents.h>

#include <EventSystem/EventSystem.h>

#include <yaml-cpp/yaml.h>

void GameLayer::OnAttach()
{
	RegisterListener<Volt::AppUpdateEvent>(VT_BIND_EVENT_FN(GameLayer::OnUpdateEvent));
	RegisterListener<Volt::WindowRenderEvent>(VT_BIND_EVENT_FN(GameLayer::OnRenderEvent));
	RegisterListener<Volt::WindowResizeEvent>(VT_BIND_EVENT_FN(GameLayer::OnWindowResizeEvent));
	RegisterListener<Volt::OnSceneTransitionEvent>(VT_BIND_EVENT_FN(GameLayer::OnSceneTransition));
	RegisterListener<Volt::OnSceneLoadedEvent>(VT_BIND_EVENT_FN(GameLayer::OnSceneLoaded));

	const auto& startScenePath = Volt::ProjectManager::GetProject().startSceneFilepath;

	if (startScenePath.empty())
	{
		throw std::runtime_error("Start scene has not been set!");
	}

	m_scene = Volt::AssetManager::GetAsset<Volt::Scene>(startScenePath);
	if (!m_scene || !m_scene->IsValid())
	{
		throw std::runtime_error("Start scene is not a valid scene!");
	}

}

void GameLayer::OnDetach()
{
	m_scene->OnRuntimeEnd();

	m_sceneRenderer = nullptr;
	m_scene = nullptr;
}

void GameLayer::LoadStartScene()
{
	Volt::SceneManager::SetActiveScene(m_scene);

	Volt::OnSceneLoadedEvent loadEvent{ m_scene };
	Volt::EventSystem::DispatchEvent(loadEvent);

	m_scene->OnRuntimeStart();
	Volt::OnScenePlayEvent playEvent{};
	Volt::EventSystem::DispatchEvent(playEvent);

}

bool GameLayer::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	if (!m_isPaused)
	{
		m_scene->Update(e.GetTimestep());
	}

	if (m_shouldLoadNewScene)
	{
		TrySceneTransition();
	}

	return false;
}

bool GameLayer::OnRenderEvent(Volt::WindowRenderEvent& e)
{
	//mySceneRenderer->OnRenderRuntime();

	//auto& swapchain = Volt::Application::Get().GetWindow().GetSwapchain();

	//const glm::uvec2 srcSize = { mySceneRenderer->GetFinalImage()->GetWidth(), mySceneRenderer->GetFinalImage()->GetHeight() };
	////const glm::uvec2 dstSize = { swapchain.GetWidth(), swapchain.GetHeight() };

	//VkCommandBufferBeginInfo beginInfo{};
	//beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//beginInfo.pNext = nullptr;
	//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	////VT_VK_CHECK(vkBeginCommandBuffer(swapchain.GetCurrentCommandBuffer(), &beginInfo));

	//VkImageBlit blitRegion{};
	//blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//blitRegion.srcSubresource.baseArrayLayer = 0;
	//blitRegion.srcSubresource.layerCount = 1;
	//blitRegion.srcSubresource.mipLevel = 0;

	//blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//blitRegion.dstSubresource.baseArrayLayer = 0;
	//blitRegion.dstSubresource.layerCount = 1;
	//blitRegion.dstSubresource.mipLevel = 0;

	//blitRegion.srcOffsets[0] = { 0, 0, 0 };
	//blitRegion.srcOffsets[1] = { (int32_t)srcSize.x, (int32_t)srcSize.y, 1 };

	//blitRegion.dstOffsets[0] = { 0, 0, 0 };
	////blitRegion.dstOffsets[1] = { (int32_t)dstSize.x, (int32_t)dstSize.y, 1 };

	//VkImageSubresourceRange subResourceRange{};
	//subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//subResourceRange.baseArrayLayer = 0;
	//subResourceRange.baseMipLevel = 0;
	//subResourceRange.layerCount = 1;
	//subResourceRange.levelCount = 1;

	//mySceneRenderer->GetFinalImage()->TransitionToLayout(swapchain.GetCurrentCommandBuffer(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	//Volt::Utility::TransitionImageLayout(swapchain.GetCurrentCommandBuffer(), swapchain.GetCurrentImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subResourceRange);

	//vkCmdBlitImage(swapchain.GetCurrentCommandBuffer(), mySceneRenderer->GetFinalImage()->GetHandle(), mySceneRenderer->GetFinalImage()->GetLayout(), swapchain.GetCurrentImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_NEAREST);

	//Volt::Utility::TransitionImageLayout(swapchain.GetCurrentCommandBuffer(), swapchain.GetCurrentImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, subResourceRange);

	//vkEndCommandBuffer(swapchain.GetCurrentCommandBuffer());

	return false;
}

bool GameLayer::OnWindowResizeEvent(Volt::WindowResizeEvent& e)
{
	m_sceneRenderer->Resize(e.GetWidth(), e.GetHeight());
	m_scene->SetRenderSize(e.GetWidth(), e.GetHeight());

	Volt::ViewportResizeEvent resizeEvent{ e.GetX(), e.GetY(), e.GetWidth(), e.GetHeight() };

	//myScene->OnEvent(resizeEvent);
	Volt::EventSystem::DispatchEvent(resizeEvent);

	return false;
}

bool GameLayer::OnSceneTransition(Volt::OnSceneTransitionEvent& e)
{
	m_storedScene = Volt::AssetManager::QueueAsset<Volt::Scene>(e.GetHandle());
	Volt::Scene::PreloadSceneAssets(Volt::AssetManager::GetFilePathFromAssetHandle(m_storedScene->handle));

	m_shouldLoadNewScene = true;

	return true;
}

bool GameLayer::OnSceneLoaded(Volt::OnSceneLoadedEvent& e)
{
	return false;
}

//bool GameLayer::OnGameStateChanged(Volt::OnGameStateChangedEvent& e)
//{
//	m_isPaused = e.GetState();
//	return false;
//}

void GameLayer::TrySceneTransition()
{
	m_storedScene = Volt::AssetManager::QueueAsset<Volt::Scene>(m_storedScene->handle);

	if (!m_storedScene->IsValid())
	{
		return;
	}

	if (!Volt::Scene::IsSceneFullyLoaded(Volt::AssetManager::GetFilePathFromAssetHandle(m_storedScene->handle)))
	{
		return;
	}

	m_shouldLoadNewScene = false;

	m_scene->OnRuntimeEnd();

	Volt::AssetManager::Get().Unload(m_scene->handle);

	if (m_scene == m_storedScene)
	{
		auto sceneHandle = m_storedScene->handle;
		m_scene = nullptr;

		m_scene = Volt::AssetManager::GetAsset<Volt::Scene>(sceneHandle);
	}
	else
	{
		m_scene = m_storedScene;
	}

	m_storedScene = nullptr;

	//myLastWidth = mySceneRenderer->GetOriginalSize().x;
	//myLastHeight = mySceneRenderer->GetOriginalSize().y;

	// Scene Renderer
	{
		Volt::SceneRendererSpecification spec{};
		spec.debugName = "Main Renderer";
		spec.scene = m_scene;
		spec.initialResolution = { m_lastWidth, m_lastHeight };

		//Volt::SceneRendererSettings settings = mySceneRenderer->GetSettings();
		m_sceneRenderer = CreateRef<Volt::SceneRenderer>(spec);
	}

	m_scene->SetRenderSize(m_lastWidth, m_lastHeight);

	Volt::SceneManager::SetActiveScene(m_scene);

	Volt::OnSceneLoadedEvent loadEvent{ m_scene };
	Volt::EventSystem::DispatchEvent(loadEvent);

	m_scene->OnRuntimeStart();

	Volt::OnScenePlayEvent playEvent{};
	Volt::EventSystem::DispatchEvent(playEvent);

	Volt::ViewportResizeEvent resizeEvent{ 0, 0, m_lastWidth, m_lastHeight };
	Volt::EventSystem::DispatchEvent(resizeEvent);
}
