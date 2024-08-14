#include "inputpch.h"
#include "Input.h"

#include "InputModule/Events/KeyboardEvents.h"
#include "InputModule/Events/MouseEvents.h"

#include <glm/vec2.hpp>

namespace Volt
{
	glm::vec2 Input::myMousePos;
	glm::vec2 Input::myViewportMousePos;
	bool Input::myDisableInput = false;
	float Input::myScrollOffset = 0;
	std::array<Input::KeyState, 349> Input::myKeyStates;

	void Input::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

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

		dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& moveEvent)
		{
			myMousePos = { moveEvent.GetX(), moveEvent.GetY() };
			return false;
		});
	}

	void Input::Update()
	{
		memset(myKeyStates.data(), 0, sizeof(KeyState) * myKeyStates.size());

		myScrollOffset = 0.f;
	}

	bool Input::IsKeyPressed(int keyCode)
	{
		return myKeyStates[keyCode] == KeyState::Pressed;
	}

	Vector<int> Input::GetAllKeyPressed()
	{
		Vector<int> keyPressedVec;
		for (int i = 0; i < myKeyStates.size(); i++)
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
		return myKeyStates[keyCode] == KeyState::Pressed;
		//TODO: Reeimplement
		/*const bool imguiEnabled = Application::Get().GetInfo().enableImGui;
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

		return pressed;*/
	}

	bool Input::IsKeyUp(int keyCode)
	{
		return myKeyStates[keyCode] == KeyState::Released;
		//TODO: Reeimplement
		/*const bool imguiEnabled = Application::Get().GetInfo().enableImGui;
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

		return pressed;*/
	}

	bool Input::IsMouseButtonDown(int button)
	{
		return myKeyStates[button] == KeyState::Pressed;
		//TODO: Reeimplement
		/*const bool imguiEnabled = Application::Get().GetInfo().enableImGui;
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

		return pressed;*/
	}

	bool Input::IsMouseButtonUp(int button)
	{
		return myKeyStates[button] == KeyState::Released;
		//TODO: Reeimplement
		/*const bool imguiEnabled = Application::Get().GetInfo().enableImGui;
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

		return pressed;*/
	}

	void Input::SetMousePosition(float x, float y)
	{
		//TODO: Reeimplement

		/*auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		glfwSetCursorPos(window, (double)x, (double)y);*/
	}

	glm::vec2 Input::GetMousePosition()
	{
		//TODO: Reeimplement
		/*auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto [wx, wy] = Volt::Application::Get().GetWindow().GetPosition();


		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		return { (float)xPos + wx, (float)yPos + wy };*/
		return myMousePos;
	}

	float Input::GetMouseX()
	{
		return myMousePos.x;
	}

	float Input::GetMouseY()
	{
		return myMousePos.y;
	}


	float Input::GetScrollOffset()
	{
		return myScrollOffset;
	}

	void Input::ShowCursor(bool state)
	{
		//TODO: Reeimplement
		//auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		//glfwSetInputMode(window, GLFW_CURSOR, state ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
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
