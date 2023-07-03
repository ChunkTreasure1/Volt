#include "vtpch.h"
#include "Window.h"

#include "Volt/Core/Graphics/GraphicsContextVolt.h"
#include "Volt/Core/Graphics/SwapchainVolt.h"

#include "Volt/Core/Application.h"

#include "Volt/Events/ApplicationEvent.h"
#include "Volt/Events/KeyEvent.h"
#include "Volt/Events/MouseEvent.h"
#include "Volt/Utility/FileSystem.h"
#include "Volt/Utility/DDSUtility.h"

#include <stb/stb_image.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <DirectXTex/DirectXTex.h>

namespace Volt
{
	static void GLFWErrorCallback(int error, const char* description)
	{
		VT_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Window::Window(const WindowProperties& aProperties)
	{
		myData.height = aProperties.height;
		myData.width = aProperties.width;
		myData.title = aProperties.title;
		myData.vsync = aProperties.vsync;
		myData.windowMode = aProperties.windowMode;
		myData.iconPath = aProperties.iconPath;
		myData.cursorPath = aProperties.cursorPath;

		myProperties = aProperties;

		Invalidate();

		if (!myData.cursorPath.empty())
		{
			SetCursor(myData.cursorPath);
		}
	}

	Window::~Window()
	{
		Shutdown();
	}

	void Window::Shutdown()
	{
		mySwapchain = nullptr;
		myGraphicsContext = nullptr;
		Release();

		for (auto [path, cursor] : myCursors)
		{
			glfwDestroyCursor(cursor);
		}

		myCursors.clear();

		glfwTerminate();
	}

	void Window::Invalidate()
	{
		if (myWindow)
		{
			Release();
		}

		if (!myHasBeenInitialized)
		{
			if (!glfwInit())
			{
				VT_CORE_ERROR("Failed to initialize GLFW!");
			}
		}

		glfwSetErrorCallback(GLFWErrorCallback);
		glfwWindowHint(GLFW_SAMPLES, 0);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_TITLEBAR, Application::Get().GetInfo().isRuntime ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_AUTO_ICONIFY, false);

		GLFWmonitor* primaryMonitor = nullptr;

		if (myData.windowMode != WindowMode::Windowed)
		{
			primaryMonitor = glfwGetPrimaryMonitor();
		}

		int32_t createWidth = (uint32_t)myData.width;
		int32_t createHeight = (uint32_t)myData.height;

		if (primaryMonitor)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
			createWidth = mode->width;
			createHeight = mode->height;
		}

		myWindow = glfwCreateWindow(createWidth, createHeight, myData.title.c_str(), primaryMonitor, nullptr);
		myWindowHandle = glfwGetWin32Window(myWindow);

		if (!myData.iconPath.empty() && FileSystem::Exists(myData.iconPath))
		{
			auto textureData = DDSUtility::GetRawDataFromDDS(myData.iconPath);

			GLFWimage image;
			image.width = (int32_t)textureData.width;
			image.height = (int32_t)textureData.height;
			image.pixels = textureData.dataBuffer.As<uint8_t>();

			glfwSetWindowIcon(myWindow, 1, &image);

			textureData.dataBuffer.Release();
		}

		bool isRawMouseMotionSupported = glfwRawMouseMotionSupported();
		if (isRawMouseMotionSupported)
		{
			glfwSetInputMode(myWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}

		if (myData.windowMode == WindowMode::Fullscreen)
		{
			myIsFullscreen = true;
		}

		if (!myHasBeenInitialized)
		{
			myGraphicsContext = GraphicsContextVolt::Create();
			mySwapchain = SwapchainVolt::Create(myWindow);
			mySwapchain->Resize(myData.width, myData.height, myData.vsync);
			myHasBeenInitialized = true;
		}

		if (myData.windowMode != WindowMode::Windowed)
		{
			SetWindowMode(myData.windowMode, true);
		}

		glfwSetWindowUserPointer(myWindow, &myData);

		glfwSetWindowSizeCallback(myWindow, [](GLFWwindow* window, int32_t width, int32_t height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.width = width;
			data.height = height;

			int32_t x, y;
			glfwGetWindowPos(window, &x, &y);

			WindowResizeEvent event((uint32_t)x, (uint32_t)y, width, height);
			data.eventCallback(event);
		});

		glfwSetWindowCloseCallback(myWindow, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event{};
			data.eventCallback(event);
		});

		if (!Application::Get().IsRuntime())
		{
			glfwSetTitlebarHitTestCallback(myWindow, [](GLFWwindow* window, int x, int y, int* hit)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowTitlebarHittestEvent event{ x, y, *hit };
				if (data.eventCallback)
				{
					data.eventCallback(event);
				}
			});
		}

		glfwSetKeyCallback(myWindow, [](GLFWwindow* window, int32_t key, int32_t, int32_t action, int32_t)
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

		glfwSetCharCallback(myWindow, [](GLFWwindow* window, uint32_t key)
		{
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
			KeyTypedEvent event(key);
			data.eventCallback(event);
		});

		glfwSetMouseButtonCallback(myWindow, [](GLFWwindow* window, int button, int action, int)
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

		glfwSetScrollCallback(myWindow, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.eventCallback(event);
		});

		glfwSetCursorPosCallback(myWindow, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			MouseMovedEvent event((float)xPos, (float)yPos);

			data.eventCallback(event);
		});

		glfwSetDropCallback(myWindow, [](GLFWwindow* window, int32_t count, const char** paths)
		{
			WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

			WindowDragDropEvent event(count, paths);
			data.eventCallback(event);
		});
	}

	void Window::Release()
	{
		if (myWindow)
		{
			glfwDestroyWindow(myWindow);
			myWindow = nullptr;
		}
	}

	void Window::SetWindowMode(WindowMode aWindowMode, bool first)
	{
		myData.windowMode = aWindowMode;

		switch (aWindowMode)
		{
			case WindowMode::Fullscreen:
			{
				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

				glfwSetWindowAttrib(myWindow, GLFW_DECORATED, false);
				glfwSetWindowAttrib(myWindow, GLFW_TITLEBAR, false);
				glfwSetWindowAttrib(myWindow, GLFW_AUTO_ICONIFY, true);
				glfwSetWindowAttrib(myWindow, GLFW_RESIZABLE, false);

				glfwWindowHint(GLFW_RED_BITS, mode->redBits);
				glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
				glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
				glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

				glfwSetWindowMonitor(myWindow, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, GLFW_DONT_CARE);

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

				glfwSetWindowAttrib(myWindow, GLFW_DECORATED, true);
				glfwSetWindowAttrib(myWindow, GLFW_TITLEBAR, true);
				glfwSetWindowAttrib(myWindow, GLFW_AUTO_ICONIFY, false);
				glfwSetWindowAttrib(myWindow, GLFW_RESIZABLE, true);

				glfwSetWindowMonitor(myWindow, nullptr, 0, 0, myProperties.width, myProperties.height, GLFW_DONT_CARE);

				const int32_t xPos = (int32_t)((mode->width / 2) - (myProperties.width / 2));
				const int32_t yPos = (int32_t)((mode->height / 2) - (myProperties.height / 2));

				glfwSetWindowPos(myWindow, xPos, yPos);

				myIsFullscreen = false;

				if (first)
				{
					Resize(myProperties.width, myProperties.height);
				}
				else
				{
					WindowResizeEvent resizeWindow{ static_cast<uint32_t>(xPos), static_cast<uint32_t>(yPos), myProperties.width, myProperties.height };
					Application::Get().OnEvent(resizeWindow);
				}
				break;
			}

			case WindowMode::Borderless:
			{
				glfwSetWindowAttrib(myWindow, GLFW_DECORATED, false);
				glfwSetWindowAttrib(myWindow, GLFW_TITLEBAR, false);
				glfwSetWindowAttrib(myWindow, GLFW_AUTO_ICONIFY, false);
				glfwSetWindowAttrib(myWindow, GLFW_RESIZABLE, false);

				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				glfwSetWindowMonitor(myWindow, nullptr, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);

				myIsFullscreen = false;

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
		myData.vsync = aState;
	}

	void Window::BeginFrame()
	{
		mySwapchain->BeginFrame();
	}

	void Window::Present()
	{
		mySwapchain->Present();
		glfwPollEvents();
	}

	void Window::Resize(uint32_t aWidth, uint32_t aHeight)
	{
		if (aWidth == 0 && aHeight == 0)
		{
			int tempWidth = 0, tempHeight = 0;
			while (tempWidth == 0 && tempHeight == 0)
			{
				glfwGetFramebufferSize(myWindow, &tempWidth, &tempHeight);
				glfwWaitEvents();
			}
		}

		myData.width = aWidth;
		myData.height = aHeight;

		if (Application::Get().IsRuntime())
		{
			if (myData.windowMode == WindowMode::Windowed)
			{
				glfwSetWindowSize(myWindow, static_cast<int32_t>(aWidth), static_cast<int32_t>(aHeight));
			}
			else if (myData.windowMode == WindowMode::Fullscreen)
			{
				glfwSetWindowMonitor(myWindow, glfwGetPrimaryMonitor(), 0, 0, static_cast<int32_t>(aWidth), static_cast<int32_t>(aHeight), GLFW_DONT_CARE);
			}
			else
			{
				return;
			}
		}

		mySwapchain->Resize(aWidth, aHeight, myData.vsync);
	}

	void Window::SetViewportSize(uint32_t width, uint32_t height)
	{
		myViewportWidth = width;

		myViewportHeight = height;
	}

	void Window::SetEventCallback(const EventCallbackFn& callback)
	{
		myData.eventCallback = callback;
	}

	void Window::Maximize() const
	{
		glfwMaximizeWindow(myWindow);
	}

	void Window::Minimize() const
	{
		glfwIconifyWindow(myWindow);
	}

	void Window::Restore() const
	{
		glfwRestoreWindow(myWindow);
	}

	bool Window::IsFocused() const
	{
		int32_t focused = glfwGetWindowAttrib(myWindow, GLFW_FOCUSED);
		return focused == GLFW_FOCUSED;
	}

	const bool Window::IsMaximized() const
	{
		return (bool)glfwGetWindowAttrib(myWindow, GLFW_MAXIMIZED);
	}

	void Window::SetCursor(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			return;
		}

		if (myCursors.contains(path))
		{
			glfwSetCursor(myWindow, myCursors.at(path));
			return;
		}

		auto textureData = DDSUtility::GetRawDataFromDDS(path);

		GLFWimage image;
		image.width = (int32_t)textureData.width;
		image.height = (int32_t)textureData.height;
		image.pixels = textureData.dataBuffer.As<uint8_t>();

		GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);
		myCursors.emplace(path, cursor);

		textureData.dataBuffer.Release();
		glfwSetCursor(myWindow, cursor);
	}

	void Window::SetOpacity(float opacity) const
	{
		glfwSetWindowOpacity(myWindow, opacity);
	}

	std::string Window::GetClipboard() const
	{
		return glfwGetClipboardString(myWindow);
	}

	void Window::SetClipboard(const std::string& string)
	{
		glfwSetClipboardString(myWindow, string.c_str());
	}

	const std::pair<float, float> Window::GetPosition() const
	{
		int32_t x, y;
		glfwGetWindowPos(myWindow, &x, &y);

		return { (float)x, (float)y };
	}

	const float Window::GetOpacity() const
	{
		return glfwGetWindowOpacity(myWindow);
	}

	Scope<Window> Window::Create(const WindowProperties& aProperties)
	{
		return CreateScope<Window>(aProperties);
	}
}
