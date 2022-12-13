#include "vtpch.h"
#include "Input.h"

#include <imgui_internal.h>

namespace Volt
{
	bool Input::IsKeyDown(int keyCode)
	{
		const bool imguiEnabled = Application::Get().GetInfo().enableImGui;
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		if (!imguiEnabled)
		{
			auto state = glfwGetKey(window, keyCode);
			myKeyStates[keyCode] = (state == GLFW_PRESS || state == GLFW_REPEAT);

			return state == GLFW_PRESS || state == GLFW_REPEAT;
		}

		ImGuiContext* context = ImGui::GetCurrentContext();
		bool pressed = false;

		for (ImGuiViewport* viewport : context->Viewports)
		{
			if (!viewport->PlatformUserData)
			{
				continue;
			}

			GLFWwindow* windowHandle = *(GLFWwindow**)viewport->PlatformUserData;
			if (!windowHandle)
			{
				continue;
			}

			auto state = glfwGetKey(windowHandle, keyCode);
			if (state == GLFW_PRESS || state == GLFW_REPEAT)
			{
				pressed = true;
				break;
			}
		}

		return pressed;
	}

	bool Input::IsKeyUp(int keyCode)
	{
		const bool imguiEnabled = Application::Get().GetInfo().enableImGui;
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		if (!imguiEnabled)
		{
			auto state = glfwGetKey(window, keyCode);
			return state == GLFW_RELEASE;
		}

		ImGuiContext* context = ImGui::GetCurrentContext();
		bool pressed = true;

		for (ImGuiViewport* viewport : context->Viewports)
		{
			if (!viewport->PlatformUserData)
			{
				continue;
			}

			GLFWwindow* windowHandle = *(GLFWwindow**)viewport->PlatformUserData;
			if (!windowHandle)
			{
				continue;
			}

			auto state = glfwGetKey(windowHandle, keyCode);
			if (state == GLFW_RELEASE)
			{
				pressed = false;
				break;
			}
		}

		return pressed;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		const bool imguiEnabled = Application::Get().GetInfo().enableImGui;
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		if (!imguiEnabled)
		{
			auto state = glfwGetMouseButton(window, button);
			return state == GLFW_PRESS;
		}

		ImGuiContext* context = ImGui::GetCurrentContext();
		bool pressed = false;

		for (ImGuiViewport* viewport : context->Viewports)
		{
			if (!viewport->PlatformUserData)
			{
				continue;
			}

			GLFWwindow* windowHandle = *(GLFWwindow**)viewport->PlatformUserData;
			if (!windowHandle)
			{
				continue;
			}

			auto state = glfwGetMouseButton(windowHandle, button);
			if (state == GLFW_PRESS || state == GLFW_REPEAT)
			{
				pressed = true;
				break;
			}
		}

		return pressed;
	}

	bool Input::IsMouseButtonReleased(int button)
	{
		const bool imguiEnabled = Application::Get().GetInfo().enableImGui;
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		if (!imguiEnabled)
		{
			auto state = glfwGetMouseButton(window, button);
			return state == GLFW_RELEASE;
		}

		ImGuiContext* context = ImGui::GetCurrentContext();
		bool pressed = true;

		for (ImGuiViewport* viewport : context->Viewports)
		{
			if (!viewport->PlatformUserData)
			{
				continue;
			}

			GLFWwindow* windowHandle = *(GLFWwindow**)viewport->PlatformUserData;
			if (!windowHandle)
			{
				continue;
			}

			auto state = glfwGetMouseButton(windowHandle, button);
			if (state == GLFW_RELEASE)
			{
				pressed = false;
				break;
			}
		}

		return pressed;
	}
}