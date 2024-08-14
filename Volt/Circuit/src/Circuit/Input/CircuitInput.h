#pragma once
#include "Circuit/Input/CiruitKeyCodes.h"

#include <memory>
#include <glm/vec2.hpp>
#include <functional>

namespace Circuit
{
	enum class InputEventType;
	class CIRCUIT_API InputEvent
	{
	public:
		bool WasJustPressed();
		bool WasJustReleased();

		bool IsDown();

		KeyCode GetKeyCode();

		float GetAxis1D();
		glm::vec2 GetAxis2D();

		//only if Mouse_Scroll
		float GetScrollDelta();

		//only if Mouse_Move event
		glm::vec2 GetMousePos();
		//only if Mouse_Move event
		glm::vec2 GetMouseMoveDelta();

	private: 
		friend class CircuitInput;

		KeyCode m_keyCode;

		bool m_currentPressedState;
		bool m_previousPressedState;

		float m_data[4];
	};
	class CIRCUIT_API CircuitInput
	{
	public:
		CircuitInput() = default;
		~CircuitInput() = default;

		static void Initialize(std::function<bool(InputEvent&)> inputEventCallback);

		static CircuitInput& Get();

		glm::vec2 GetMousePos();

		//for recieving Input
		void OnKeyPressed(KeyCode keycode);
		void OnKeyReleased(KeyCode keycode);
		void OnMouseScrolled(float delta);
		void OnMouseMoved(float xPos, float yPos);
	private:
		inline static std::unique_ptr<CircuitInput> s_instance = nullptr;
	private:
		void BroadcastInputEvent(InputEvent& inputEvent);

		std::set<KeyCode> m_keyState;
		glm::vec2 m_mousePos;



		std::function<bool(InputEvent&)> m_inputEventCallback;

		
	};
}
