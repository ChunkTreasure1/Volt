#pragma once

#include "Volt/Events/Event.h"
#include "Volt/Core/Base.h"

#include <functional>
#include <Windows.h>

#include <glm/glm.hpp>

struct GLFWwindow;
struct GLFWcursor;

namespace Volt
{
	namespace RHI
	{
		class GraphicsContext;
		class Swapchain;
	}

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
		{
		}

		std::string title;
		uint32_t width;
		uint32_t height;
		bool vsync;
		WindowMode windowMode;
		std::filesystem::path iconPath;
		std::filesystem::path cursorPath;
	};

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
		void SetViewportSize(uint32_t width, uint32_t height);

		void SetEventCallback(const EventCallbackFn& callback);
		void SetWindowMode(WindowMode aWindowMode, bool first = false);

		void SetVsync(bool aState);

		void Maximize() const;
		void Minimize() const;
		void Restore() const;
		bool IsFocused() const;

		const bool IsMaximized() const;

		void SetCursor(const std::filesystem::path& path);

		void SetOpacity(float opacity) const;
		std::string GetClipboard() const;
		void SetClipboard(const std::string& string);

		const std::pair<float, float> GetPosition() const;

		const float GetOpacity() const;

		inline const uint32_t GetWidth() const { return m_data.width; }
		inline const uint32_t GetHeight() const { return m_data.height; }
		inline const uint32_t GetViewportWidth() const { return m_viewportWidth; }
		inline const uint32_t GetViewportHeight() const { return m_viewportHeight; }

		inline const bool IsVSync() const { return m_data.vsync; }
		inline const WindowMode GetWindowMode() const { return m_data.windowMode; }
		inline GLFWwindow* GetNativeWindow() const { return m_window; }
		inline HWND GetHWND() const { return m_windowHandle; }

		inline const RHI::Swapchain& GetSwapchain() const { return *m_swapchain; }

		static Scope<Window> Create(const WindowProperties& aProperties = WindowProperties());

	private:
		GLFWwindow* m_window = nullptr;
		HWND m_windowHandle = nullptr;
		bool m_hasBeenInitialized = false;
		bool m_isFullscreen = false;

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

		} m_data;

		Ref<RHI::GraphicsContext> m_graphicsContext;
		Ref<RHI::Swapchain> m_swapchain;

		glm::uvec2 m_startPosition = 0;
		glm::uvec2 m_startSize = 0;

		uint32_t m_viewportWidth = 0;
		uint32_t m_viewportHeight = 0;

		WindowProperties m_properties;
		std::unordered_map<std::filesystem::path, GLFWcursor*> m_cursors;
	};
}
