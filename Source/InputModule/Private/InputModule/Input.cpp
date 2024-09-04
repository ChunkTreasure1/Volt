#include "inputpch.h"
#include "Input.h"

#include "InputModule/Events/KeyboardEvents.h"
#include "InputModule/Events/MouseEvents.h"

#include <glm/vec2.hpp>

namespace Volt
{
	Input::Input()
	{
		VT_ENSURE(s_instance == nullptr);
		s_instance = this;

		RegisterListener<KeyPressedEvent>([&](KeyPressedEvent& keyEvent)
		{
			if (keyEvent.GetRepeatCount() == 1)
			{
				return false;
			}

			s_instance->m_keyStates[keyEvent.GetKeyCode()] = KeyState::Pressed;
			return false;
		});

		RegisterListener<KeyReleasedEvent>([&](KeyReleasedEvent& keyEvent)
		{
			s_instance->m_keyStates[keyEvent.GetKeyCode()] = KeyState::Released;
			return false;
		});

		RegisterListener<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& keyEvent)
		{
			s_instance->m_keyStates[keyEvent.GetMouseButton()] = KeyState::Pressed;
			return false;
		});

		RegisterListener<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& keyEvent)
		{
			s_instance->m_keyStates[keyEvent.GetMouseButton()] = KeyState::Released;
			return false;
		});

		RegisterListener<MouseScrolledEvent>([&](MouseScrolledEvent& scrollEvent)
		{
			s_instance->m_scrollOffset = scrollEvent.GetYOffset();
			return false;
		});

		RegisterListener<MouseMovedEvent>([&](MouseMovedEvent& moveEvent)
		{
			s_instance->m_mousePos = { moveEvent.GetX(), moveEvent.GetY() };
			return false;
		});
	}

	Input::~Input()
	{
		s_instance = nullptr;
	}

	bool Input::IsKeyPressed(int keyCode)
	{
		return s_instance->m_keyStates[keyCode] == KeyState::Pressed;
	}

	Vector<int> Input::GetAllKeyPressed()
	{
		Vector<int> keyPressedVec;
		for (size_t i = 0; i < s_instance->m_keyStates.size(); i++)
		{
			if (s_instance->m_keyStates[i] == KeyState::Pressed)
			{
				keyPressedVec.push_back(static_cast<int32_t>(i));
			}
		}
		return keyPressedVec;
	}

	bool Input::IsKeyReleased(int keyCode)
	{
		return s_instance->m_keyStates[keyCode] == KeyState::Released;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		return s_instance->m_keyStates[button] == KeyState::Pressed;
	}

	bool Input::IsMouseButtonReleased(int button)
	{
		return s_instance->m_keyStates[button] == KeyState::Released;
	}

	bool Input::IsKeyDown(int keyCode)
	{
		return s_instance->m_keyStates[keyCode] == KeyState::Pressed;
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
		return s_instance->m_keyStates[keyCode] == KeyState::Released;
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
		return s_instance->m_keyStates[button] == KeyState::Pressed;
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
		return s_instance->m_keyStates[button] == KeyState::Released;
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
		return s_instance->m_mousePos;
	}

	float Input::GetMouseX()
	{
		return s_instance->m_mousePos.x;
	}

	float Input::GetMouseY()
	{
		return s_instance->m_mousePos.y;
	}


	float Input::GetScrollOffset()
	{
		return s_instance->m_scrollOffset;
	}

	void Input::ShowCursor(bool state)
	{
		//TODO: Reeimplement
		//auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		//glfwSetInputMode(window, GLFW_CURSOR, state ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	}

	void Input::DisableInput(bool state)
	{
		s_instance->m_disableInput = state;
	}

	const glm::vec2& Input::GetViewportMousePosition()
	{
		return s_instance->m_viewportMousePos;
	}

	void Input::SetViewportMousePosition(const glm::vec2& viewportPos)
	{
		s_instance->m_viewportMousePos = viewportPos;
	}
}
