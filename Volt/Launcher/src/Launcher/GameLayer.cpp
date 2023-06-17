#include "GameLayer.h"

#include <Volt/Scene/Scene.h>
#include <Volt/Scene/SceneManager.h>
#include <Volt/Core/Graphics/Swapchain.h>

#include <Volt/Events/SettingsEvent.h>

#include <Volt/Asset/AssetManager.h>

#include <Volt/Rendering/SceneRenderer.h>
#include <Volt/Rendering/Texture/Image2D.h>

#include <Volt/Utility/ImageUtility.h>

#include <Navigation/Core/NavigationSystem.h>

#include <Volt/Core/Application.h>
#include <Volt/Scripting/Mono/MonoScriptEngine.h>

#include <Volt/Input/KeyCodes.h>

#include <yaml-cpp/yaml.h>

void GameLayer::OnAttach()
{
	const auto& startScenePath = Volt::ProjectManager::GetProject().startScenePath;

	if (startScenePath.empty())
	{
		throw std::runtime_error("Start scene has not been set!");
	}

	myScene = Volt::AssetManager::GetAsset<Volt::Scene>(startScenePath);
	if (!myScene || !myScene->IsValid())
	{
		throw std::runtime_error("Start scene is not a valid scene!");
	}

	// Scene Renderer
	{
		Volt::SceneRendererSpecification spec{};
		spec.debugName = "Main Renderer";
		spec.scene = myScene;

		spec.initialResolution = { Volt::Application::Get().GetWindow().GetWidth(), Volt::Application::Get().GetWindow().GetHeight() };


		Volt::SceneRendererSettings settings{}; //= LoadGraphicSettings();
		settings.enableUI = true;
		settings.enablePostProcessing = true;
		settings.enableVolumetricFog = true;

		mySceneRenderer = CreateRef<Volt::SceneRenderer>(spec, settings);

		myScene->SetRenderSize(spec.initialResolution.x, spec.initialResolution.y);
	}
}

void GameLayer::OnDetach()
{
	myScene->OnRuntimeEnd();

	// #TODO: Remove
	if (Volt::Application::Get().GetNetHandler().IsRunning())
		Volt::Application::Get().GetNetHandler().Stop();

	mySceneRenderer = nullptr;
	myScene = nullptr;
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

	dispatcher.Dispatch<Volt::OnRenderScaleChangedEvent>([&](Volt::OnRenderScaleChangedEvent& e)
	{
		mySceneRenderer->GetSettings().renderScale = e.GetRenderScale();
		mySceneRenderer->ApplySettings();

		return true;
	});

	dispatcher.Dispatch<Volt::KeyPressedEvent>([&](Volt::KeyPressedEvent& e) 
	{
		if (e.GetKeyCode() == VT_KEY_F6)
		{
			Volt::RenderMode renderMode = mySceneRenderer->GetCurrentRenderMode();
			(*(uint32_t*)&renderMode)++;

			if (renderMode == Volt::RenderMode::COUNT)
			{
				renderMode = Volt::RenderMode::Default;
			}

			mySceneRenderer->SetRenderMode(renderMode);
		}

		return false;
	});

	myScene->OnEvent(e);
}

void GameLayer::LoadStartScene()
{
	Volt::SceneManager::SetActiveScene(myScene);

	Volt::OnSceneLoadedEvent loadEvent{ myScene };
	Volt::Application::Get().OnEvent(loadEvent);

	myScene->OnRuntimeStart();
	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);

	// #TODO: Remove
	if (!Volt::Application::Get().GetNetHandler().IsRunning())
		Volt::Application::Get().GetNetHandler().StartSinglePlayer();
}

bool GameLayer::OnUpdateEvent(Volt::AppUpdateEvent& e)
{
	if (!isPaused)
	{
		myScene->Update(e.GetTimestep());
	}

	if (myShouldLoadNewScene)
	{
		TrySceneTransition();
	}

	return false;
}

bool GameLayer::OnRenderEvent(Volt::AppRenderEvent& e)
{
	mySceneRenderer->OnRenderRuntime();

	auto& swapchain = Volt::Application::Get().GetWindow().GetSwapchain();

	const gem::vec2ui srcSize = { mySceneRenderer->GetFinalImage()->GetWidth(), mySceneRenderer->GetFinalImage()->GetHeight() };
	const gem::vec2ui dstSize = { swapchain.GetWidth(), swapchain.GetHeight() };

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VT_VK_CHECK(vkBeginCommandBuffer(swapchain.GetCurrentCommandBuffer(), &beginInfo));

	VkImageBlit blitRegion{};
	blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.srcSubresource.mipLevel = 0;

	blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.dstSubresource.mipLevel = 0;

	blitRegion.srcOffsets[0] = { 0, 0, 0 };
	blitRegion.srcOffsets[1] = { (int32_t)srcSize.x, (int32_t)srcSize.y, 1 };

	blitRegion.dstOffsets[0] = { 0, 0, 0 };
	blitRegion.dstOffsets[1] = { (int32_t)dstSize.x, (int32_t)dstSize.y, 1 };

	VkImageSubresourceRange subResourceRange{};
	subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResourceRange.baseArrayLayer = 0;
	subResourceRange.baseMipLevel = 0;
	subResourceRange.layerCount = 1;
	subResourceRange.levelCount = 1;

	mySceneRenderer->GetFinalImage()->TransitionToLayout(swapchain.GetCurrentCommandBuffer(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	Volt::Utility::TransitionImageLayout(swapchain.GetCurrentCommandBuffer(), swapchain.GetCurrentImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subResourceRange);

	vkCmdBlitImage(swapchain.GetCurrentCommandBuffer(), mySceneRenderer->GetFinalImage()->GetHandle(), mySceneRenderer->GetFinalImage()->GetLayout(), swapchain.GetCurrentImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_NEAREST);

	Volt::Utility::TransitionImageLayout(swapchain.GetCurrentCommandBuffer(), swapchain.GetCurrentImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, subResourceRange);

	vkEndCommandBuffer(swapchain.GetCurrentCommandBuffer());

	return false;
}

bool GameLayer::OnWindowResizeEvent(Volt::WindowResizeEvent& e)
{
	mySceneRenderer->Resize(e.GetWidth(), e.GetHeight());
	myScene->SetRenderSize(e.GetWidth(), e.GetHeight());

	Volt::ViewportResizeEvent resizeEvent{ e.GetX(), e.GetY(), e.GetWidth(), e.GetHeight() };

	//myScene->OnEvent(resizeEvent);
	Volt::Application::Get().OnEvent(resizeEvent);

	return false;
}

bool GameLayer::OnSceneTransition(Volt::OnSceneTransitionEvent& e)
{
	myStoredScene = Volt::AssetManager::QueueAsset<Volt::Scene>(e.GetHandle());
	Volt::Scene::PreloadSceneAssets(myStoredScene->path);

	myShouldLoadNewScene = true;

	return true;
}

bool GameLayer::OnSceneLoaded(Volt::OnSceneLoadedEvent& e)
{
	Volt::MonoScriptEngine::OnSceneLoaded();
	return false;
}

bool GameLayer::OnGameStateChanged(Volt::OnGameStateChangedEvent& e)
{
	isPaused = e.GetState();
	return false;
}

Volt::SceneRendererSettings GameLayer::LoadGraphicSettings()
{
	Volt::SceneRendererSettings settings{};

	std::filesystem::path path = Volt::ProjectManager::GetDirectory() / "Assets/Settings/GameSettings.yaml";
	std::ifstream file(path);
	std::stringstream sstream;
	sstream << file.rdbuf();

	YAML::Node root = YAML::Load(sstream.str());

	if (root["Shadows"])
	{
		settings.enableShadows = root["Shadows"].as<bool>();
	}
	if (root["AO"])
	{
		settings.enableAO = root["AO"].as<bool>();
	}
	if (root["Bloom"])
	{
		settings.enableBloom = root["Bloom"].as<bool>();
	}
	if (root["AA"])
	{
		settings.enableAntiAliasing = root["AA"].as<bool>();
	}
	if (root["RenderScale"])
	{
		settings.renderScale = root["RenderScale"].as<float>();
	}
	return settings;
}

void GameLayer::TrySceneTransition()
{
	myStoredScene = Volt::AssetManager::QueueAsset<Volt::Scene>(myStoredScene->handle);

	if (!myStoredScene->IsValid())
	{
		return;
	}

	if (!Volt::Scene::IsSceneFullyLoaded(myStoredScene->path))
	{
		return;
	}

	myShouldLoadNewScene = false;

	myScene->OnRuntimeEnd();

	Volt::AssetManager::Get().Unload(myScene->handle);

	if (myScene == myStoredScene)
	{
		auto sceneHandle = myStoredScene->handle;
		myScene = nullptr;

		myScene = Volt::AssetManager::GetAsset<Volt::Scene>(sceneHandle);
	}
	else
	{
		myScene = myStoredScene;
	}

	myStoredScene = nullptr;

	myLastWidth = mySceneRenderer->GetOriginalSize().x;
	myLastHeight = mySceneRenderer->GetOriginalSize().y;

	// Scene Renderer
	{
		Volt::SceneRendererSpecification spec{};
		spec.debugName = "Main Renderer";
		spec.scene = myScene;
		spec.initialResolution = { myLastWidth, myLastHeight };

		Volt::SceneRendererSettings settings = mySceneRenderer->GetSettings();
		mySceneRenderer = CreateRef<Volt::SceneRenderer>(spec, settings);
	}

	myScene->SetRenderSize(myLastWidth, myLastHeight);

	Volt::SceneManager::SetActiveScene(myScene);

	Volt::OnSceneLoadedEvent loadEvent{ myScene };
	Volt::Application::Get().OnEvent(loadEvent);

	myScene->OnRuntimeStart();

	Volt::OnScenePlayEvent playEvent{};
	Volt::Application::Get().OnEvent(playEvent);

	Volt::ViewportResizeEvent resizeEvent{ 0, 0, myLastWidth, myLastHeight };
	myScene->OnEvent(resizeEvent);
}
