#pragma once

#include "WindowModule/WindowHandle.h"
#include "WindowModule/WindowProperties.h"

#include "WindowModule/Config.h"

#include <CoreUtilities/Core.h>

#include <unordered_map>

namespace Volt
{
	class Window;

	class WindowManager
	{
	public:
		WINDOWMODULE_API static void InitializeGLFW();
		WINDOWMODULE_API static void Initialize(const WindowProperties& mainWindowProperties);
		WINDOWMODULE_API static void Shutdown();
		WINDOWMODULE_API static void ShutdownGLFW();

		WINDOWMODULE_API static WindowManager& Get();

		WINDOWMODULE_API const WindowHandle CreateNewWindow(const WindowProperties& windowProperties);
		WINDOWMODULE_API void DestroyWindow(const WindowHandle handle);

		WINDOWMODULE_API void BeginFrame();
		WINDOWMODULE_API void Render();
		WINDOWMODULE_API void Present();

		WINDOWMODULE_API WindowHandle GetMainWindowHandle() const;

		WINDOWMODULE_API Window& GetMainWindow() const;
		WINDOWMODULE_API Window& GetWindow(const WindowHandle handle) const;

	private:

		inline static Scope<WindowManager> s_instance;

		WindowHandle m_mainWindowHandle;
		std::unordered_map<WindowHandle, Scope<Window>> m_windows;
	};
}
