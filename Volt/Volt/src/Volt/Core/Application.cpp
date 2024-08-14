#include "vtpch.h"
#include "Application.h"

#include <AssetSystem/AssetManager.h>

#include <InputModule/Input.h>
#include <InputModule/KeyCodes.h>

#include "Volt/Core/Layer/Layer.h"
#include "Volt/Steam/SteamImplementation.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/Shader/ShaderMap.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"
#include "Volt/Project/ProjectManager.h"
#include "Volt/Scene/SceneManager.h"

#include "Volt/Physics/Physics.h"
#include "Volt/Utility/FileSystem.h"
#include "Volt/Discord/DiscordSDK.h"

#include "Volt/Utility/Noise.h"
#include "Volt/Utility/UIUtility.h"

#include "Volt/Asset/Importers/MeshTypeImporter.h"
#include "Volt/Asset/Importers/TextureImporter.h"

#include <RenderCore/RenderGraph/RenderGraphExecutionThread.h>

#include <RHIModule/ImGui/ImGuiImplementation.h>
#include <RHIModule/Graphics/GraphicsContext.h>

#include <VulkanRHIModule/VulkanRHIProxy.h>
#include <D3D12RHIModule/D3D12RHIProxy.h>

#include <Amp/AudioManager/AudioManager.h>
#include <Amp/WwiseAudioManager/WwiseAudioManager.h>
#include <Amp/WWiseEngine/WWiseEngine.h>
#include <Navigation/Core/NavigationSystem.h>

#include <CoreUtilities/FileSystem.h>
#include <LogModule/Log.h>

#include <InputModule/Events/KeyboardEvents.h>
#include <InputModule/Events/MouseEvents.h>

#include <WindowModule/Events/WindowEvents.h>
#include <WindowModule/WindowManager.h>
#include <WindowModule/Window.h>

namespace Volt
{
	Application::Application(const ApplicationInfo& info)
		: m_frameTimer(100)
	{
		VT_ASSERT_MSG(!s_instance, "Application already exists!");
		s_instance = this;

		m_log = CreateScope<Log>();
		m_log->SetLogOutputFilepath(m_info.projectPath / "Log/Log.txt");

		m_jobSystem = CreateScope<JobSystem>();

		m_info = info;
		Noise::Initialize();

		ProjectManager::SetupProject(m_info.projectPath);

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
			windowProperties.CursorPath = ProjectManager::GetProject().cursorPath;
			windowProperties.IconPath = ProjectManager::GetProject().iconPath;
		}

		if (ProjectManager::GetProject().isDeprecated)
		{
			windowProperties.UseTitlebar = true;
		}

		// This is required because glfwInit must be called before setting up graphics device
		WindowManager::InitializeGLFW();
		CreateGraphicsContext();
		WindowManager::Initialize(windowProperties);

		WindowManager::Get().GetMainWindow().SetEventCallback(VT_BIND_EVENT_FN(Application::OnEvent));

		FileSystem::Initialize();
		
		
		MeshTypeImporter::Initialize();
		TextureImporter::Initialize();
		
		Renderer::PreInitialize();
		m_assetmanager = CreateScope<AssetManager>(ProjectManager::GetDirectory(), ProjectManager::GetAssetsDirectory(), ProjectManager::GetEngineDirectory());
		
		ShaderMap::Initialize();
		Renderer::Initialize();

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

		Amp::WWiseEngine::Get().TermWwise();

		m_assetmanager = nullptr;

		ShaderMap::Shutdown();
		Renderer::Shutdown();

		TextureImporter::Shutdown();
		MeshTypeImporter::Shutdown();

		FileSystem::Shutdown();
		WindowManager::Shutdown()
		m_graphicsContext = nullptr;
		WindowManager::ShutdownGLFW();
		
		m_rhiProxy = nullptr;

		m_jobSystem = nullptr;
		m_log = nullptr;
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

	void Application::OnEvent(Event& event)
	{
		VT_PROFILE_SCOPE((std::string("Application::OnEvent: ") + std::string(event.GetName())).c_str());

		if (event.GetGUID() == MouseMovedEvent::GetStaticGUID())
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
			if (event.IsHandled())
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
			WindowManager::Get().Present();
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
