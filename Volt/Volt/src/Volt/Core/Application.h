#pragma once

#include "Volt/Net/NetHandler.h"

#include "Volt/Core/Window.h"
#include "Volt/Core/Base.h"
#include "Volt/Core/Layer/LayerStack.h"
#include "Volt/Core/MultiTimer.h"

#include "Volt/Core/Threading/ThreadPool.h"

#include "Volt/Core/Layer/LayerStack.h"
#include "Volt/Core/WindowManager.h"

#include "Volt/Utility/Version.h"

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

		Version version = VT_VERSION;
	};

	class SteamImplementation;
	class AssetManager;

	namespace RHI
	{
		class ImGuiImplementation;
		class GraphicsContext;
		class RHIProxy;
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

		Window& GetWindow() const;
		inline static Application& Get() { return *s_instance; }
		inline static ThreadPool& GetThreadPool() { return Get().m_threadPool; }
		inline static WindowManager& GetWindowManager() { return Get().m_windowManager; }

		inline const bool IsRuntime() const { return m_info.isRuntime; }
		inline const ApplicationInfo& GetInfo() const { return m_info; }

		inline const float GetAverageFrameTime() const { return m_frameTimer.GetAverageTime(); }
		inline const float GetMaxFrameTime() const { return m_frameTimer.GetMaxFrameTime(); }

		inline void SetTimeScale(const float aTimeScale) { m_timeScale = aTimeScale; }

		NetHandler& GetNetHandler() { return *m_netHandler; }
		AI::NavigationSystem& GetNavigationSystem() { return *m_navigationSystem; }
		SteamImplementation& GetSteam() { return *m_steamImplementation; }

	private:
		void MainUpdate();
		void CreateGraphicsContext();

		bool OnAppUpdateEvent(AppUpdateEvent& e);
		bool OnWindowCloseEvent(WindowCloseEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);
		bool OnViewportResizeEvent(ViewportResizeEvent& e);
		bool OnKeyPressedEvent(KeyPressedEvent& e);

		void SetupWindowPreferences(WindowProperties& windowProperties);
		
		inline static Application* s_instance = nullptr;

		bool m_isRunning = false;
		bool m_isMinimized = false;
		bool m_hasSentMouseMovedEvent = false;

		float m_timeScale = 1.f;
		float m_currentDeltaTime = 0.f;
		float m_lastTotalTime = 0.f;

		ApplicationInfo m_info;

		LayerStack m_layerStack;
		MultiTimer m_frameTimer;
		WindowManager m_windowManager;

		Ref<RHI::ImGuiImplementation> m_imguiImplementation;
		Ref<RHI::GraphicsContext> m_graphicsContext;
		Ref<RHI::RHIProxy> m_rhiProxy;

		WindowHandle m_windowHandle = 0;
		Scope<AssetManager> m_assetmanager;
		Scope<NetHandler> m_netHandler;
		Scope<AI::NavigationSystem> m_navigationSystem;

		Scope<SteamImplementation> m_steamImplementation;

		ThreadPool m_threadPool;
	};

	static Application* CreateApplication(const std::filesystem::path& appPath);
}
