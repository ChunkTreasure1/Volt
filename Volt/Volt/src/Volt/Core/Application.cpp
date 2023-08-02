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

#include <VoltRHI/ImGui/ImGuiImplementation.h>

///////////////// TEMPORARY //////////////////
#include <VoltRHI/Shader/ShaderCompiler.h>
//////////////////////////////////////////////

#include <Amp/AudioManager/AudioManager.h>
#include <Amp/WwiseAudioManager/WwiseAudioManager.h>
#include <Amp/WWiseEngine/WWiseEngine.h>
#include <Navigation/Core/NavigationSystem.h>

#include <GLFW/glfw3.h>
#include <imgui.h>

namespace Volt
{
	Application::Application(const ApplicationInfo& info)
		: m_frameTimer(100)
	{
		VT_CORE_ASSERT(!m_instance, "Application already exists!");
		m_instance = this;

		m_info = info;
		Log::Initialize();
		Noise::Initialize();

		if (!m_info.isRuntime)
		{
			ProjectManager::SetupWorkingDirectory();
		}

		ProjectManager::SetupProject(m_info.projectPath);
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

		m_window = Window::Create(windowProperties);
		m_window->SetEventCallback(VT_BIND_EVENT_FN(Application::OnEvent));

		FileSystem::Initialize();

		m_threadPool.Initialize(std::thread::hardware_concurrency());
		m_renderThreadPool.Initialize(std::thread::hardware_concurrency() / 2);
		m_assetmanager = CreateScope<AssetManager>();

		///// TEMPORARY /////
		{
			RHI::ShaderCompilerCreateInfo shaderCompilerInfo{};
			shaderCompilerInfo.flags = RHI::ShaderCompilerFlags::WarningsAsErrors;
			shaderCompilerInfo.includeDirectories =
			{
				"Engine/Shaders/Source/Includes",
				"Engine/Shaders/Source/HLSL",
				"Engine/Shaders/Source/HLSL/Includes",
				ProjectManager::GetAssetsDirectory()
			};


			m_shaderCompiler = RHI::ShaderCompiler::Create(shaderCompilerInfo);
		}
		/////////////////////

		//Renderer::Initialize();
		//ShaderRegistry::Initialize();
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
			RHI::ImGuiCreateInfo createInfo{};
			createInfo.swapchain = m_window->GetSwapchainPtr();
			createInfo.window = m_window->GetNativeWindow();

			m_imguiImplementation = RHI::ImGuiImplementation::Create(createInfo);
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
		//GraphicsContextVolt::GetDevice()->WaitForIdle();

		m_navigationSystem = nullptr;
		m_layerStack.Clear();
		m_imguiImplementation = nullptr;
		SceneManager::Shutdown();

		m_layerStack.Clear();

		Physics::SaveLayers();
		Physics::Shutdown();
		Physics::SaveSettings();

		MonoScriptEngine::Shutdown();

		//DebugRenderer::Shutdown();
		//UIRenderer::Shutdown();

		//ShaderRegistry::Shutdown();
		//Renderer::Shutdown();

		//Amp::AudioManager::Shutdown();
		Amp::WWiseEngine::Get().TermWwise();

		m_assetmanager = nullptr;
		m_threadPool.Shutdown();
		m_renderThreadPool.Shutdown();

		//Renderer::FlushResourceQueues();

		FileSystem::Shutdown();

		m_window = nullptr;
		m_instance = nullptr;
		Log::Shutdown();
	}

	void Application::Run()
	{
		VT_PROFILE_THREAD("Main");

		m_isRunning = true;

		while (m_isRunning)
		{
			VT_PROFILE_FRAME("Frame");

			m_hasSentMouseMovedEvent = false;

			m_window->BeginFrame();

			float time = (float)glfwGetTime();
			m_currentDeltaTime = time - m_lastTotalTime;
			m_lastTotalTime = time;

			{
				VT_PROFILE_SCOPE("Application::Update");

				AppUpdateEvent updateEvent(m_currentDeltaTime * m_timeScale);
				OnEvent(updateEvent);
				Amp::WWiseEngine::Get().Update();
			}

			{
				VT_PROFILE_SCOPE("Application::Render");

				//Renderer::Flush();
				//Renderer::UpdateDescriptors();

				AppRenderEvent renderEvent;
				OnEvent(renderEvent);
			}


			if (m_info.enableImGui)
			{
				VT_PROFILE_SCOPE("Application::ImGui");

				m_imguiImplementation->Begin();

				ImGui::ShowDemoWindow();

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

			m_window->Present();

			m_frameTimer.Accumulate();
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

		m_window->Resize(e.GetWidth(), e.GetHeight());
		return false;
	}

	bool Application::OnViewportResizeEvent(ViewportResizeEvent& e)
	{
		m_window->SetViewportSize(e.GetWidth(), e.GetHeight());
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
