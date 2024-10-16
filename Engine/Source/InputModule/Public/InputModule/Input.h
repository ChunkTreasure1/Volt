#pragma once

#include "InputModuleConfig.h"

#include "InputCodes.h"

#include <EventSystem/EventListener.h>
#include <EventSystem/ApplicationEvents.h>

#include <SubSystem/SubSystem.h>
#include <CoreUtilities/Containers/Vector.h>

#include <glm/fwd.hpp>

namespace Volt
{
	/*class InputMapper
	{
	public:
		static int GetKey(const std::string& name) { if (mykeyMap.contains(name)) { return mykeyMap.at(name); } else { return -1; } };
		static void SetKey(const std::string& name, int keyCode) { mykeyMap[name] = keyCode; };
		static void ResetKey(const std::string& name) { mykeyMap.erase(name); };

	private:
		inline static std::unordered_map<std::string, int> mykeyMap;
	};*/

	class INPUTMODULE_API Input : public SubSystem, public EventListener
	{
	public:
		Input();
		~Input();

		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;

		static Vector<int> GetAllPressedButtons();

		// Frame events
		static bool IsKeyPressed(InputCode keyCode);
		static bool IsKeyReleased(InputCode keyCode);
		static bool IsMouseButtonPressed(InputCode mouseButtonCode);
		static bool IsMouseButtonReleased(InputCode mouseButtonCode);

		// Holding or not pressing
		static bool IsKeyDown(InputCode keyCode);
		static bool IsKeyUp(InputCode keyCode);
		static bool IsMouseButtonDown(InputCode mouseButtonCode);
		static bool IsMouseButtonUp(InputCode mouseButtonCode);

		static void SetMousePosition(float x, float y);
		static glm::vec2 GetMousePosition();

		static float GetMouseX();
		static float GetMouseY();

		static void ShowCursor(bool state);
		static void DisableInput(bool state);
		static bool IsInputDisabled() { return s_instance->m_disableInput; };

		static void SetViewportMousePosition(const glm::vec2& viewportPos);
		static const glm::vec2& GetViewportMousePosition();

		VT_DECLARE_SUBSYSTEM("{A85034B9-EAED-4BAA-B1E8-AF93F5323E74}"_guid)

	private:
		enum class KeyState
		{
			None = 0,
			Released,
			Pressed
		};

		bool OnPostFrameUpdateEvent(AppPostFrameUpdateEvent& event);

		inline static Input* s_instance = nullptr;

		glm::vec2 m_mousePos;
		glm::vec2 m_viewportMousePos;
		bool m_disableInput;

		//we use unknown here because it is at the end of the enum
		std::array<KeyState, static_cast<size_t>(InputCode::Unknown)> m_keyStates;
		std::array<KeyState, static_cast<size_t>(InputCode::Unknown)> m_frameKeyStateEvents;
	};
}
