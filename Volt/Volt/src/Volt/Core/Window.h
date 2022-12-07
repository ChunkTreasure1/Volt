#pragma once

#include "Volt/Events/Event.h"
#include "Volt/Core/Base.h"

#include <functional>
#include <Windows.h>

#include <GEM/gem.h>

struct GLFWwindow;
struct GLFWcursor;

namespace Volt
{
	enum class WindowMode : uint32_t
	{
		Windowed = 0,
		Fullscreen,
		Borderless
	};

	struct WindowProperties
	{
		WindowProperties(const std::string& aTitle = "Volt", uint32_t aWidth = 1280, uint32_t aHeight = 720, bool aVSync = true, WindowMode aWindowMode = WindowMode::Windowed)
			: title(aTitle), width(aWidth), height(aHeight), vsync(aVSync), windowMode(aWindowMode)
		{}

		std::string title;
		uint32_t width;
		uint32_t height;
		bool vsync;
		WindowMode windowMode;
		std::filesystem::path iconPath;
		std::filesystem::path cursorPath;
	};

	class GraphicsContext;
	class Swapchain;

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window(const WindowProperties& aProperties);
		~Window();

		void Shutdown();

		void Invalidate();
		void Release();

		void BeginFrame();
		void Present();

		void Resize(uint32_t aWidth, uint32_t aHeight);
		void SetEventCallback(const EventCallbackFn& callback);
		void SetWindowMode(WindowMode aWindowMode);

		void Maximize() const;
		void Minimize() const;
		void Restore() const;

		const bool IsMaximized() const;
 
		void ShowCursor(bool aShow) const;
		void SetCursor(const std::filesystem::path& path);

		void SetOpacity(float opacity) const;

		const std::pair<float, float> GetPosition() const;

		inline const uint32_t GetWidth() const { return myData.width; }
		inline const uint32_t GetHeight() const { return myData.height; }
		inline const bool IsVSync() const { return myData.vsync; }
		inline const WindowMode GetWindowMode() const { return myData.windowMode; }
		inline GLFWwindow* GetNativeWindow() const { return myWindow; }
		inline HWND GetHWND() const { return myWindowHandle; }

		inline const Swapchain& GetSwapchain() const { return *mySwapchain; }

		static Scope<Window> Create(const WindowProperties& aProperties = WindowProperties());

	private:
		GLFWwindow* myWindow = nullptr;
		HWND myWindowHandle = nullptr;
		bool myHasBeenInitialized = false;
		bool myIsFullscreen = false;

		struct WindowData
		{
			std::string title;
			std::filesystem::path iconPath;
			std::filesystem::path cursorPath;
			uint32_t width;
			uint32_t height;
			bool vsync;
			WindowMode windowMode;

			EventCallbackFn eventCallback;

		} myData;

		Ref<GraphicsContext> myGraphicsContext;
		Ref<Swapchain> mySwapchain;

		gem::vec2ui myStartPosition = 0;
		gem::vec2ui myStartSize = 0;

		WindowProperties myProperties;
		std::unordered_map<std::filesystem::path, GLFWcursor*> myCursors;
	};
}