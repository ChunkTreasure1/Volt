namespace Volt
{
    public enum KeyCode : int
    {
        Unknown = -1,

        Space = 32,
        Apostrophe = 39,
        Comma = 44,
        Minus = 45,
        Period = 46,
        Slash = 47,

        Key_0 = 48,
        Key_1 = 49,
        Key_2 = 50,
        Key_3 = 51,
        Key_4 = 52,
        Key_5 = 53,
        Key_6 = 54,
        Key_7 = 55,
        Key_8 = 56,
        Key_9 = 57,

        Semicolon = 59,
        Equal = 61,

        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,

        Left_Bracket = 91,
        Backslash = 92,
        Right_Bracket = 93,
        Grave_Accent = 96,
        World_1 = 161,
        World_2 = 162,

        Escape = 256,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Insert = 260,
        Delete = 261,
        Right = 262,
        Left = 263,
        Down = 264,
        Up = 265,
        Page_Up = 266,
        Page_Down = 267,
        Home = 268,
        End = 269,

        Caps_Lock = 280,
        Scroll_Lock = 281,
        Num_Lock = 282,
        Print_Screen = 283,
        Pause = 284,

        F1 = 290,
        F2 = 291,
        F3 = 292,
        F4 = 293,
        F5 = 294,
        F6 = 295,
        F7 = 296,
        F8 = 297,
        F9 = 298,
        F10 = 299,
        F11 = 300,
        F12 = 301,
        F13 = 302,
        F14 = 303,
        F15 = 304,
        F16 = 305,
        F17 = 306,
        F18 = 307,
        F19 = 308,
        F20 = 309,
        F21 = 310,
        F22 = 311,
        F23 = 312,
        F24 = 313,
        F25 = 314,

        KP_0 = 320,
        KP_1 = 321,
        KP_2 = 322,
        KP_3 = 323,
        KP_4 = 324,
        KP_5 = 325,
        KP_6 = 326,
        KP_7 = 327,
        KP_8 = 328,
        KP_9 = 329,

        KP_Decimal = 330,
        KP_Divide = 331,
        KP_Multiply = 332,
        KP_Subtract = 333,
        KP_Add = 334,
        KP_Enter = 335,
        KP_Equal = 336,

        Left_Shift = 340,
        Left_Control = 341,
        Left_Alt = 342,
        Left_Super = 343,

        Right_Shift = 344,
        Right_Control = 345,
        Right_Alt = 346,
        Right_Super = 347,
        Menu = 348
    }

    public enum MouseButton : int
    {
        Unknown = -1,

        Mouse_1 = 0,
        Mouse_2 = 1,
        Mouse_3 = 2,
        Mouse_4 = 3,
        Mouse_5 = 4,
        Mouse_6 = 5,
        Mouse_7 = 6,
        Mouse_8 = 7,

        Last = Mouse_8,
        Left = Mouse_1,
        Right = Mouse_2,
        Middle = Mouse_3
    }

    public static class InputMapper
    {
        public static KeyCode GetKey(string key)
        {
            return (KeyCode)InternalCalls.InputMapper_GetKey(key);
        }

        public static void SetKey(string key, KeyCode keyCode)
        {
            InternalCalls.InputMapper_SetKey(key, (int)keyCode);
        }

        public static void ResetKey(string key)
        {
            InternalCalls.InputMapper_ResetKey(key);
        }
    }

    public static class Input
    {
        public static bool IsKeyPressed(KeyCode keyCode)
        {
            if (keyCode == KeyCode.Unknown) { return false; }
            return InternalCalls.Input_KeyPressed(keyCode);
        }

        public static uint[] GetAllKeyPressed()
        {
            return InternalCalls.Input_GetAllKeyPressed();
        }

        public static bool IsKeyReleased(KeyCode keyCode)
        {
            if (keyCode == KeyCode.Unknown) { return false; }
            return InternalCalls.Input_KeyReleased(keyCode);
        }

        public static bool IsMousePressed(MouseButton button)
        {
            if (button == MouseButton.Unknown) { return false; }
            return InternalCalls.Input_MousePressed(button);
        }

        public static bool IsMouseReleased(MouseButton button)
        {
            if (button == MouseButton.Unknown) { return false; }
            return InternalCalls.Input_MouseReleased(button);
        }

        public static bool IsKeyDown(KeyCode keyCode)
        {
            if (keyCode == KeyCode.Unknown) { return false; }
            return InternalCalls.Input_KeyDown(keyCode);
        }

        public static bool IsKeyUp(KeyCode keyCode)
        {
            if (keyCode == KeyCode.Unknown) { return false; }
            return InternalCalls.Input_KeyUp(keyCode);
        }

        public static bool IsMouseDown(MouseButton button)
        {
            if (button == MouseButton.Unknown) { return false; }
            return InternalCalls.Input_MouseDown(button);
        }

        public static bool IsMouseUp(MouseButton button)
        {
            if (button == MouseButton.Unknown) { return false; }
            return InternalCalls.Input_MouseUp(button);
        }

        public static Vector2 GetMousePosition()
        {
            InternalCalls.Input_GetMousePosition(out Vector2 position);
            return position;
        }

        public static void SetMousePosition(float x, float y)
        {
            InternalCalls.Input_SetMousePosition(x, y);
        }

        public static void ShowCursor(bool state)
        {
            InternalCalls.Input_ShowCursor(state);
        }
    }
}
