#include "inputpch.h"
#include "Input.h"

#include "Events/KeyboardEvents.h"
#include "Events/MouseEvents.h"

#include <EventSystem/EventSystem.h>

#include <glm/vec2.hpp>

namespace Volt
{
	VT_REGISTER_SUBSYSTEM(Input, PreEngine, 2);

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
			s_instance->m_frameKeyStateEvents[static_cast<size_t>(keyEvent.GetKeyCode())] = KeyState::Pressed;
			return false;
		});

		RegisterListener<KeyReleasedEvent>([&](KeyReleasedEvent& keyEvent)
		{
			s_instance->m_keyStates[static_cast<size_t>(keyEvent.GetKeyCode())] = KeyState::Released;
			s_instance->m_frameKeyStateEvents[static_cast<size_t>(keyEvent.GetKeyCode())] = KeyState::Released;
			return false;
		});

		RegisterListener<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& mouseEvent)
		{
			s_instance->m_keyStates[static_cast<size_t>(mouseEvent.GetMouseButton())] = KeyState::Pressed;
			s_instance->m_frameKeyStateEvents[static_cast<size_t>(mouseEvent.GetMouseButton())] = KeyState::Pressed;
			return false;
		});

		RegisterListener<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& mouseEvent)
		{
			s_instance->m_keyStates[static_cast<size_t>(mouseEvent.GetMouseButton())] = KeyState::Released;
			s_instance->m_frameKeyStateEvents[static_cast<size_t>(mouseEvent.GetMouseButton())] = KeyState::Released;
			return false;
		});

		RegisterListener<MouseMovedEvent>([&](MouseMovedEvent& moveEvent)
		{
			s_instance->m_mousePos = { moveEvent.GetX(), moveEvent.GetY() };
			return false;
		});

		RegisterListener<AppPostFrameUpdateEvent>(VT_BIND_EVENT_FN(Input::OnPostFrameUpdateEvent));
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

	bool Input::IsKeyPressed(InputCode keyCode)
	{
		return s_instance->m_frameKeyStateEvents[static_cast<size_t>(keyCode)] == KeyState::Pressed;
	}

	bool Input::IsKeyReleased(InputCode keyCode)
	{
		return s_instance->m_frameKeyStateEvents[static_cast<size_t>(keyCode)] == KeyState::Released;
	}

	bool Input::IsMouseButtonPressed(InputCode mouseButtonCode)
	{
		return s_instance->m_frameKeyStateEvents[static_cast<size_t>(mouseButtonCode)] == KeyState::Pressed;
	}

	bool Input::IsMouseButtonReleased(InputCode mouseButtonCode)
	{
		return s_instance->m_frameKeyStateEvents[static_cast<size_t>(mouseButtonCode)] == KeyState::Released;
	}

	bool Input::IsKeyDown(InputCode keyCode)
	{
		return s_instance->m_keyStates[static_cast<size_t>(keyCode)] == KeyState::Pressed;
	}

	bool Input::IsKeyUp(InputCode keyCode)
	{
		return s_instance->m_keyStates[static_cast<size_t>(keyCode)] == KeyState::Released;
	}

	bool Input::IsMouseButtonDown(InputCode mouseButtonCode)
	{
		return s_instance->m_keyStates[static_cast<size_t>(mouseButtonCode)] == KeyState::Pressed;
	}

	bool Input::IsMouseButtonUp(InputCode mouseButtonCode)
	{
		return s_instance->m_keyStates[static_cast<size_t>(mouseButtonCode)] == KeyState::Released;
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

	bool Input::OnPostFrameUpdateEvent(AppPostFrameUpdateEvent& event)
	{
		// Reset all frame events
		memset(m_frameKeyStateEvents.data(), 0, sizeof(KeyState) * m_frameKeyStateEvents.size());

		return false;
	}

	void Input::SetViewportMousePosition(const glm::vec2& viewportPos)
	{
		s_instance->m_viewportMousePos = viewportPos;
	}
}
