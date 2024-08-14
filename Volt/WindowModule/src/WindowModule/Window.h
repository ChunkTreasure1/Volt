#pragma once

#include "WindowMode.h"
#include "WindowProperties.h"

#include "WindowModule/Config.h"

#include <RHIModule/Graphics/Swapchain.h>

#include <CoreUtilities/Pointers/RefPtr.h>
#include <CoreUtilities/Pointers/WeakPtr.h>

#include <functional>
#include <Windows.h>

#include <glm/glm.hpp>


struct GLFWwindow;
struct GLFWcursor;

namespace Volt
{
	class Event;

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		WINDOWMODULE_API Window(const WindowProperties& aProperties);
		WINDOWMODULE_API ~Window();

		WINDOWMODULE_API void Shutdown();

		WINDOWMODULE_API void Invalidate();
		WINDOWMODULE_API void Release();

		WINDOWMODULE_API void BeginFrame();
		WINDOWMODULE_API void Render();
		WINDOWMODULE_API void Present();

		WINDOWMODULE_API void Resize(uint32_t aWidth, uint32_t aHeight);
		WINDOWMODULE_API void SetViewportSize(uint32_t width, uint32_t height);

		WINDOWMODULE_API void SetEventCallback(const EventCallbackFn& callback);
		WINDOWMODULE_API void SetWindowMode(WindowMode aWindowMode, bool first = false);

		WINDOWMODULE_API void SetVsync(bool aState);

		WINDOWMODULE_API void SetIcon(const std::filesystem::path& path);

		WINDOWMODULE_API void Maximize() const;
		WINDOWMODULE_API void Minimize() const;
		WINDOWMODULE_API void Restore() const;
		WINDOWMODULE_API bool IsFocused() const;

		WINDOWMODULE_API const bool IsMaximized() const;

		WINDOWMODULE_API void SetCursor(const std::filesystem::path& path);

		WINDOWMODULE_API void SetOpacity(float opacity) const;
		WINDOWMODULE_API std::string GetClipboard() const;
		WINDOWMODULE_API void SetClipboard(const std::string& string);

		WINDOWMODULE_API const std::pair<float, float> GetPosition() const;

		WINDOWMODULE_API const float GetOpacity() const;
		WINDOWMODULE_API const float GetTime() const;

		WINDOWMODULE_API const std::string& GetTitle();

		WINDOWMODULE_API inline const uint32_t GetWidth() const { return m_data.Width; }
		WINDOWMODULE_API inline const uint32_t GetHeight() const { return m_data.Height; }
		WINDOWMODULE_API inline const uint32_t GetViewportWidth() const { return m_viewportWidth; }
		WINDOWMODULE_API inline const uint32_t GetViewportHeight() const { return m_viewportHeight; }

		WINDOWMODULE_API inline const bool IsVSync() const { return m_data.VSync; }
		WINDOWMODULE_API inline const WindowMode GetWindowMode() const { return m_data.WindowMode; }
		WINDOWMODULE_API inline GLFWwindow* GetNativeWindow() const { return m_window; }
		WINDOWMODULE_API inline HWND GetHWND() const { return m_windowHandle; }

		WINDOWMODULE_API inline const RHI::Swapchain& GetSwapchain() const { return *m_swapchain; }
		WINDOWMODULE_API inline const WeakPtr<RHI::Swapchain> GetSwapchainPtr() const { return m_swapchain; }

		static Scope<Window> Create(const WindowProperties& aProperties = WindowProperties());

	private:
		static bool CheckEventCallback(GLFWwindow* window);

		GLFWwindow* m_window = nullptr;
		HWND m_windowHandle = nullptr;
		bool m_hasBeenInitialized = false;
		bool m_isFullscreen = false;

		struct WindowData
		{
			std::string Title;
			std::filesystem::path IconPath;
			std::filesystem::path CursorPath;
			uint32_t Width;
			uint32_t Height;
			bool VSync;
			WindowMode WindowMode;

			EventCallbackFn EventCallback;

		} m_data;

		RefPtr<RHI::Swapchain> m_swapchain;

		glm::uvec2 m_startPosition = 0;
		glm::uvec2 m_startSize = 0;

		uint32_t m_viewportWidth = 0;
		uint32_t m_viewportHeight = 0;

		WindowProperties m_properties;
		std::unordered_map<std::filesystem::path, GLFWcursor*> m_cursors;
	};
}
