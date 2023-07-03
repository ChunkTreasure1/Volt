#include "vtpch.h"
#include "Input.h"

#include "Volt/Events/KeyEvent.h"
#include "Volt/Events/MouseEvent.h"

#include <imgui_internal.h>
#include <GLFW/glfw3.h>

namespace Volt
{
	void Input::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppUpdateEvent>([&](AppUpdateEvent&)
		{
			memset(myKeyStates.data(), 0, sizeof(KeyState) * myKeyStates.size());

			myScrollOffset = 0.f;

			return false;
		});

		dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& keyEvent)
		{
			if (keyEvent.GetRepeatCount() == 1)
			{
				return false;
			}

			myKeyStates[keyEvent.GetKeyCode()] = KeyState::Pressed;
			return false;
		});

		dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent& keyEvent)
		{
			myKeyStates[keyEvent.GetKeyCode()] = KeyState::Released;
			return false;
		});

		dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& keyEvent)
		{
			myKeyStates[keyEvent.GetMouseButton()] = KeyState::Pressed;
			return false;
		});

		dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& keyEvent)
		{
			myKeyStates[keyEvent.GetMouseButton()] = KeyState::Released;
			return false;
		});

		dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent& scrollEvent) 
		{
			myScrollOffset = scrollEvent.GetYOffset();
			return false;
		});
	}

	bool Input::IsKeyPressed(int keyCode)
	{
		return myKeyStates[keyCode] == KeyState::Pressed;
	}

	std::vector<int> Input::GetAllKeyPressed()
	{
		std::vector<int> keyPressedVec;
		for(int i = 0; i < myKeyStates.size(); i++)
		{
			if (myKeyStates[i] == KeyState::Pressed)
			{
				keyPressedVec.push_back(i);
			}
		}
		return keyPressedVec;
	}

	bool Input::IsKeyReleased(int keyCode)
	{
		return myKeyStates[keyCode] == KeyState::Released;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		return myKeyStates[button] == KeyState::Pressed;
	}

	bool Input::IsMouseButtonReleased(int button)
	{
		return myKeyStates[button] == KeyState::Released;
	}

	bool Input::IsKeyDown(int keyCode)
	{
		const bool imguiEnabled = Application::Get().GetInfo().enableImGui;
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		if (!imguiEnabled)
		{
			auto state = glfwGetKey(window, keyCode);
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

	bool Input::IsMouseButtonDown(int button)
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

	bool Input::IsMouseButtonUp(int button)
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

	void Input::SetMousePosition(float x, float y)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		glfwSetCursorPos(window, (double)x, (double)y);
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto [wx, wy] = Volt::Application::Get().GetWindow().GetPosition();


		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		return { (float)xPos + wx, (float)yPos + wy };
	}
	
	float Input::GetScrollOffset()
	{
		return myScrollOffset;
	}

	void Input::ShowCursor(bool state)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		glfwSetInputMode(window, GLFW_CURSOR, state ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}

	void Input::DisableInput(bool state)
	{
		myDisableInput = state;
	}

	const glm::vec2& Input::GetViewportMousePosition()
	{
		return myViewportMousePos;
	}

	void Input::SetViewportMousePosition(const glm::vec2& viewportPos)
	{
		myViewportMousePos = viewportPos;
	}
}
