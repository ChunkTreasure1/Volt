#include "vtpch.h"
#include "Volt/Core/Application.h"

#include <AssetSystem/AssetManager.h>

#include <InputModule/Input.h>

#include "Volt/Core/Layer/Layer.h"
#include "Volt/Core/DynamicLibraryManager.h"
#include "Volt/Steam/SteamImplementation.h"

#include "Volt/Rendering/Renderer.h"

#include "Volt/Project/ProjectManager.h"
#include "Volt/Scene/SceneManager.h"

#include "Volt/PluginSystem/PluginRegistry.h"
#include "Volt/PluginSystem/PluginSystem.h"

#include "Volt/Physics/Physics.h"

#include "Volt/Utility/Noise.h"
#include "Volt/Utility/UIUtility.h"

#include <RenderCore/RenderGraph/RenderGraphExecutionThread.h>

#include <RHIModule/ImGui/ImGuiImplementation.h>
#include <RHIModule/Graphics/GraphicsContext.h>

#include <VulkanRHIModule/VulkanRHIProxy.h>
#include <D3D12RHIModule/D3D12RHIProxy.h>

#include <Amp/WWiseEngine/WWiseEngine.h>
#include <Navigation/Core/NavigationSystem.h>

#include <CoreUtilities/FileSystem.h>
#include <LogModule/Log.h>

#include <InputModule/Events/KeyboardEvents.h>
#include <InputModule/Events/MouseEvents.h>

#include <WindowModule/Events/WindowEvents.h>
#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

#include <EventSystem/EventSystem.h>

#include <CoreUtilities/ThreadUtilities.h>

#include <EntitySystem/Scripting/ECSEventDispatcher.h>
#include <Volt/Physics/PhysicsEvents.h>

using TestEntity = ECS::Access
	::Write<Volt::TransformComponent>
	::As<ECS::Type::Entity>;

void SystemCallback(Volt::OnCollisionEnterEvent& event)
{

}

namespace Volt
{
	ApplicationEventListener::ApplicationEventListener(Application& application)
		: m_application(application)
	{ 
		RegisterListener<AppUpdateEvent>(VT_BIND_EVENT_FN(ApplicationEventListener::OnAppUpdateEvent));
		RegisterListener<WindowCloseEvent>(VT_BIND_EVENT_FN(ApplicationEventListener::OnWindowCloseEvent));
		RegisterListener<WindowResizeEvent>(VT_BIND_EVENT_FN(ApplicationEventListener::OnWindowResizeEvent));
		RegisterListener<ViewportResizeEvent>(VT_BIND_EVENT_FN(ApplicationEventListener::OnViewportResizeEvent));
		RegisterListener<KeyPressedEvent>(VT_BIND_EVENT_FN(ApplicationEventListener::OnKeyPressedEvent));
	}

	bool ApplicationEventListener::OnAppUpdateEvent(AppUpdateEvent& e)
	{
		return m_application.OnAppUpdateEvent(e);
	}

	bool ApplicationEventListener::OnWindowCloseEvent(WindowCloseEvent& e)
	{
		return m_application.OnWindowCloseEvent(e);
	}

	bool ApplicationEventListener::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		return m_application.OnWindowResizeEvent(e);
	}

	bool ApplicationEventListener::OnViewportResizeEvent(ViewportResizeEvent& e)
	{
		return m_application.OnViewportResizeEvent(e);
	}

	bool ApplicationEventListener::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		return m_application.OnKeyPressedEvent(e);
	}

	Application::Application(const ApplicationInfo& info)
		: m_frameTimer(100), m_info(info)
	{
		VT_ASSERT_MSG(!s_instance, "Application already exists!");
		s_instance = this;

		m_eventSystem = CreateScope<EventSystem>();

		m_log = CreateScope<Log>();
		m_log->SetLogOutputFilepath(m_info.projectPath.parent_path() / "Log/Log.txt");

		m_input = CreateScope<Input>();

		m_jobSystem = CreateScope<JobSystem>();
		m_dynamicLibraryManager = CreateScope<DynamicLibraryManager>();
		m_pluginRegistry = CreateScope<PluginRegistry>();
		m_pluginSystem = CreateScope<PluginSystem>(*m_pluginRegistry);

		Noise::Initialize();

		ProjectManager::LoadProject(m_info.projectPath, *m_pluginRegistry);
		m_pluginRegistry->BuildPluginDependencies();
		m_pluginSystem->LoadPlugins(ProjectManager::GetProject());

		WindowProperties windowProperties{};
		windowProperties.Width = info.width;
		windowProperties.Height = info.height;
		windowProperties.VSync = info.useVSync;
		windowProperties.Title = info.title;
		windowProperties.WindowMode = info.windowMode;
		windowProperties.IconPath = info.iconPath;
		windowProperties.CursorPath = info.cursorPath;
		windowProperties.UseTitlebar = info.UseTitlebar;
		windowProperties.UseCustomTitlebar = info.UseCustomTitlebar;

		if (m_info.isRuntime)
		{
			windowProperties.Title = ProjectManager::GetProject().name;
			windowProperties.CursorPath = ProjectManager::GetProject().cursorFilepath;
			windowProperties.IconPath = ProjectManager::GetProject().iconFilepath;
		}

		if (ProjectManager::GetProject().isDeprecated)
		{
			windowProperties.UseTitlebar = true;
		}

		// This is required because glfwInit must be called before setting up graphics device
		WindowManager::InitializeGLFW();
		CreateGraphicsContext();
		WindowManager::Initialize(windowProperties);

		FileSystem::Initialize();
		
		Renderer::PreInitialize();
		m_assetManager = CreateScope<AssetManager>(ProjectManager::GetRootDirectory(), ProjectManager::GetAssetsDirectory(), ProjectManager::GetEngineDirectory());
		m_sourceAssetManager = CreateScope<SourceAssetManager>();
		Renderer::Initialize();

		//UIRenderer::Initialize();
		//DebugRenderer::Initialize();

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
			auto& window = WindowManager::Get().GetMainWindow();

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

		m_navigationSystem = CreateScope<Volt::AI::NavigationSystem>();

		// Extras

		if (info.enableSteam)
		{
			m_steamImplementation = SteamImplementation::Create();
		}

		m_scriptingEngine = CreateScope<ScriptingEngine>();

		m_pluginSystem->InitializePlugins();
		m_eventListener = CreateScope<ApplicationEventListener>(*this);
	}

	Application::~Application()
	{
		m_eventListener = nullptr;
		m_pluginSystem->ShutdownPlugins();

		m_scriptingEngine = nullptr;

		m_navigationSystem = nullptr;
		m_layerStack.Clear();
		m_imguiImplementation = nullptr;
		SceneManager::Shutdown();

		Physics::SaveLayers();
		Physics::Shutdown();
		Physics::SaveSettings();

		//DebugRenderer::Shutdown();
		//UIRenderer::Shutdown();

		Amp::WWiseEngine::Get().TermWwise();

		m_assetManager = nullptr;

		Renderer::Shutdown();

		FileSystem::Shutdown();
		WindowManager::Shutdown();
		m_graphicsContext = nullptr;
		WindowManager::ShutdownGLFW();
		
		m_rhiProxy = nullptr;

		m_pluginSystem->UnloadPlugins();
		m_pluginSystem = nullptr;
		m_pluginRegistry = nullptr;
		m_jobSystem = nullptr;
		m_input = nullptr;
		m_log = nullptr;
		m_eventSystem = nullptr;
		s_instance = nullptr;
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

	void Application::PushLayer(Layer* layer)
	{
		m_layerStack.PushLayer(layer);
	}

	void Application::PopLayer(Layer* layer)
	{
		m_layerStack.PopLayer(layer);
	}

	void Application::InitializeMainThread()
	{
		Thread::AssignThreadToCore(Thread::GetCurrentThreadHandle(), 0);
	}

	void Application::MainUpdate()
	{
		m_hasSentMouseMovedEvent = false;

		RHI::GraphicsContext::Update();

		WindowManager::Get().BeginFrame();

		const float time = WindowManager::Get().GetMainWindow().GetTime();
		m_currentDeltaTime = time - m_lastTotalTime;
		m_lastTotalTime = time;

		{
			VT_PROFILE_SCOPE("Application::Render");

			Renderer::Flush();
			WindowManager::Get().Render();
			Renderer::Update();
		}

		{
			VT_PROFILE_SCOPE("Application::Update");
			AppUpdateEvent updateEvent(m_currentDeltaTime);
			EventSystem::DispatchEvent(updateEvent);

			AssetManager::Update();
		}

		{
			VT_PROFILE_SCOPE("Application::UpdateAudio");
			Amp::WWiseEngine::Get().Update();
		}

		{
			VT_PROFILE_SCOPE("Application::RunMainThreadJobs");
			m_jobSystem->ExecuteMainThreadJobs();
		}

		if (m_info.enableImGui)
		{
			VT_PROFILE_SCOPE("Application::ImGui");

			m_imguiImplementation->Begin();

			AppImGuiUpdateEvent imguiEvent{};
			EventSystem::DispatchEvent(imguiEvent);

			// #TODO_Ivar: HACK! Will keep this here for now. We need to make sure that the scene renderer output image is ready. 
			RenderGraphExecutionThread::WaitForFinishedExecution();
			m_imguiImplementation->End();
		}
		else
		{
			RenderGraphExecutionThread::WaitForFinishedExecution();
		}

		Renderer::EndOfFrameUpdate();

		{
			WindowManager::Get().Present();
		}

		{
			VT_PROFILE_SCOPE("Application::PostFrameUpdate");
			AppPostFrameUpdateEvent postFrameUpdateEvent{ m_currentDeltaTime };
			EventSystem::DispatchEvent(postFrameUpdateEvent);
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
		else if (cinfo.graphicsApi == RHI::GraphicsAPI::D3D12)
		{
			m_rhiProxy = RHI::CreateD3D12RHIProxy();
		}

		{
			RHI::RHICallbackInfo callbackInfo{};
			callbackInfo.resourceManagementInfo.resourceDeletionCallback = Renderer::DestroyResource;
			callbackInfo.requestCloseEventCallback = []()
			{
				WindowCloseEvent closeEvent{};
				EventSystem::DispatchEvent(closeEvent);
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

		WindowManager::Get().GetMainWindow().Resize(e.GetWidth(), e.GetHeight());

		MainUpdate();

		return false;
	}

	bool Application::OnViewportResizeEvent(ViewportResizeEvent& e)
	{
		WindowManager::Get().GetMainWindow().SetViewportSize(e.GetWidth(), e.GetHeight());
		return false;
	}

	bool Application::OnKeyPressedEvent(KeyPressedEvent&)
	{
		return false;
	}
}
