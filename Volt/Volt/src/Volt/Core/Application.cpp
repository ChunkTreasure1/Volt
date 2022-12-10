#include "vtpch.h"
#include "Application.h"

#include "Volt/Asset/AssetManager.h"
#include "Volt/Asset/Mesh/MaterialRegistry.h"

#include "Volt/Animation/AnimationManager.h"

#include "Volt/Core/Window.h"
#include "Volt/Core/Graphics/Swapchain.h"
#include "Volt/Core/Profiling.h"
#include "Volt/Core/Layer/Layer.h"
#include "Volt/ImGui/ImGuiImplementation.h"

#include "Volt/Rendering/Renderer.h"
#include "Volt/Rendering/Buffer/ConstantBufferRegistry.h"
#include "Volt/Rendering/Shader/ShaderRegistry.h"

#include "Volt/Scripting/Mono/MonoScriptEngine.h"

#include "Volt/Physics/Physics.h"

#include "Volt/Utility/FileSystem.h"

#include <Amp/AudioManager/AudioManager.h>


#include <GLFW/glfw3.h>
#include <imgui.h>

namespace Volt
{
	Application::Application(const ApplicationInfo& info)
	{
		VT_CORE_ASSERT(!myInstance, "Application already exists!");
		myInstance = this;

		// Set working directory
		if (FileSystem::HasEnvironmentVariable("VOLT_PATH"))
		{
			const std::string pathEnv = FileSystem::GetEnvVariable("VOLT_PATH");
			if (!pathEnv.empty())
			{
				std::filesystem::current_path(pathEnv);
			}
		}

		myInfo = info;
		Log::Initialize();

		WindowProperties windowProperties{};
		windowProperties.width = info.width;
		windowProperties.height = info.height;
		windowProperties.vsync = info.useVSync;
		windowProperties.title = info.title;
		windowProperties.windowMode = info.windowMode;
		windowProperties.iconPath = info.iconPath;
		windowProperties.cursorPath = info.cursorPath;

		myWindow = Window::Create(windowProperties);
		myWindow->SetEventCallback(VT_BIND_EVENT_FN(Application::OnEvent));

		if (!myInfo.isRuntime)
		{
			myWindow->SetOpacity(0.f);
		}

		myAssetManager = CreateScope<AssetManager>();

		ConstantBufferRegistry::Initialize();
		Renderer::InitializeBuffers();
		ShaderRegistry::Initialize();
		MaterialRegistry::Initialize();
		Renderer::Initialize();

#ifdef VT_ENABLE_MONO	
		MonoScriptEngine::Initialize();
#endif

		Physics::LoadSettings();
		Physics::Initialize();
		Physics::LoadLayers();

		//Init AudioEngine
		{
			Amp::InitInsturct instruct;
			instruct.aFileDirectory = "Assets/Audio/Banks";
			instruct.aMasterbank = "Master.bank";
			instruct.aMasterStringsBank = "Master.strings.bank";
			Amp::AudioManager::Init(instruct);
		}

		if (info.enableImGui)
		{
			myImGuiImplementation = ImGuiImplementation::Create();
		}

		if (!myInfo.isRuntime)
		{
			myShouldFancyOpen = true;
		}
	}

	Application::~Application()
	{
		myLayerStack.Clear();
		myImGuiImplementation = nullptr;

		Physics::SaveLayers();
		Physics::Shutdown();
		Physics::SaveSettings();

#ifdef VT_ENABLE_MONO	
		MonoScriptEngine::Shutdown();
#endif

		Renderer::Shutdown();
		MaterialRegistry::Shutdown();
		ShaderRegistry::Shutdown();
		ConstantBufferRegistry::Shutdown();
		Log::Shutdown();
		Amp::AudioManager::Shutdown();


		myAssetManager = nullptr;
		myWindow = nullptr;
		myInstance = nullptr;
	}

	void Application::Run()
	{
		VT_PROFILE_THREAD("Main");

		myIsRunning = true;

		while (myIsRunning)
		{
			VT_PROFILE_FRAME("Frame");
			myWindow->BeginFrame();

			float time = (float)glfwGetTime();
			myCurrentFrameTime = time - myLastFrameTime;
			myLastFrameTime = time;

			if (myShouldFancyClose)
			{
				myFancyCloseTimer -= myCurrentFrameTime;
				if (myFancyCloseTimer >= 0.f)
				{
					myWindow->SetOpacity(gem::lerp(0.f, 1.f, myFancyCloseTimer / myFancyCloseTime));
				}

				if (myFancyCloseTimer <= 0.f)
				{
					myIsRunning = false;
				}
			}

			if (myShouldFancyOpen && myCurrentFrameTime < 0.1f)
			{
				myFancyOpenTimer -= myCurrentFrameTime;
				if (myFancyOpenTimer >= 0.f)
				{
					myWindow->SetOpacity(gem::lerp(1.f, 0.f, myFancyOpenTimer / myFancyOpenTime));
				}

				if (myFancyOpenTimer <= 0.f)
				{
					myWindow->SetOpacity(1.f);
					myShouldFancyOpen = false;
				}
			}
			
			{
				VT_PROFILE_SCOPE("Application::Update");

				AppUpdateEvent updateEvent(myCurrentFrameTime);
				OnEvent(updateEvent);
			}

			{
				VT_PROFILE_SCOPE("Application::Render");

				AppRenderEvent renderEvent;
				OnEvent(renderEvent);
			}

			myWindow->GetSwapchain().Bind();

			if (myInfo.enableImGui)
			{
				VT_PROFILE_SCOPE("Application::ImGui")

				myImGuiImplementation->Begin();

				AppImGuiUpdateEvent imguiEvent{};
				OnEvent(imguiEvent);

				myImGuiImplementation->End();
			}

			myWindow->Present();
		}
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(VT_BIND_EVENT_FN(Application::OnWindowCloseEvent));
		dispatcher.Dispatch<WindowResizeEvent>(VT_BIND_EVENT_FN(Application::OnWindowResizeEvent));

		for (auto layer : myLayerStack)
		{
			layer->OnEvent(event);
			if (event.handled)
			{
				break;
			}
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		myLayerStack.PushLayer(layer);
	}

	void Application::PopLayer(Layer* layer)
	{
		myLayerStack.PopLayer(layer);
	}

	bool Application::OnWindowCloseEvent(WindowCloseEvent&)
	{
		if (!myInfo.isRuntime)
		{
			myShouldFancyClose = true;
		}
		else
		{
			myIsRunning = false;
		}

		return false;
	}

	bool Application::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			myIsMinimized = true;
			return false;
		}

		myIsMinimized = false;
		myWindow->Resize(e.GetWidth(), e.GetHeight());
		return false;
	}
}