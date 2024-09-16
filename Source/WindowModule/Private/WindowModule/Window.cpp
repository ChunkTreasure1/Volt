#include "windowpch.h"
#include "Window.h"

#include "WindowLogCategory.h"

#include "Utilities/DDSUtility.h"

#include "Events/WindowEvents.h"

#include <InputModule/Events/KeyboardEvents.h>
#include <InputModule/Events/MouseEvents.h>  

#include <EventSystem/EventSystem.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <LogModule/Log.h>

namespace Volt
{
	Window::Window(const WindowProperties& properties)
	{
		m_data.Height = properties.Height;
		m_data.Width = properties.Width;
		m_data.Title = properties.Title;
		m_data.VSync = properties.VSync;
		m_data.WindowMode = properties.WindowMode;
		m_data.IconPath = properties.IconPath;
		m_data.CursorPath = properties.CursorPath;

		m_properties = properties;

		Invalidate();

		if (!m_data.CursorPath.empty())
		{
			SetCursor(m_data.CursorPath);
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
		glfwWindowHint(GLFW_TITLEBAR, (m_properties.UseTitlebar && !m_properties.UseCustomTitlebar) ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_AUTO_ICONIFY, false);

		GLFWmonitor* primaryMonitor = nullptr;

		if (m_data.WindowMode != WindowMode::Windowed)
		{
			primaryMonitor = glfwGetPrimaryMonitor();
		}

		int32_t createWidth = (uint32_t)m_data.Width;
		int32_t createHeight = (uint32_t)m_data.Height;

		if (primaryMonitor)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
			createWidth = mode->width;
			createHeight = mode->height;
		}

		m_window = glfwCreateWindow(createWidth, createHeight, m_data.Title.c_str(), primaryMonitor, nullptr);
		m_windowHandle = glfwGetWin32Window(m_window);

		if (!m_data.IconPath.empty() && std::filesystem::exists(m_data.IconPath))
		{
			SetIcon(m_data.IconPath);
		}

		bool isRawMouseMotionSupported = glfwRawMouseMotionSupported();
		if (isRawMouseMotionSupported)
		{
			glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}

		if (m_data.WindowMode == WindowMode::Fullscreen)
		{
			m_isFullscreen = true;
		}

		if (!m_hasBeenInitialized)
		{
			m_swapchain = RHI::Swapchain::Create(m_window);
			m_swapchain->Resize(m_data.Width, m_data.Height, m_data.VSync);
			m_hasBeenInitialized = true;
		}

		if (m_data.WindowMode != WindowMode::Windowed)
		{
			SetWindowMode(m_data.WindowMode, true);
		}

		glfwSetWindowUserPointer(m_window, &m_data);

		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int32_t width, int32_t height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			data.Width = width;
			data.Height = height;

			int32_t x, y;
			glfwGetWindowPos(window, &x, &y);

			WindowResizeEvent event((uint32_t)x, (uint32_t)y, width, height);
			EventSystem::DispatchEvent(event);
		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
		{
			WindowCloseEvent event{};
			EventSystem::DispatchEvent(event);
		});

		if (m_properties.UseTitlebar && m_properties.UseCustomTitlebar)
		{
			glfwSetTitlebarHitTestCallback(m_window, [](GLFWwindow* window, int x, int y, int* hit)
			{
				WindowTitlebarHittestEvent event{ x, y, *hit };
				EventSystem::DispatchEvent(event);
			});
		}

		glfwSetKeyCallback(m_window, [](GLFWwindow* window, int32_t key, int32_t, int32_t action, int32_t)
		{
			if (key == -1)
			{
				return;
			}

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					EventSystem::DispatchEvent(event);
					break;
				}

				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					EventSystem::DispatchEvent(event);
					break;
				}

				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					EventSystem::DispatchEvent(event);
					break;
				}
			}
		});		

		glfwSetCharCallback(m_window, [](GLFWwindow* window, uint32_t key)
		{
			KeyTypedEvent event(key);
			EventSystem::DispatchEvent(event);
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int)
		{
			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					EventSystem::DispatchEvent(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					EventSystem::DispatchEvent(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			EventSystem::DispatchEvent(event);
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos)
		{
			int32_t x, y;
			glfwGetWindowPos(window, &x, &y);
			MouseMovedEvent event((float)xPos + x, (float)yPos + y);
			EventSystem::DispatchEvent(event);
		});

		glfwSetDropCallback(m_window, [](GLFWwindow* window, int32_t count, const char** paths)
		{
			WindowDragDropEvent event(count, paths);
			EventSystem::DispatchEvent(event);
		});

		if (!m_properties.UseTitlebar || (m_properties.UseTitlebar && m_properties.UseCustomTitlebar))
		{
			glfwSetWindowSize(m_window, static_cast<int32_t>(createWidth + 1), static_cast<int32_t>(createHeight + 1));
			glfwSetWindowSize(m_window, static_cast<int32_t>(createWidth), static_cast<int32_t>(createHeight));
		}
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
		m_data.WindowMode = aWindowMode;

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
					//const auto [wx, wy] = GetPosition();
					//WindowResizeEvent resizeWindow{ static_cast<uint32_t>(wx), static_cast<uint32_t>(wy), static_cast<uint32_t>(mode->width), static_cast<uint32_t>(mode->height) };
					//Application::Get().OnEvent(resizeWindow);
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

				glfwSetWindowMonitor(m_window, nullptr, 0, 0, m_properties.Width, m_properties.Height, GLFW_DONT_CARE);

				const int32_t xPos = (int32_t)((mode->width / 2) - (m_properties.Width / 2));
				const int32_t yPos = (int32_t)((mode->height / 2) - (m_properties.Height / 2));

				glfwSetWindowPos(m_window, xPos, yPos);

				m_isFullscreen = false;

				if (first)
				{
					Resize(m_properties.Width, m_properties.Height);
				}
				else
				{
					/*WindowResizeEvent resizeWindow{ static_cast<uint32_t>(xPos), static_cast<uint32_t>(yPos), m_properties.width, m_properties.height };
					Application::Get().OnEvent(resizeWindow);*/
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
					//const auto [wx, wy] = GetPosition();
					/*WindowResizeEvent resizeWindow{ static_cast<uint32_t>(wx), static_cast<uint32_t>(wy), static_cast<uint32_t>(mode->width), static_cast<uint32_t>(mode->height) };
					Application::Get().OnEvent(resizeWindow);*/
				}
				break;
			}
		}
	}

	void Window::SetVsync(bool aState)
	{
		m_data.VSync = aState;
	}

	void Window::SetIcon(const std::filesystem::path& path)
	{
		m_data.IconPath = path;

		auto textureData = DDSUtility::GetRawDataFromDDS(m_data.IconPath);

		GLFWimage image;
		image.width = (int32_t)textureData.width;
		image.height = (int32_t)textureData.height;
		image.pixels = textureData.dataBuffer.As<uint8_t>();

		glfwSetWindowIcon(m_window, 1, &image);

		textureData.dataBuffer.Release();
	}

	void Window::BeginFrame()
	{
		m_swapchain->BeginFrame();
		WindowBeginFrameEvent beginFrameEvent;
		EventSystem::DispatchEvent(beginFrameEvent);
	}

	void Window::Render()
	{
		WindowRenderEvent renderEvent;
		EventSystem::DispatchEvent(renderEvent);
	}

	void Window::Present()
	{
		m_swapchain->Present();
		glfwPollEvents();

		WindowPresentFrameEvent presentFrameEvent;
		EventSystem::DispatchEvent(presentFrameEvent);
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

		m_data.Width = aWidth;
		m_data.Height = aHeight;

		/*if (Application::Get().IsRuntime())
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
		}*/

		m_swapchain->Resize(aWidth, aHeight, m_data.VSync);
	}

	void Window::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_viewportWidth = width;

		m_viewportHeight = height;
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
		opacity = std::clamp(opacity, 0.f, 1.f);
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

	WINDOWMODULE_API const std::string& Window::GetTitle()
	{
		return m_data.Title;
	}

	Scope<Window> Window::Create(const WindowProperties& aProperties)
	{
		return CreateScope<Window>(aProperties);
	}
}
