#pragma once

#include "Volt-Core/Layer/LayerStack.h"
#include "Volt-Core/MultiTimer.h"

#include "Volt-Core/Version.h"
#include "Volt-Core/Project/ProjectManager.h"

#include <SubSystem/SubSystemManager.h>

#include <AssetSystem/SourceAssetManager.h>

#include <EventSystem/EventListener.h>
#include <WindowModule/WindowMode.h>
#include <JobSystem/JobSystem.h>

#include <EntitySystem/Scripting/ScriptingSystem.h>

#include <CoreUtilities/Pointers/RefPtr.h>

namespace Amp
{
	class AudioManager;

	enum class WindowMode : uint32_t;
}

class Log;

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
		bool UseTitlebar = true;
		bool UseCustomTitlebar = false;

		Version version = VT_VERSION;
	};

	class SteamImplementation;
	class AssetManager;
	class PluginRegistry;
	class PluginSystem;
	class DynamicLibraryManager;
	class EventSystem;
	class Input;
	class WindowManager;

	namespace RHI
	{
		class ImGuiImplementation;
		class GraphicsContext;
		class RHIProxy;
	}

	class Application;

	class ApplicationEventListener : public EventListener
	{
	public:
		ApplicationEventListener(Application& application);

	private:
		bool OnAppUpdateEvent(class AppUpdateEvent& e);
		bool OnWindowCloseEvent(class WindowCloseEvent& e);
		bool OnWindowResizeEvent(class WindowResizeEvent& e);
		bool OnViewportResizeEvent(class ViewportResizeEvent& e);
		bool OnKeyPressedEvent(class KeyPressedEvent& e);

		Application& m_application;
	};

	class Application
	{
	public:
		Application(const ApplicationInfo& info = ApplicationInfo());
		virtual ~Application();

		void Run();

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

		inline static Application& Get() { return *s_instance; }

		inline static const uint64_t GetFrameIndex() { return Get().m_frameIndex; }

		inline const bool IsRuntime() const { return m_info.isRuntime; }
		inline const ApplicationInfo& GetInfo() const { return m_info; }

		inline const float GetAverageFrameTime() const { return m_frameTimer.GetAverageTime(); }
		inline const float GetMaxFrameTime() const { return m_frameTimer.GetMaxFrameTime(); }

		AI::NavigationSystem& GetNavigationSystem() { return *m_navigationSystem; }
		SteamImplementation& GetSteam() { return *m_steamImplementation; }

	private:
		friend class ApplicationEventListener;

		void InitializeMainThread();
		void MainUpdate();
		void CreateGraphicsContext();

		bool OnAppUpdateEvent(class AppUpdateEvent& e);
		bool OnWindowCloseEvent(class WindowCloseEvent& e);
		bool OnWindowResizeEvent(class WindowResizeEvent& e);
		bool OnViewportResizeEvent(class ViewportResizeEvent& e);
		bool OnKeyPressedEvent(class KeyPressedEvent& e);

		inline static Application* s_instance = nullptr;

		bool m_isRunning = false;
		bool m_isMinimized = false;
		bool m_hasSentMouseMovedEvent = false;

		float m_currentDeltaTime = 0.f;
		float m_lastTotalTime = 0.f;

		uint64_t m_frameIndex = 0;

		ApplicationInfo m_info;

		LayerStack m_layerStack;
		MultiTimer m_frameTimer;

		Scope<SubSystemManager> m_subSystemManager;

		Scope<ScriptingSystem> m_scriptingSystem;

		Scope<ApplicationEventListener> m_eventListener;

		RefPtr<RHI::ImGuiImplementation> m_imguiImplementation;
		RefPtr<RHI::GraphicsContext> m_graphicsContext;
		RefPtr<RHI::RHIProxy> m_rhiProxy;

		Scope<AssetManager> m_assetManager;
		Scope<SourceAssetManager> m_sourceAssetManager;
		Scope<AI::NavigationSystem> m_navigationSystem;

		ProjectManager* m_projectManager = nullptr;
		PluginRegistry* m_pluginRegistry = nullptr;
		PluginSystem* m_pluginSystem = nullptr;
		WindowManager* m_windowManager = nullptr;

		Scope<SteamImplementation> m_steamImplementation;
	};

	static Application* CreateApplication(const std::filesystem::path& appPath);
}
