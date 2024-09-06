#include "circuitpch.h"
#include "Input/CircuitInput.h"

namespace Circuit
{
	bool InputEvent::WasJustPressed()
	{
		return !m_previousPressedState && m_currentPressedState;
	}

	bool InputEvent::WasJustReleased()
	{
		return m_previousPressedState && !m_currentPressedState;
	}

	bool InputEvent::IsDown()
	{
		return m_currentPressedState;
	}

	KeyCode InputEvent::GetKeyCode()
	{
		return m_keyCode;
	}

	float InputEvent::GetAxis1D()
	{
		return m_data[0];
	}

	glm::vec2 InputEvent::GetAxis2D()
	{
		return reinterpret_cast<glm::vec2&>(m_data[0]);
	}

	float InputEvent::GetScrollDelta()
	{
		return m_data[0];
	}

	glm::vec2 InputEvent::GetMousePos()
	{
		return reinterpret_cast<glm::vec2&>(m_data[0]);
	}

	glm::vec2 InputEvent::GetMouseMoveDelta()
	{
		return reinterpret_cast<glm::vec2&>(m_data[2]);
	}


	CircuitInput& CircuitInput::Get()
	{
		assert(s_instance.get() != nullptr && "CircuitInput instance is null");
		return *s_instance;
	}

	glm::vec2 CircuitInput::GetMousePos()
	{
		return m_mousePos;
	}

	void CircuitInput::OnKeyPressed(KeyCode keycode)
	{
		const bool wasPressed = m_keyState.contains(keycode);
		if (!wasPressed)
		{
			m_keyState.insert(keycode);
		}

		InputEvent keyPressedEvent;
		keyPressedEvent.m_keyCode = keycode;
		keyPressedEvent.m_previousPressedState = wasPressed;
		keyPressedEvent.m_currentPressedState = true;

		BroadcastInputEvent(keyPressedEvent);
	}

	void CircuitInput::OnKeyReleased(KeyCode keycode)
	{
		const bool wasPressed = m_keyState.contains(keycode);
		if (wasPressed)
		{
			m_keyState.erase(keycode);
		}

		InputEvent keyPressedEvent;
		keyPressedEvent.m_keyCode = keycode;
		keyPressedEvent.m_previousPressedState = wasPressed;
		keyPressedEvent.m_currentPressedState = true;

		BroadcastInputEvent(keyPressedEvent);
	}

	void CircuitInput::OnMouseScrolled(float delta)
	{
		InputEvent mouseScrolledEvent;
		mouseScrolledEvent.m_data[0] = delta;
		mouseScrolledEvent.m_keyCode = KeyCode::Mouse_Scroll;

		BroadcastInputEvent(mouseScrolledEvent);
	}

	void CircuitInput::OnMouseMoved(float xPos, float yPos)
	{
		const glm::vec2 delta = glm::vec2(xPos, yPos) - m_mousePos;
		m_mousePos = { xPos, yPos };

		InputEvent mouseMovedEvent;
		mouseMovedEvent.GetKeyCode();
	}

	void CircuitInput::BroadcastInputEvent(InputEvent& inputEvent)
	{
		m_inputEventCallback(inputEvent);
	}

	void CircuitInput::Initialize(std::function<bool(InputEvent&)> inputEventCallback)
	{
		s_instance = std::make_unique<CircuitInput>();
		s_instance->m_inputEventCallback = inputEventCallback;
	}


	

}
