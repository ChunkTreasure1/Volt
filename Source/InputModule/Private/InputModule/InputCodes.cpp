#include "inputpch.h"
#include "InputCodes.h"

// From glfw3.h
#define VT_KEY_SPACE              32
#define VT_KEY_APOSTROPHE         39  /* ' */
#define VT_KEY_COMMA              44  /* , */
#define VT_KEY_MINUS              45  /* - */
#define VT_KEY_PERIOD             46  /* . */
#define VT_KEY_SLASH              47  /* / */
#define VT_KEY_0                  48
#define VT_KEY_1                  49
#define VT_KEY_2                  50
#define VT_KEY_3                  51
#define VT_KEY_4                  52
#define VT_KEY_5                  53
#define VT_KEY_6                  54
#define VT_KEY_7                  55
#define VT_KEY_8                  56
#define VT_KEY_9                  57
#define VT_KEY_SEMICOLON          59  /* ; */
#define VT_KEY_EQUAL              61  /* = */
#define VT_KEY_A                  65
#define VT_KEY_B                  66
#define VT_KEY_C                  67
#define VT_KEY_D                  68
#define VT_KEY_E                  69
#define VT_KEY_F                  70
#define VT_KEY_G                  71
#define VT_KEY_H                  72
#define VT_KEY_I                  73
#define VT_KEY_J                  74
#define VT_KEY_K                  75
#define VT_KEY_L                  76
#define VT_KEY_M                  77
#define VT_KEY_N                  78
#define VT_KEY_O                  79
#define VT_KEY_P                  80
#define VT_KEY_Q                  81
#define VT_KEY_R                  82
#define VT_KEY_S                  83
#define VT_KEY_T                  84
#define VT_KEY_U                  85
#define VT_KEY_V                  86
#define VT_KEY_W                  87
#define VT_KEY_X                  88
#define VT_KEY_Y                  89
#define VT_KEY_Z                  90
#define VT_KEY_LEFT_BRACKET       91  /* [ */
#define VT_KEY_BACKSLASH          92  /* \ */
#define VT_KEY_RIGHT_BRACKET      93  /* ] */
#define VT_KEY_GRAVE_ACCENT       96  /* ` */
#define VT_KEY_WORLD_1            161 /* non-US #1 */
#define VT_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define VT_KEY_ESCAPE             256
#define VT_KEY_ENTER              257
#define VT_KEY_TAB                258
#define VT_KEY_BACKSPACE          259
#define VT_KEY_INSERT             260
#define VT_KEY_DELETE             261
#define VT_KEY_RIGHT              262
#define VT_KEY_LEFT               263
#define VT_KEY_DOWN               264
#define VT_KEY_UP                 265
#define VT_KEY_PAGE_UP            266
#define VT_KEY_PAGE_DOWN          267
#define VT_KEY_HOME               268
#define VT_KEY_END                269
#define VT_KEY_CAPS_LOCK          280
#define VT_KEY_SCROLL_LOCK        281
#define VT_KEY_NUM_LOCK           282
#define VT_KEY_PRINT_SCREEN       283
#define VT_KEY_PAUSE              284
#define VT_KEY_F1                 290
#define VT_KEY_F2                 291
#define VT_KEY_F3                 292
#define VT_KEY_F4                 293
#define VT_KEY_F5                 294
#define VT_KEY_F6                 295
#define VT_KEY_F7                 296
#define VT_KEY_F8                 297
#define VT_KEY_F9                 298
#define VT_KEY_F10                299
#define VT_KEY_F11                300
#define VT_KEY_F12                301
#define VT_KEY_F13                302
#define VT_KEY_F14                303
#define VT_KEY_F15                304
#define VT_KEY_F16                305
#define VT_KEY_F17                306
#define VT_KEY_F18                307
#define VT_KEY_F19                308
#define VT_KEY_F20                309
#define VT_KEY_F21                310
#define VT_KEY_F22                311
#define VT_KEY_F23                312
#define VT_KEY_F24                313
#define VT_KEY_F25                314
#define VT_KEY_KP_0               320
#define VT_KEY_KP_1               321
#define VT_KEY_KP_2               322
#define VT_KEY_KP_3               323
#define VT_KEY_KP_4               324
#define VT_KEY_KP_5               325
#define VT_KEY_KP_6               326
#define VT_KEY_KP_7               327
#define VT_KEY_KP_8               328
#define VT_KEY_KP_9               329
#define VT_KEY_KP_DECIMAL         330
#define VT_KEY_KP_DIVIDE          331
#define VT_KEY_KP_MULTIPLY        332
#define VT_KEY_KP_SUBTRACT        333
#define VT_KEY_KP_ADD             334
#define VT_KEY_KP_ENTER           335
#define VT_KEY_KP_EQUAL           336
#define VT_KEY_LEFT_SHIFT         340
#define VT_KEY_LEFT_CONTROL       341
#define VT_KEY_LEFT_ALT           342
#define VT_KEY_LEFT_SUPER         343
#define VT_KEY_RIGHT_SHIFT        344
#define VT_KEY_RIGHT_CONTROL      345
#define VT_KEY_RIGHT_ALT          346
#define VT_KEY_RIGHT_SUPER        347
#define VT_KEY_MENU               348


//mouse codes
#define VT_MOUSE_BUTTON_1         0
#define VT_MOUSE_BUTTON_2         1
#define VT_MOUSE_BUTTON_3         2
#define VT_MOUSE_BUTTON_4         3
#define VT_MOUSE_BUTTON_5         4
#define VT_MOUSE_BUTTON_6         5
#define VT_MOUSE_BUTTON_7         6
#define VT_MOUSE_BUTTON_8         7
#define VT_MOUSE_BUTTON_LAST      VT_MOUSE_BUTTON_8
#define VT_MOUSE_BUTTON_LEFT     VT_MOUSE_BUTTON_1
#define VT_MOUSE_BUTTON_RIGHT     VT_MOUSE_BUTTON_2
#define VT_MOUSE_BUTTON_MIDDLE    VT_MOUSE_BUTTON_3


namespace Volt
{
	InputCode GLFWKeyCodeToInputCode(uint32_t glfwKeyCode)
	{
		switch (glfwKeyCode)
		{
			case VT_KEY_SPACE: return InputCode::Spacebar;
			case VT_KEY_APOSTROPHE: return InputCode::Apostrophe;
			case VT_KEY_COMMA: return InputCode::Comma;
			case VT_KEY_MINUS: return InputCode::Minus;
			case VT_KEY_PERIOD: return InputCode::Period;
			case VT_KEY_SLASH: return InputCode::Slash;
			case VT_KEY_0: return InputCode::Key_0;
			case VT_KEY_1: return InputCode::Key_1;
			case VT_KEY_2: return InputCode::Key_2;
			case VT_KEY_3: return InputCode::Key_3;
			case VT_KEY_4: return InputCode::Key_4;
			case VT_KEY_5: return InputCode::Key_5;
			case VT_KEY_6: return InputCode::Key_6;
			case VT_KEY_7: return InputCode::Key_7;
			case VT_KEY_8: return InputCode::Key_8;
			case VT_KEY_9: return InputCode::Key_9;
			case VT_KEY_SEMICOLON: return InputCode::Semicolon;
			case VT_KEY_EQUAL: return InputCode::Equal;
			case VT_KEY_A: return InputCode::A;
			case VT_KEY_B: return InputCode::B;
			case VT_KEY_C: return InputCode::C;
			case VT_KEY_D: return InputCode::D;
			case VT_KEY_E: return InputCode::E;
			case VT_KEY_F: return InputCode::F;
			case VT_KEY_G: return InputCode::G;
			case VT_KEY_H: return InputCode::H;
			case VT_KEY_I: return InputCode::I;
			case VT_KEY_J: return InputCode::J;
			case VT_KEY_K: return InputCode::K;
			case VT_KEY_L: return InputCode::L;
			case VT_KEY_M: return InputCode::M;
			case VT_KEY_N: return InputCode::N;
			case VT_KEY_O: return InputCode::O;
			case VT_KEY_P: return InputCode::P;
			case VT_KEY_Q: return InputCode::Q;
			case VT_KEY_R: return InputCode::R;
			case VT_KEY_S: return InputCode::S;
			case VT_KEY_T: return InputCode::T;
			case VT_KEY_U: return InputCode::U;
			case VT_KEY_V: return InputCode::V;
			case VT_KEY_W: return InputCode::W;
			case VT_KEY_X: return InputCode::X;
			case VT_KEY_Y: return InputCode::Y;
			case VT_KEY_Z: return InputCode::Z;
			case VT_KEY_LEFT_BRACKET: return InputCode::LeftBracket;
			case VT_KEY_BACKSLASH: return InputCode::Backslash;
			case VT_KEY_RIGHT_BRACKET: return InputCode::RightBracket;
			case VT_KEY_GRAVE_ACCENT: return InputCode::GraveAccent;
			case VT_KEY_WORLD_1: return InputCode::World_1;
			case VT_KEY_WORLD_2: return InputCode::World_2;
			case VT_KEY_ESCAPE: return InputCode::Esc;
			case VT_KEY_ENTER: return InputCode::Return;
			case VT_KEY_TAB: return InputCode::Tab;
			case VT_KEY_BACKSPACE: return InputCode::Backspace;
			case VT_KEY_INSERT: return InputCode::Insert;
			case VT_KEY_DELETE: return InputCode::Delete;
			case VT_KEY_RIGHT: return InputCode::RightArrow;
			case VT_KEY_LEFT: return InputCode::LeftArrow;
			case VT_KEY_DOWN: return InputCode::DownArrow;
			case VT_KEY_UP: return InputCode::UpArrow;
			case VT_KEY_PAGE_UP: return InputCode::PageUp;
			case VT_KEY_PAGE_DOWN: return InputCode::PageDown;
			case VT_KEY_HOME: return InputCode::Home;
			case VT_KEY_END: return InputCode::End;
			case VT_KEY_CAPS_LOCK: return InputCode::CapsLock;
			case VT_KEY_SCROLL_LOCK: return InputCode::ScrollLock;
			case VT_KEY_NUM_LOCK: return InputCode::NumLock;
			case VT_KEY_PRINT_SCREEN: return InputCode::PrintScreen;
			case VT_KEY_PAUSE: return InputCode::Pause;
			case VT_KEY_F1: return InputCode::F1;
			case VT_KEY_F2: return InputCode::F2;
			case VT_KEY_F3: return InputCode::F3;
			case VT_KEY_F4: return InputCode::F4;
			case VT_KEY_F5: return InputCode::F5;
			case VT_KEY_F6: return InputCode::F6;
			case VT_KEY_F7: return InputCode::F7;
			case VT_KEY_F8: return InputCode::F8;
			case VT_KEY_F9: return InputCode::F9;
			case VT_KEY_F10: return InputCode::F10;
			case VT_KEY_F11: return InputCode::F11;
			case VT_KEY_F12: return InputCode::F12;
			case VT_KEY_F13: return InputCode::F13;
			case VT_KEY_F14: return InputCode::F14;
			case VT_KEY_F15: return InputCode::F15;
			case VT_KEY_F16: return InputCode::F16;
			case VT_KEY_F17: return InputCode::F17;
			case VT_KEY_F18: return InputCode::F18;
			case VT_KEY_F19: return InputCode::F19;
			case VT_KEY_F20: return InputCode::F20;
			case VT_KEY_F21: return InputCode::F21;
			case VT_KEY_F22: return InputCode::F22;
			case VT_KEY_F23: return InputCode::F23;
			case VT_KEY_F24: return InputCode::F24;
			case VT_KEY_KP_0: return InputCode::Numpad_0;
			case VT_KEY_KP_1: return InputCode::Numpad_1;
			case VT_KEY_KP_2: return InputCode::Numpad_2;
			case VT_KEY_KP_3: return InputCode::Numpad_3;
			case VT_KEY_KP_4: return InputCode::Numpad_4;
			case VT_KEY_KP_5: return InputCode::Numpad_5;
			case VT_KEY_KP_6: return InputCode::Numpad_6;
			case VT_KEY_KP_7: return InputCode::Numpad_7;
			case VT_KEY_KP_8: return InputCode::Numpad_8;
			case VT_KEY_KP_9: return InputCode::Numpad_9;
			case VT_KEY_KP_DECIMAL: return InputCode::Decimal;
			case VT_KEY_KP_DIVIDE: return InputCode::Divide;
			case VT_KEY_KP_MULTIPLY: return InputCode::Multiply;
			case VT_KEY_KP_SUBTRACT: return InputCode::Subtract;
			case VT_KEY_KP_ADD: return InputCode::Add;
			case VT_KEY_KP_ENTER: return InputCode::Enter;
			case VT_KEY_KP_EQUAL: return InputCode::Equal;
			case VT_KEY_LEFT_SHIFT: return InputCode::LeftShift;
			case VT_KEY_LEFT_CONTROL: return InputCode::LeftControl;
			case VT_KEY_LEFT_ALT: return InputCode::LeftAlt;
			case VT_KEY_LEFT_SUPER: return InputCode::LeftSuper;
			case VT_KEY_RIGHT_SHIFT: return InputCode::RightShift;
			case VT_KEY_RIGHT_CONTROL: return InputCode::RightControl;
			case VT_KEY_RIGHT_ALT: return InputCode::RightAlt;
			case VT_KEY_RIGHT_SUPER: return InputCode::RightSuper;
			case VT_KEY_MENU: return InputCode::Menu;

			default: return InputCode::Unknown;
		}
	}

	InputCode GLFWMouseCodeToInputCode(uint32_t glfwMouseCode)
	{
		switch (glfwMouseCode)
		{
			case VT_MOUSE_BUTTON_1: return InputCode::Mouse_LB;
			case VT_MOUSE_BUTTON_2: return InputCode::Mouse_RB;
			case VT_MOUSE_BUTTON_3: return InputCode::Mouse_MB;
			case VT_MOUSE_BUTTON_4: return InputCode::Mouse_X1;
			case VT_MOUSE_BUTTON_5: return InputCode::Mouse_X2;
			case VT_MOUSE_BUTTON_6: return InputCode::Mouse_X3;
			case VT_MOUSE_BUTTON_7: return InputCode::Mouse_X4;
			case VT_MOUSE_BUTTON_8: return InputCode::Mouse_X5;

			default: return InputCode::Unknown;
		}
	}

}
