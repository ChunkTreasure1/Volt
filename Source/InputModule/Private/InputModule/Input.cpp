#include "inputpch.h"
#include "Input.h"

#include "Events/KeyboardEvents.h"
#include "Events/MouseEvents.h"

#include "EventSystem/EventSystem.h"

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

			s_instance->m_keyStates[static_cast<size_t>(keyEvent.GetKeyCode())] = KeyState::Pressed;
			return false;
		});

		RegisterListener<KeyReleasedEvent>([&](KeyReleasedEvent& keyEvent)
		{
			s_instance->m_keyStates[static_cast<size_t>(keyEvent.GetKeyCode())] = KeyState::Released;
			return false;
		});

		RegisterListener<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& keyEvent)
		{
			s_instance->m_keyStates[static_cast<size_t>(keyEvent.GetMouseButton())] = KeyState::Pressed;
			return false;
		});

		RegisterListener<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& keyEvent)
		{
			s_instance->m_keyStates[static_cast<size_t>(keyEvent.GetMouseButton())] = KeyState::Released;
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

	Vector<int> Input::GetAllPressedButtons()
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

	bool Input::IsButtonDown(InputCode keyCode)
	{
		return s_instance->m_keyStates[static_cast<size_t>(keyCode)] == KeyState::Pressed;
	}

	bool Input::IsButtonUp(InputCode keyCode)
	{
		return s_instance->m_keyStates[static_cast<size_t>(keyCode)] == KeyState::Released;
	}

	void Input::SetMousePosition(float x, float y)
	{
		SetMousePositionEvent event(x, y);
		EventSystem::DispatchEvent(event);
	}

	glm::vec2 Input::GetMousePosition()
	{
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

	void Input::ShowCursor(bool state)
	{
		SetShowCursorEvent event(state);
		EventSystem::DispatchEvent(event);
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
