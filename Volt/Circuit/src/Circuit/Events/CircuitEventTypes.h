#pragma once
namespace Circuit
{
	enum class CircuitTellEventType : unsigned int
	{
		//window events
		OpenWindow,
		CloseWindow,
		SetWindowPosition,
		SetWindowSize,
		SetWindowFocus,
	};

	enum class CircuitListenEventType : unsigned int
	{
		OnKeyPressed, // first time key is pressed
		OnKeyReleased, // key is released

		//Window Events
		WindowOpened,
		WindowClosed,

		WindowResized,

		//WindowTitlebarHittest,


		//input events
		KeyPressed,KeyReleased,	KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled, MouseMovedViewport,
		
	};
}
