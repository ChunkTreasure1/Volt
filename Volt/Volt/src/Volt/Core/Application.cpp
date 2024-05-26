#include "vtpch.h"
#include "Application.h"

#include "Volt/Asset/AssetManager.h"

#include "Volt/Input/Input.h"
#include "Volt/Input/KeyCodes.h"

#include "Volt/Core/Window.h"
#include "Volt/Core/Layer/Layer.h"
#include "Volt/Steam/SteamImplementation.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/RenderGraph/RenderGraphExecutionThread.h"
#include "Volt/Rendering/Shader/ShaderMap.h"

#include "Volt/Core/ScopedTimer.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Project/ProjectManager.h"
#include "Volt/Project/SessionPreferences.h"
#include "Volt/Scene/SceneManager.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Utility/FileSystem.h"
#include "Volt/Discord/DiscordSDK.h"

#include "Volt/Utility/Noise.h"
#include "Volt/Utility/UIUtility.h"

#include <VoltRHI/ImGui/ImGuiImplementation.h>
#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltVulkan/VulkanRHIProxy.h>

#include <Amp/AudioManager/AudioManager.h>
#include <Amp/WwiseAudioManager/WwiseAudioManager.h>
#include <Amp/WWiseEngine/WWiseEngine.h>
#include <Navigation/Core/NavigationSystem.h>

namespace Volt
{
	inline static void RHILogCallback(RHI::LogSeverity severity, std::string_view msg)
	{
		switch (severity)
		{
			case RHI::LogSeverity::Trace:
				VT_CORE_TRACE(msg);
				break;
			case RHI::LogSeverity::Info:
				VT_CORE_INFO(msg);
				break;
			case RHI::LogSeverity::Warning:
				VT_CORE_WARN(msg);
				break;
			case RHI::LogSeverity::Error:
				VT_CORE_ERROR(msg);
				break;
		}
	}

	Application::Application(const ApplicationInfo& info)
		: m_frameTimer(100)
	{
		VT_CORE_ASSERT(!s_instance, "Application already exists!");
		s_instance = this;

		m_info = info;
		Noise::Initialize();

		Log::Initialize();
		ProjectManager::SetupProject(m_info.projectPath);
		Log::InitializeFileSinks();
		
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

		if (m_info.isRuntime)
		{
			windowProperties.title = ProjectManager::GetProject().name;
			windowProperties.cursorPath = ProjectManager::GetProject().cursorPath;
			windowProperties.iconPath = ProjectManager::GetProject().iconPath;
		}
	
		// This is required because glfwInit must be called before setting up graphics device
		Window::StaticInitialize();
		CreateGraphicsContext();

		if (ProjectManager::GetProject().isDeprecated)
		{
			windowProperties.useTitlebar = true;
		}

		// Setup main window
		{
			m_windowHandle = m_windowManager.CreateNewWindow(windowProperties);

			auto& window = m_windowManager.GetWindow(m_windowHandle);
			window.SetEventCallback(VT_BIND_EVENT_FN(Application::OnEvent));
		}

		FileSystem::Initialize();
		
		m_threadPool.Initialize(std::thread::hardware_concurrency() / 2);

		Renderer::PreInitialize();
		m_assetmanager = CreateScope<AssetManager>();
		
		ShaderMap::Initialize();
		Renderer::Initialize();

		//Renderer::Initialize();
		//Renderer::LateInitialize();

		//UIRenderer::Initialize();
		//DebugRenderer::Initialize();

		MonoScriptEngine::Initialize();

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
			auto& window = m_windowManager.GetWindow(m_windowHandle);

			RHI::ImGuiCreateInfo createInfo{};
			createInfo.swapchain = window.GetSwapchainPtr();
			createInfo.window = window.GetNativeWindow();

			m_imguiImplementation = RHI::ImGuiImplementation::Create(createInfo);
			auto defaultFont = m_imguiImplementation->AddFont("Engine/Fonts/Inter/inter-regular.ttf", 16.f);

			UI::SetFont(FontType::Regular_12, m_imguiImplementation->AddFont("Engine/Fonts/Inter/inter-regular.ttf", 12.f));
			UI::SetFont(FontType::Regular_16, defaultFont);
			UI::SetFont(FontType::Regular_17, m_imguiImplementation->AddFont("Engine/Fonts/Inter/inter-regular.ttf", 17.f));
			UI::SetFont(FontType::Regular_20, m_imguiImplementation->AddFont("Engine/Fonts/Inter/inter-regular.ttf", 20.f));

			UI::SetFont(FontType::Bold_12, m_imguiImplementation->AddFont("Engine/Fonts/Inter/inter-bold.ttf", 12.f));
			UI::SetFont(FontType::Bold_16, m_imguiImplementation->AddFont("Engine/Fonts/Inter/inter-bold.ttf", 16.f));
			UI::SetFont(FontType::Bold_17, m_imguiImplementation->AddFont("Engine/Fonts/Inter/inter-bold.ttf", 17.f));
			UI::SetFont(FontType::Bold_20, m_imguiImplementation->AddFont("Engine/Fonts/Inter/inter-bold.ttf", 20.f));
			UI::SetFont(FontType::Bold_90, m_imguiImplementation->AddFont("Engine/Fonts/Inter/inter-bold.ttf", 90.f));

			m_imguiImplementation->SetDefaultFont(defaultFont);
			ImGui::SetCurrentContext(m_imguiImplementation->GetContext());
		}

		if (info.netEnabled)
		{
			m_netHandler = CreateScope<Volt::NetHandler>();
		}

		m_navigationSystem = CreateScope<Volt::AI::NavigationSystem>();

		// Extras

		if (info.enableSteam)
		{
			m_steamImplementation = SteamImplementation::Create();
		}
	}

	Application::~Application()
	{
		m_navigationSystem = nullptr;
		m_layerStack.Clear();
		m_imguiImplementation = nullptr;
		SceneManager::Shutdown();

		Physics::SaveLayers();
		Physics::Shutdown();
		Physics::SaveSettings();

		MonoScriptEngine::Shutdown();

		//DebugRenderer::Shutdown();
		//UIRenderer::Shutdown();

		//Renderer::Shutdown();

		//Amp::AudioManager::Shutdown();
		Amp::WWiseEngine::Get().TermWwise();

		m_assetmanager = nullptr;
		m_threadPool.Shutdown();

		ShaderMap::Shutdown();
		Renderer::Shutdown();
		FileSystem::Shutdown();

		m_windowManager.DestroyWindow(m_windowHandle);
		m_graphicsContext = nullptr;
		m_rhiProxy = nullptr;
		Window::StaticShutdown();

		s_instance = nullptr;
		Log::Shutdown();
	}

	void Application::Run()
	{
		VT_PROFILE_THREAD("Main");

		m_isRunning = true;

		while (m_isRunning)
		{
			VT_PROFILE_FRAME("Frame");
			MainUpdate();

			m_frameIndex++;
		}
	}

	void Application::OnEvent(Event& event)
	{
		VT_PROFILE_SCOPE((std::string("Application::OnEvent: ") + std::string(event.GetName())).c_str());

		if (event.GetEventType() == MouseMoved)
		{
			if (m_hasSentMouseMovedEvent)
			{
				return;
			}
			else
			{
				m_hasSentMouseMovedEvent = true;
			}
		}

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<AppUpdateEvent>(VT_BIND_EVENT_FN(Application::OnAppUpdateEvent));
		dispatcher.Dispatch<WindowCloseEvent>(VT_BIND_EVENT_FN(Application::OnWindowCloseEvent));
		dispatcher.Dispatch<WindowResizeEvent>(VT_BIND_EVENT_FN(Application::OnWindowResizeEvent));
		dispatcher.Dispatch<ViewportResizeEvent>(VT_BIND_EVENT_FN(Application::OnViewportResizeEvent));
		dispatcher.Dispatch<KeyPressedEvent>(VT_BIND_EVENT_FN(Application::OnKeyPressedEvent));

		m_netHandler->OnEvent(event);

		if (m_navigationSystem)
		{
			m_navigationSystem->OnEvent(event);
		}

		for (auto layer : m_layerStack)
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
		m_layerStack.PushLayer(layer);
	}

	void Application::PopLayer(Layer* layer)
	{
		m_layerStack.PopLayer(layer);
	}

	Window& Application::GetWindow() const
	{
		return m_windowManager.GetWindow(m_windowHandle);
	}

	void Application::MainUpdate()
	{
		m_hasSentMouseMovedEvent = false;

		RHI::GraphicsContext::Update();

		{
			m_windowManager.BeginFrame();
			AppBeginFrameEvent beginFrameEvent{};
			OnEvent(beginFrameEvent);
		}

		float time = GetWindow().GetTime();
		m_currentDeltaTime = time - m_lastTotalTime;
		m_lastTotalTime = time;

		{
			VT_PROFILE_SCOPE("Application::Render");

			Renderer::Flush();
			AppRenderEvent renderEvent;
			OnEvent(renderEvent);
			Renderer::Update();
		}

		{
			VT_PROFILE_SCOPE("Application::Update");
			AppUpdateEvent updateEvent(m_currentDeltaTime * m_timeScale);
			OnEvent(updateEvent);

			AssetManager::Update();
		}

		{
			VT_PROFILE_SCOPE("Application::UpdateAudio");
			Amp::WWiseEngine::Get().Update();
		}

		if (m_info.enableImGui)
		{
			VT_PROFILE_SCOPE("Application::ImGui");

			m_imguiImplementation->Begin();

			AppImGuiUpdateEvent imguiEvent{};
			OnEvent(imguiEvent);

			m_imguiImplementation->End();
		}

		if (m_info.netEnabled)
		{
			VT_PROFILE_SCOPE("Application::Net");
			m_netHandler->Update(m_currentDeltaTime);
		}

		{
			VT_PROFILE_SCOPE("Discord SDK");
			DiscordSDK::Update();
		}

		RenderGraphExecutionThread::WaitForFinishedExecution();
		Renderer::EndOfFrameUpdate();

		{
			m_windowManager.Present();

			AppPresentFrameEvent presentEvent{};
			OnEvent(presentEvent);
		}

		{
			VT_PROFILE_SCOPE("Application::PostFrameUpdate");
			AppPostFrameUpdateEvent postFrameUpdateEvent{ m_currentDeltaTime * m_timeScale };
			OnEvent(postFrameUpdateEvent);
		}

		m_frameTimer.Accumulate();
	}

	void Application::CreateGraphicsContext()
	{
		RHI::GraphicsContextCreateInfo cinfo{};
		cinfo.graphicsApi = RHI::GraphicsAPI::Vulkan;

		if (cinfo.graphicsApi == RHI::GraphicsAPI::Vulkan)
		{
			m_rhiProxy = RHI::CreateVulkanRHIProxy();
		}

		{
			RHI::LogInfo logHook{};
			logHook.enabled = true;
			logHook.logCallback = RHILogCallback;

			m_rhiProxy->SetLogInfo(logHook);

			RHI::RHICallbackInfo callbackInfo{};
			callbackInfo.resourceManagementInfo.resourceDeletionCallback = Renderer::DestroyResource;
			callbackInfo.requestCloseEventCallback = []() 
			{
				WindowCloseEvent closeEvent{};
				Application::Get().OnEvent(closeEvent);
			};

			m_rhiProxy->SetRHICallbackInfo(callbackInfo);
		}

		m_graphicsContext = RHI::GraphicsContext::Create(cinfo);
	}

	bool Application::OnAppUpdateEvent(AppUpdateEvent&)
	{
		if (m_steamImplementation)
		{
			m_steamImplementation->Update();
		}
		return false;
	}

	bool Application::OnWindowCloseEvent(WindowCloseEvent&)
	{
		m_isRunning = false;
		return false;
	}

	bool Application::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_isMinimized = true;
		}
		else
		{
			m_isMinimized = false;
		}

		GetWindow().Resize(e.GetWidth(), e.GetHeight());

		MainUpdate();

		return false;
	}

	bool Application::OnViewportResizeEvent(ViewportResizeEvent& e)
	{
		GetWindow().SetViewportSize(e.GetWidth(), e.GetHeight());
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
