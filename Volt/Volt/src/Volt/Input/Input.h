#pragma once

#include "Volt/Core/Application.h"
#include "Volt/Core/Window.h"
#include <bitset>

#include <utility> 

namespace Volt
{
	class InputMapper
	{
	public:
		static int GetKey(const std::string& name) { if (mykeyMap.contains(name)) { return mykeyMap.at(name); } else { return -1; } };
		static void SetKey(const std::string& name, int keyCode) { mykeyMap[name] = keyCode; };
		static void ResetKey(const std::string& name) { mykeyMap.erase(name); };

	private:
		inline static std::unordered_map<std::string, int> mykeyMap;
	};

	class Input
	{
	protected:
		Input() = default;

	public:
		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;

		static void OnEvent(Event& e);

		static bool IsKeyPressed(int keyCode);
		static std::vector<int> GetAllKeyPressed();
		static bool IsKeyReleased(int keyCode);
		static bool IsMouseButtonPressed(int button);
		static bool IsMouseButtonReleased(int button);

		static bool IsKeyDown(int keyCode);
		static bool IsKeyUp(int keyCode);
		static bool IsMouseButtonDown(int button);
		static bool IsMouseButtonUp(int button);
		static void SetMousePosition(float x, float y);
		static std::pair<float, float> GetMousePosition();

		inline static float GetMouseX()
		{
			auto [x, y] = GetMousePosition();
			return x;
		}

		inline static float GetMouseY()
		{
			auto [x, y] = GetMousePosition();
			return y;
		}

		static void ShowCursor(bool state);
		static void DisableInput(bool state);
		static bool IsInputDisabled() { return myDisableInput; };

		static void SetViewportMousePosition(const gem::vec2& viewportPos);
		static const gem::vec2& GetViewportMousePosition();

	private:
		enum class KeyState
		{
			None = 0,
			Released,
			Pressed
		};

		inline static gem::vec2 myViewportMousePos;
		static inline bool myDisableInput = false;
		static inline std::array<KeyState, 349> myKeyStates;
	};
}
