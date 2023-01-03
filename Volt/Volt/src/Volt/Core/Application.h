#pragma once

#include "Volt/Core/Window.h"
#include "Volt/Core/Base.h"
#include "Volt/Core/Layer/LayerStack.h"

#include "Volt/Core/Layer/LayerStack.h"
#include "Volt/Events/ApplicationEvent.h"

#include <string>

namespace Amp 
{
	class AudioManager;
}


namespace Volt
{
	struct ApplicationInfo
	{
		ApplicationInfo(const std::string& aTitle = "Volt", WindowMode aWindowMode = WindowMode::Windowed, uint32_t aWidth = 1280, uint32_t aHeight = 720, bool aUseVSync = true, bool aEnableImGui = true)
			: title(aTitle), width(aWidth), height(aHeight), useVSync(aUseVSync), enableImGui(aEnableImGui), windowMode(aWindowMode)
		{}

		std::string title;
		std::filesystem::path iconPath;
		std::filesystem::path cursorPath;
		std::filesystem::path projectPath;
		WindowMode windowMode;
		uint32_t width;
		uint32_t height;
		bool useVSync;
		bool enableImGui;
		bool isRuntime = false;

		std::string version = "1.0";
	};

	class ImGuiImplementation;
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

		inline const bool IsRuntime() const { return myInfo.isRuntime; }
		inline const ApplicationInfo& GetInfo() const { return myInfo; }

	private:
		bool OnWindowCloseEvent(WindowCloseEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);

		bool myIsRunning = false;
		bool myIsMinimized = false;

		bool myShouldFancyOpen = false;
		bool myShouldFancyClose = false;
		
		float myFancyCloseTimer = 0.5f;
		const float myFancyCloseTime = 0.5f;

		float myFancyOpenTimer = 0.5f;
		const float myFancyOpenTime = 0.5f;

		float myCurrentFrameTime = 0.f;
		float myLastFrameTime = 0.f;

		ApplicationInfo myInfo;
		inline static Application* myInstance;

		LayerStack myLayerStack;

		Scope<AssetManager> myAssetManager;
		Scope<Window> myWindow;
		Scope<ImGuiImplementation> myImGuiImplementation;
	};

	static Application* CreateApplication(const std::filesystem::path& appPath);
}