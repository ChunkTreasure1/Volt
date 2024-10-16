#include "windowpch.h"
#include "WindowManager.h"

#include "WindowLogCategory.h"

#include "Window.h"

#include <GLFW/glfw3.h>

#include <LogModule/Log.h>

namespace Volt
{
	VT_REGISTER_SUBSYSTEM(WindowManager, PreEngine, 4);

	inline static void GLFWErrorCallback(int error, const char* description)
	{
		VT_LOGC(Error, LogWindowManagement, "GLFW Error ({0}): {1}", error, description);
	}

	WindowManager::WindowManager()
	{
		VT_ASSERT(!s_instance);
		s_instance = this;
	}

	WindowManager::~WindowManager()
	{
		s_instance = nullptr;
	}

	void WindowManager::Initialize()
	{
		VT_LOGC(Trace, LogWindowManagement, "Initializing WindowManager");
	}

	void WindowManager::Shutdown()
	{
		VT_LOGC(Trace, LogWindowManagement, "Shutting down WindowManager");
	}

	void WindowManager::CreateMainWindow(const WindowProperties& windowProperties)
	{
		m_mainWindowHandle = CreateNewWindow(windowProperties);
	}

	void WindowManager::DestroyMainWindow()
	{
		DestroyWindow(m_mainWindowHandle);
	}

	WindowManager& WindowManager::Get()
	{
		return *s_instance;
	}

	const WindowHandle WindowManager::CreateNewWindow(const WindowProperties& windowProperties)
	{
		VT_LOGC(Trace, LogWindowManagement, "Creating New Window with Title: '{0}'", windowProperties.Title);
		Scope<Window> window = Window::Create(windowProperties);
		WindowHandle handle{};

		m_windows[handle] = std::move(window);
		return handle;
	}

	void WindowManager::DestroyWindow(const WindowHandle handle)
	{
		if (m_windows.contains(handle))
		{
			VT_LOGC(Trace, LogWindowManagement, "Destroying Window with Title: '{0}'", m_windows[handle]->GetTitle());
			m_windows.erase(handle);
		}
		else
		{
			VT_LOGC(Trace, LogWindowManagement, "Failed to Window with Handle: '{0}'", handle);
		}
	}

	void WindowManager::BeginFrame()
	{
		for (const auto& [handle, window] : m_windows)
		{
			window->BeginFrame();
		}
	}

	void WindowManager::Render()
	{
		for (const auto& [handle, window] : m_windows)
		{
			window->Render();
		}
	}

	void WindowManager::Present()
	{
		for (const auto& [handle, window] : m_windows)
		{
			window->Present();
		}
	}

	WindowHandle WindowManager::GetMainWindowHandle() const
	{
		return m_mainWindowHandle;
	}

	Window& WindowManager::GetMainWindow() const
	{
		return *m_windows.at(m_mainWindowHandle);
	}

	Window& WindowManager::GetWindow(const WindowHandle handle) const
	{
		return *m_windows.at(handle);
	}

	void WindowManager::InitializeGLFW()
	{
		VT_LOGC(Trace, LogWindowManagement, "Initializing GLFW");
		static bool glfwIsInitialized = false;
		if (!glfwIsInitialized)
		{
			glfwIsInitialized = true;
			if (!glfwInit())
			{
				VT_LOGC(Critical, LogWindowManagement, "Failed to initialize GLFW!");
			}

			glfwSetErrorCallback(GLFWErrorCallback);
		}
	}

	void WindowManager::ShutdownGLFW()
	{
		VT_LOGC(Trace, LogWindowManagement, "Shutting Down GLFW");
		glfwTerminate();
	}
}
