#include "vtpch.h"
#include "WindowManager.h"

#include "Volt/Core/Window.h"

namespace Volt
{
	const WindowHandle WindowManager::CreateNewWindow(const WindowProperties& windowProperties)
	{
		Scope<Window> window = Window::Create(windowProperties);
		WindowHandle handle{};
	
		m_windows[handle] = std::move(window);
		return handle;
	}

	void WindowManager::DestroyWindow(const WindowHandle handle)
	{
		if (m_windows.contains(handle))
		{
			m_windows.erase(handle);
		}
	}

	void WindowManager::BeginFrame()
	{
		for (const auto& [handle, window] : m_windows)
		{
			window->BeginFrame();
		}
	}

	void WindowManager::Present()
	{
		for (const auto& [handle, window] : m_windows)
		{
			window->Present();
		}
	}

	Window& WindowManager::GetWindow(const WindowHandle handle) const
	{
		return *m_windows.at(handle);
	}
}
