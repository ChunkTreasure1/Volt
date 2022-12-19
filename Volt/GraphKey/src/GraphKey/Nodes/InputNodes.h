#pragma once

#include "GraphKey/Node.h"

#include <Volt/Input/KeyCodes.h>
#include <Volt/Events/KeyEvent.h>
#include <Volt/Utility/StringUtility.h>

#include <GEM/gem.h>

namespace GraphKey
{
	inline static int32_t GetKeyCodeFromKeyName(const std::string& name)
	{
		const std::string lowerName = Utils::ToLower(name);
		if (lowerName == "space" || lowerName == " ") return VT_KEY_SPACE;
		if (lowerName == "apostrophe" || lowerName == "'") return VT_KEY_APOSTROPHE;
		if (lowerName == "comma" || lowerName == ",") return VT_KEY_COMMA;
		if (lowerName == "minus" || lowerName == "-") return VT_KEY_MINUS;
		if (lowerName == "period" || lowerName == ".") return VT_KEY_PERIOD;
		if (lowerName == "slash" || lowerName == "/") return VT_KEY_SLASH;
		if (lowerName == "semicolon" || lowerName == ";") return VT_KEY_SEMICOLON;
		if (lowerName == "equal" || lowerName == "=") return VT_KEY_EQUAL;
		if (lowerName == "0") return VT_KEY_0;
		if (lowerName == "1") return VT_KEY_1;
		if (lowerName == "2") return VT_KEY_2;
		if (lowerName == "3") return VT_KEY_3;
		if (lowerName == "4") return VT_KEY_4;
		if (lowerName == "5") return VT_KEY_5;
		if (lowerName == "6") return VT_KEY_6;
		if (lowerName == "7") return VT_KEY_7;
		if (lowerName == "8") return VT_KEY_8;
		if (lowerName == "9") return VT_KEY_9;
		if (lowerName == "a") return VT_KEY_A;
		if (lowerName == "b") return VT_KEY_B;
		if (lowerName == "c") return VT_KEY_C;
		if (lowerName == "d") return VT_KEY_D;
		if (lowerName == "e") return VT_KEY_E;
		if (lowerName == "f") return VT_KEY_F;
		if (lowerName == "g") return VT_KEY_G;
		if (lowerName == "h") return VT_KEY_H;
		if (lowerName == "i") return VT_KEY_I;
		if (lowerName == "j") return VT_KEY_J;
		if (lowerName == "k") return VT_KEY_K;
		if (lowerName == "l") return VT_KEY_L;
		if (lowerName == "m") return VT_KEY_M;
		if (lowerName == "n") return VT_KEY_N;
		if (lowerName == "o") return VT_KEY_O;
		if (lowerName == "p") return VT_KEY_P;
		if (lowerName == "q") return VT_KEY_Q;
		if (lowerName == "r") return VT_KEY_R;
		if (lowerName == "s") return VT_KEY_S;
		if (lowerName == "t") return VT_KEY_T;
		if (lowerName == "u") return VT_KEY_U;
		if (lowerName == "v") return VT_KEY_V;
		if (lowerName == "w") return VT_KEY_W;
		if (lowerName == "x") return VT_KEY_X;
		if (lowerName == "y") return VT_KEY_Y;
		if (lowerName == "z") return VT_KEY_Z;

		return 0;
	}

	class KeyPressedNode : public Node
	{
	public:
		inline KeyPressedNode()
		{
			inputs =
			{
				AttributeConfig<std::string>("Key", GraphKey::AttributeDirection::Input, false, nullptr, false)
			};

			outputs =
			{
				AttributeConfig("Pressed", AttributeDirection::Output)
			};
		}

		inline const std::string GetName() override { return "Key Pressed"; }
		inline const gem::vec4 GetColor() override { return { 1.f, 0.37f, 0.53f, 1.f }; }
		
		inline void OnEvent(Volt::Event& e) override
		{
			Volt::EventDispatcher dispatcher(e);
			dispatcher.Dispatch<Volt::KeyPressedEvent>([&](Volt::KeyPressedEvent& e) 
				{
					const std::string keyName = GetInput<std::string>(0);
					const int32_t keyCode = GetKeyCodeFromKeyName(keyName);

					if (e.GetKeyCode() == keyCode)
					{
						ActivateOutput(0);
					}
					
					return false;
				});
		}
	};

	class KeyReleasedNode : public Node
	{
	public:
		inline KeyReleasedNode()
		{
			inputs =
			{
				AttributeConfig<std::string>("Key", GraphKey::AttributeDirection::Input, false, nullptr, false)
			};

			outputs =
			{
				AttributeConfig("Released", AttributeDirection::Output)
			};
		}

		inline const std::string GetName() override { return "Key Released"; }
		inline const gem::vec4 GetColor() override { return { 1.f, 0.37f, 0.53f, 1.f }; }

		inline void OnEvent(Volt::Event& e) override
		{
			Volt::EventDispatcher dispatcher(e);
			dispatcher.Dispatch<Volt::KeyReleasedEvent>([&](Volt::KeyReleasedEvent& e)
				{
					const std::string keyName = GetInput<std::string>(0);
					const int32_t keyCode = GetKeyCodeFromKeyName(keyName);

					if (e.GetKeyCode() == keyCode)
					{
						ActivateOutput(0);
					}

					return false;
				});
		}
	};
}