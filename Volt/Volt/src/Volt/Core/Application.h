#pragma once

#include "Volt/Net/NetHandler.h"

#include "Volt/Core/Window.h"
#include "Volt/Core/Base.h"
#include "Volt/Core/Layer/LayerStack.h"
#include "Volt/Core/MultiTimer.h"

#include "Volt/Core/Threading/ThreadPool.h"

#include "Volt/Core/Layer/LayerStack.h"

#include "Volt/Events/ApplicationEvent.h"
#include "Volt/Events/KeyEvent.h"

#include <string>

namespace Amp
{
	class AudioManager;
}

namespace Volt
{
	namespace AI
	{
		class NavigationSystem;
	}

	struct ApplicationInfo
	{
		ApplicationInfo(const std::string& aTitle = "Volt", WindowMode aWindowMode = WindowMode::Windowed, uint32_t aWidth = 1280, uint32_t aHeight = 720, bool aUseVSync = true, bool aEnableImGui = true)
			: title(aTitle), width(aWidth), height(aHeight), useVSync(aUseVSync), enableImGui(aEnableImGui), windowMode(aWindowMode)
		{
		}

		std::string title;
		std::filesystem::path iconPath;
		std::filesystem::path cursorPath;
		std::filesystem::path projectPath;
		WindowMode windowMode;
		uint32_t width;
		uint32_t height;
		bool useVSync = true;
		bool enableImGui = true;
		bool enableSteam = false;
		bool isRuntime = false;
		bool netEnabled = true;
		std::string version = "1.0";
	};

	class SteamImplementation;
	class AssetManager;

	namespace RHI
	{
		class ImGuiImplementation;
		class ShaderCompiler;
	}

	class Application
	{
	public:
		Application(const ApplicationInfo& info = ApplicationInfo());
		virtual ~Application();

		void Run();
		void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

		inline Window& GetWindow() const { return *m_window; }
		inline static Application& Get() { return *m_instance; }
		inline static ThreadPool& GetThreadPool() { return Get().m_threadPool; }
		inline static ThreadPool& GetRenderThreadPool() { return Get().m_renderThreadPool; }

		inline const bool IsRuntime() const { return m_info.isRuntime; }
		inline const ApplicationInfo& GetInfo() const { return m_info; }

		inline const float GetAverageFrameTime() const { return m_frameTimer.GetAverageTime(); }
		inline const float GetMaxFrameTime() const { return m_frameTimer.GetMaxFrameTime(); }

		inline void SetTimeScale(const float aTimeScale) { m_timeScale = aTimeScale; }

		NetHandler& GetNetHandler() { return *m_netHandler; }
		AI::NavigationSystem& GetNavigationSystem() { return *m_navigationSystem; }
		SteamImplementation& GetSteam() { return *m_steamImplementation; }

	private:
		bool OnAppUpdateEvent(AppUpdateEvent& e);
		bool OnWindowCloseEvent(WindowCloseEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);
		bool OnViewportResizeEvent(ViewportResizeEvent& e);
		bool OnKeyPressedEvent(KeyPressedEvent& e);

		void SetupWindowPreferences(WindowProperties& windowProperties);
		
		bool m_isRunning = false;
		bool m_isMinimized = false;
		bool m_hasSentMouseMovedEvent = false;

		float m_timeScale = 1.f;
		float m_currentDeltaTime = 0.f;
		float m_lastTotalTime = 0.f;

		ApplicationInfo m_info;
		inline static Application* m_instance;

		LayerStack m_layerStack;

		MultiTimer m_frameTimer;

		Ref<RHI::ImGuiImplementation> m_imguiImplementation;
		Ref<RHI::ShaderCompiler> m_shaderCompiler;
		
		Scope<AssetManager> m_assetmanager;
		Scope<Window> m_window;
		Scope<NetHandler> m_netHandler;
		Scope<AI::NavigationSystem> m_navigationSystem;

		Scope<SteamImplementation> m_steamImplementation;

		ThreadPool m_threadPool;
		ThreadPool m_renderThreadPool;
	};

	static Application* CreateApplication(const std::filesystem::path& appPath);
}
