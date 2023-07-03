#include "vtpch.h"
#include "Application.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Prefab.h"

#include "Volt/Input/Input.h"
#include "Volt/Input/KeyCodes.h"

#include "Volt/Core/Window.h"
#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/SwapchainVolt.h"
#include "Volt/Core/Graphics/GraphicsDeviceVolt.h"
#include "Volt/Core/Profiling.h"
#include "Volt/Core/Layer/Layer.h"
#include "Volt/ImGui/ImGuiImplementation.h"
#include "Volt/Steam/SteamImplementation.h"

#include "Volt/Rendering/UIRenderer.h"
#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/DebugRenderer.h"
#include "Volt/Rendering/Buffer/UniformBufferSet.h"
#include "Volt/Rendering/Buffer/UniformBuffer.h"

#include "Volt/Rendering/RenderPipeline/ShaderRegistry.h"

#include "Volt/Core/ScopedTimer.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Project/ProjectManager.h"
#include "Volt/Project/SessionPreferences.h"
#include "Volt/Scene/SceneManager.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Utility/FileSystem.h"
#include "Volt/Discord/DiscordSDK.h"

#include "Volt/Utility/Noise.h"

#include <VoltRHI/Graphics/GraphicsContext.h>

#include <Amp/AudioManager/AudioManager.h>
#include <Amp/WwiseAudioManager/WwiseAudioManager.h>
#include <Amp/WWiseEngine/WWiseEngine.h>
#include <Navigation/Core/NavigationSystem.h>

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace Volt
{
	Application::Application(const ApplicationInfo& info)
		: myFrameTimer(100)
	{
		VT_CORE_ASSERT(!myInstance, "Application already exists!");
		myInstance = this;

		myInfo = info;
		Log::Initialize();
		Noise::Initialize();

		if (!myInfo.isRuntime)
		{
			ProjectManager::SetupWorkingDirectory();
		}

		GraphicsContextCreateInfo cinfo{};
		cinfo.graphicsApi = GraphicsAPI::Mock;
		Ref<GraphicsContext> context = GraphicsContext::Create(cinfo);

		ProjectManager::SetupProject(myInfo.projectPath);
		SessionPreferences::Initialize();

		WindowProperties windowProperties{};
		windowProperties.width = info.width;
		windowProperties.height = info.height;
		windowProperties.vsync = info.useVSync;
		windowProperties.title = info.title;
		windowProperties.windowMode = info.windowMode;
		windowProperties.iconPath = info.iconPath;
		windowProperties.cursorPath = info.cursorPath;

		SetupWindowPreferences(windowProperties);

		if (myInfo.isRuntime)
		{
			windowProperties.title = ProjectManager::GetProject().name;
			windowProperties.cursorPath = ProjectManager::GetProject().cursorPath;
			windowProperties.iconPath = ProjectManager::GetProject().iconPath;
		}

		myWindow = Window::Create(windowProperties);
		myWindow->SetEventCallback(VT_BIND_EVENT_FN(Application::OnEvent));

		FileSystem::Initialize();

		myThreadPool.Initialize(std::thread::hardware_concurrency());
		myRenderThreadPool.Initialize(std::thread::hardware_concurrency() / 2);
		myAssetManager = CreateScope<AssetManager>();

		Renderer::Initialize();
		ShaderRegistry::Initialize();
		Renderer::LateInitialize();

		UIRenderer::Initialize();
		DebugRenderer::Initialize();

		MonoScriptEngine::Initialize();
		Prefab::PreloadAllPrefabs();

		Physics::LoadSettings();
		Physics::Initialize();
		Physics::LoadLayers();

		//Init AudioEngine
		{
			std::filesystem::path defaultPath = ProjectManager::GetAudioBanksDirectory();
			Amp::WWiseEngine::Get().InitWWise(defaultPath.c_str());
			if (FileSystem::Exists(defaultPath))
			{
				for (auto bankFile : std::filesystem::directory_iterator(ProjectManager::GetAudioBanksDirectory()))
				{
					if (bankFile.path().extension() == L".bnk")
					{
						Amp::WWiseEngine::Get().LoadBank(bankFile.path().filename().string().c_str());
					}
				}
			}
		}

		if (info.enableImGui)
		{
			myImGuiImplementation = ImGuiImplementation::Create();
		}

		if (info.netEnabled)
		{
			myNetHandler = CreateScope<Volt::NetHandler>();
		}

		myNavigationSystem = CreateScope<Volt::AI::NavigationSystem>();

		// Extras

		if (info.enableSteam)
		{
			mySteamImplementation = SteamImplementation::Create();
		}
	}

	Application::~Application()
	{
		GraphicsContextVolt::GetDevice()->WaitForIdle();

		myNavigationSystem = nullptr;
		myLayerStack.Clear();
		myImGuiImplementation = nullptr;
		SceneManager::Shutdown();

		myLayerStack.Clear();

		Physics::SaveLayers();
		Physics::Shutdown();
		Physics::SaveSettings();

		MonoScriptEngine::Shutdown();

		DebugRenderer::Shutdown();
		UIRenderer::Shutdown();

		ShaderRegistry::Shutdown();
		Renderer::Shutdown();

		//Amp::AudioManager::Shutdown();
		Amp::WWiseEngine::Get().TermWwise();

		myAssetManager = nullptr;
		myThreadPool.Shutdown();
		myRenderThreadPool.Shutdown();

		Renderer::FlushResourceQueues();

		FileSystem::Shutdown();

		myWindow = nullptr;
		myInstance = nullptr;
		Log::Shutdown();
	}

	void Application::Run()
	{
		VT_PROFILE_THREAD("Main");

		myIsRunning = true;

		while (myIsRunning)
		{
			VT_PROFILE_FRAME("Frame");

			myHasSentMouseMovedEvent = false;

			myWindow->BeginFrame();

			float time = (float)glfwGetTime();
			myCurrentDeltaTime = time - myLastTotalTime;
			myLastTotalTime = time;

			{
				VT_PROFILE_SCOPE("Application::Update");

				AppUpdateEvent updateEvent(myCurrentDeltaTime * myTimeScale);
				OnEvent(updateEvent);
				Amp::WWiseEngine::Get().Update();
			}

			{
				VT_PROFILE_SCOPE("Application::Render");

				Renderer::Flush();
				Renderer::UpdateDescriptors();

				AppRenderEvent renderEvent;
				OnEvent(renderEvent);
			}


			if (myInfo.enableImGui)
			{
				VT_PROFILE_SCOPE("Application::ImGui");

				myImGuiImplementation->Begin();

				AppImGuiUpdateEvent imguiEvent{};
				OnEvent(imguiEvent);

				myImGuiImplementation->End();
			}

			if (myInfo.netEnabled)
			{
				VT_PROFILE_SCOPE("Application::Net");
				myNetHandler->Update(myCurrentDeltaTime);
			}

			{
				VT_PROFILE_SCOPE("Discord SDK");
				DiscordSDK::Update();
			}

			myWindow->Present();

			myFrameTimer.Accumulate();
		}
	}

	void Application::OnEvent(Event& event)
	{
		VT_PROFILE_SCOPE((std::string("Application::OnEvent: ") + std::string(event.GetName())).c_str());

		if (event.GetEventType() == MouseMoved)
		{
			if (myHasSentMouseMovedEvent)
			{
				return;
			}
			else
			{
				myHasSentMouseMovedEvent = true;
			}
		}

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<AppUpdateEvent>(VT_BIND_EVENT_FN(Application::OnAppUpdateEvent));
		dispatcher.Dispatch<WindowCloseEvent>(VT_BIND_EVENT_FN(Application::OnWindowCloseEvent));
		dispatcher.Dispatch<WindowResizeEvent>(VT_BIND_EVENT_FN(Application::OnWindowResizeEvent));
		dispatcher.Dispatch<ViewportResizeEvent>(VT_BIND_EVENT_FN(Application::OnViewportResizeEvent));
		dispatcher.Dispatch<KeyPressedEvent>(VT_BIND_EVENT_FN(Application::OnKeyPressedEvent));

		myNetHandler->OnEvent(event);

		if (myNavigationSystem)
		{
			myNavigationSystem->OnEvent(event);
		}

		for (auto layer : myLayerStack)
		{
			layer->OnEvent(event);
			if (event.handled)
			{
				break;
			}
		}
		Input::OnEvent(event);
	}

	void Application::PushLayer(Layer* layer)
	{
		myLayerStack.PushLayer(layer);
	}

	void Application::PopLayer(Layer* layer)
	{
		myLayerStack.PopLayer(layer);
	}

	bool Application::OnAppUpdateEvent(AppUpdateEvent&)
	{
		if (mySteamImplementation)
		{
			mySteamImplementation->Update();
		}
		return false;
	}

	bool Application::OnWindowCloseEvent(WindowCloseEvent&)
	{
		myIsRunning = false;
		return false;
	}

	bool Application::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			myIsMinimized = true;
		}
		else
		{
			myIsMinimized = false;
		}

		myWindow->Resize(e.GetWidth(), e.GetHeight());
		return false;
	}

	bool Application::OnViewportResizeEvent(ViewportResizeEvent& e)
	{
		myWindow->SetViewportSize(e.GetWidth(), e.GetHeight());
		return false;
	}

	bool Application::OnKeyPressedEvent(KeyPressedEvent&)
	{
		return false;
	}

	void Application::SetupWindowPreferences(WindowProperties& windowProperties)
	{
		if (SessionPreferences::HasKey("WINDOW_WIDTH"))
		{
			windowProperties.width = static_cast<uint32_t>(SessionPreferences::GetInt("WINDOW_WIDTH"));
		}

		if (SessionPreferences::HasKey("WINDOW_HEIGHT"))
		{
			windowProperties.height = static_cast<uint32_t>(SessionPreferences::GetInt("WINDOW_HEIGHT"));
		}

		if (SessionPreferences::HasKey("WINDOW_MODE"))
		{
			windowProperties.windowMode = (WindowMode)static_cast<uint32_t>(SessionPreferences::GetInt("WINDOW_MODE"));
		}
	}
}
