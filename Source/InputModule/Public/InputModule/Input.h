#pragma once

#include "InputModule/Config.h"

#include <EventSystem/EventListener.h>

#include <CoreUtilities/Containers/Vector.h>

#include <glm/fwd.hpp>
#include <bitset>
#include <utility> 

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

	class INPUTMODULE_API Input : public EventListener
	{
	public:
		Input();
		~Input();

		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;
		
		static bool IsKeyPressed(int keyCode);
		static Vector<int> GetAllKeyPressed();
		static bool IsKeyReleased(int keyCode);
		static bool IsMouseButtonPressed(int button);
		static bool IsMouseButtonReleased(int button);

		static bool IsKeyDown(int keyCode);
		static bool IsKeyUp(int keyCode);
		static bool IsMouseButtonDown(int button);
		static bool IsMouseButtonUp(int button);
		static void SetMousePosition(float x, float y);
		static glm::vec2 GetMousePosition();

		static float GetMouseX();
		static float GetMouseY();

		static float GetScrollOffset();

		static void ShowCursor(bool state);
		static void DisableInput(bool state);
		static bool IsInputDisabled() { return s_instance->m_disableInput; };

		static void SetViewportMousePosition(const glm::vec2& viewportPos);
		static const glm::vec2& GetViewportMousePosition();

	private:
		enum class KeyState
		{
			None = 0,
			Released,
			Pressed
		};

		inline static Input* s_instance = nullptr;

		glm::vec2 m_mousePos;
		glm::vec2 m_viewportMousePos;
		bool m_disableInput;
		float m_scrollOffset;
		std::array<KeyState, 349> m_keyStates;
	};
}
