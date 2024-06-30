#include "vtpch.h"
#include "Window.h"

#include "Volt/Core/Application.h"

#include "Volt/Events/ApplicationEvent.h"
#include "Volt/Events/KeyEvent.h"
#include "Volt/Events/MouseEvent.h"
#include "Volt/Utility/FileSystem.h"
#include "Volt/Utility/DDSUtility.h"

#include "Volt/Rendering/Renderer.h"

#include <VoltRHI/Graphics/GraphicsContext.h>
#include <VoltRHI/Graphics/Swapchain.h>

#include <stb/stb_image.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <DirectXTex/DirectXTex.h>

namespace Volt
{
	inline static void GLFWErrorCallback(int error, const char* description)
	{
		VT_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Window::Window(const WindowProperties& aProperties)
	{
		m_data.height = aProperties.height;
		m_data.width = aProperties.width;
		m_data.title = aProperties.title;
		m_data.vsync = aProperties.vsync;
		m_data.windowMode = aProperties.windowMode;
		m_data.iconPath = aProperties.iconPath;
		m_data.cursorPath = aProperties.cursorPath;

		m_properties = aProperties;

		Invalidate();

		if (!m_data.cursorPath.empty())
		{
			SetCursor(m_data.cursorPath);
		}
	}

	Window::~Window()
	{
		Shutdown();
	}

	void Window::Shutdown()
	{
		m_swapchain = nullptr;
		Release();

		for (auto [path, cursor] : m_cursors)
		{
			glfwDestroyCursor(cursor);
		}

		m_cursors.clear();
	}

	void Window::Invalidate()
	{
		if (m_window)
		{
			Release();
		}

		glfwWindowHint(GLFW_SAMPLES, 0);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_TITLEBAR, Application::Get().GetInfo().isRuntime || m_properties.useTitlebar ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_AUTO_ICONIFY, false);

		GLFWmonitor* primaryMonitor = nullptr;

		if (m_data.windowMode != WindowMode::Windowed)
		{
			primaryMonitor = glfwGetPrimaryMonitor();
		}

		int32_t createWidth = (uint32_t)m_data.width;
		int32_t createHeight = (uint32_t)m_data.height;

		if (primaryMonitor)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
			createWidth = mode->width;
			createHeight = mode->height;
		}

		m_window = glfwCreateWindow(createWidth, createHeight, m_data.title.c_str(), primaryMonitor, nullptr);
		m_windowHandle = glfwGetWin32Window(m_window);

		if (!m_data.iconPath.empty() && FileSystem::Exists(m_data.iconPath))
		{
			auto textureData = DDSUtility::GetRawDataFromDDS(m_data.iconPath);

			GLFWimage image;
			image.width = (int32_t)textureData.width;
			image.height = (int32_t)textureData.height;
			image.pixels = textureData.dataBuffer.As<uint8_t>();

			glfwSetWindowIcon(m_window, 1, &image);

			textureData.dataBuffer.Release();
		}

		bool isRawMouseMotionSupported = glfwRawMouseMotionSupported();
		if (isRawMouseMotionSupported)
		{
			glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}

		if (m_data.windowMode == WindowMode::Fullscreen)
		{
			m_isFullscreen = true;
		}

		if (!m_hasBeenInitialized)
		{
			m_swapchain = RHI::Swapchain::Create(m_window);
			m_swapchain->Resize(m_data.width, m_data.height, m_data.vsync);
			m_hasBeenInitialized = true;
		}

		if (m_data.windowMode != WindowMode::Windowed)
		{
			SetWindowMode(m_data.windowMode, true);
		}

		glfwSetWindowUserPointer(m_window, &m_data);

		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int32_t width, int32_t height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.width = width;
			data.height = height;

			int32_t x, y;
			glfwGetWindowPos(window, &x, &y);

			WindowResizeEvent event((uint32_t)x, (uint32_t)y, width, height);
			data.eventCallback(event);
		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event{};
			data.eventCallback(event);
		});

		if (!Application::Get().IsRuntime())
		{
			glfwSetTitlebarHitTestCallback(m_window, [](GLFWwindow* window, int x, int y, int* hit)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowTitlebarHittestEvent event{ x, y, *hit };
				if (data.eventCallback)
				{
					data.eventCallback(event);
				}
			});
		}

		glfwSetKeyCallback(m_window, [](GLFWwindow* window, int32_t key, int32_t, int32_t action, int32_t)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			if (key == -1)
			{
				return;
			}

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.eventCallback(event);
					break;
				}

				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.eventCallback(event);
					break;
				}

				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.eventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_window, [](GLFWwindow* window, uint32_t key)
		{
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			KeyTypedEvent event(key);
			data.eventCallback(event);
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int)
		{
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.eventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.eventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.eventCallback(event);
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseMovedEvent event((float)xPos, (float)yPos);

			data.eventCallback(event);
		});

		glfwSetDropCallback(m_window, [](GLFWwindow* window, int32_t count, const char** paths)
		{
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			WindowDragDropEvent event(count, paths);
			data.eventCallback(event);
		});
	}

	void Window::Release()
	{
		if (m_window)
		{
			glfwDestroyWindow(m_window);
			m_window = nullptr;
		}
	}

	void Window::SetWindowMode(WindowMode aWindowMode, bool first)
	{
		m_data.windowMode = aWindowMode;

		switch (aWindowMode)
		{
			case WindowMode::Fullscreen:
			{
				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

				glfwSetWindowAttrib(m_window, GLFW_DECORATED, false);
				glfwSetWindowAttrib(m_window, GLFW_TITLEBAR, false);
				glfwSetWindowAttrib(m_window, GLFW_AUTO_ICONIFY, true);
				glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, false);

				glfwWindowHint(GLFW_RED_BITS, mode->redBits);
				glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
				glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
				glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

				glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, GLFW_DONT_CARE);

				if (first)
				{
					Resize(mode->width, mode->height);
				}
				else
				{
					const auto [wx, wy] = GetPosition();
					WindowResizeEvent resizeWindow{ static_cast<uint32_t>(wx), static_cast<uint32_t>(wy), static_cast<uint32_t>(mode->width), static_cast<uint32_t>(mode->height) };
					Application::Get().OnEvent(resizeWindow);
				}

				break;
			}

			case WindowMode::Windowed:
			{
				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

				glfwSetWindowAttrib(m_window, GLFW_DECORATED, true);
				glfwSetWindowAttrib(m_window, GLFW_TITLEBAR, true);
				glfwSetWindowAttrib(m_window, GLFW_AUTO_ICONIFY, false);
				glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, true);

				glfwSetWindowMonitor(m_window, nullptr, 0, 0, m_properties.width, m_properties.height, GLFW_DONT_CARE);

				const int32_t xPos = (int32_t)((mode->width / 2) - (m_properties.width / 2));
				const int32_t yPos = (int32_t)((mode->height / 2) - (m_properties.height / 2));

				glfwSetWindowPos(m_window, xPos, yPos);

				m_isFullscreen = false;

				if (first)
				{
					Resize(m_properties.width, m_properties.height);
				}
				else
				{
					WindowResizeEvent resizeWindow{ static_cast<uint32_t>(xPos), static_cast<uint32_t>(yPos), m_properties.width, m_properties.height };
					Application::Get().OnEvent(resizeWindow);
				}
				break;
			}

			case WindowMode::Borderless:
			{
				glfwSetWindowAttrib(m_window, GLFW_DECORATED, false);
				glfwSetWindowAttrib(m_window, GLFW_TITLEBAR, false);
				glfwSetWindowAttrib(m_window, GLFW_AUTO_ICONIFY, false);
				glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, false);

				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				glfwSetWindowMonitor(m_window, nullptr, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);

				m_isFullscreen = false;

				if (first)
				{
					Resize(mode->width, mode->height);
				}
				else
				{
					const auto [wx, wy] = GetPosition();
					WindowResizeEvent resizeWindow{ static_cast<uint32_t>(wx), static_cast<uint32_t>(wy), static_cast<uint32_t>(mode->width), static_cast<uint32_t>(mode->height) };
					Application::Get().OnEvent(resizeWindow);
				}
				break;
			}
		}
	}

	void Window::SetVsync(bool aState)
	{
		m_data.vsync = aState;
	}

	void Window::BeginFrame()
	{
		m_swapchain->BeginFrame();
	}

	void Window::Present()
	{
		m_swapchain->Present();
		glfwPollEvents();
	}

	void Window::Resize(uint32_t aWidth, uint32_t aHeight)
	{
		if (aWidth == 0 && aHeight == 0)
		{
			int tempWidth = 0, tempHeight = 0;
			while (tempWidth == 0 && tempHeight == 0)
			{
				glfwGetFramebufferSize(m_window, &tempWidth, &tempHeight);
				glfwWaitEvents();
			}
		}

		m_data.width = aWidth;
		m_data.height = aHeight;

		if (Application::Get().IsRuntime())
		{
			if (m_data.windowMode == WindowMode::Windowed)
			{
				glfwSetWindowSize(m_window, static_cast<int32_t>(aWidth), static_cast<int32_t>(aHeight));
			}
			else if (m_data.windowMode == WindowMode::Fullscreen)
			{
				glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, static_cast<int32_t>(aWidth), static_cast<int32_t>(aHeight), GLFW_DONT_CARE);
			}
			else
			{
				return;
			}
		}

		m_swapchain->Resize(aWidth, aHeight, m_data.vsync);
	}

	void Window::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_viewportWidth = width;

		m_viewportHeight = height;
	}

	void Window::SetEventCallback(const EventCallbackFn& callback)
	{
		m_data.eventCallback = callback;
	}

	void Window::Maximize() const
	{
		glfwMaximizeWindow(m_window);
	}

	void Window::Minimize() const
	{
		glfwIconifyWindow(m_window);
	}

	void Window::Restore() const
	{
		glfwRestoreWindow(m_window);
	}

	bool Window::IsFocused() const
	{
		int32_t focused = glfwGetWindowAttrib(m_window, GLFW_FOCUSED);
		return focused == GLFW_FOCUSED;
	}

	const bool Window::IsMaximized() const
	{
		return (bool)glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED);
	}

	void Window::SetCursor(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			return;
		}

		if (m_cursors.contains(path))
		{
			glfwSetCursor(m_window, m_cursors.at(path));
			return;
		}

		auto textureData = DDSUtility::GetRawDataFromDDS(path);

		GLFWimage image;
		image.width = (int32_t)textureData.width;
		image.height = (int32_t)textureData.height;
		image.pixels = textureData.dataBuffer.As<uint8_t>();

		GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);
		m_cursors.emplace(path, cursor);

		textureData.dataBuffer.Release();
		glfwSetCursor(m_window, cursor);
	}

	void Window::SetOpacity(float opacity) const
	{
		glfwSetWindowOpacity(m_window, opacity);
	}

	std::string Window::GetClipboard() const
	{
		return glfwGetClipboardString(m_window);
	}

	void Window::SetClipboard(const std::string& string)
	{
		glfwSetClipboardString(m_window, string.c_str());
	}

	const std::pair<float, float> Window::GetPosition() const
	{
		int32_t x, y;
		glfwGetWindowPos(m_window, &x, &y);

		return { (float)x, (float)y };
	}

	const float Window::GetOpacity() const
	{
		return glfwGetWindowOpacity(m_window);
	}

	const float Window::GetTime() const
	{
		return static_cast<float>(glfwGetTime());
	}

	Scope<Window> Window::Create(const WindowProperties& aProperties)
	{
		return CreateScope<Window>(aProperties);
	}

	void Window::StaticInitialize()
	{
		static bool glfwIsInitialized = false;
		if (!glfwIsInitialized)
		{
			glfwIsInitialized = true;
			if (!glfwInit())
			{
				VT_CORE_ERROR("Failed to initialize GLFW!");
			}

			glfwSetErrorCallback(GLFWErrorCallback);
		}
	}

	void Window::StaticShutdown()
	{
		glfwTerminate();
	}
}
