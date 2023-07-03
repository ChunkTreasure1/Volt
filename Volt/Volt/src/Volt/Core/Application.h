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

	class ImGuiImplementation;
	class SteamImplementation;
	class AssetManager;
	class Application
	{
	public:
		Application(const ApplicationInfo& info = ApplicationInfo());
		virtual ~Application();

		void Run();
		void OnEvent(Event& event);

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

		inline Window& GetWindow() const { return *myWindow; }
		inline static Application& Get() { return *myInstance; }
		inline static ThreadPool& GetThreadPool() { return Get().myThreadPool; }
		inline static ThreadPool& GetRenderThreadPool() { return Get().myRenderThreadPool; }

		inline const bool IsRuntime() const { return myInfo.isRuntime; }
		inline const ApplicationInfo& GetInfo() const { return myInfo; }

		inline const float GetAverageFrameTime() const { return myFrameTimer.GetAverageTime(); }
		inline const float GetMaxFrameTime() const { return myFrameTimer.GetMaxFrameTime(); }

		inline void SetTimeScale(const float aTimeScale) { myTimeScale = aTimeScale; }

		NetHandler& GetNetHandler() { return *myNetHandler; }
		AI::NavigationSystem& GetNavigationSystem() { return *myNavigationSystem; }
		SteamImplementation& GetSteam() { return *mySteamImplementation; }

	private:
		bool OnAppUpdateEvent(AppUpdateEvent& e);
		bool OnWindowCloseEvent(WindowCloseEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);
		bool OnViewportResizeEvent(ViewportResizeEvent& e);
		bool OnKeyPressedEvent(KeyPressedEvent& e);

		void SetupWindowPreferences(WindowProperties& windowProperties);
		
		bool myIsRunning = false;
		bool myIsMinimized = false;
		bool myHasSentMouseMovedEvent = false;

		float myTimeScale = 1.f;
		float myCurrentDeltaTime = 0.f;
		float myLastTotalTime = 0.f;

		ApplicationInfo myInfo;
		inline static Application* myInstance;

		LayerStack myLayerStack;

		MultiTimer myFrameTimer;

		Scope<AssetManager> myAssetManager;
		Scope<Window> myWindow;
		Scope<ImGuiImplementation> myImGuiImplementation;
		Scope<NetHandler> myNetHandler;
		Scope<AI::NavigationSystem> myNavigationSystem;

		Scope<SteamImplementation> mySteamImplementation;

		ThreadPool myThreadPool;
		ThreadPool myRenderThreadPool;
	};

	static Application* CreateApplication(const std::filesystem::path& appPath);
}
